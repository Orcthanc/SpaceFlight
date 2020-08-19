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
#include <fstream>

namespace fs = std::filesystem;

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
			config.res.x,
			config.res.y,
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
	create_image_views();
	create_render_pass();
	create_pipeline();
	create_framebuffers();
	create_command_pool();
	alloc_command_buffers();
	create_semaphores();
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
		"VK_LAYER_KHRONOS_validation",
		"VK_LAYER_MANGOHUD_overlay",
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
			best_name = std::string( properties.deviceName );
			best_score = score;
			this->phys_dev = phys_dev;
		}
	}

	logger << LogChannel::Video << LogLevel::Info << "Chose physical device " << best_name << " with score of " << best_score;
}

SpaceAppVideo::QueueFamilyIndices SpaceApplication::find_queue_families( vk::PhysicalDevice phys_dev ){
	SpaceAppVideo::QueueFamilyIndices indices;

	auto qfprops = phys_dev.getQueueFamilyProperties();

	for( size_t i = 0; i < qfprops.size(); ++i ){
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

	vk::Extent2D window_size{ config.res.x, config.res.y };
	window_size.width = std::max( swapchain_support.capabilities.minImageExtent.width,
			std::min( swapchain_support.capabilities.maxImageExtent.width, window_size.width ));
	window_size.height = std::max( swapchain_support.capabilities.minImageExtent.height,
			std::min( swapchain_support.capabilities.maxImageExtent.height, window_size.height ));

	return window_size;
}

void SpaceApplication::create_image_views(){
	swapchain_img_views.clear();

	for( size_t i = 0; i < swapchain_imgs.size(); ++i ){
		vk::ImageViewCreateInfo cr_inf;
		cr_inf.image = swapchain_imgs[i];
		cr_inf.viewType = vk::ImageViewType::e2D;
		cr_inf.format = swapchain_img_fmt;
		cr_inf.components = vk::ComponentMapping(
				vk::ComponentSwizzle::eIdentity,
				vk::ComponentSwizzle::eIdentity,
				vk::ComponentSwizzle::eIdentity,
				vk::ComponentSwizzle::eIdentity );
		cr_inf.subresourceRange = vk::ImageSubresourceRange(
				vk::ImageAspectFlagBits::eColor,
				0, 1,
				0, 1);

		swapchain_img_views.push_back( device->createImageViewUnique( cr_inf ));
	}

	logger << LogChannel::Video << LogLevel::Info << "Created " << swapchain_imgs.size() << " image views";
}

void SpaceApplication::create_render_pass(){
	vk::AttachmentDescription attachment_description(
			{},
			swapchain_img_fmt,
			vk::SampleCountFlagBits::e1,
			vk::AttachmentLoadOp::eClear,
			vk::AttachmentStoreOp::eStore,
			vk::AttachmentLoadOp::eDontCare,
			vk::AttachmentStoreOp::eDontCare,
			vk::ImageLayout::eUndefined,
			vk::ImageLayout::ePresentSrcKHR
		);

	vk::AttachmentReference attachment_reference( 0, vk::ImageLayout::eColorAttachmentOptimal );

	std::vector attachment_references{ attachment_reference };
	vk::SubpassDescription subpass_description(
			{},
			vk::PipelineBindPoint::eGraphics,
			{},
			attachment_references,
			{},
			{},
			{}
		);

	vk::SubpassDependency sub_dependency( 
			VK_SUBPASS_EXTERNAL, 0,
			vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eColorAttachmentOutput,
			{}, vk::AccessFlagBits::eColorAttachmentWrite,
			{}
		);

	vk::RenderPassCreateInfo render_pass_info(
			{},
			1, &attachment_description,
			1, &subpass_description,
			1, &sub_dependency
		);

	render_pass = device->createRenderPassUnique( render_pass_info );
	logger << LogChannel::Video << LogLevel::Info << "Created a render pass";
}

vk::UniqueShaderModule SpaceApplication::create_shader_module( const fs::path& path ){
	std::ifstream code_file( path, std::ios::ate | std::ios::binary );

	if( !code_file.is_open()){
		logger << LogChannel::Video << LogLevel::Error << "Could not open file: " << path;
		throw std::runtime_error( "Could not open file" );
	}

	size_t file_size = ( size_t )code_file.tellg();
	std::vector<char> code( file_size );

	code_file.seekg( 0 );
	code_file.read( code.data(), file_size );

	code_file.close();

	vk::ShaderModuleCreateInfo cr_inf( vk::ShaderModuleCreateFlags{}, code.size(), reinterpret_cast<const uint32_t*>( code.data() ));

	return device->createShaderModuleUnique( cr_inf );
}

void SpaceApplication::create_pipeline(){
	auto vert = create_shader_module( "res/shader/basic.vert.glsl.spv" );
	auto frag = create_shader_module( "res/shader/basic.frag.glsl.spv" );

	logger << LogChannel::Video << LogLevel::Info << "Created shader modules";

	std::vector<vk::PipelineShaderStageCreateInfo> stage_infos{
			{ {}, vk::ShaderStageFlagBits::eVertex, *vert, "main", {} },
			{ {}, vk::ShaderStageFlagBits::eFragment, *frag, "main", {} },
		};

	vk::PipelineVertexInputStateCreateInfo vertex_input_info(
			{},
			0,
			nullptr,
			0,
			nullptr
		);

	vk::PipelineInputAssemblyStateCreateInfo input_assembly_info(
			{},
			vk::PrimitiveTopology::eTriangleList,
			VK_FALSE
		);

	vk::Viewport viewport(
			0, 0,
			(float)swapchain_img_size.width, (float)swapchain_img_size.height,
			0, 1
		);

	vk::Rect2D scissor(
			{},
			swapchain_img_size
		);

	std::vector viewports{ viewport };
	std::vector scissors{ scissor };
	vk::PipelineViewportStateCreateInfo viewport_state_info(
			{},
			viewports,
			scissors
		);

	vk::PipelineRasterizationStateCreateInfo rasterization_state_info(
			{},
			VK_FALSE,
			VK_FALSE,
			vk::PolygonMode::eFill,
			vk::CullModeFlagBits::eBack,
			vk::FrontFace::eClockwise,
			VK_FALSE,
			0,
			0,
			0,
			1
		);

	vk::PipelineMultisampleStateCreateInfo multisample_state_info(
			{},
			vk::SampleCountFlagBits::e1,
			VK_FALSE,
			1,
			nullptr,
			VK_FALSE,
			VK_FALSE
		);

	vk::PipelineColorBlendAttachmentState color_blend_attachment(
			VK_FALSE,
			vk::BlendFactor::eOne,
			vk::BlendFactor::eZero,
			vk::BlendOp::eAdd,
			vk::BlendFactor::eOne,
			vk::BlendFactor::eZero,
			vk::BlendOp::eAdd,
			vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
		);

	std::vector color_blend_attachments{ color_blend_attachment };
	vk::PipelineColorBlendStateCreateInfo color_blend_info(
			{},
			VK_FALSE,
			vk::LogicOp::eCopy,
			color_blend_attachments,
			{ 0, 0, 0, 0 } //TODO may not work
		);

	vk::PipelineLayoutCreateInfo pipeline_layout_info(
			{},
			{},
			{}
		);

	pipeline_layout = device->createPipelineLayoutUnique( pipeline_layout_info );

	vk::GraphicsPipelineCreateInfo pipeline_create_info(
			{},
			stage_infos,
			&vertex_input_info,
			&input_assembly_info,
			nullptr,
			&viewport_state_info,
			&rasterization_state_info,
			&multisample_state_info,
			nullptr,
			&color_blend_info,
			nullptr,
			*pipeline_layout,
			*render_pass,
			0,
			vk::Pipeline{},
			-1
		);

	std::vector pipeline_create_infos{ pipeline_create_info };
	pipelines = device->createGraphicsPipelinesUnique( {}, pipeline_create_infos ).value;

	logger << LogChannel::Video << LogLevel::Info << "Created a pipeline";
}

void SpaceApplication::create_framebuffers(){
	swapchain_framebuffers.resize( swapchain_img_views.size() );

	for( size_t i = 0; i < swapchain_img_views.size(); ++i ){
		std::vector<vk::ImageView> attachments{ *swapchain_img_views[i] };

		vk::FramebufferCreateInfo frame_cr_inf(
				{},
				*render_pass,
				attachments,
				swapchain_img_size.width,
				swapchain_img_size.height,
				1
			);

		swapchain_framebuffers[i] = device->createFramebufferUnique( frame_cr_inf );
	}
	logger << LogChannel::Video << LogLevel::Info << "Created framebuffers";
}

void SpaceApplication::create_command_pool(){
	vk::CommandPoolCreateInfo cmd_cr_inf(
			{},
			queue_indices.graphics.value()
		);

	command_pool = device->createCommandPoolUnique( cmd_cr_inf );

	logger << LogChannel::Video << LogLevel::Info << "Created command pool";
}

void SpaceApplication::alloc_command_buffers(){
	vk::CommandBufferAllocateInfo alloc_inf(
			*command_pool,
			vk::CommandBufferLevel::ePrimary,
			swapchain_framebuffers.size()
		);

	command_buffers = device->allocateCommandBuffersUnique( alloc_inf );

	for( size_t i = 0; i < command_buffers.size(); ++i ){
		vk::CommandBufferBeginInfo cb_begin_info( {}, {} );

		command_buffers[i]->begin( cb_begin_info );

		std::vector<vk::ClearValue> clear_colors{{std::array<float, 4>{ 0, 0, 0, 1 }}};
		vk::RenderPassBeginInfo r_begin_info(
				*render_pass,
				*swapchain_framebuffers[i],
				{{ 0, 0 }, swapchain_img_size },
				clear_colors
			);

		command_buffers[i]->beginRenderPass( r_begin_info, vk::SubpassContents::eInline );
		command_buffers[i]->bindPipeline( vk::PipelineBindPoint::eGraphics, *pipelines[0] );
		command_buffers[i]->draw( 3, 1, 0, 0 );

		command_buffers[i]->endRenderPass();
		command_buffers[i]->end();
	}
}

void SpaceApplication::create_semaphores(){
	vk::SemaphoreCreateInfo sema_cr_inf( vk::SemaphoreCreateFlags{} );
	vk::FenceCreateInfo fence_cr_inf( vk::FenceCreateFlagBits::eSignaled );

	for( size_t i = 0; i < SpaceAppVideo::MAX_FRAMES_IN_FLIGHT; ++i ){
		img_available_sema.push_back( device->createSemaphoreUnique( sema_cr_inf ));
		img_ready_sema.push_back( device->createSemaphoreUnique( sema_cr_inf ));
		inflight_fences.push_back( device->createFenceUnique( fence_cr_inf ));
	}
	inflight_imgs.resize( swapchain_imgs.size() );
}

void SpaceApplication::main_loop(){
	logger << LogChannel::Default << LogLevel::Info << "Entering main loop";

	while( !glfwWindowShouldClose( window )){
		glfwPollEvents();
		draw_frame();
	}

	device->waitIdle();
}

void SpaceApplication::cleanup(){
	logger << LogChannel::Video << LogLevel::Info << "Started cleaning up window";

	glfwDestroyWindow( window );
	glfwTerminate();
}

void SpaceApplication::draw_frame(){
	static size_t current_frame = 0;
	if( vk::Result::eSuccess != device->waitForFences( 1, &*inflight_fences[current_frame], VK_TRUE, UINT64_MAX ))
		throw std::runtime_error( "Wait for fence failed" );
	
	auto img = device->acquireNextImageKHR( *swapchain, UINT64_MAX, *img_available_sema[current_frame], {} ).value;

	if( inflight_imgs[img] != vk::Fence{} )
		if( vk::Result::eSuccess != device->waitForFences( 1, &inflight_imgs[img], VK_TRUE, UINT64_MAX ))
			throw std::runtime_error( "Wait for fence failed" );

	std::vector wait_semas{ *img_available_sema[current_frame] };
	std::vector<vk::Flags<vk::PipelineStageFlagBits>> wait_stages{ vk::PipelineStageFlagBits::eColorAttachmentOutput };
	std::vector cmd_bufs{ *command_buffers[img] };
	std::vector signal_semas{ *img_ready_sema[current_frame] };

	vk::SubmitInfo sub_inf(
			wait_semas,
			wait_stages,
			cmd_bufs,
			signal_semas
		);

	if( vk::Result::eSuccess != device->resetFences( 1, &*inflight_fences[current_frame] ))
		throw std::runtime_error( "Reset fence failed" );

	graphics_queue.submit( { sub_inf }, *inflight_fences[current_frame] );

	std::vector swapchains{ *swapchain };
	std::vector indices{ img };
	vk::PresentInfoKHR pres_inf( signal_semas, swapchains, indices, {} );

	switch( present_queue.presentKHR( pres_inf )){
		default:
			break;
	}

	current_frame = ( current_frame + 1 ) % SpaceAppVideo::MAX_FRAMES_IN_FLIGHT;
}
