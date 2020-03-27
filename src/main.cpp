/*
 * =====================================================================================
 *
 *       Filename:  main.cpp
 *
 *    Description:  Entry point
 *
 *        Version:  1.0
 *        Created:  03/27/2020 08:59:03 PM
 *       Revision:  none
 *
 *         Author:  Samuel Knoethig (), samuel@knoethig.net
 *
 * =====================================================================================
 */

#include "Util.hpp"
#include "Application.hpp"

void setupLogging(){
	logger.channel_to_string = Logger::channel_to_string;
	logger.loglevel_to_string = []( size_t i ){ return Logger::level_to_string( static_cast<LogLevel::LogLevel>( i ));};

	logger.enable( LogChannel::Default );
	logger.enable( LogChannel::Video );
}

int main( int argc, char** argv ){
	setupLogging();

	logger << LogChannel::Default << LogLevel::Verbose << "Verbose test";
	logger << LogChannel::Default << LogLevel::Info << "Info test";
	logger << LogChannel::Default << LogLevel::Warning << "Warning test";
	logger << LogChannel::Default << LogLevel::Error << "Error test";
	logger << LogChannel::Default << LogLevel::Critical << "Critical test";

	SpaceApplication app;

	app();
}
