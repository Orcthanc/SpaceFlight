/*
 * =====================================================================================
 *
 *       Filename:  Application.cpp
 *
 *    Description:  Source file defining things from Application.hpp
 *
 *        Version:  1.0
 *        Created:  03/27/2020 09:32:05 PM
 *       Revision:  none
 *
 *         Author:  Samuel Knoethig (), samuel@knoethig.net
 *
 * =====================================================================================
 */

#include "Util.hpp"
#include "Application.hpp"

void SpaceApplication::operator()(){
	init_vk();
	main_loop();
	deinit_vk();
}

void SpaceApplication::init_vk(){
	logger << LogChannel::Video << LogLevel::Info << "Started initialising Vulkan";
}

void SpaceApplication::main_loop(){
	logger << LogChannel::Default << LogLevel::Info << "Entering main loop";
}

void SpaceApplication::deinit_vk(){
	logger << LogChannel::Video << LogLevel::Info << "Started terminating vulkan";
}
