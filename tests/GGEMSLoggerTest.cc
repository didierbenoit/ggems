// ************************************************************************
// * This file is part of GGEMS.                                          *
// *                                                                      *
// * GGEMS is free software: you can redistribute it and/or modify        *
// * it under the terms of the GNU General Public License as published by *
// * the Free Software Foundation, either version 3 of the License, or    *
// * (at your option) any later version.                                  *
// *                                                                      *
// * GGEMS is distributed in the hope that it will be useful,             *
// * but WITHOUT ANY WARRANTY; without even the implied warranty of       *
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        *
// * GNU General Public License for more details.                         *
// *                                                                      *
// * You should have received a copy of the GNU General Public License    *
// * along with GGEMS.  If not, see <https://www.gnu.org/licenses/>.      *
// *                                                                      *
// ************************************************************************

#include <gtest/gtest.h>
#include "GGEMS/tools/GGEMSLogger.hh"
#include <sstream>
#include <iostream>

class GGEMSLoggerTest : public ::testing::Test {
protected:
  GGEMSLoggerTest(void) {
    GGEMSLoggerManager::GetInstance().SetLevelInfos(gglog::Level::INFO4);
  }

  ~GGEMSLoggerTest(void) = default;

  void SetUp(void) override {
    old_cout_ = std::cout.rdbuf(buffer_.rdbuf());
  }

  void TearDown(void) override {
    std::cout.rdbuf(old_cout_);
    buffer_.str("");
    buffer_.clear();
  }

  std::string const GetOutput(void) const {
    return buffer_.str();
  }

protected:
  std::streambuf*   old_cout_;
  std::stringstream buffer_;
};

/// --- INFO Output Test

TEST_F(GGEMSLoggerTest, InfoMessage) {
  gglog::info() << "Info test" << gglog::endl;
  EXPECT_EQ(GetOutput(), "[GGEMS INFO] Info test\n");
}

TEST_F(GGEMSLoggerTest, InfoMessageClassMethod) {
  gglog::info("Class", "Method") << "Info test" << gglog::endl;
  EXPECT_EQ(GetOutput(), "[GGEMS INFO] (Class::Method) Info test\n");
}

TEST_F(GGEMSLoggerTest, InfoMessageMethod) {
  gglog::info("", "Method") << "Info test" << gglog::endl;
  EXPECT_EQ(GetOutput(), "[GGEMS INFO] (::Method) Info test\n");
}

TEST_F(GGEMSLoggerTest, InfoMessageClass) {
  gglog::info("Class", "") << "Info test" << gglog::endl;
  EXPECT_EQ(GetOutput(), "[GGEMS INFO] (Class::) Info test\n");
}

TEST_F(GGEMSLoggerTest, InfoMessageEmpty) {
  gglog::info("", "") << "Info test" << gglog::endl;
  EXPECT_EQ(GetOutput(), "[GGEMS INFO] Info test\n");
}

/// --- INFO2 Output Test

TEST_F(GGEMSLoggerTest, Info2Message) {
  gglog::info2() << "Info2 test" << gglog::endl;
  EXPECT_EQ(GetOutput(), "[GGEMS INFO2] Info2 test\n");
}

TEST_F(GGEMSLoggerTest, Info2MessageClassMethod) {
  gglog::info2("Class", "Method") << "Info2 test" << gglog::endl;
  EXPECT_EQ(GetOutput(), "[GGEMS INFO2] (Class::Method) Info2 test\n");
}

TEST_F(GGEMSLoggerTest, Info2MessageMethod) {
  gglog::info2("", "Method") << "Info2 test" << gglog::endl;
  EXPECT_EQ(GetOutput(), "[GGEMS INFO2] (::Method) Info2 test\n");
}

TEST_F(GGEMSLoggerTest, Info2MessageClass) {
  gglog::info2("Class", "") << "Info2 test" << gglog::endl;
  EXPECT_EQ(GetOutput(), "[GGEMS INFO2] (Class::) Info2 test\n");
}

TEST_F(GGEMSLoggerTest, Info2MessageEmpty) {
  gglog::info2("", "") << "Info2 test" << gglog::endl;
  EXPECT_EQ(GetOutput(), "[GGEMS INFO2] Info2 test\n");
}

/// --- INFO3 Output Test

TEST_F(GGEMSLoggerTest, Info3Message) {
  gglog::info3() << "Info3 test" << gglog::endl;
  EXPECT_EQ(GetOutput(), "[GGEMS INFO3] Info3 test\n");
}

TEST_F(GGEMSLoggerTest, Info3MessageClassMethod) {
  gglog::info3("Class", "Method") << "Info3 test" << gglog::endl;
  EXPECT_EQ(GetOutput(), "[GGEMS INFO3] (Class::Method) Info3 test\n");
}

TEST_F(GGEMSLoggerTest, Info3MessageMethod) {
  gglog::info3("", "Method") << "Info3 test" << gglog::endl;
  EXPECT_EQ(GetOutput(), "[GGEMS INFO3] (::Method) Info3 test\n");
}

TEST_F(GGEMSLoggerTest, Info3MessageClass) {
  gglog::info3("Class", "") << "Info3 test" << gglog::endl;
  EXPECT_EQ(GetOutput(), "[GGEMS INFO3] (Class::) Info3 test\n");
}

