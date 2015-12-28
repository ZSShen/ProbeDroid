/*
 * Copyright (C) 2011 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _UTIL_LOG_H_
#define _UTIL_LOG_H_


#include "globals.h"
#include "macros.h"


typedef int LogSeverity;
const int VERBOSE = 0, DEBUG = 1, INFO = 2, WARNING = 3, ERROR = 4, FATAL = 5;


#define CHECK(x)                                                            \
    if (UNLIKELY(!(x)))                                                     \
        LogMessage(__FILE__, __LINE__, FATAL, -1).stream()                  \
        << "Check failed: " #x << " "

#define CHECK_OP(LHS, RHS, OP)                                              \
    for (auto _values = MakeEagerEvaluator(LHS, RHS);                       \
        UNLIKELY(!(_values.lhs OP _values.rhs)); /* empty */)               \
            LogMessage(__FILE__, __LINE__, FATAL, -1).stream()              \
            << "Check failed: " << #LHS << " " << #OP << " " << #RHS        \
            << " (" #LHS "=" << _values.lhs << ", " #RHS "=" << _values.rhs << ") "

#define CHECK_EQ(x, y) CHECK_OP(x, y, ==)
#define CHECK_NE(x, y) CHECK_OP(x, y, !=)
#define CHECK_LE(x, y) CHECK_OP(x, y, <=)
#define CHECK_LT(x, y) CHECK_OP(x, y, <)
#define CHECK_GE(x, y) CHECK_OP(x, y, >=)
#define CHECK_GT(x, y) CHECK_OP(x, y, >)

#define CHECK_STROP(s1, s2, sense)                                          \
    if (UNLIKELY((strcmp(s1, s2) == 0) != sense))                           \
        LOG(FATAL) << "Check failed: "                                      \
        << "\"" << s1 << "\""                                               \
        << (sense ? " == " : " != ")                                        \
        << "\"" << s2 << "\""

#define CHECK_STREQ(s1, s2) CHECK_STROP(s1, s2, true)
#define CHECK_STRNE(s1, s2) CHECK_STROP(s1, s2, false)

#define LOG(severity) LogMessage(__FILE__, __LINE__, severity, -1).stream()
#define PLOG(severity) LogMessage(__FILE__, __LINE__, severity, errno).stream()
#define TIP() Inform().stream()


template <typename LHS, typename RHS>
struct EagerEvaluator
{
    EagerEvaluator(LHS lhs, RHS rhs)
     : lhs(lhs), rhs(rhs)
    {}
    LHS lhs;
    RHS rhs;
};

template <typename LHS, typename RHS>
EagerEvaluator<LHS, RHS> MakeEagerEvaluator(LHS lhs, RHS rhs)
{
    return EagerEvaluator<LHS, RHS>(lhs, rhs);
}

struct LogMessageData
{
  public:
    LogMessageData(const char* file, int line, LogSeverity severity, int error);
    std::ostringstream buffer;
    const char* const file;
    const int line_number;
    const LogSeverity severity;
    const int error;

  private:
    DISALLOW_COPY_AND_ASSIGN(LogMessageData);
};

class LogMessage
{
  public:
    LogMessage(const char* file, int line, LogSeverity severity, int error)
     : data_(new LogMessageData(file, line, severity, error))
    {}

    ~LogMessage();

    std::ostream& stream()
    {
        return data_->buffer;
    }

  private:
    static void LogLine(const LogMessageData& data, const char*);

    const std::unique_ptr<LogMessageData> data_;

    DISALLOW_COPY_AND_ASSIGN(LogMessage);
};

class Inform
{
  public:
    Inform()
    {}

    ~Inform();

    std::ostream& stream()
    {
        return buffer_;
    }

  private:
    std::ostringstream buffer_;

    DISALLOW_COPY_AND_ASSIGN(Inform);
};

#endif