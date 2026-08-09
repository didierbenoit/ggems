#include "GGEMS/core/GGEMSException.hh"

#include <cstdint>
#include <format>
#include <string>
#include <string_view>

#include <gtest/gtest.h>

#include "GGEMS/frameworks/GGEMSOpenCLUtils.hh"

namespace {

[[noreturn]] auto
ThrowRecoverableFromKnownLine(std::int32_t &expected_line) -> void {
  expected_line = static_cast<std::int32_t>(__LINE__ + 1);
  throw ggems::core::GGEMSRecoverable("source-location marker");
}

auto CheckOpenCLFromKnownLine(std::int32_t &expected_line) -> void {
  expected_line = static_cast<std::int32_t>(__LINE__ + 1);
  ggems::ocl::CheckCLError<ggems::core::GGEMSRecoverable>(
      CL_INVALID_VALUE, "source-location OpenCL context");
}

template <typename Exception>
auto ExpectOpenCLCategory(std::string_view expected_category) -> void {
  try {
    ggems::ocl::CheckCLError<Exception>(CL_INVALID_VALUE,
                                       "OpenCL test context");
    FAIL() << "Expected an OpenCL exception.";
  } catch (ggems::core::GGEMSExceptionBase const &exception) {
    EXPECT_STREQ(expected_category.data(), exception.GetCategory());
    EXPECT_NE(std::string_view{exception.what()}.find(
                  "OpenCL test context (code -30): CL_INVALID_VALUE - One or "
                  "more arguments have invalid values."),
              std::string_view::npos);
  }
}

} // namespace

TEST(GGEMSExceptionTest, PreservesCategories) {
  EXPECT_STREQ("Recoverable",
               ggems::core::GGEMSRecoverable{"recoverable"}.GetCategory());
  EXPECT_STREQ("Internal",
               ggems::core::GGEMSInternal{"internal"}.GetCategory());
  EXPECT_STREQ("Fatal", ggems::core::GGEMSFatal{"fatal"}.GetCategory());
}

TEST(GGEMSExceptionTest, PreservesCompleteFormattedDiagnostic) {
  constexpr std::string_view message = "original message marker";
  ggems::core::GGEMSInternal const exception{std::string{message}};

  auto const expected = std::format(
      "\n[GGEMS Exception]\n"
      "  Type     : {}\n"
      "  File     : {}\n"
      "  Line     : {}\n"
      "  Function : {}\n"
      "  Message  : {}\n",
      exception.GetCategory(), exception.GetFileName(), exception.GetLine(),
      exception.GetFunctionName(), message);

  EXPECT_STREQ(expected.c_str(), exception.what());
}

TEST(GGEMSExceptionTest, CapturesTheDirectThrowSite) {
  std::int32_t expected_line = 0;

  try {
    ThrowRecoverableFromKnownLine(expected_line);
    FAIL() << "Expected a recoverable exception.";
  } catch (ggems::core::GGEMSRecoverable const &exception) {
    EXPECT_EQ(expected_line, exception.GetLine());
    EXPECT_NE(std::string_view{exception.GetFileName()}.find(
                  "GGEMSExceptionTest.cc"),
              std::string_view::npos);
    EXPECT_NE(std::string_view{exception.GetFunctionName()}.find(
                  "ThrowRecoverableFromKnownLine"),
              std::string_view::npos);
    EXPECT_STREQ("Recoverable", exception.GetCategory());
    EXPECT_NE(std::string_view{exception.what()}.find("source-location marker"),
              std::string_view::npos);
  }
}

TEST(GGEMSOpenCLErrorTest, SuccessDoesNotThrow) {
  EXPECT_NO_THROW(
      ggems::ocl::CheckCLError(CL_SUCCESS, "OpenCL success context"));
}

TEST(GGEMSOpenCLErrorTest, PreservesCategoryAndErrorMessageMapping) {
  ExpectOpenCLCategory<ggems::core::GGEMSFatal>("Fatal");
  ExpectOpenCLCategory<ggems::core::GGEMSInternal>("Internal");
  ExpectOpenCLCategory<ggems::core::GGEMSRecoverable>("Recoverable");
}

TEST(GGEMSOpenCLErrorTest, CapturesTheDirectCheckSite) {
  std::int32_t expected_line = 0;

  try {
    CheckOpenCLFromKnownLine(expected_line);
    FAIL() << "Expected an OpenCL exception.";
  } catch (ggems::core::GGEMSRecoverable const &exception) {
    EXPECT_EQ(expected_line, exception.GetLine());
    EXPECT_NE(std::string_view{exception.GetFileName()}.find(
                  "GGEMSExceptionTest.cc"),
              std::string_view::npos);
    EXPECT_NE(std::string_view{exception.GetFunctionName()}.find(
                  "CheckOpenCLFromKnownLine"),
              std::string_view::npos);
  }
}
