set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
    ${PROJECT_SOURCE_DIR}/ui/icons/catalog.tsv)
file(READ ${PROJECT_SOURCE_DIR}/ui/icons/catalog.tsv FORGE_ICON_CATALOG_TEXT)
configure_file(ForgeIconCatalogData.hpp.in ${CMAKE_CURRENT_BINARY_DIR}/ForgeIconCatalogData.hpp @ONLY)
add_library(slintforge_sdk STATIC src/ForgeSdk.cpp src/ForgeIconCatalog.cpp src/ForgeWindowCorners.cpp src/ForgeStaggeredLayout.cpp src/ForgeCapture.cpp src/ForgeWidgets.cpp src/ForgeCharts.cpp src/ForgeGrids.cpp ${PROJECT_SOURCE_DIR}/third_party/qrcodegen/cpp/qrcodegen.cpp)
add_library(SlintForge::Sdk ALIAS slintforge_sdk)
target_sources(slintforge_sdk PRIVATE ../common/ForgePlotCore.cpp src/ForgePlot.cpp)
target_sources(slintforge_sdk PRIVATE ../common/ForgeFlowCore.cpp)
target_include_directories(slintforge_sdk PRIVATE ${PROJECT_SOURCE_DIR}/third_party/nlohmann)
target_include_directories(slintforge_sdk PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/third_party>)
install(FILES ${PROJECT_SOURCE_DIR}/third_party/nlohmann/json.hpp DESTINATION include/nlohmann)
install(FILES ../common/ForgeFlowCore.h DESTINATION include)
install(FILES ${PROJECT_SOURCE_DIR}/third_party/nlohmann/LICENSE.MIT ${PROJECT_SOURCE_DIR}/third_party/nlohmann/SOURCE.md DESTINATION share/slintforge/licenses/nlohmann)
target_include_directories(slintforge_sdk PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../common>)
install(FILES ../common/ForgePlotCore.h DESTINATION include)
set_target_properties(slintforge_sdk PROPERTIES EXPORT_NAME Sdk DEBUG_POSTFIX d)
target_compile_features(slintforge_sdk PUBLIC cxx_std_20)
target_link_libraries(slintforge_sdk PUBLIC Slint::Slint)
if(WIN32)
    target_link_libraries(slintforge_sdk PUBLIC dwmapi windowscodecs ole32 gdi32)
endif()
target_include_directories(slintforge_sdk PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)
target_include_directories(slintforge_sdk PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(slintforge_sdk PRIVATE ${PROJECT_SOURCE_DIR}/third_party/qrcodegen/cpp)

function(slintforge_target_sources target source)
    get_property(forge_embed_resources_set TARGET ${target} PROPERTY SLINT_EMBED_RESOURCES SET)
    if(NOT forge_embed_resources_set)
        set_property(TARGET ${target} PROPERTY SLINT_EMBED_RESOURCES embed-files)
    endif()
    slint_target_sources(${target} ${source}
        LIBRARY_PATHS "slintforge=${CMAKE_CURRENT_FUNCTION_LIST_DIR}/../../ui/slintforge.slint")
endfunction()

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
install(TARGETS slintforge_sdk EXPORT SlintForgeTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(DIRECTORY ${PROJECT_SOURCE_DIR}/ui/
    DESTINATION ${CMAKE_INSTALL_DATADIR}/slintforge/ui
    FILES_MATCHING PATTERN "*.slint" PATTERN "*.ttf" PATTERN "*.json" PATTERN "*.tsv" PATTERN "LICENSE-*.txt" PATTERN "SOURCE.md")
install(FILES ${PROJECT_SOURCE_DIR}/third_party/qrcodegen/LICENSE.txt
    ${PROJECT_SOURCE_DIR}/third_party/qrcodegen/SOURCE.md
    DESTINATION ${CMAKE_INSTALL_DATADIR}/slintforge/licenses/qrcodegen)
install(EXPORT SlintForgeTargets FILE SlintForgeTargets.cmake NAMESPACE SlintForge::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/SlintForge)
configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/SlintForgeConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/SlintForgeConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/SlintForge)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/SlintForgeConfigVersion.cmake
    VERSION ${PROJECT_VERSION} COMPATIBILITY SameMajorVersion)
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/SlintForgeConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/SlintForgeConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/SlintForge)
