set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/../data/navigation.txt
    ${CMAKE_CURRENT_SOURCE_DIR}/../data/component-status.txt)
file(READ ${CMAKE_CURRENT_SOURCE_DIR}/../data/navigation.txt FORGE_GALLERY_NAV_TEXT)
file(READ ${CMAKE_CURRENT_SOURCE_DIR}/../data/component-status.txt FORGE_GALLERY_STATUS_TEXT)
configure_file(ForgeGalleryNavigationData.hpp.in
    ${CMAKE_CURRENT_BINARY_DIR}/ForgeGalleryNavigationData.hpp @ONLY)

add_executable(slintforge_gallery_cpp main.cpp ForgeGalleryNavigation.cpp ForgeGallerySettings.cpp ForgeGalleryCapture.cpp ForgeGalleryCaptureScene.cpp ForgeGalleryDemos.cpp ForgeGalleryGrids.cpp ForgeGalleryPlot.cpp ForgeGalleryComponentQa.cpp)
if(MSVC)
    # 发布包中的 smoke 验证也保留断言。
    target_compile_options(slintforge_gallery_cpp PRIVATE /bigobj /UNDEBUG)
    # 与 Rust Gallery 保持相同栈预算，容纳固定页面和扩展示例的属性刷新与焦点恢复。
    target_link_options(slintforge_gallery_cpp PRIVATE /STACK:8388608)
endif()
if(WIN32)
    set(FORGE_GALLERY_ICON_RC "${CMAKE_CURRENT_SOURCE_DIR}/../ui/assets/brand/slintforge-gallery.ico")
    configure_file(gallery.rc.in ${CMAKE_CURRENT_BINARY_DIR}/gallery.rc @ONLY)
    target_sources(slintforge_gallery_cpp PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/gallery.rc)
endif()
set_property(TARGET slintforge_gallery_cpp PROPERTY SLINT_BUNDLE_TRANSLATIONS "${CMAKE_CURRENT_SOURCE_DIR}/../i18n")
set_property(TARGET slintforge_gallery_cpp PROPERTY SLINT_NO_DEFAULT_TRANSLATION_CONTEXT TRUE)
# 运行包在开发目录之外启动，图片和字体必须嵌入，避免保留构建机路径。
set_property(TARGET slintforge_gallery_cpp PROPERTY SLINT_EMBED_RESOURCES embed-files)
target_include_directories(slintforge_gallery_cpp PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(slintforge_gallery_cpp PRIVATE Slint::Slint SlintForge::Sdk)
slint_target_sources(slintforge_gallery_cpp ../ui/gallery.slint COMPILATION_UNITS 4
    LIBRARY_PATHS "slintforge=${PROJECT_SOURCE_DIR}/ui/slintforge.slint")
