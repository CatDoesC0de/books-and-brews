/*
 * -------------------------------
 * Copyright (C) 2025 Connor Taylor.
 * Released under the MIT License.
 * -------------------------------
 *
 * Program name: books_and_brews.cpp
 * Author: Connor Taylor
 * Date last updated: 10/3/2025
 * Purpose: Define a logger and provide utililty functions
 */
#pragma once

#include "bb_common.hpp"

#define BB_INFO(logger, Format, ...)                                           \
    Log(logger, FormatString(log_level::log_info, Format, ##__VA_ARGS__))
#define BB_WARN(logger, Format, ...)                                           \
    Log(logger, FormatString(log_level::log_warning, Format, ##__VA_ARGS__))
#define BB_ERROR(logger, Format, ...)                                          \
    Log(logger, FormatString(log_level::log_error, Format, ##__VA_ARGS__))

enum class log_level
{
    log_error,
    log_info,
    log_warning
};

struct logger
{
    const char* Name;
    char** Messages;
    u32 MessagesSize;
};

logger CreateLogger(const char* Name, u32 Messages);
void FreeLogger(logger& Logger);
void ClearLogs(logger& Logger);
void Log(logger& Logger, const char* String);
void PrintLogMessages(logger& Logger);
char* FormatString(log_level LogLevel, const char* Format, ...);
