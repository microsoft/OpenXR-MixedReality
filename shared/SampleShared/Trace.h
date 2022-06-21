// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <iostream>
#include <iomanip>
#include <fstream>
#include <chrono>
#include <thread>
#include <string_view>
#include <processthreadsapi.h>

#define FMT_HEADER_ONLY
#include <fmt/format.h>
#include <fmt/xchar.h>

namespace sample {

    inline void FormatHeader(fmt::memory_buffer& buffer, const char* formatStr) {
        using namespace std::chrono;
        const auto now = system_clock::now();
        const auto posixTime = system_clock::to_time_t(now);
        const auto remainingTime = now - system_clock::from_time_t(posixTime);
        const uint64_t remainingMicroseconds = duration_cast<microseconds>(remainingTime).count();
        const uint32_t threadId = ::GetCurrentThreadId();

        tm localTime;
        ::localtime_s(&localTime, &posixTime);

        fmt::format_to(
            fmt::appender(buffer), formatStr, localTime.tm_hour, localTime.tm_min, localTime.tm_sec, remainingMicroseconds, threadId);
    }

    template <typename... Args>
    inline void Trace(std::string_view format_str, const Args&... args) {
        fmt::memory_buffer buffer;
        FormatHeader(buffer, "[{:02d}-{:02d}-{:02d}.{:06d}] (t:{:04x}): ");
        fmt::format_to(fmt::appender(buffer), format_str, args...);
        buffer.push_back('\n');
        buffer.push_back('\0');
        ::OutputDebugStringA(buffer.data());
    }
} // namespace sample
