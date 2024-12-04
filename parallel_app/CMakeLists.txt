cmake_minimum_required( VERSION 3.29 )
set(CMAKE_CXX_STANDARD 17 )
set(CMAKE_CXX_STANDARD_REQUIRED True )

project(parallel_app)

# 3rd party includes
set(JSON_BuildTests OFF CACHE INTERNAL "")
add_subdirectory(3rd_party/nlohmann_json)

# Build parallel app
include_directories( ${CMAKE_CURRENT_LIST_DIR}/common/inc )
if( ${CMAKE_SYSTEM_NAME} STREQUAL "Windows" )
    include_directories( ${CMAKE_CURRENT_LIST_DIR}/platform/windows/inc )
elseif( ${CMAKE_SYSTEM_NAME} STREQUAL "Linux" )
    include_directories( ${CMAKE_CURRENT_LIST_DIR}/platform/linux/inc )
endif()

include( files.cmake )

link_directories(${CMAKE_ARCHIVE_OUTPUT_DIRECTORY} )

add_executable(${PROJECT_NAME} ${_PARALLEL_APP_SOURCEFILES}) 

if( ${CMAKE_SYSTEM_NAME} STREQUAL "Windows" )
    target_link_libraries(${PROJECT_NAME} PRIVATE ws2_32)
elseif( ${CMAKE_SYSTEM_NAME} STREQUAL "Linux" )

endif()

target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json)