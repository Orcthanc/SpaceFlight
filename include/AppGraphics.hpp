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

#include <glm/glm.hpp>

namespace SpaceAppVideo {
	constexpr int MAX_FRAMES_IN_FLIGHT{ 2 };

	/**
	 *	Struct to save queue family indices
	 */
	struct QueueFamilyIndices {
		bool complete();

		std::optional<uint32_t> graphics;
		std::optional<uint32_t> present;
	};

	struct SwapchainDetails {
		SwapchainDetails() = default;
		SwapchainDetails( vk::PhysicalDevice phys_dev, vk::UniqueSurfaceKHR& surface );

		vk::SurfaceCapabilitiesKHR capabilities;
		std::vector<vk::SurfaceFormatKHR> formats;
		std::vector<vk::PresentModeKHR> present_modes;
	};

	struct Vertex {
		glm::vec3 pos;
		glm::vec3 col;

		constexpr static vk::VertexInputBindingDescription getBindingDesc(){
			return { 0, sizeof( Vertex ), vk::VertexInputRate::eVertex };
		}

		constexpr static std::array<vk::VertexInputAttributeDescription, 2> getAttribDescs(){
			return {
				vk::VertexInputAttributeDescription( 0, 0, vk::Format::eR32G32B32Sfloat, offsetof( Vertex, pos )),
				vk::VertexInputAttributeDescription( 1, 0, vk::Format::eR32G32B32Sfloat, offsetof( Vertex, col ))
			};
		}
	};

}
