/*
 * =====================================================================================
 *
 *       Filename:  Util.hpp
 *
 *    Description:  Contains useful includes, functions, globals, etc
 *
 *        Version:  1.0
 *        Created:  03/27/2020 09:07:49 PM
 *       Revision:  none
 *
 *         Author:  Samuel Knoethig (), samuel@knoethig.net
 *
 * =====================================================================================
 */
#pragma once

#include <vector>
#include <string>

#include "Logger.hpp"
#include "Parser.hpp"

// To be expanded
enum class LogChannel {
	Default,
	Video,
};

namespace LogLevel {
	// Numbers are subject to change
	enum LogLevel {
		Verbose = 0,
		Info = 1,
		Warning = 2,
		Error = 3,
		Critical = 4,
	};
}
