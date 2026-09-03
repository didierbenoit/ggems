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

/*!
 * \file
 * \brief Implements a non-statistical fixed-request TestU01 transport probe.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#include <bbattery.h>
#include <unif01.h>

#include <errno.h>
#include <fcntl.h>
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

// =============================================================================
// =============================================================================

enum {
  BITS_PER_BYTE = 8,
  BBATTERY_RESULT_CAPACITY = 201,
  BYTE_SHIFT_1 = 8,
  BYTE_SHIFT_2 = 16,
  BYTE_SHIFT_3 = 24,
  BYTES_PER_WORD = 4,
  EXIT_TECHNICAL_ERROR = 2,
  SMALLCRUSH_SLOT_COUNT = 15,
  TESTU01_NAME_CAPACITY = 121,
  TRACE_BUFFER_SIZE = 1024 * 1024
};

// =============================================================================
// =============================================================================

_Static_assert(CHAR_BIT == BITS_PER_BYTE,
               "the fixed-request trace requires eight-bit bytes");
_Static_assert(sizeof(uint32_t) == BYTES_PER_WORD,
               "the fixed-request trace requires a 32-bit uint32_t");
_Static_assert(sizeof(unsigned int) == BYTES_PER_WORD,
               "the external TestU01 callback must return exactly 32 bits");
_Static_assert(UINT_MAX == UINT32_MAX,
               "the external TestU01 callback must return exactly 32 bits");
_Static_assert(ULONG_MAX >= UINT32_MAX,
               "unif01_Gen.GetBits must preserve every uint32_t value");

// =============================================================================
// =============================================================================

typedef struct {
  int descriptor;
  size_t begin;
  size_t end;
  uint8_t buffer[TRACE_BUFFER_SIZE];
} TraceState;

// =============================================================================
// =============================================================================

const char ggems_testu01_fixed_request_backend_identity[] =
    "GGEMS_TESTU01_FIXED_REQUEST_BACKEND_V1; no TestU01 statistical battery "
    "was executed";

// =============================================================================
// =============================================================================

double bbattery_pVal[BBATTERY_RESULT_CAPACITY];
char *bbattery_TestNames[BBATTERY_RESULT_CAPACITY];
int bbattery_NTests;

// =============================================================================
// =============================================================================

static char smallcrush_names[SMALLCRUSH_SLOT_COUNT][TESTU01_NAME_CAPACITY] = {
    "BirthdaySpacings", "Collision",     "Gap",           "SimpPoker",
    "CouponCollector",  "MaxOft",        "MaxOft AD",     "WeightDistrib",
    "MatrixRank",       "HammingIndep",  "RandomWalk1 H", "RandomWalk1 M",
    "RandomWalk1 J",    "RandomWalk1 R", "RandomWalk1 C",
};

// =============================================================================
// =============================================================================

static unsigned int (*external_callback)(void) = NULL;
static unif01_Gen external_generator = {0};
static TraceState trace_state = {
    .descriptor = -1,
    .begin = 0,
    .end = 0,
};
static uint64_t requested_word_count = 0;
static uint64_t returned_word_count = 0;
static bool trace_cleanup_registered = false;
static bool smallcrush_started = false;

// These exact constants implement TestU01's uint32-to-U01 scaling and neutral
// synthetic probe result; they are protocol values rather than tunables.
static const double uint32_unit_interval_divisor = 0x1p32;
static const double fixed_probe_p_value = 0x1p-1;

// =============================================================================
// =============================================================================

_Noreturn static void fail_technical(const char *message) {
  (void)fprintf(stderr,
                "ggems_testu01_fixed_request_backend: technical error: %s\n",
                message);
  (void)fflush(stderr);
  exit(EXIT_TECHNICAL_ERROR);
}

// =============================================================================
// =============================================================================

_Noreturn static void fail_technical_errno(const char *message,
                                           int system_errno) {
  (void)fprintf(stderr,
                "ggems_testu01_fixed_request_backend: technical error: %s: "
                "%s\n",
                message, strerror(system_errno));
  (void)fflush(stderr);
  exit(EXIT_TECHNICAL_ERROR);
}

// =============================================================================
// =============================================================================

static bool flush_trace_buffer(void) {
  while (trace_state.begin < trace_state.end) {
    const size_t remaining = trace_state.end - trace_state.begin;
    ssize_t written = 0;

    do {
      written = write(trace_state.descriptor,
                      trace_state.buffer + trace_state.begin, remaining);
    } while (written < 0 && errno == EINTR);

    if (written < 0) {
      return false;
    }
    if (written == 0) {
      errno = EIO;
      return false;
    }
    trace_state.begin += (size_t)written;
  }

  trace_state.begin = 0;
  trace_state.end = 0;
  return true;
}

// =============================================================================
// =============================================================================

static void cleanup_trace_at_exit(void) {
  const int saved_errno = errno;

  if (trace_state.descriptor >= 0) {
    if (!flush_trace_buffer()) {
      const int write_errno = errno;
      (void)fprintf(stderr,
                    "ggems_testu01_fixed_request_backend: could not flush the "
                    "callback trace during process exit: %s\n",
                    strerror(write_errno));
    }

    {
      const int descriptor = trace_state.descriptor;
      trace_state.descriptor = -1;
      if (close(descriptor) != 0) {
        const int close_errno = errno;
        (void)fprintf(stderr,
                      "ggems_testu01_fixed_request_backend: could not close "
                      "the callback trace during process exit: %s\n",
                      strerror(close_errno));
      }
    }
  }

  errno = saved_errno;
}

// =============================================================================
// =============================================================================

static bool parse_uint64_decimal(const char *text, uint64_t *value) {
  const unsigned char *cursor = (const unsigned char *)text;
  uint64_t parsed = 0;

  if (*cursor == 0U) {
    return false;
  }
  if (*cursor == (unsigned char)'0' && cursor[1] != 0U) {
    return false;
  }

  while (*cursor != 0U) {
    uint64_t digit = 0;

    if (*cursor < (unsigned char)'0' || *cursor > (unsigned char)'9') {
      return false;
    }
    digit = (uint64_t)(*cursor - (unsigned char)'0');
    if (parsed > (UINT64_MAX - digit) / UINT64_C(10)) {
      return false;
    }
    parsed = (parsed * UINT64_C(10)) + digit;
    ++cursor;
  }

  *value = parsed;
  return true;
}

// =============================================================================
// =============================================================================

static void open_trace(void) {
  const char *const word_count_text = getenv("GGEMS_FIXED_REQUEST_WORD_COUNT");
  const char *const trace_path = getenv("GGEMS_FIXED_REQUEST_TRACE_PATH");
  int descriptor = -1;

  if (word_count_text == NULL ||
      !parse_uint64_decimal(word_count_text, &requested_word_count)) {
    fail_technical(
        "GGEMS_FIXED_REQUEST_WORD_COUNT must be a canonical uint64 decimal");
  }
  if (trace_path == NULL || trace_path[0] == '\0' ||
      strcmp(trace_path, "-") == 0) {
    fail_technical("GGEMS_FIXED_REQUEST_TRACE_PATH must name a new trace file");
  }
  if (!trace_cleanup_registered) {
    if (atexit(cleanup_trace_at_exit) != 0) {
      fail_technical("could not register callback-trace cleanup");
    }
    trace_cleanup_registered = true;
  }

  descriptor = open(trace_path, O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC,
                    S_IRUSR | S_IWUSR);
  if (descriptor < 0) {
    const int open_errno = errno;
    fail_technical_errno("could not create the callback trace exclusively",
                         open_errno);
  }
  trace_state.descriptor = descriptor;
}

// =============================================================================
// =============================================================================

static void append_trace_word(uint32_t word) {
  if (trace_state.descriptor < 0) {
    fail_technical("a callback word was returned before the trace was opened");
  }
  if ((TRACE_BUFFER_SIZE - trace_state.end) < sizeof(uint32_t) &&
      !flush_trace_buffer()) {
    const int write_errno = errno;
    fail_technical_errno("writing the buffered callback trace failed",
                         write_errno);
  }

  trace_state.buffer[trace_state.end] = (uint8_t)(word & UINT32_C(0xff));
  trace_state.buffer[trace_state.end + 1U] =
      (uint8_t)((word >> BYTE_SHIFT_1) & UINT32_C(0xff));
  trace_state.buffer[trace_state.end + 2U] =
      (uint8_t)((word >> BYTE_SHIFT_2) & UINT32_C(0xff));
  trace_state.buffer[trace_state.end + 3U] =
      (uint8_t)((word >> BYTE_SHIFT_3) & UINT32_C(0xff));
  trace_state.end += sizeof(uint32_t);

  if (trace_state.end == TRACE_BUFFER_SIZE && !flush_trace_buffer()) {
    const int write_errno = errno;
    fail_technical_errno("writing the buffered callback trace failed",
                         write_errno);
  }
}

// =============================================================================
// =============================================================================

static void close_trace(void) {
  int descriptor = -1;

  if (trace_state.descriptor < 0) {
    fail_technical("the callback trace was not open at normal completion");
  }
  if (!flush_trace_buffer()) {
    const int write_errno = errno;
    fail_technical_errno("flushing the callback trace failed", write_errno);
  }

  descriptor = trace_state.descriptor;
  trace_state.descriptor = -1;
  if (close(descriptor) != 0) {
    const int close_errno = errno;
    fail_technical_errno("closing the callback trace failed", close_errno);
  }
}

// =============================================================================
// =============================================================================

// The TestU01 unif01_Gen.GetBits ABI fixes two adjacent void-pointer
// parameters; changing that signature would invalidate the interposition.
static unsigned long get_bits(void *parameter, void *state) {
  unsigned int callback_word = 0;

  (void)parameter;
  (void)state;

  if (external_callback == NULL) {
    fail_technical("the external uint32 callback is unavailable");
  }
  if (!smallcrush_started || returned_word_count >= requested_word_count) {
    fail_technical("the fixed-request backend received an extra word request");
  }

  callback_word = external_callback();
  append_trace_word((uint32_t)callback_word);
  ++returned_word_count;
  return (unsigned long)callback_word;
}

// =============================================================================
// =============================================================================

static double get_u01(void *parameter, void *state) {
  return (double)get_bits(parameter, state) / uint32_unit_interval_divisor;
}

// =============================================================================
// =============================================================================

unif01_Gen *unif01_CreateExternGenBits(char *name, unsigned int (*genB)(void)) {
  if (name == NULL || genB == NULL || external_callback != NULL) {
    return NULL;
  }

  external_callback = genB;
  external_generator.state = NULL;
  external_generator.param = NULL;
  external_generator.name = name;
  external_generator.GetU01 = get_u01;
  external_generator.GetBits = get_bits;
  external_generator.Write = NULL;
  return &external_generator;
}

// =============================================================================
// =============================================================================

void unif01_DeleteExternGenBits(unif01_Gen *generator) {
  if (generator != &external_generator || external_callback == NULL) {
    fail_technical("the consumer deleted an unknown external generator");
  }
  external_callback = NULL;
}

// =============================================================================
// =============================================================================

static void populate_smallcrush_results(void) {
  int index = 0;

  bbattery_NTests = SMALLCRUSH_SLOT_COUNT;
  for (index = 0; index < SMALLCRUSH_SLOT_COUNT; ++index) {
    bbattery_TestNames[index] = smallcrush_names[index];
    bbattery_pVal[index] = fixed_probe_p_value;
  }
}

// =============================================================================
// =============================================================================

static void write_stdout_marker(void) {
  if (puts(ggems_testu01_fixed_request_backend_identity) == EOF ||
      fflush(stdout) == EOF) {
    const int output_errno = errno;
    fail_technical_errno("writing the non-statistical probe marker failed",
                         output_errno == 0 ? EIO : output_errno);
  }
}

// =============================================================================
// =============================================================================

void bbattery_SmallCrush(unif01_Gen *generator) {
  if (smallcrush_started) {
    fail_technical("the fixed-request SmallCrush probe was invoked twice");
  }
  if (generator != &external_generator || generator->GetBits != get_bits ||
      external_callback == NULL) {
    fail_technical("the fixed-request probe received an unknown generator");
  }

  smallcrush_started = true;
  open_trace();
  while (returned_word_count < requested_word_count) {
    const unsigned long word =
        generator->GetBits(generator->param, generator->state);

    if (word > (unsigned long)UINT32_MAX) {
      fail_technical("the TestU01 GetBits adapter returned more than 32 bits");
    }
  }
  if (returned_word_count != requested_word_count) {
    fail_technical("the fixed-request callback count is inconsistent");
  }

  close_trace();
  populate_smallcrush_results();
  write_stdout_marker();
}

// =============================================================================
// =============================================================================

_Noreturn static void refuse_official_battery(const char *battery_name) {
  write_stdout_marker();
  (void)fprintf(stderr,
                "ggems_testu01_fixed_request_backend: technical error: %s is "
                "disabled; this executable is only a non-statistical "
                "fixed-request transport probe\n",
                battery_name);
  (void)fflush(stderr);
  exit(EXIT_TECHNICAL_ERROR);
}

// =============================================================================
// =============================================================================

void bbattery_Crush(unif01_Gen *generator) {
  (void)generator;
  refuse_official_battery("Crush");
}

// =============================================================================
// =============================================================================

void bbattery_BigCrush(unif01_Gen *generator) {
  (void)generator;
  refuse_official_battery("BigCrush");
}
