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

#include <set>

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
			Resolution::xres,
			Resolution::yres,
			"SpaceApp", nullptr, nullptr );


}

void SpaceApplication::init_vk(){
	logger << LogChannel::Video << LogLevel::Info << "Started initialising Vulkan";

	create_instance();
	create_surface();
	choose_physical_dev({});
	create_device();
	graphics_queue = device->getQueue( queue_indices.graphics.value(), 0 );
	present_queue = device->getQueue( queue_indices.present.value(), 0 );
	create_swapchain();
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
		lmsg << "Available instance extensions:";

		for( auto& l: av_exts )
			lmsg << "\n\t" << l.extensionName;
	}

	vk::InstanceCreateInfo instance_cr_inf( {}, &appinfo, layers.size(), layers.data(), extensions.size(), extensions.data() );

	instance = vk::createInstanceUnique( instance_cr_inf );

	logger << LogChannel::Video << LogLevel::Info << "Created instance";
}

void SpaceApplication::create_surface(){
	VkSurfaceKHR temp;
	glfwCreateWindowSurface( *instance, window, nullptr, &temp );
	surface = vk::UniqueSurfaceKHR{ temp, *instance };
	logger << LogChannel::Video << LogLevel::Info << "Created surface";
}

static std::set<std::string> get_missing_dev_extensions( vk::PhysicalDevice dev, const std::vector<const char*>& extensions ){
	auto available_exts = dev.enumerateDeviceExtensionProperties();

	std::set<std::string> req_exts{ extensions.begin(), extensions.end() };

	for( auto& ext: available_exts ){
		req_exts.erase( ext.extensionName );
	}

	return req_exts;
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

		auto qfindices = find_queue_families( phys_dev );
		if( !qfindices.complete() )
			continue;

		if( !get_missing_dev_extensions( phys_dev, dev_exts ).empty() )
			continue;

		SpaceAppVideo::SwapchainDetails swapchain_details( phys_dev, surface );
		if( swapchain_details.formats.empty() || swapchain_details.present_modes.empty() )
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
			swapchain_support = swapchain_details;
			best_name = properties.deviceName;
			best_score = score;
			this->phys_dev = phys_dev;
		}
	}

	logger << LogChannel::Video << LogLevel::Info << "Chose physical device " << best_name << " with score of " << best_score;
}

SpaceAppVideo::QueueFamilyIndices SpaceApplication::find_queue_families( vk::PhysicalDevice phys_dev ){
	SpaceAppVideo::QueueFamilyIndices indices;

	auto qfprops = phys_dev.getQueueFamilyProperties();

	for( int i = 0; i < qfprops.size(); ++i ){
		if( qfprops[i].queueFlags & vk::QueueFlagBits::eGraphics )
			indices.graphics = i;

		if( phys_dev.getSurfaceSupportKHR( i, *surface ))
			indices.present = i;

		if( indices.complete())
			break;
	}

	return indices;
}

void SpaceApplication::create_device(){
	queue_indices = find_queue_families( phys_dev );

	std::vector<vk::DeviceQueueCreateInfo> dev_q_cr_infs;
	std::set<uint32_t> unique_families{ queue_indices.graphics.value(), queue_indices.present.value() };

	const float priorities[] = { 1.0f };

	for( auto qf: unique_families ){
		dev_q_cr_infs.push_back({ {}, qf, 1, priorities });
	}

	vk::PhysicalDeviceFeatures features;

	vk::DeviceCreateInfo dev_cr_inf(
			{},
			dev_q_cr_infs.size(), dev_q_cr_infs.data(),
			0, nullptr,
			dev_exts.size(), dev_exts.data(),
			&features
		);

	device = phys_dev.createDeviceUnique( dev_cr_inf );
	logger << LogChannel::Video << LogLevel::Info << "Created a logical device";
}

void SpaceApplication::create_swapchain(){
	vk::SurfaceFormatKHR format = choose_swapchain_surface_format();
	vk::PresentModeKHR present_mode = choose_swapchain_present_mode();
	vk::Extent2D extent = choose_swapchain_extent();

	uint32_t image_count = swapchain_support.capabilities.minImageCount + 1;
	if( swapchain_support.capabilities.maxImageCount > 0 && swapchain_support.capabilities.maxImageCount < image_count )
		image_count = swapchain_support.capabilities.maxImageCount;

	vk::SwapchainCreateInfoKHR cr_inf( 
			{},
			*surface,
			image_count,
			format.format,
			format.colorSpace,
			extent,
			1,
			vk::ImageUsageFlagBits::eColorAttachment,
			vk::SharingMode::eExclusive,
			{},
			nullptr,
			swapchain_support.capabilities.currentTransform,
			vk::CompositeAlphaFlagBitsKHR::eOpaque,
			present_mode,
			true,
			{} );

	uint32_t queue_family_indices[] = { queue_indices.graphics.value(), queue_indices.present.value() };
	if( queue_indices.graphics != queue_indices.present ){
		cr_inf.imageSharingMode = vk::SharingMode::eConcurrent;
		cr_inf.queueFamilyIndexCount = 2;
		cr_inf.pQueueFamilyIndices = queue_family_indices;
	}

	swapchain = device->createSwapchainKHRUnique( cr_inf );
	logger << LogChannel::Video << LogLevel::Info << "Successfully created a swapchain";

	swapchain_imgs = device->getSwapchainImagesKHR( *swapchain );
	swapchain_img_fmt = format.format;
	swapchain_img_size = extent;
}

vk::SurfaceFormatKHR SpaceApplication::choose_swapchain_surface_format(){
	for( const auto& format: swapchain_support.formats ){
		if( format.format == vk::Format::eB8G8R8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear )
			return format;
	}

	logger << LogChannel::Video << LogLevel::Warning <<
		"Preferred surfaceformat/colorspace not available. Falling back to " <<
		vk::to_string( swapchain_support.formats[0].format ) << " / " <<
		vk::to_string( swapchain_support.formats[0].format );

	return swapchain_support.formats[0];
}

vk::PresentModeKHR SpaceApplication::choose_swapchain_present_mode(){
	for( const auto& mode: swapchain_support.present_modes ){
		if( mode == vk::PresentModeKHR::eMailbox )
			return mode;
	}
	logger << LogChannel::Video << LogLevel::Warning <<
		"Preferred presentmode not available. Falling back to FIFO";
	return vk::PresentModeKHR::eFifo;
}

vk::Extent2D SpaceApplication::choose_swapchain_extent(){
	if( swapchain_support.capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max() ){
		return swapchain_support.capabilities.currentExtent;
	}

	vk::Extent2D window_size{ Resolution::xres, Resolution::yres };
	window_size.width = std::max( swapchain_support.capabilities.minImageExtent.width,
			std::min( swapchain_support.capabilities.maxImageExtent.width, window_size.width ));
	window_size.height = std::max( swapchain_support.capabilities.minImageExtent.height,
			std::min( swapchain_support.capabilities.maxImageExtent.height, window_size.height ));

	return window_size;
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
