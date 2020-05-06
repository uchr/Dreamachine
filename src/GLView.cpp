#include "GLView.h"

#include "Scene.h"
#include "SceneIndex.h"
#include "SceneNode.h"

#include <Corrade/Containers/Optional.h>
#include <Corrade/PluginManager/Manager.h>
#include <Corrade/Utility/Arguments.h>
#include <Corrade/Utility/DebugStl.h>
#include <Magnum/ImageView.h>
#include <Magnum/Mesh.h>
#include <magnum/Math/Vector2.h>
#include <magnum/Math/Vector3.h>
#include <Magnum/PixelFormat.h>
#include <Magnum/GL/DefaultFramebuffer.h>
#include <Magnum/GL/Mesh.h>
#include <Magnum/GL/Renderer.h>
#include <Magnum/GL/Framebuffer.h>
#include <Magnum/GL/Texture.h>
#include <Magnum/GL/TextureFormat.h>
#include <Magnum/MeshTools/Compile.h>
#include <Magnum/Trade/AbstractImporter.h>
#include <Magnum/Trade/ImageData.h>
#include <Magnum/Trade/MeshData3D.h>
#include <Magnum/Trade/MeshObjectData3D.h>
#include <Magnum/Trade/PhongMaterialData.h>
#include <Magnum/Trade/SceneData.h>
#include <Magnum/Trade/TextureData.h>

/* If your application is using anything from QtGui, you might get warnings
   about GLEW and errors regarding GLclampf. If that's the case, uncomment the
   following and place it as early as possible (but again *after* including
   Magnum GL headers) */
#if 0
typedef GLfloat GLclampf;
#undef __glew_h__ /* shh, Qt, shh */
#undef __GLEW_H__
#include <QtGui/qopenglfunctions.h>
#endif

using namespace Magnum;
using namespace Math::Literals;

void fillMeshData(const parser::SceneNode& node, std::vector<Trade::MeshData3D>& exportedMeshData)
{
    /*aiNode* aNode = new aiNode();

    float scale = 1.0f / node.scale;
    magnum::Vector3 position(node.position.x, node.position.y, node.position.z);
    magnum::Quaternion rotation(magnum::Vector3(node.rotation.x, node.rotation.y, node.rotation.z), node.rotation.w);
    if (rotation.length() < 0.01f)
        rotation = magnum::Quaternion<float>();
    else {
        scale *= rotation.dot();
        rotation = rotation.normalized();
    }
    magnum::Matrix4<float> rotationMatrix = magnum::Matrix4<float>::from(rotation.toMatrix(), {});
    magnum::Matrix4<float> scalingMatrix = magnum::Matrix4<float>::scaling(magnum::Vector3(scale));
    magnum::Matrix4<float> translationMatrix = magnum::Matrix4<float>::translation(position);
    magnum::Matrix4<float> transformation = scalingMatrix * rotationMatrix * translationMatrix;

    aNode->mTransformation = aiMatrix4x4(transformation[0][0], transformation[1][0], transformation[2][0], transformation[3][0],
                                         transformation[0][1], transformation[1][1], transformation[2][1], transformation[3][1],
                                         transformation[0][2], transformation[1][2], transformation[2][2], transformation[3][2],
                                         transformation[0][3], transformation[1][3], transformation[2][3], transformation[3][3]);
    */

    if (node.mesh.has_value()) {
        auto& mesh = *node.mesh;

        std::vector<std::vector<Vector3>> positions(1);
        std::vector<std::vector<Vector3>> normals(1);
        std::vector<std::vector<Vector2>> textureCoords2D(1);
        std::vector<UnsignedInt> indices(mesh.indices.begin(), mesh.indices.end());

        for (size_t i = 0; i < mesh.vertices.size(); ++i) {
            const auto& v = mesh.vertices[i];
            positions[0].emplace_back(v.x, v.y, v.z);
        }

        for (size_t i = 0; i < mesh.vertices.size(); ++i) {
            const auto& n = mesh.normals[i];
            normals[0].emplace_back(n.x, n.y, n.z);
        }

        for (size_t i = 0; i < mesh.vertices.size(); ++i) {
            const auto& uv = mesh.uvs[i];
            textureCoords2D[0].emplace_back(uv.x, uv.y);
        }

        Trade::MeshData3D meshData(MeshPrimitive::Triangles,
                                   indices,
                                   positions,
                                   normals,
                                   textureCoords2D,
                                   {});
        exportedMeshData.push_back(std::move(meshData));
    }

    if (!node.children.empty()) {
        for (size_t i = 0; i < node.children.size(); ++i)
            fillMeshData(node.children[i], exportedMeshData);
    }
}


GLView::GLView(Platform::GLContext& context, QWidget* parent, const parser::SceneIndex& sceneIndex)
    : QOpenGLWidget(parent)
    , m_context(context)
    , m_sceneIndex(sceneIndex)
{
}

