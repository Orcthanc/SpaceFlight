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

#include <optional>

namespace SpaceAppVideo {
	/**
	 *	Struct to save queue family indices
	 */
	struct QueueFamilyIndices {
		bool complete();

		std::optional<uint32_t> graphics;
	};

}
