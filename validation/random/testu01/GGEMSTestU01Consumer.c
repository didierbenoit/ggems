// *****************************************************************************
// * This file is part of GGEMS.                                               *
// *                                                                           *
// * SPDX-License-Identifier: GPL-3.0-or-later                                 *
// * Copyright (C) 2017-2026 CHRU de Brest, Université de Bretagne Occidentale,*
// * Inserm.                                                                   *
// *                                                                           *
// * GGEMS is free software: you can redistribute it and/or modify             *
// * it under the terms of the GNU General Public License as published by      *
// * the Free Software Foundation, either version 3 of the License, or         *
// * (at your option) any later version.                                       *
// *                                                                           *
// * GGEMS is distributed in the hope that it will be useful,                  *
// * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the              *
// * GNU General Public License for more details.                              *
// *                                                                           *
// * You should have received a copy of the GNU General Public License         *
// * along with GGEMS. If not, see <https://www.gnu.org/licenses/>.            *
// *****************************************************************************

#include <bbattery.h>
#include <unif01.h>

#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

// =============================================================================
// =============================================================================

_Static_assert(CHAR_BIT == 8, "GGEMS TestU01 input requires 8-bit bytes");
_Static_assert(sizeof(uint32_t) == 4,
               "GGEMS TestU01 input requires a 32-bit uint32_t");
_Static_assert(sizeof(unsigned int) == 4,
               "unif01_CreateExternGenBits requires a 32-bit unsigned int");
_Static_assert(UINT_MAX == UINT32_MAX,
               "unsigned int must represent every uint32_t value exactly");

// =============================================================================
// =============================================================================

enum {
  INPUT_BUFFER_SIZE = 1024 * 1024,
  RESULT_SCHEMA_VERSION = 1,
  TESTU01_NAME_CAPACITY = 121,
  EXIT_TECHNICAL_ERROR = 2
};

typedef enum { BATTERY_SMALLCRUSH, BATTERY_CRUSH, BATTERY_BIGCRUSH } Battery;

typedef struct {
  Battery battery;
  const char *battery_name;
  int expected_slot_count;
  void (*run)(unif01_Gen *generator);
} BatteryConfiguration;

typedef struct {
  const char *input_path;
  const char *result_path;
  Battery battery;
  bool battery_set;
} Options;

typedef struct {
  int descriptor;
  bool descriptor_owned;
  bool is_regular_file;
  uint64_t regular_file_size;
  uint8_t buffer[INPUT_BUFFER_SIZE];
  size_t begin;
  size_t end;
  uint64_t bytes_read;
  uint64_t words_delivered;
} InputState;

typedef struct {
  size_t index;
  char name[TESTU01_NAME_CAPACITY];
  bool name_present;
  double p_value;
} ResultSlot;

typedef enum {
  COPY_SUCCESS,
  COPY_ALLOCATION_FAILED,
  COPY_UNTERMINATED_NAME
} CopyStatus;

// =============================================================================
// =============================================================================

static const char *const RESULT_SCHEMA = "ggems_testu01_consumer_result";

static InputState input_state = {
    .descriptor = -1,
    .descriptor_owned = false,
    .is_regular_file = false,
    .regular_file_size = 0,
    .begin = 0,
    .end = 0,
    .bytes_read = 0,
    .words_delivered = 0,
};

static FILE *result_file = NULL;
static const BatteryConfiguration *active_battery = NULL;
static const char *active_input_path = NULL;

// =============================================================================
// =============================================================================

static void print_usage(FILE *stream, const char *program_name) {
  (void)fprintf(stream,
                "Usage: %s --battery smallcrush|crush|bigcrush "
                "--input <path|-> --result <path>\n",
                program_name);
}

static bool parse_battery(const char *value, Battery *battery) {
  if (strcmp(value, "smallcrush") == 0) {
    *battery = BATTERY_SMALLCRUSH;
    return true;
  }
  if (strcmp(value, "crush") == 0) {
    *battery = BATTERY_CRUSH;
    return true;
  }
  if (strcmp(value, "bigcrush") == 0) {
    *battery = BATTERY_BIGCRUSH;
    return true;
  }
  return false;
}

