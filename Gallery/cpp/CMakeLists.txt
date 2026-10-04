set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/../data/navigation.txt
    ${CMAKE_CURRENT_SOURCE_DIR}/../data/component-status.txt)
file(READ ${CMAKE_CURRENT_SOURCE_DIR}/../data/navigation.txt FORGE_GALLERY_NAV_TEXT)
file(READ ${CMAKE_CURRENT_SOURCE_DIR}/../data/component-status.txt FORGE_GALLERY_STATUS_TEXT)
configure_file(ForgeGalleryNavigationData.hpp.in
    ${CMAKE_CURRENT_BINARY_DIR}/ForgeGalleryNavigationData.hpp @ONLY)

add_executable(slintforge_gallery_cpp main.cpp ForgeGalleryNavigation.cpp ForgeGallerySettings.cpp ForgeGalleryCapture.cpp ForgeGalleryCaptureScene.cpp ForgeGalleryDemos.cpp ForgeGalleryGrids.cpp ForgeGalleryPlot.cpp ForgeGalleryComponentQa.cpp)
if(MSVC)
    target_compile_options(slintforge_gallery_cpp PRIVATE /bigobj)
endif()
if(WIN32)
    set(FORGE_GALLERY_ICON_RC "${CMAKE_CURRENT_SOURCE_DIR}/../ui/assets/brand/slintforge-gallery.ico")
    configure_file(gallery.rc.in ${CMAKE_CURRENT_BINARY_DIR}/gallery.rc @ONLY)
    target_sources(slintforge_gallery_cpp PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/gallery.rc)
endif()
set_property(TARGET slintforge_gallery_cpp PROPERTY SLINT_BUNDLE_TRANSLATIONS "${CMAKE_CURRENT_SOURCE_DIR}/../i18n")
set_property(TARGET slintforge_gallery_cpp PROPERTY SLINT_NO_DEFAULT_TRANSLATION_CONTEXT TRUE)
target_include_directories(slintforge_gallery_cpp PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(slintforge_gallery_cpp PRIVATE Slint::Slint SlintForge::Sdk)
slint_target_sources(slintforge_gallery_cpp ../ui/gallery.slint COMPILATION_UNITS 4
    LIBRARY_PATHS "slintforge=${PROJECT_SOURCE_DIR}/ui/slintforge.slint")