TEST_F(GGEMSLoggerTest, Info3MessageEmpty) {
  gglog::info3("", "") << "Info3 test" << gglog::endl;
  EXPECT_EQ(GetOutput(), "[GGEMS INFO3] Info3 test\n");
}

/// --- INFO4 Output Test

TEST_F(GGEMSLoggerTest, Info4Message) {
  gglog::info4() << "Info4 test" << gglog::endl;
  EXPECT_EQ(GetOutput(), "[GGEMS INFO4] Info4 test\n");
}

TEST_F(GGEMSLoggerTest, Info4MessageClassMethod) {
  gglog::info4("Class", "Method") << "Info4 test" << gglog::endl;
  EXPECT_EQ(GetOutput(), "[GGEMS INFO4] (Class::Method) Info4 test\n");
}

TEST_F(GGEMSLoggerTest, Info4MessageMethod) {
  gglog::info4("", "Method") << "Info4 test" << gglog::endl;
  EXPECT_EQ(GetOutput(), "[GGEMS INFO4] (::Method) Info4 test\n");
}

TEST_F(GGEMSLoggerTest, Info4MessageClass) {
  gglog::info4("Class", "") << "Info4 test" << gglog::endl;
  EXPECT_EQ(GetOutput(), "[GGEMS INFO4] (Class::) Info4 test\n");
}

TEST_F(GGEMSLoggerTest, Info4MessageEmpty) {
  gglog::info4("", "") << "Info4 test" << gglog::endl;
  EXPECT_EQ(GetOutput(), "[GGEMS INFO4] Info4 test\n");
}

/// --- DEBUG Output Test

TEST_F(GGEMSLoggerTest, DebugMessage) {
  gglog::debug() << "Debug test" << gglog::endl;
  EXPECT_EQ(GetOutput(), "[GGEMS DEBUG] Debug test\n");
}

TEST_F(GGEMSLoggerTest, DebugClassMethod) {
  gglog::debug("Class", "Method") << "Debug test" << gglog::endl;
  EXPECT_EQ(GetOutput(), "[GGEMS DEBUG] (Class::Method) Debug test\n");
}

TEST_F(GGEMSLoggerTest, DebugMessageMethod) {
  gglog::debug("", "Method") << "Debug test" << gglog::endl;
  EXPECT_EQ(GetOutput(), "[GGEMS DEBUG] (::Method) Debug test\n");
}

TEST_F(GGEMSLoggerTest, DebugMessageClass) {
  gglog::debug("Class", "") << "Debug test" << gglog::endl;
  EXPECT_EQ(GetOutput(), "[GGEMS DEBUG] (Class::) Debug test\n");
}

TEST_F(GGEMSLoggerTest, DebugMessageEmpty) {
  gglog::debug("", "") << "Debug test" << gglog::endl;
  EXPECT_EQ(GetOutput(), "[GGEMS DEBUG] Debug test\n");
}

/// --- WARNING Output Test

TEST_F(GGEMSLoggerTest, WarningMessage) {
  gglog::warn() << "Warning test" << gglog::endl;
  EXPECT_EQ(GetOutput(), "[GGEMS WARNING] Warning test\n");
}

TEST_F(GGEMSLoggerTest, WarningClassMethod) {
  gglog::warn("Class", "Method") << "Warning test" << gglog::endl;
  EXPECT_EQ(GetOutput(), "[GGEMS WARNING] (Class::Method) Warning test\n");
}

TEST_F(GGEMSLoggerTest, WarningMessageMethod) {
  gglog::warn("", "Method") << "Warning test" << gglog::endl;
  EXPECT_EQ(GetOutput(), "[GGEMS WARNING] (::Method) Warning test\n");
}

TEST_F(GGEMSLoggerTest, WarningMessageClass) {
  gglog::warn("Class", "") << "Warning test" << gglog::endl;
  EXPECT_EQ(GetOutput(), "[GGEMS WARNING] (Class::) Warning test\n");
}

TEST_F(GGEMSLoggerTest, WarningMessageEmpty) {
  gglog::warn("", "") << "Warning test" << gglog::endl;
  EXPECT_EQ(GetOutput(), "[GGEMS WARNING] Warning test\n");
}

/// --- ERROR Output Test

TEST_F(GGEMSLoggerTest, ErrorMessage) {
  gglog::err() << "Error test" << gglog::endl;
  EXPECT_EQ(GetOutput(), "[GGEMS ERROR] Error test\n");
}

TEST_F(GGEMSLoggerTest, ErrorClassMethod) {
  gglog::err("Class", "Method") << "Error test" << gglog::endl;
  EXPECT_EQ(GetOutput(), "[GGEMS ERROR] (Class::Method) Error test\n");
}

TEST_F(GGEMSLoggerTest, ErrorMessageMethod) {
  gglog::err("", "Method") << "Error test" << gglog::endl;
  EXPECT_EQ(GetOutput(), "[GGEMS ERROR] (::Method) Error test\n");
}

TEST_F(GGEMSLoggerTest, ErrorMessageClass) {
  gglog::err("Class", "") << "Error test" << gglog::endl;
  EXPECT_EQ(GetOutput(), "[GGEMS ERROR] (Class::) Error test\n");
}

TEST_F(GGEMSLoggerTest, ErrorMessageEmpty) {
  gglog::err("", "") << "Error test" << gglog::endl;
  EXPECT_EQ(GetOutput(), "[GGEMS ERROR] Error test\n");
}