static bool parse_options(int argc, char *argv[], Options *options) {
  int index = 1;

  *options = (Options){
      .input_path = NULL,
      .result_path = NULL,
      .battery = BATTERY_SMALLCRUSH,
      .battery_set = false,
  };

  while (index < argc) {
    const char *argument = argv[index];

    if (strcmp(argument, "--help") == 0) {
      print_usage(stdout, argv[0]);
      exit(EXIT_SUCCESS);
    }
    if ((index + 1) >= argc) {
      (void)fprintf(stderr, "Missing value after %s\n", argument);
      return false;
    }

    if (strcmp(argument, "--battery") == 0) {
      if (options->battery_set ||
          !parse_battery(argv[index + 1], &options->battery)) {
        (void)fprintf(stderr, "Invalid or duplicate --battery option\n");
        return false;
      }
      options->battery_set = true;
    } else if (strcmp(argument, "--input") == 0) {
      if (options->input_path != NULL) {
        (void)fprintf(stderr, "--input may be specified only once\n");
        return false;
      }
      options->input_path = argv[index + 1];
    } else if (strcmp(argument, "--result") == 0) {
      if (options->result_path != NULL) {
        (void)fprintf(stderr, "--result may be specified only once\n");
        return false;
      }
      options->result_path = argv[index + 1];
    } else {
      (void)fprintf(stderr, "Unknown option: %s\n", argument);
      return false;
    }
    index += 2;
  }

  if (!options->battery_set || options->input_path == NULL ||
      options->result_path == NULL) {
    (void)fprintf(stderr,
                  "--battery, --input, and --result are all required\n");
    return false;
  }
  if (options->input_path[0] == '\0' || options->result_path[0] == '\0' ||
      strcmp(options->result_path, "-") == 0) {
    (void)fprintf(stderr, "Invalid input or result path\n");
    return false;
  }
  return true;
}

static const BatteryConfiguration *get_battery_configuration(Battery battery) {
  static const BatteryConfiguration configurations[] = {
      {BATTERY_SMALLCRUSH, "smallcrush", 15, bbattery_SmallCrush},
      {BATTERY_CRUSH, "crush", 144, bbattery_Crush},
      {BATTERY_BIGCRUSH, "bigcrush", 160, bbattery_BigCrush},
  };
  size_t index = 0;

  for (index = 0; index < sizeof(configurations) / sizeof(configurations[0]);
       ++index) {
    if (configurations[index].battery == battery) {
      return &configurations[index];
    }
  }
  return NULL;
}

// =============================================================================
// =============================================================================

static bool write_json_string(FILE *stream, const char *value) {
  const unsigned char *cursor = (const unsigned char *)value;

  if (fputc('"', stream) == EOF) {
    return false;
  }
  while (*cursor != 0U) {
    const unsigned char byte = *cursor;
    bool written = true;

    switch (byte) {
    case '"':
      written = fputs("\\\"", stream) != EOF;
      break;
    case '\\':
      written = fputs("\\\\", stream) != EOF;
      break;
    case '\b':
      written = fputs("\\b", stream) != EOF;
      break;
    case '\f':
      written = fputs("\\f", stream) != EOF;
      break;
    case '\n':
      written = fputs("\\n", stream) != EOF;
      break;
    case '\r':
      written = fputs("\\r", stream) != EOF;
      break;
    case '\t':
      written = fputs("\\t", stream) != EOF;
      break;
    default:
      if (byte < 0x20U || byte >= 0x7fU) {
        written = fprintf(stream, "\\u%04x", (unsigned int)byte) >= 0;
      } else {
        written = fputc((int)byte, stream) != EOF;
      }
      break;
    }
    if (!written) {
      return false;
    }
    ++cursor;
  }
  return fputc('"', stream) != EOF;
}

static bool write_nullable_json_string(FILE *stream, const char *value) {
  if (value == NULL) {
    return (bool)(fputs("null", stream) != EOF);
  }

  return write_json_string(stream, value);
}

