file( GLOB_RECURSE ${PROJECT_NAME}_SOURCES "*.cpp" )

add_executable( ${PROJECT_NAME} ${${PROJECT_NAME}_SOURCES} )

target_include_directories( ${PROJECT_NAME} PUBLIC "../include" )
target_link_libraries( ${PROJECT_NAME} PUBLIC ConfigParserLib Logger_Lib )
