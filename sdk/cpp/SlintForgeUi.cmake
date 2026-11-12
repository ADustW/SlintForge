# Copyright © 2026 陕西禾又科技有限公司
# SPDX-License-Identifier: GPL-3.0-only

# SlintForge 的两端共享样式编译入口，路径同时支持源码树和安装 SDK。
function(slintforge_target_sources target)
    get_property(forge_embed_resources_set TARGET ${target} PROPERTY SLINT_EMBED_RESOURCES SET)
    if(NOT forge_embed_resources_set)
        set_property(TARGET ${target} PROPERTY SLINT_EMBED_RESOURCES embed-files)
    endif()
    cmake_parse_arguments(FORGE "" "COMPILATION_UNITS" "LIBRARY_PATHS" ${ARGN})
    if(NOT DEFINED FORGE_COMPILATION_UNITS)
        set(FORGE_COMPILATION_UNITS 1)
    endif()
    get_filename_component(forge_ui_root "${SLINTFORGE_UI_LIBRARY}" DIRECTORY)
    set(forge_style "${forge_ui_root}/styles/slintforge-winui3")
    set(forge_includes -I "${forge_ui_root}/styles" -I "${forge_style}"
        -I "${forge_style}/upstream/fluent" -I "${forge_style}/upstream/common")
    set(forge_libraries -L "slintforge=${SLINTFORGE_UI_LIBRARY}")
    foreach(forge_library IN LISTS FORGE_LIBRARY_PATHS)
        if(NOT forge_library MATCHES "^slintforge=")
            list(APPEND forge_libraries -L "${forge_library}")
        endif()
    endforeach()
    foreach(forge_source IN LISTS FORGE_UNPARSED_ARGUMENTS)
        get_filename_component(forge_name "${forge_source}" NAME_WE)
        get_filename_component(forge_absolute "${forge_source}" REALPATH BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
        set(forge_cpp_files "")
        set(forge_cpp_arguments "")
        if(FORGE_COMPILATION_UNITS GREATER 0)
            foreach(forge_unit RANGE 1 ${FORGE_COMPILATION_UNITS})
                set(forge_cpp "${CMAKE_CURRENT_BINARY_DIR}/slint_generated_${forge_name}_${forge_unit}.cpp")
                list(APPEND forge_cpp_files "${forge_cpp}")
                list(APPEND forge_cpp_arguments "--cpp-file=${forge_cpp}")
            endforeach()
        endif()
        set(forge_translations "$<TARGET_PROPERTY:${target},SLINT_BUNDLE_TRANSLATIONS>")
        set(forge_no_context "$<BOOL:$<TARGET_PROPERTY:${target},SLINT_NO_DEFAULT_TRANSLATION_CONTEXT>>")
        add_custom_command(
            OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${forge_name}.h" ${forge_cpp_files}
            COMMAND ${CMAKE_COMMAND} -E env SLINT_ENABLE_EXPERIMENTAL_FEATURES=1
                $<TARGET_FILE:Slint::slint-compiler> "${forge_absolute}" -f cpp
                -o "${CMAKE_CURRENT_BINARY_DIR}/${forge_name}.h"
                --depfile "${CMAKE_CURRENT_BINARY_DIR}/${forge_name}.d"
                --style slintforge-winui3 "--embed-resources=$<TARGET_PROPERTY:${target},SLINT_EMBED_RESOURCES>"
                "--translation-domain=${target}"
                "$<$<BOOL:${forge_translations}>:--bundle-translations=${forge_translations}>"
                "$<${forge_no_context}:--no-default-translation-context>"
                ${forge_includes} ${forge_libraries} ${forge_cpp_arguments}
            DEPENDS Slint::slint-compiler "${forge_absolute}"
            DEPFILE "${CMAKE_CURRENT_BINARY_DIR}/${forge_name}.d"
            COMMENT "Generating ${forge_name}.h with SlintForge WinUI 3"
            VERBATIM COMMAND_EXPAND_LISTS)
        target_sources(${target} PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/${forge_name}.h" ${forge_cpp_files})
    endforeach()
    target_include_directories(${target} PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>")
    slintforge_deploy_runtime(${target})
endfunction()

# 原生消费者在构建目录中即可启动，不依赖开发机 PATH。
function(slintforge_deploy_runtime target)
    if(WIN32)
        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_RUNTIME_DLLS:${target}> $<TARGET_FILE_DIR:${target}>
            COMMAND_EXPAND_LISTS VERBATIM)
    endif()
endfunction()