static bool write_common_result_prefix(FILE *stream, const char *status) {
  if (fprintf(stream,
              "{\n"
              "  \"schema\": \"%s\",\n"
              "  \"schema_version\": %d,\n"
              "  \"consumer_status\": \"%s\",\n"
              "  \"battery\": \"%s\",\n"
              "  \"expected_slot_count\": %d,\n"
              "  \"input\": ",
              RESULT_SCHEMA, RESULT_SCHEMA_VERSION, status,
              active_battery->battery_name,
              active_battery->expected_slot_count) < 0) {
    return false;
  }
  if (!write_json_string(stream, active_input_path)) {
    return false;
  }
  if (fprintf(stream,
              ",\n"
              "  \"input_byte_order\": \"little\",\n"
              "  \"callback_word_bits\": 32,\n"
              "  \"input_bytes_read\": \"%" PRIu64 "\",\n"
              "  \"consumed_word_count\": \"%" PRIu64 "\",\n"
              "  \"consumed_byte_count\": \"%" PRIu64 "\",\n"
              "  \"regular_file_size_bytes\": ",
              input_state.bytes_read, input_state.words_delivered,
              input_state.words_delivered * UINT64_C(4)) < 0) {
    return false;
  }

  if (input_state.is_regular_file) {
    return fprintf(stream, "\"%" PRIu64 "\",\n",
                   input_state.regular_file_size) >= 0;
  }
  return fputs("null,\n", stream) != EOF;
}

static bool write_technical_result(const char *kind, const char *message,
                                   int system_errno) {
  bool written = true;

  if (result_file == NULL || active_battery == NULL) {
    return false;
  }
  written = (bool)(write_common_result_prefix(result_file, "technical_error") &&
                   written);
  written = (bool)(fputs("  \"reported_slot_count\": null,\n"
                         "  \"copied_slot_count\": 0,\n"
                         "  \"result_collection_status\": null,\n"
                         "  \"error\": {\n"
                         "    \"kind\": ",
                         result_file) != EOF &&
                   written);
  written = (bool)(write_json_string(result_file, kind) && written);
  written =
      (bool)(fputs(",\n    \"message\": ", result_file) != EOF && written);
  written = (bool)(write_json_string(result_file, message) && written);
  written = (bool)(fprintf(result_file,
                           ",\n    \"system_errno\": %d\n"
                           "  },\n"
                           "  \"slots\": [],\n"
                           "  \"terminal\": true\n"
                           "}\n",
                           system_errno) >= 0 &&
                   written);
  if (fflush(result_file) == EOF || ferror(result_file) != 0) {
    written = false;
  }
  return written;
}

_Noreturn static void terminate_technical(const char *kind, int system_errno,
                                          const char *message) {
  const bool result_written =
      write_technical_result(kind, message, system_errno);

  (void)fprintf(stderr, "ggems_testu01_consumer: %s: %s", kind, message);
  if (system_errno != 0) {
    (void)fprintf(stderr, ": %s", strerror(system_errno));
  }
  if (!result_written) {
    (void)fputs(" (structured result could not be written)", stderr);
  }
  (void)fputc('\n', stderr);
  (void)fflush(NULL);
  exit(EXIT_TECHNICAL_ERROR);
}

// =============================================================================
// =============================================================================

static void checked_add_bytes_read(size_t amount) {
  if ((uint64_t)amount > UINT64_MAX - input_state.bytes_read) {
    terminate_technical("input_byte_counter_overflow", 0,
                        "input-byte counter overflowed");
  }
  input_state.bytes_read += (uint64_t)amount;
}

static void checked_record_delivered_word(void) {
  if (input_state.words_delivered == UINT64_MAX) {
    terminate_technical("word_counter_overflow", 0,
                        "delivered-word counter overflowed");
  }
  ++input_state.words_delivered;
}

static void make_word_available(void) {
  size_t available = input_state.end - input_state.begin;

  if (available > 0U && input_state.begin > 0U) {
    (void)memmove(input_state.buffer, input_state.buffer + input_state.begin,
                  available);
  }
  input_state.begin = 0;
  input_state.end = available;

  while (available < sizeof(uint32_t)) {
    const size_t capacity = sizeof(input_state.buffer) - input_state.end;
    ssize_t count = 0;

    do {
      count = read(input_state.descriptor, input_state.buffer + input_state.end,
                   capacity);
    } while (count < 0 && errno == EINTR);

    if (count < 0) {
      const int read_errno = errno;
      terminate_technical("input_io_error", read_errno,
                          "reading the uint32 stream failed");
    }
    if (count == 0) {
      if (available == 0U) {
        terminate_technical("unexpected_eof", 0,
                            "TestU01 requested data after input EOF");
      }
      terminate_technical("truncated_final_word", 0,
                          "input ended in the middle of a uint32 word");
    }

    input_state.end += (size_t)count;
    checked_add_bytes_read((size_t)count);
    available = input_state.end - input_state.begin;
  }
}

