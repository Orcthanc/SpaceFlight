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
std::unique_ptr<Config::Config<Options>> options;

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
			return "\033[90mVerbose  \033[0m";
		case Info:
			return "Info     ";
		case Warning:
			return "\033[33mWarning  \033[0m";
		case Error:
			return "\033[31mError    \033[0m";
		case Critical:
			return "\033[1;31mCritical \033[0m";
		default:
			return "Unknown loglevel level encountered";
	}
#elif
	switch( level ){
		#define LOGLEVEL( a ) case a: return #a;
		LOGLEVELS
		default:
			return "Unknown loglevel level encountered";
	}
#endif
}
