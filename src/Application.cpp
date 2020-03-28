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
	init_window();
	init_vk();
	main_loop();
	cleanup();
}

void SpaceApplication::init_window(){
	logger << LogChannel::Video << LogLevel::Info << "Started creating window";

	glfwInit();
	glfwWindowHint( GLFW_CLIENT_API, GLFW_NO_API );
	window = glfwCreateWindow(
			std::stoi( config->options.at( CfgOption::xres )),
			std::stoi( config->options.at( CfgOption::yres )),
			"SpaceApp", nullptr, nullptr );


}

void SpaceApplication::init_vk(){
	logger << LogChannel::Video << LogLevel::Info << "Started initialising Vulkan";

	create_instance();
	choose_physical_dev({});
}

void SpaceApplication::create_instance(){
	vk::ApplicationInfo appinfo( "SpaceApp", VK_MAKE_VERSION( 0, 1, 0 ), "SpaceEngine", VK_MAKE_VERSION( 0, 1, 0 ), VK_API_VERSION_1_2 );
#ifndef NDEBUG

	auto av_layers = vk::enumerateInstanceLayerProperties();
	{
		Logger::LoggerHelper lmsg = logger << LogChannel::Video << LogLevel::Verbose;
		lmsg << "Available debug layers:";

		for( auto& l: av_layers )
			lmsg << "\n\t" << l.layerName;
	}

	const std::vector<const char*> layers = { 
		"VK_LAYER_LUNARG_standard_validation", 
//		"VK_LAYER_LUNARG_api_dump",
	};
#elif //NDEBUG
	const std::vector<const char*> layers;
#endif //NDEBUG
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;

	glfwExtensions = glfwGetRequiredInstanceExtensions( &glfwExtensionCount );

	const std::vector<const char*> extensions( glfwExtensions, glfwExtensions + glfwExtensionCount );

	auto av_exts = vk::enumerateInstanceExtensionProperties();
	{
		Logger::LoggerHelper lmsg = logger << LogChannel::Video << LogLevel::Verbose;
		lmsg << "Available extensions:";

		for( auto& l: av_exts )
			lmsg << "\n\t" << l.extensionName;
	}

	vk::InstanceCreateInfo instance_cr_inf( {}, &appinfo, layers.size(), layers.data(), extensions.size(), extensions.data() );

	instance = vk::createInstanceUnique( instance_cr_inf );

	logger << LogChannel::Video << LogLevel::Info << "Created instance";
}

void SpaceApplication::choose_physical_dev( const std::vector<vk::ExtensionProperties>& required_exts ){
	auto physical_devs{ instance->enumeratePhysicalDevices() };

	size_t best_score = 0;
	std::string best_name;

	for( auto& phys_dev: physical_devs ){
		size_t score = 0;
		std::vector<vk::ExtensionProperties> exts = phys_dev.enumerateDeviceExtensionProperties();

		bool supported = true;
		for( auto& ext: required_exts ){
			if( std::find( exts.begin(), exts.end(), ext ) == exts.end()){
				supported = false;
			}
		}
		if( !supported )
			continue;

		auto properties = phys_dev.getProperties();
		if( properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu )
			score += 50000;
		else if( properties.deviceType == vk::PhysicalDeviceType::eIntegratedGpu )
			score += 5000;

		score += properties.limits.maxImageDimension2D;

		logger << LogChannel::Video << LogLevel::Verbose << "Found suitable physical device " <<
			properties.deviceName << " with score " << score;

		if( score > best_score ){
			best_name = properties.deviceName;
			best_score = score;
			this->phys_dev = phys_dev;
		}
	}

	logger << LogChannel::Video << LogLevel::Info << "Chose physical device " << best_name << " with score of " << best_score;
}

void SpaceApplication::main_loop(){
	logger << LogChannel::Default << LogLevel::Info << "Entering main loop";

	while( !glfwWindowShouldClose( window )){
		glfwPollEvents();
	}
}

void SpaceApplication::cleanup(){
	logger << LogChannel::Video << LogLevel::Info << "Started cleaning up window";

	glfwDestroyWindow( window );
	glfwTerminate();
}