static unsigned int next_ggems_word(void) {
  const uint8_t *bytes = NULL;
  uint32_t word = 0;

  if ((input_state.end - input_state.begin) < sizeof(uint32_t)) {
    make_word_available();
  }

  bytes = input_state.buffer + input_state.begin;
  word = (uint32_t)bytes[0] | ((uint32_t)bytes[1] << 8U) |
         ((uint32_t)bytes[2] << 16U) | ((uint32_t)bytes[3] << 24U);
  input_state.begin += sizeof(uint32_t);
  checked_record_delivered_word();
  return (unsigned int)word;
}

// =============================================================================
// =============================================================================

static bool open_input(const char *path, char *message, size_t message_size) {
  struct stat metadata;

  if (strcmp(path, "-") == 0) {
    input_state.descriptor = STDIN_FILENO;
    input_state.descriptor_owned = false;
  } else {
    input_state.descriptor = open(path, O_RDONLY | O_CLOEXEC);
    input_state.descriptor_owned = true;
    if (input_state.descriptor < 0) {
      (void)snprintf(message, message_size, "cannot open input file");
      return false;
    }
  }

  if (fstat(input_state.descriptor, &metadata) != 0) {
    (void)snprintf(message, message_size, "cannot inspect input metadata");
    return false;
  }
  if (S_ISDIR(metadata.st_mode)) {
    errno = EISDIR;
    (void)snprintf(message, message_size, "input path is a directory");
    return false;
  }
  if (S_ISREG(metadata.st_mode)) {
    if (metadata.st_size < 0 ||
        (metadata.st_size % (off_t)sizeof(uint32_t)) != 0) {
      errno = 0;
      (void)snprintf(message, message_size,
                     "regular input size must be a multiple of four bytes");
      return false;
    }
    input_state.is_regular_file = true;
    input_state.regular_file_size = (uint64_t)metadata.st_size;
  }
  return true;
}

// =============================================================================
// =============================================================================

static ResultSlot *copy_battery_results(int expected_slot_count,
                                        int *copied_slot_count,
                                        CopyStatus *copy_status,
                                        int *failing_slot_index) {
  ResultSlot *slots = NULL;
  int index = 0;

  *copied_slot_count = 0;
  *failing_slot_index = -1;
  slots = calloc((size_t)expected_slot_count, sizeof(*slots));
  if (slots == NULL) {
    *copy_status = COPY_ALLOCATION_FAILED;
    return NULL;
  }

  for (index = 0; index < expected_slot_count; ++index) {
    const char *source_name = bbattery_TestNames[index];

    slots[index].index = (size_t)index;
    slots[index].p_value = bbattery_pVal[index];
    if (source_name != NULL) {
      const size_t length = strnlen(source_name, TESTU01_NAME_CAPACITY);
      if (length == TESTU01_NAME_CAPACITY) {
        *copy_status = COPY_UNTERMINATED_NAME;
        *failing_slot_index = index;
        free(slots);
        return NULL;
      }
      (void)memcpy(slots[index].name, source_name, length + 1U);
      slots[index].name_present = true;
    }
  }

  *copied_slot_count = expected_slot_count;
  *copy_status = COPY_SUCCESS;
  return slots;
}