void GLView::initializeGL() {
    m_context.create();

    GL::Renderer::enable(GL::Renderer::Feature::DepthTest);
    GL::Renderer::enable(GL::Renderer::Feature::FaceCulling);

    m_cameraObject
        .setParent(&m_scene)
        .translate(Vector3::zAxis(5.0f));
    (*(m_camera = new SceneGraph::Camera3D{m_cameraObject}))
        .setAspectRatioPolicy(SceneGraph::AspectRatioPolicy::Extend)
        .setProjectionMatrix(Matrix4::perspectiveProjection(35.0_degf, 1.0f, 0.01f, 1000.0f))
        .setViewport(GL::defaultFramebuffer.viewport().size());

    m_manipulator.setParent(&m_scene);

    m_coloredShader = std::make_unique<Shaders::Phong>();
    m_coloredShader
        ->setAmbientColor(0x111111_rgbf)
        .setSpecularColor(0xffffff_rgbf)
        .setShininess(80.0f);
    m_texturedShader = std::make_unique<Shaders::Phong>();
    m_texturedShader
        ->setAmbientColor(0x111111_rgbf)
        .setSpecularColor(0x111111_rgbf)
        .setShininess(80.0f);

    parser::Scene scene(m_sceneIndex.sirs[4].sirPath, m_sceneIndex.bundleName);
    std::vector<Trade::MeshData3D> exportedMeshData;
    assert(scene.sceneRoot.has_value());
    fillMeshData(*scene.sceneRoot, exportedMeshData);

    m_shader = std::make_unique<Shaders::Phong>();

    m_meshes = Containers::Array<Containers::Optional<GL::Mesh>>{exportedMeshData.size()};
    for (size_t i = 0; i < exportedMeshData.size(); ++i) {
        if(!exportedMeshData[i].hasNormals() || exportedMeshData[i].primitive() != MeshPrimitive::Triangles) {
            Warning{} << "Cannot load the mesh, skipping";
            continue;
        }
        m_meshes[i] = MeshTools::compile(exportedMeshData[i]);
    }

    //m_mesh = std::make_unique<GL::Mesh>(m_meshes[0]);

    m_transformation = Matrix4::rotationX(30.0_degf) * Matrix4::rotationY(40.0_degf);
    const float aspectRatio = Vector2(width(), height()).aspectRatio();
    m_projection = Matrix4::perspectiveProjection(35.0_degf, aspectRatio, 0.01f, 100.0f) * Matrix4::translation(Vector3::zAxis(-10.0f));
    m_color = Color3::fromHsv({35.0_degf, 1.0f, 1.0f});

    /* Clean up Magnum state when giving control back to Qt */
    GL::Context::current().resetState(GL::Context::State::EnterExternal);
}

void GLView::paintGL() {
    /* Reset state to avoid Qt affecting Magnum */
    GL::Context::current().resetState(GL::Context::State::ExitExternal);

    /* Using framebuffer provided by Qt as default framebuffer */
    auto qtDefaultFramebuffer = GL::Framebuffer::wrap(defaultFramebufferObject(), {{}, {width(), height()}});

    qtDefaultFramebuffer.clear(GL::FramebufferClear::Color | GL::FramebufferClear::Depth);

    m_shader->setLightPosition({7.0f, 5.0f, 2.5f})
             .setLightColor(Color3{1.0f})
             .setDiffuseColor(m_color)
             .setAmbientColor(Color3::fromHsv({m_color.hue(), 1.0f, 0.3f}))
             .setTransformationMatrix(m_transformation)
             .setNormalMatrix(m_transformation.normalMatrix())
             .setProjectionMatrix(m_projection);
    m_meshes[0]->draw(*m_shader);

    /* Clean up Magnum state when giving control back to Qt */
    GL::Context::current().resetState(GL::Context::State::EnterExternal);
}

void GLView::resizeGL(int w, int h)
{
    const float aspectRatio = Vector2(w, h).aspectRatio();
    m_projection = Matrix4::perspectiveProjection(35.0_degf, aspectRatio, 0.01f, 100.0f) * Matrix4::translation(Vector3::zAxis(-10.0f));
}

void GLView::mousePressEvent(QMouseEvent* event) {
    if (!event->buttons().testFlag(Qt::LeftButton))
        return;

    m_previousCursorPosition = Vector2(event->localPos().x(), event->localPos().y());
}

void GLView::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton)
        return;

    m_color = Color3::fromHsv({m_color.hue() + 50.0_degf, 1.0f, 1.0f});

    update();
}

void GLView::mouseMoveEvent(QMouseEvent* event) {
    if (!event->buttons().testFlag(Qt::LeftButton))
        return;

    Vector2 currrentPos(event->localPos().x(), event->localPos().y());
    Vector2 delta = 3.0f * (currrentPos - m_previousCursorPosition) / Vector2(width(), height());

    m_previousCursorPosition = currrentPos;
    m_transformation = Matrix4::rotationX(Rad{delta.y()}) * m_transformation * Matrix4::rotationY(Rad{delta.x()});

    update();
}