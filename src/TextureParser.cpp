#include "TextureParser.h"
#include "GeometryData.h"
#include "BinReader.h"
#include "PAKParser.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
#define STBI_ONLY_PNG
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <Magnum/Magnum.h>
#include <Magnum/Math/Vector3.h>

#include <d3d11.h>
#include <DirectXTex.h>

#include <spdlog/spdlog.h>

#include <queue>
#include <locale>
#include <sstream>
#include <optional>

namespace parser
{
namespace
{

std::filesystem::path createAlphaTexture(const std::filesystem::path& texturePath, const std::filesystem::path& outputPath) {
    int x, y, component;
    unsigned char *data = stbi_load(texturePath.string().c_str(), &x, &y, &component, 0);
    for (int i = 0; i < x * y; ++i) {
        data[i * 4 + 0] = data[i * 4 + 3];
        data[i * 4 + 1] = data[i * 4 + 3];
        data[i * 4 + 2] = data[i * 4 + 3];
        data[i * 4 + 3] = 255;
    }

    stbi_write_png(outputPath.string().c_str(), x, y, component, data, 0);
    stbi_image_free(data);

    return outputPath;
}

// This is a workaround because almost any texture has an alpha channel, but not every material should be transparent.
std::optional<std::filesystem::path> exportAlphaTexture(const std::filesystem::path& path) {
    auto isPathContainSubstring = [&](const std::vector<std::string>& substrings) {
        auto substringIt = std::find_if(substrings.begin(), substrings.end(),
            [&](const std::string& substring) { return path.filename().string().find(substring) != std::string::npos; });
        return substringIt != substrings.end();
    };

    auto getAlphaTexturePath = [](const std::filesystem::path& diffuseTexturePath) {
        return diffuseTexturePath.parent_path() / (diffuseTexturePath.filename().replace_extension("").string() + "_alpha.png");
    };

    const std::vector<std::string> diffuseTransparentTextures = {
        "glow",
        "win_big",
        "japan_streets_background",
        "sun",
        "branches_winter",
        "jiva_corridor_glass",
        "jdr_gate"
    };

    const std::vector<std::string> nmlTransparentTextures = {
        "plant_ivy",
        "leaf",
        "jdr_flowers",
        "puddles",
        "casa_grass"
    };

    if (isPathContainSubstring(diffuseTransparentTextures)) {
        std::filesystem::path alphaOutputPath = getAlphaTexturePath(path);
        if (!std::filesystem::exists(alphaOutputPath))
            createAlphaTexture(path, alphaOutputPath);
        return alphaOutputPath;
    }
    
    if (isPathContainSubstring(nmlTransparentTextures) && path.string().find("nml") != std::string::npos) {
        std::string filename = path.filename().string();
        filename.replace(filename.begin() + filename.find("_nml"), filename.end(), "");
        std::filesystem::path alphaOutputPath = getAlphaTexturePath(path.parent_path() / filename);
        if (!std::filesystem::exists(alphaOutputPath))
            createAlphaTexture(path, alphaOutputPath);
        return alphaOutputPath;
    }

    return std::nullopt;
}

std::wstring widen(const std::string& str)
{
    std::wostringstream wstm ;
    const std::ctype<wchar_t>& ctfacet = std::use_facet<std::ctype<wchar_t>>(wstm.getloc()) ;
    for( size_t i = 0; i < str.size(); ++i)
        wstm << ctfacet.widen(str[i]);
    return wstm.str() ;
}

struct NMLImage
{
    std::vector<uint32_t> data;
    size_t sizeX, sizeY;
    NMLImage(size_t sizeX, size_t sizeY)
        : sizeX(sizeX)
        , sizeY(sizeY)
        , data(sizeX * sizeY)
    {
    }

    void set(int x, int y, Magnum::Vector3 color, uint32_t alpha) {
        uint32_t red = static_cast<uint32_t>((color.x() + 1.0f) * 127.5f);
        uint32_t green = static_cast<uint32_t>((color.y() + 1.0f) * 127.5f);
        uint32_t blue = static_cast<uint32_t>((color.z() + 1.0f) * 127.5f);
        data[x + y * sizeX] = (alpha << 24) + (blue << 16) + (green << 8) + red;
    }