static bool write_completed_result(const ResultSlot *slots,
                                   int reported_slot_count,
                                   int copied_slot_count) {
  int index = 0;
  bool written = true;

  written =
      (bool)(write_common_result_prefix(result_file, "battery_returned") &&
             written);
  written =
      (bool)(fprintf(result_file,
                     "  \"reported_slot_count\": %d,\n"
                     "  \"copied_slot_count\": %d,\n"
                     "  \"result_collection_status\": \"%s\",\n"
                     "  \"error\": null,\n"
                     "  \"slots\": [",
                     reported_slot_count, copied_slot_count,
                     reported_slot_count == active_battery->expected_slot_count
                         ? "complete"
                         : "slot_count_mismatch") >= 0 &&
             written);

  for (index = 0; index < copied_slot_count; ++index) {
    const double p_value = slots[index].p_value;

    written =
        (bool)(fputs(index == 0 ? "\n" : ",\n", result_file) != EOF && written);
    written = (bool)(fprintf(result_file, "    {\"index\": %zu, \"name\": ",
                             slots[index].index) >= 0 &&
                     written);

    const char *slot_name = NULL;

    if (slots[index].name_present) {
      slot_name = slots[index].name;
    }

    written =
        (bool)(write_nullable_json_string(result_file, slot_name) && written);

    written = (bool)(fprintf(result_file,
                             ", \"p_value_hex\": \"%a\", "
                             "\"p_value_decimal\": \"%.17g\", "
                             "\"testu01_not_computed\": %s}",
                             p_value, p_value,
                             p_value == -1.0 ? "true" : "false") >= 0 &&
                     written);
  }

  if (copied_slot_count > 0) {
    written = (bool)(fputc('\n', result_file) != EOF && written);
  }
  written =
      (bool)(fputs("  ],\n  \"terminal\": true\n}\n", result_file) != EOF &&
             written);
  if (fflush(result_file) == EOF || ferror(result_file) != 0) {
    written = false;
  }
  return written;
}

// =============================================================================
// =============================================================================

int main(int argc, char *argv[]) {
  Options options;
  char input_error[256] = {0};
  char generator_name[] = "GGEMS canonical little-endian uint32 stream";
  unif01_Gen *generator = NULL;
  ResultSlot *slots = NULL;
  int reported_slot_count = 0;
  int copied_slot_count = 0;
  int failing_slot_index = -1;
  CopyStatus copy_status = COPY_SUCCESS;

  if (!parse_options(argc, argv, &options)) {
    print_usage(stderr, argv[0]);
    return EXIT_TECHNICAL_ERROR;
  }

  active_battery = get_battery_configuration(options.battery);
  if (active_battery == NULL) {
    (void)fputs("Internal error: no selected battery configuration\n", stderr);
    return EXIT_TECHNICAL_ERROR;
  }
  active_input_path = options.input_path;

  result_file = fopen(options.result_path, "wx");
  if (result_file == NULL) {
    (void)fprintf(stderr, "Cannot create result file %s: %s\n",
                  options.result_path, strerror(errno));
    return EXIT_TECHNICAL_ERROR;
  }

  if (!open_input(options.input_path, input_error, sizeof(input_error))) {
    const int saved_errno = errno;
    terminate_technical("input_open_or_metadata_error", saved_errno,
                        input_error);
  }

  generator = unif01_CreateExternGenBits(generator_name, next_ggems_word);
  if (generator == NULL) {
    terminate_technical("generator_creation_failed", 0,
                        "unif01_CreateExternGenBits returned NULL");
  }

  active_battery->run(generator);

  reported_slot_count = bbattery_NTests;
  slots = copy_battery_results(active_battery->expected_slot_count,
                               &copied_slot_count, &copy_status,
                               &failing_slot_index);
  unif01_DeleteExternGenBits(generator);
  generator = NULL;

  if (copy_status == COPY_ALLOCATION_FAILED) {
    terminate_technical("result_allocation_failed", errno,
                        "cannot allocate result slots");
  }
  if (copy_status == COPY_UNTERMINATED_NAME) {
    char message[160] = {0};
    (void)snprintf(message, sizeof(message),
                   "TestU01 display name at slot %d is too long",
                   failing_slot_index);
    terminate_technical("unterminated_testu01_display_name", 0, message);
  }

  if (fflush(stdout) == EOF) {
    free(slots);
    terminate_technical("testu01_stdout_flush_failed", errno,
                        "flushing TestU01 stdout failed");
  }

  if (input_state.descriptor_owned && close(input_state.descriptor) != 0) {
    free(slots);
    terminate_technical("input_close_failed", errno, "closing input failed");
  }
  input_state.descriptor = -1;

  if (!write_completed_result(slots, reported_slot_count, copied_slot_count)) {
    (void)fputs("Failed to write structured TestU01 result\n", stderr);
    free(slots);
    (void)fclose(result_file);
    result_file = NULL;
    return EXIT_TECHNICAL_ERROR;
  }
  free(slots);

  if (fclose(result_file) != 0) {
    result_file = NULL;
    (void)fprintf(stderr, "Failed to close structured result: %s\n",
                  strerror(errno));
    return EXIT_TECHNICAL_ERROR;
  }
  result_file = NULL;
  return EXIT_SUCCESS;
}
