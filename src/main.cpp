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

#include <stdint.h>

void setupLogging(){
	logger.channel_to_string = Logger::channel_to_string;
	logger.loglevel_to_string = []( size_t i ){ return Logger::level_to_string( static_cast<LogLevel::LogLevel>( i ));};

	logger.enable( LogChannel::Default );
	logger.enable( LogChannel::Video );
	logger.enable( LogChannel::Config );
}

void handle_config( int argc, char** argv ){
	Config::ConfigParser<CfgOption> parser( 
			[]( std::string msg ){ logger << LogChannel::Config << LogLevel::Error << msg; },
			[]( std::string msg ){ logger << LogChannel::Config << LogLevel::Warning << msg; });

	parser.add({ CfgOption::xres, "xres", 'x', required_argument, "Sets the x resolution the program window should use", "800" });
	parser.add({ CfgOption::yres, "yres", 'y', required_argument, "Sets the y resolution the program window should use", "600" });
	parser.add({ CfgOption::fullscreen, "fullscreen", 'f', no_argument, "Sets wether the window should be fullscreen", "0" });

	bool should_exit = false;
	config = std::make_unique<Config::Config<CfgOption>>( parser.read_config( "./config.cfg", argc, argv, &should_exit ));
	if( should_exit )
		exit( 0 );
}

int main( int argc, char** argv ){
	setupLogging();
	handle_config( argc, argv );

	logger << LogChannel::Config << LogLevel::Info << "Succesfully read config";

	SpaceApplication app;

	app();

	config->writeConfig( "./config.cfg" );
}
