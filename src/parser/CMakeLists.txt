find_package(Magnum REQUIRED)
find_package(spdlog CONFIG REQUIRED)

add_library(parser
    BinReader.cpp
    BundleHeader.cpp
    BundleParser.cpp
    PAKParser.cpp
    SceneNode.cpp
    SceneParser.cpp
    SharkNode.cpp
    SharkParser.cpp
    TextureParser.cpp
    Utils.cpp
)

target_link_libraries(parser PRIVATE
    Magnum::Magnum
    spdlog::spdlog
    cppmmf
)

target_include_directories(parser PUBLIC "..")