    void set(int x, int y, uint32_t value) {
        data[x + y * sizeX] = value;
    }

    uint32_t get(int x, int y) {
        if (x < 0 || x >= sizeX || y < 0 || y >= sizeY)
            return 0x8080ff;
        return data[x + y * sizeX];
    }
};

struct Image16
{
    std::vector<uint16_t> data;
    int sizeX, sizeY;

    int size() const {
        return sizeX * sizeY;
    }

    int numberOfBytes() const {
        return sizeX * sizeY * 2;
    }

    float msb(int x, int y) {
        return (float)(data[x + y * sizeX] / 0x100) / 255.0f;
    }

    float lsb(int x, int y) {
        return (float)(data[x + y * sizeX] & 0xFF) / 255.0f;
    }

    float med(int x, int y) {
        return (msb(x, y)+lsb(x, y)) * 0.5f;
    }
    
    Image16(int sizeX, int sizeY)
        : sizeX(sizeX)
        , sizeY(sizeY)
        , data(sizeX * sizeY)
    {
    }

    void readLine(BinReader& binReader, int index, int line)
    {
        for (int i = 0; i < line; i++)
            data[index * line + i] = binReader.read<uint16_t>();
    }
};

uint32_t quadFilter(int x, int y, Image16 sub, int sizeX, int sizeY)
{
    if (sizeX == 1 || sizeY == 1)
        return static_cast<uint32_t>(255.0f * sub.med(x * sizeX / sub.sizeX, y * sizeY / sub.sizeY));
    
    float dx = static_cast<float>(x) * static_cast<float>(sub.sizeX - 1.0f) / static_cast<float>(sizeX - 1.0f);
    float dy = static_cast<float>(y) * static_cast<float>(sub.sizeY - 1.0f) / static_cast<float>(sizeY - 1.0f);
    float cx = dx - std::floor(dx);
    float cy = dy - std::floor(dy);
    float val = (1.0f - cx) * (1.0f - cy) * sub.med(static_cast<int>(std::floor(dx)), static_cast<int>(std::floor(dy)));
    val += (1.0f - cx) * cy * sub.med(static_cast<int>(std::floor(dx)), static_cast<int>(std::ceil(dy)));
    val += cx * (1.0f - cy) * sub.med(static_cast<int>(std::ceil(dx)), static_cast<int>(std::floor(dy)));
    val += cx * cy * sub.med(static_cast<int>(std::ceil(dx)), static_cast<int>(std::ceil(dy)));
    return static_cast<uint32_t>(255.0f * val);
}

NMLImage createNML(Image16& rgb, Image16& alpha)
{
    float aspect = static_cast<float>(rgb.sizeX) / rgb.sizeY;
    float scaleX = (aspect < 1) ? 1 : aspect;
    float scaleY = (aspect < 1) ? (1.0f / aspect) : 1;

    NMLImage nmlImage(rgb.sizeX, rgb.sizeY);
    
    for (int i = 1; i < rgb.sizeX - 1; i++)
    {
        for (int j = 1; j < rgb.sizeY - 1; j++)
        {
            Magnum::Vector3 dirI(2, 0, rgb.msb(i + 1, j) - rgb.msb(i - 1, j));
            Magnum::Vector3 dirJ(0, 2, rgb.msb(i, j + 1) - rgb.msb(i, j - 1));
            Magnum::Vector3 normal = Magnum::Math::cross(dirI, dirJ);

            normal.x() *= scaleX;
            normal.y() *= scaleY;
            normal = normal.normalized();
            uint32_t a = quadFilter(i,j, alpha, rgb.sizeX, rgb.sizeY);
            nmlImage.set(i, j, normal, a);
        }
    }

    for (int i = 0; i < rgb.sizeX; i++)
    {
        nmlImage.set(i, 0, nmlImage.get(i, 1));
        nmlImage.set(i, rgb.sizeY - 1, nmlImage.get(i, rgb.sizeY - 2));
    }
    for (int j = 0; j < rgb.sizeY; j++)
    {
        nmlImage.set(0, j, nmlImage.get(1, j));
        nmlImage.set(rgb.sizeX - 1, j, nmlImage.get(rgb.sizeX - 2, j));
    }

    return nmlImage;
}

bool loadNML(const std::filesystem::path& path, const std::filesystem::path& exportPath) {
    BinReaderMmap binReader(path);

    const std::vector<byte> magic = { 0x53, 0x54, 0x46, 0x55, 0x34, 0x9a, 0x22, 0x44, 0, 0, 0, 0 };
    for (int i = 0; i < magic.size(); i++) {
        if (binReader.readByte() != magic[i])
        {
            spdlog::warn("Loading nml failed (STFU magic wrong) : {} ", path.string());
            return false;
        }
    }

    if (binReader.read<int32_t>() != 1)
    {
        spdlog::warn("Loading nml failed (Version wrong) : {} ", path.string());
        return false;
    }

    int msizex = binReader.read<int32_t>();
    int msizey = binReader.read<int32_t>();
    int mipLevels = binReader.read<int32_t>();
    int sth = binReader.read<int32_t>();

    std::queue<Image16> alphaQueue;
    for (int i = 0; i < mipLevels; i++)
    {
        int len = binReader.read<int32_t>();
        Image16 rgb(binReader.read<int32_t>(), binReader.read<int32_t>());
        int aSizeX = binReader.read<int32_t>();
        Image16 alpha(aSizeX, (len - rgb.numberOfBytes()) / 2 / aSizeX);

        for (int lineIndex = 0; lineIndex < alpha.sizeY; lineIndex++)
        {
            alpha.readLine(binReader, lineIndex, alpha.sizeX);
            rgb.readLine(binReader, lineIndex, rgb.size() / alpha.sizeY);
        }
        alphaQueue.push(alpha);
        while (alphaQueue.front().sizeX > rgb.sizeX || alphaQueue.front().sizeY > rgb.sizeY) {
            alphaQueue.pop();
            if (alphaQueue.empty())
                return false; // TODO: Investigate problem with rotated texture
        }

        NMLImage nmlImage = createNML(rgb, alpha);
        int result = stbi_write_png(exportPath.string().c_str(), nmlImage.sizeX, nmlImage.sizeY, 4, reinterpret_cast<const char*>(nmlImage.data.data()), 0);
        if (result > 0)
            return true;
    }

    return false;
}
}

void parseTextures(MeshPart& meshPart, const std::vector<std::filesystem::path>& texturesPath, const std::filesystem::path& exportFolder) {
    std::vector<std::filesystem::path> texturePaths;
    for (int i = 0; i < texturesPath.size(); ++i) {
        if (texturesPath[i] == "")
            continue;

        PAKParser::instance().tryExtract(texturesPath[i]);

        std::filesystem::path exportPath = exportFolder / texturesPath[i];
        bool isLoaded = std::filesystem::exists(exportPath);
        if (!isLoaded) {
            std::filesystem::create_directories(exportPath.parent_path());
            DirectX::ScratchImage imageData;
            HRESULT hr = DirectX::LoadFromWICFile(widen(texturesPath[i].string()).c_str(), DirectX::WIC_FLAGS_NONE, nullptr, imageData);
            if (!SUCCEEDED(hr))
                hr = DirectX::LoadFromDDSFile(widen(texturesPath[i].string()).c_str(), DirectX::DDS_FLAGS_NO_LEGACY_EXPANSION, nullptr, imageData);

            if (SUCCEEDED(hr)) {
                isLoaded = true;
                hr = DirectX::SaveToWICFile(*imageData.GetImage(0, 0, 0), DirectX::WIC_FLAGS_NONE, DirectX::GetWICCodec(DirectX::WIC_CODEC_PNG), widen(exportPath.string()).c_str());
            }

            if (!isLoaded)
                isLoaded = loadNML(texturesPath[i], exportPath);
        }

        if (isLoaded) {
            texturePaths.emplace_back(exportPath);
            auto alphaTexture = exportAlphaTexture(exportPath);
            if (alphaTexture.has_value())
                meshPart.alphaTexture = alphaTexture;
        }
        else {
            spdlog::error("Textured {} not exported", texturesPath[i].string());
        }
    }

    meshPart.textures = texturePaths;
}

}