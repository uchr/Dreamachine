find_package(Qt5Core REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(Corrade REQUIRED Main)
find_package(FBXSDK REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(CLI11 CONFIG REQUIRED)
find_package(Magnum REQUIRED
    GL
    MeshTools
    Primitives
    SceneGraph
    Trade
    Shaders
)

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

if(MAGNUM_TARGET_GLES AND NOT MAGNUM_TARGET_DESKTOP_GLES)
    find_package(Magnum REQUIRED EglContext)
elseif(CORRADE_TARGET_WINDOWS)
    find_package(Magnum REQUIRED WglContext)
elseif(CORRADE_TARGET_APPLE)
    find_package(Magnum REQUIRED CglContext)
elseif(CORRADE_TARGET_UNIX)
    find_package(Magnum REQUIRED GlxContext)
else()
    message(FATAL_ERROR "Magnum context creation is not supported on this platform")
endif()

find_library(DIRECTXTEX_LIB
    NAMES DirectXTex
    PATHS ${LIB_PATH}/Release
)
find_library(DIRECTXTEX_LIB_DEBUG
    NAMES DirectXTex
    PATHS ${LIB_PATH}/Debug
)

set_directory_properties(PROPERTIES CORRADE_USE_PEDANTIC_FLAGS ON)

add_executable(DreamfallTLJViewer
    BundleListWindow.cpp
    InputManager.cpp
    main.cpp
    MainWindow.cpp
    MeshExporter.cpp
    TimeManager.cpp
    View.cpp
    ViewScene.cpp
)

if (FBXSDK_FOUND)
  include_directories(${FBXSDK_INCLUDE_DIRS})
  target_link_libraries(DreamfallTLJViewer PRIVATE optimized ${FBXSDK_LIBRARIES})
  target_link_libraries(DreamfallTLJViewer PRIVATE debug ${FBXSDK_LIBRARIES_DEBUG})
endif (FBXSDK_FOUND)

set_target_properties(DreamfallTLJViewer
    PROPERTIES
    CMAKE_AUTOMOC ON
)

target_link_libraries(DreamfallTLJViewer PRIVATE
    Corrade::Main
    Magnum::GLContext
    Magnum::GL
    Magnum::Magnum
    Magnum::SceneGraph
    Magnum::Shaders
    Magnum::MeshTools
    Magnum::Trade
    Magnum::Primitives
    Qt5::Core
    Qt5::Widgets
    fmt::fmt
    spdlog::spdlog
    CLI11::CLI11
    parser
    "d3d11.lib"
)

target_link_libraries(DreamfallTLJViewer PRIVATE optimized ${DIRECTXTEX_LIB})
target_link_libraries(DreamfallTLJViewer PRIVATE debug ${DIRECTXTEX_LIB_DEBUG})

add_custom_command(TARGET DreamfallTLJViewer POST_BUILD 
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different
                   ${FBXSDK_DLL}
                   $<TARGET_FILE_DIR:DreamfallTLJViewer>)

# Make the executable a default target to build & run in Visual Studio
set_property(DIRECTORY ${PROJECT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT DreamfallTLJViewer)
