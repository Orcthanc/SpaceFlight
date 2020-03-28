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

		GLFWwindow* window;
		vk::UniqueInstance instance;
};
