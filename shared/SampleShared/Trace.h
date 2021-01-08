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

namespace sample {

    template <typename CharT>
    inline void FormatHeader(fmt::basic_memory_buffer<CharT>& buffer, const CharT* formatStr) {
        using namespace std::chrono;
        const auto now = system_clock::now();
        const auto posixTime = system_clock::to_time_t(now);
        const auto remainingTime = now - system_clock::from_time_t(posixTime);
        const uint64_t remainingMicroseconds = duration_cast<microseconds>(remainingTime).count();
        const uint32_t threadId = ::GetCurrentThreadId();

        tm localTime;
        ::localtime_s(&localTime, &posixTime);

        fmt::format_to(buffer, formatStr, localTime.tm_hour, localTime.tm_min, localTime.tm_sec, remainingMicroseconds, threadId);
    }

    template <typename... Args>
    inline void Trace(std::wstring_view format_str, const Args&... args) {
        // fmt::wmemory_buffer allows this function to avoid std::wstring heap allocations
        // if the resulting string is less than 500 characters.
        // If it is more than 500 chars then it will do a heap allocation.
        fmt::wmemory_buffer buffer;
        FormatHeader(buffer, L"[{:02d}-{:02d}-{:02d}.{:06d}] (t:{:04x}): ");
        fmt::format_to(buffer, format_str, args...);
        buffer.push_back(L'\n');
        buffer.push_back(L'\0');
        ::OutputDebugStringW(buffer.data());
    }

    template <typename... Args>
    inline void Trace(std::string_view format_str, const Args&... args) {
        fmt::memory_buffer buffer;
        FormatHeader(buffer, "[{:02d}-{:02d}-{:02d}.{:06d}] (t:{:04x}): ");
        fmt::format_to(buffer, format_str, args...);
        buffer.push_back('\n');
        buffer.push_back('\0');
        ::OutputDebugStringA(buffer.data());
    }
} // namespace sample
