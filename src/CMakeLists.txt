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

add_executable(Dreamachine
    main.cpp
    MainWindow.cpp
    GLView.cpp
    InputManager.cpp
    TextureParser.cpp
    SceneParser.cpp
    SceneNode.cpp
    MeshExporter.cpp
    BundleHeader.cpp
    Utils.cpp
    BUNParser.cpp
    PAKParser.cpp
    BinReader.cpp
    SharkNode.cpp
    SharkParser.cpp
)

if (FBXSDK_FOUND)
  include_directories(${FBXSDK_INCLUDE_DIRS})
  target_link_libraries(Dreamachine PRIVATE optimized ${FBXSDK_LIBRARIES})
  target_link_libraries(Dreamachine PRIVATE debug ${FBXSDK_LIBRARIES_DEBUG})
endif (FBXSDK_FOUND)

target_link_libraries(Dreamachine PRIVATE
    Corrade::Main
    Magnum::GLContext
    Magnum::GL
    Magnum::Magnum
    Magnum::SceneGraph
    Magnum::Shaders
    Magnum::MeshTools
    Magnum::Trade
    Magnum::Primitives
    Qt5::Widgets
    fmt::fmt
    spdlog::spdlog
    cppmmf
    CLI11::CLI11
    "d3d11.lib"
)

target_link_libraries(Dreamachine PRIVATE optimized ${DIRECTXTEX_LIB})
target_link_libraries(Dreamachine PRIVATE debug ${DIRECTXTEX_LIB_DEBUG})

# Make the executable a default target to build & run in Visual Studio
set_property(DIRECTORY ${PROJECT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Dreamachine)
