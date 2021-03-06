/*
 * =====================================================================================
 *
 *       Filename:  Util.cpp
 *
 *    Description:  Contains external definitions from Util.hpp
 *
 *        Version:  1.0
 *        Created:  03/27/2020 09:37:18 PM
 *       Revision:  none
 *
 *         Author:  Samuel Knoethig (), samuel@knoethig.net
 *
 * =====================================================================================
 */

#include "Util.hpp"

Logger::Logger<LogChannel> logger;
Config::Config config( 
			[]( std::string msg ){ logger << LogChannel::Config << LogLevel::Error << msg; },
			[]( std::string msg ){ logger << LogChannel::Config << LogLevel::Warning << msg; });

std::string Logger::channel_to_string( LogChannel channel ){
	switch( channel ){
		#define CHANNEL( a ) case LogChannel::a: return #a;
		CHANNELS
	}

	return "Should never be called... Location: " __FILE__ ":" + std::to_string( __LINE__ );
}

std::string Logger::level_to_string( LogLevel::LogLevel level ){
	using namespace LogLevel;
#if COLOR_CONSOLE
	switch( level ){
		case Verbose:
			return "\033[90m Verbose ";
		case Info:
			return "   Info  ";
		case Warning:
			return "\033[33m Warning \033[0m";
		case Error:
			return "\033[31m  Error  \033[0m";
		case Critical:
			return "\033[1;31mCritical \033[0m";
		default:
			return "Unknown loglevel " + std::to_string( level ) + " encountered";
	}
#elif
	switch( level ){
		#define LOGLEVEL( a ) case a: return #a;
		LOGLEVELS
		default:
			return "Unknown loglevel " + std::to_string( level ) + " encountered";
	}
#endif
}
