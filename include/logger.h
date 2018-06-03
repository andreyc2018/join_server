#pragma once

#include "singleton.h"

#define SPDLOG_TRACE_ON
#define SPDLOG_DEBUG_ON
#include <spdlog/spdlog.h>

using LoggerPtr = std::shared_ptr<spdlog::logger>;

extern LoggerPtr gLogger;

#define TRACE()  SPDLOG_TRACE(gLogger, "{}", __PRETTY_FUNCTION__)
