/*
 * =====================================================================================
 *
 *       Filename:  Application.hpp
 *
 *    Description:  Application class that handles everything
 *
 *        Version:  1.0
 *        Created:  03/27/2020 09:27:20 PM
 *       Revision:  none
 *
 *         Author:  Samuel Knoethig (), samuel@knoethig.net
 *
 * =====================================================================================
 */
#pragma once

#include "Util.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>

#include "AppGraphics.hpp"

/**
 *	Class representing the whole application
 */
struct SpaceApplication {
	public:
		/**
		 *	Used to start the application, contains the main loop, returns upon application-exit
		 */
		void operator()();

	private:
		void init_window();
		void init_vk();
		void main_loop();
		void cleanup();

		void create_instance();
		void create_surface();
		void choose_physical_dev( const std::vector<vk::ExtensionProperties>& required_exts );
		SpaceAppVideo::QueueFamilyIndices find_queue_families( vk::PhysicalDevice phys_dev );
		void create_device();
		void create_swapchain();

		vk::SurfaceFormatKHR choose_swapchain_surface_format();
		vk::PresentModeKHR choose_swapchain_present_mode();
		vk::Extent2D choose_swapchain_extent();

		GLFWwindow* window;
		vk::UniqueInstance instance;
		vk::UniqueSurfaceKHR surface;
		vk::PhysicalDevice phys_dev;
		SpaceAppVideo::SwapchainDetails swapchain_support;
		vk::UniqueDevice device;
		SpaceAppVideo::QueueFamilyIndices queue_indices;
		vk::Queue graphics_queue;
		vk::Queue present_queue;
		vk::UniqueSwapchainKHR swapchain;
		std::vector<vk::Image> swapchain_imgs;
		vk::Format swapchain_img_fmt;
		vk::Extent2D swapchain_img_size;

		std::vector<const char*> dev_exts = {
			"VK_KHR_swapchain",
		};
};
