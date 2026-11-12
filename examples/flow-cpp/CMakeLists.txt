# Copyright © 2026 陕西禾又科技有限公司
# SPDX-License-Identifier: GPL-3.0-only

cmake_minimum_required(VERSION 3.21)
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(SlintForgeFlowExample LANGUAGES CXX)
endif()
set(CMAKE_CXX_STANDARD 20)
set(SLINT_STYLE fluent CACHE STRING "Slint style")
if(NOT TARGET SlintForge::Sdk)
    find_package(SlintForge CONFIG REQUIRED)
endif()
add_executable(slintforge_flow_example main.cpp)
target_link_libraries(slintforge_flow_example PRIVATE SlintForge::Sdk)
if(MSVC)
    target_compile_options(slintforge_flow_example PRIVATE /utf-8 /UNDEBUG)
endif()
slintforge_target_sources(slintforge_flow_example ../ui/flow.slint)
configure_file(../ui/flow-qa.json ${CMAKE_CURRENT_BINARY_DIR}/flow-qa.json COPYONLY)
