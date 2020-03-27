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
		void init_vk();
		void main_loop();
		void deinit_vk();
};
