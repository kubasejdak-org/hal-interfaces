/////////////////////////////////////////////////////////////////////////////////////
///
/// @file
/// @author Kuba Sejdak
/// @copyright BSD 2-Clause License
///
/// Copyright (c) 2020-2023, Kuba Sejdak <kuba.sejdak@gmail.com>
/// All rights reserved.
///
/// Redistribution and use in source and binary forms, with or without
/// modification, are permitted provided that the following conditions are met:
///
/// 1. Redistributions of source code must retain the above copyright notice, this
///    list of conditions and the following disclaimer.
///
/// 2. Redistributions in binary form must reproduce the above copyright notice,
///    this list of conditions and the following disclaimer in the documentation
///    and/or other materials provided with the distribution.
///
/// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
/// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
/// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
/// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
/// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
/// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
/// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
/// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
/// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
/// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
///
/////////////////////////////////////////////////////////////////////////////////////

#include "hal/time/IRtc.hpp"

#include "hal/Error.hpp"
#include "hal/logger/interfaces.hpp"

#include <cassert>
#include <cerrno>
#include <cstring>

namespace hal::time {

/// Checks if the given std::tm object is valid in terms of allowed field values.
/// @param tm               Time object to be checked.
/// @return Flag indicating if the given std::tm object is valid in terms of allowed field values.
/// @retval true            Given time object is valid.
/// @retval false           Given time object is invalid.
/// @note This function exploits the fact, that std::mktime converts even an invalid std::tm into the std::time_t.
///       This allows to check, if after a double conversion we still have the same value.
///       This function may modify input tm object, by filling tm_wday and tm_yday fields based on all other fields.
static bool isValidTime(std::tm& tm)
{
    std::tm toConvert = tm;
    auto time = std::mktime(&toConvert);
    if (time == static_cast<std::time_t>(-1)) {
        RtcLogger::error("Invalid std::tm value: std::mktime() returned err={}", std::strerror(errno));
        return false;
    }

    std::tm converted{};
    [[maybe_unused]] auto* result = gmtime_r(&time, &converted);
    assert(result);

    tm.tm_wday = converted.tm_wday;
    tm.tm_yday = converted.tm_yday;
    tm.tm_isdst = converted.tm_isdst;

    auto isSame = [](const std::tm& a, const std::tm& b) {
        bool sameHour = (a.tm_hour == b.tm_hour) && (a.tm_min == b.tm_min) && (a.tm_sec == b.tm_sec);
        bool sameDay = (a.tm_mday == b.tm_mday) && (a.tm_mon == b.tm_mon) && (a.tm_year == b.tm_year);
        bool sameCalendar = (a.tm_wday == b.tm_wday) && (a.tm_yday == b.tm_yday) && (a.tm_isdst == b.tm_isdst);

        return (sameHour && sameDay && sameCalendar);
    };

    return isSame(tm, converted);
}

IRtc::IRtc()
    : Device(SharingPolicy::eShared)
{}

Result<std::tm> IRtc::getTime()
{
    auto [tm, error] = drvGetTime();
    if (error)
        return error;

    if (!isValidTime(*tm))
        return Error::eHardwareError;

    return *tm;
}

std::error_code IRtc::setTime(const std::tm& tm)
{
    std::tm toSet = tm;
    if (!isValidTime(toSet)) {
        RtcLogger::error("Failed to set time: invalid argument");
        return Error::eInvalidArgument;
    }

    auto error = drvSetTime(toSet);
    if (error)
        RtcLogger::error("Failed to set time: drvSetTime() returned err={}", error.message());

    m_initialized = static_cast<bool>(!error);
    return error;
}

} // namespace hal::time
