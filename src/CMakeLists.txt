file( GLOB_RECURSE ${PROJECT_NAME}_SOURCES "*.cpp" )

add_executable( ${PROJECT_NAME} ${${PROJECT_NAME}_SOURCES} )

find_package( glfw3 3.2 REQUIRED )
find_package( Vulkan REQUIRED )

if( COLOR_CONSOLE )
	target_compile_definitions( ${PROJECT_NAME} PRIVATE COLOR_CONSOLE=1 )
endif( COLOR_CONSOLE )

target_include_directories( ${PROJECT_NAME} PUBLIC "../include" )
target_link_libraries( ${PROJECT_NAME} PUBLIC ConfigParserLib Logger_Lib Vulkan::Vulkan glfw )
