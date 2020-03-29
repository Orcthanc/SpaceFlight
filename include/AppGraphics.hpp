/*
 * =====================================================================================
 *
 *       Filename:  AppGraphics.hpp
 *
 *    Description:  Contains useful graphics stuff
 *
 *        Version:  1.0
 *        Created:  03/28/2020 05:43:47 PM
 *       Revision:  none
 *
 *         Author:  Samuel Knoethig (), samuel@knoethig.net
 *
 * =====================================================================================
 */
#pragma once

#include <vulkan/vulkan.hpp>
#include <optional>

namespace SpaceAppVideo {
	/**
	 *	Struct to save queue family indices
	 */
	struct QueueFamilyIndices {
		bool complete();

		std::optional<uint32_t> graphics;
		std::optional<uint32_t> present;
	};

	struct SwapchainDetails {
		SwapchainDetails( vk::PhysicalDevice phys_dev, vk::UniqueSurfaceKHR& surface );

		vk::SurfaceCapabilitiesKHR capabilities;
		std::vector<vk::SurfaceFormatKHR> formats;
		std::vector<vk::PresentModeKHR> present_modes;
	};

}
