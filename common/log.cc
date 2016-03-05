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

#include <cstdio>
#include <cstdlib>

#include "log.h"


void LogMessage::LogLine(const LogMessageData& data, const char* message)
{
    char severity = "VDIWEFF"[data.severity];
    fprintf(stderr, "%c %s:%d] %s\n", severity, data.file, data.line_number, message);
}

LogMessageData::LogMessageData(const char* file, int line, LogSeverity severity, int error)
    : file(file),
      line_number(line),
      severity(severity),
      error(error)
{
    const char* last_slash = strrchr(file, '/');
    file = (last_slash == NULL) ? file : last_slash + 1;
}

LogMessage::~LogMessage()
{
    if (data_->error != -1)
        data_->buffer << ": " << strerror(data_->error);
    std::string msg(data_->buffer.str());

    if (msg.find('\n') == std::string::npos)
        LogLine(*data_, msg.c_str());
    else {
        msg += '\n';
        size_t i = 0;
        while (i < msg.size()) {
            size_t nl = msg.find('\n', i);
            msg[nl] = '\0';
            LogLine(*data_, &msg[i]);
            i = nl + 1;
        }
    }

    if (data_->severity == FATAL)
        exit(EXIT_FAILURE);
}

Inform::~Inform()
{
    std::string msg(buffer_.str());
    fputs(msg.c_str(), stdout);
}