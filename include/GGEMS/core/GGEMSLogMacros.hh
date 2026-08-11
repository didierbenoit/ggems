#pragma once

#include <source_location>

#include "GGEMS/core/GGEMSLogger.hh"

#define GGEMS_DEBUG(MODULE, FMT, ...)                                          \
  ggems::core::GGEMSLogger::GetInstance()                                      \
      .LogFmt<ggems::core::LogLevel::Debug>(-1, (MODULE), (FMT),               \
                                            std::source_location::current()    \
                                                __VA_OPT__(, __VA_ARGS__))

#define GGEMS_INFO(MODULE, FMT, ...)                                           \
  ggems::core::GGEMSLogger::GetInstance().LogFmt<ggems::core::LogLevel::Info>( \
      0, (MODULE), (FMT),                                                      \
      std::source_location::current() __VA_OPT__(, __VA_ARGS__))

#define GGEMS_WARN(MODULE, FMT, ...)                                           \
  ggems::core::GGEMSLogger::GetInstance().LogFmt<ggems::core::LogLevel::Warn>( \
      -1, (MODULE), (FMT),                                                     \
      std::source_location::current() __VA_OPT__(, __VA_ARGS__))

#define GGEMS_ERROR(MODULE, FMT, ...)                                          \
  ggems::core::GGEMSLogger::GetInstance()                                      \
      .LogFmt<ggems::core::LogLevel::Error>(-1, (MODULE), (FMT),               \
                                            std::source_location::current()    \
                                                __VA_OPT__(, __VA_ARGS__))

#define GGEMS_INFOEX(MODULE, DEPTH, FMT, ...)                                  \
  ggems::core::GGEMSLogger::GetInstance().LogFmt<ggems::core::LogLevel::Info>( \
      (DEPTH), (MODULE), (FMT),                                                \
      std::source_location::current() __VA_OPT__(, __VA_ARGS__))
