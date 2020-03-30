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

#ifndef CFGOPTIONS
#define CFGOPTIONS												\
CFGOPTION( res, ::Config::Resolution, ::Config::Resolution{})	\
CFGOPTION( fullscreen, bool, false )
#endif //CFGOPTIONS

namespace Config {
	struct Resolution {
		uint32_t x = 1920, y = 1080;
	};

	template <typename T>
	inline std::string to_string( const T& val );

	template <typename T>
	inline T from_string( const std::string& val );

	template <>
	inline std::string to_string<Resolution>( const Resolution& val ){
		return std::to_string( val.x ) + "x" + std::to_string( val.y );
	}

	template <>
	inline Resolution from_string<Resolution>( const std::string& val ){
		int i = 0;
		bool second = false;
		Resolution res;
		for( auto& c: val ){
			if( c >= '0' && c <= '9' )
				i = i * 10 + c - '0';
			else if(( c == 'x' || c == 'X' ) && second == false ){
				res.x = i;
				i = 0;
			}
		}
		res.y = i;
		return res;
	}
}

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

extern Logger::Logger<LogChannel> logger;
extern Config::Config config;

namespace Logger {
	std::string channel_to_string( LogChannel channel );
	std::string level_to_string( LogLevel::LogLevel level );
}
