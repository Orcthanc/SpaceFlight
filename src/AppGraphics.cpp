/*
 * =====================================================================================
 *
 *       Filename:  AppGraphics.cpp
 *
 *    Description:  Contains the implementations of things defined in the corresponding header
 *
 *        Version:  1.0
 *        Created:  03/28/2020 05:59:19 PM
 *       Revision:  none
 *
 *         Author:  Samuel Knoethig (), samuel@knoethig.net
 *
 * =====================================================================================
 */

#include "AppGraphics.hpp"

using namespace SpaceAppVideo;

bool QueueFamilyIndices::complete(){
	return graphics.has_value() && present.has_value();
}

SwapchainDetails::SwapchainDetails( vk::PhysicalDevice phys_dev, vk::UniqueSurfaceKHR& surf ){
	capabilities = phys_dev.getSurfaceCapabilitiesKHR( *surf );
	formats = phys_dev.getSurfaceFormatsKHR( *surf );
	present_modes = phys_dev.getSurfacePresentModesKHR( *surf );
}
