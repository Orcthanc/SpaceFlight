/*
 * =====================================================================================
 *
 *       Filename:  Util.hpp
 *
 *    Description:  Contains useful includes, functions, globals, etc
 *
 *        Version:  1.0
 *        Created:  03/27/2020 09:07:49 PM
 *       Revision:  none
 *
 *         Author:  Samuel Knoethig (), samuel@knoethig.net
 *
 * =====================================================================================
 */
#pragma once

#include <vector>
#include <string>
#include <memory>
#include <algorithm>

#include "Logger.hpp"
#include "Parser.hpp"

#ifndef CHANNELS
#define CHANNELS		\
	CHANNEL( Default )	\
	CHANNEL( Video )	\
	CHANNEL( Config )
#endif //CHANNELS

#ifndef LOGLEVELS
#define LOGLEVELS		\
	LOGLEVEL( Verbose )	\
	LOGLEVEL( Info )	\
	LOGLEVEL( Warning )	\
	LOGLEVEL( Error )	\
	LOGLEVEL( Critical )
#endif //LOGLEVELS

enum class LogChannel {
	#define CHANNEL( a ) a,
	CHANNELS
	#undef CHANNEL
};

namespace LogLevel {
	// Numbers are subject to change
	enum LogLevel {
		#define LOGLEVEL( a ) a,
		LOGLEVELS
		#undef LOGLEVEL
	};
}

#ifndef CFGOPTIONS
#define CFGOPTIONS			\
	CFGOPTION( xres )		\
	CFGOPTION( yres )		\
	CFGOPTION( fullscreen )
#endif //CFGOPTIONS

enum class CfgOption {
	#define CFGOPTION( a ) a,
	CFGOPTIONS
	#undef CFGOPTION
};

extern Logger::Logger<LogChannel> logger;
extern std::unique_ptr<Config::Config<CfgOption>> config;

namespace Resolution {
	extern uint32_t xres, yres;
}

namespace Logger {
	std::string channel_to_string( LogChannel channel );
	std::string level_to_string( LogLevel::LogLevel level );
}
