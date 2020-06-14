#include "GLView.h"

#include <parser/SceneParser.h>
#include <parser/SceneIndex.h>
#include <parser/SceneNode.h>

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

using namespace Magnum;
using namespace Math::Literals;

class TexturedDrawable: public SceneGraph::Drawable3D {
    public:
        explicit TexturedDrawable(Object3D& object, Shaders::Phong& shader, GL::Mesh& mesh, GL::Texture2D& texture, SceneGraph::DrawableGroup3D& group)
            : SceneGraph::Drawable3D{object, &group}
            , m_shader(shader)
            , m_mesh(mesh)
            , m_texture(texture)
        {
        }

    private:
        void draw(const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera) override;

        Shaders::Phong& m_shader;
        GL::Mesh& m_mesh;
        GL::Texture2D& m_texture;
};

void TexturedDrawable::draw(const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera) {
    m_shader.setLightPosition(camera.cameraMatrix().transformPoint(Vector3(-50, 100, 100)))
            .setTransformationMatrix(transformationMatrix)
            .setNormalMatrix(transformationMatrix.normalMatrix())
            .setProjectionMatrix(camera.projectionMatrix())
            .bindDiffuseTexture(m_texture)
            .bindAmbientTexture(m_texture);
    m_mesh.draw(m_shader);
}

Trade::MeshData3D createMeshData(const parser::Mesh& mesh, size_t meshPartIndex)
{
    const parser::MeshPart meshPart = mesh.meshParts[meshPartIndex];

    std::vector<std::vector<Vector3>> positions(1);
    std::vector<std::vector<Vector3>> normals;
    std::vector<std::vector<Vector2>> textureCoords2D(1);

    for (size_t i = meshPart.vertexInterval.first; i < meshPart.vertexInterval.second; ++i) {
        const auto& v = mesh.vertices[i];
        positions[0].emplace_back(v.x, v.y, v.z);
    }

    if (!mesh.normals.empty()) {
        normals.resize(1);
        for (size_t i = meshPart.vertexInterval.first; i < meshPart.vertexInterval.second; ++i) {
            const auto& n = mesh.normals[i];
            normals[0].emplace_back(n.x, n.y, n.z);
        }
    }

    for (size_t i = meshPart.vertexInterval.first; i < meshPart.vertexInterval.second; ++i) {
        const auto& uv = mesh.uvs[i];
        textureCoords2D[0].emplace_back(uv.x, uv.y);
    }

    int firstVertexIndex = meshPart.vertexInterval.first;
    std::vector<UnsignedInt> indices;
    for (size_t i = meshPart.indexInterval.first; i < meshPart.indexInterval.second; ++i) {
        indices.push_back(mesh.indices[i] - firstVertexIndex);
    }

    Trade::MeshData3D meshData(MeshPrimitive::Triangles,
                               std::move(indices),
                               std::move(positions),
                               std::move(normals),
                               std::move(textureCoords2D),
                               {});
    return meshData;
}

GLView::GLView(Platform::GLContext& context, QWidget* parent, const parser::SceneIndex& sceneIndex)
    : QOpenGLWidget(parent)
    , m_context(context)
    , m_sceneIndex(sceneIndex)
    , m_phi(3.14f)
    , m_theta(1.3f)
    , m_cameraPosition(0.5f, 32.0f, 8.0f)
{
}

void GLView::initializeGL() {
    m_context.create();

    GL::Renderer::enable(GL::Renderer::Feature::DepthTest);
    GL::Renderer::enable(GL::Renderer::Feature::FaceCulling);

    m_cameraObject.setParent(&m_scene);
    updateCameraObject();

    (*(m_camera = new SceneGraph::Camera3D{m_cameraObject}))
        .setAspectRatioPolicy(SceneGraph::AspectRatioPolicy::Extend)
        .setProjectionMatrix(Matrix4::perspectiveProjection(35.0_degf, 1.0f, 0.01f, 1000.0f))
        .setViewport(GL::defaultFramebuffer.viewport().size());

    m_manipulator.setParent(&m_scene);

    std::unique_ptr<parser::SceneParser> scene = std::make_unique<parser::SceneParser>(m_sceneIndex.sirs[0], m_sceneIndex.bundleName);
    for (int i = 1; !scene->sceneRoot.has_value() && i < m_sceneIndex.sirs.size(); ++i)
       scene = std::make_unique<parser::SceneParser>(m_sceneIndex.sirs[i], m_sceneIndex.bundleName);
    assert(scene->sceneRoot.has_value());

    m_meshes = Containers::Array<Containers::Optional<GL::Mesh>>{scene->sceneRoot->numberOfMeshes()};
    m_textures = Containers::Array<Containers::Optional<Magnum::GL::Texture2D>>{scene->sceneRoot->numberOfMeshes()};

    m_texturedShader = std::make_unique<Shaders::Phong>(Shaders::Phong::Flag::DiffuseTexture | Shaders::Phong::Flag::AmbientTexture);
    m_texturedShader
        ->setAmbientColor(0x5c5c5c_rgbf)
        .setSpecularColor(0x000000_rgbf)
        .setShininess(0.0f);

    setupScene(*scene->sceneRoot);

    m_time.start();
    m_oldTime = m_time.elapsed();
    m_deltaTime = 0.0f;

    /* Clean up Magnum state when giving control back to Qt */
    GL::Context::current().resetState(GL::Context::State::EnterExternal);
}

void GLView::paintGL() {
    /* Reset state to avoid Qt affecting Magnum */
    GL::Context::current().resetState(GL::Context::State::ExitExternal);
    auto qtDefaultFramebuffer = GL::Framebuffer::wrap(defaultFramebufferObject(), {{}, {width(), height()}});

    m_currentTime = m_time.elapsed();
    m_deltaTime = (m_currentTime - m_oldTime) / 1000.0f;
    m_oldTime = m_currentTime;

    updateCameraObject();

    qtDefaultFramebuffer.clear(GL::FramebufferClear::Color | GL::FramebufferClear::Depth);

    m_camera->draw(m_drawables);

    /* Clean up Magnum state when giving control back to Qt */
    GL::Context::current().resetState(GL::Context::State::EnterExternal);

    update();
}

void GLView::resizeGL(int width, int height)
{
    m_camera->setViewport(Vector2i{width, height});
}

void GLView::mousePressEvent(QMouseEvent* event) {
    if (!event->buttons().testFlag(Qt::LeftButton))
        return;

    m_previousCursorPosition = Vector2(event->localPos().x(), event->localPos().y());
}

void GLView::mouseReleaseEvent(QMouseEvent* event) {
    if (!event->buttons().testFlag(Qt::LeftButton))
        return;
}

void GLView::mouseMoveEvent(QMouseEvent* event) {
    if (!event->buttons().testFlag(Qt::LeftButton))
        return;

    Vector2 currrentPos(event->localPos().x(), event->localPos().y());
    Vector2 delta = 3.0f * (currrentPos - m_previousCursorPosition) / Vector2(width(), height());

    m_phi += delta.x();
    m_theta += delta.y();
    m_theta = Math::clamp(m_theta, 0.0f, Math::Constants<float>::pi());

    m_previousCursorPosition = currrentPos;
}

void GLView::keyPressEvent(QKeyEvent* event) {
    if (event->isAutoRepeat())
        return;

    m_inputManager.keyPress(event->key());
}

void GLView::keyReleaseEvent(QKeyEvent* event) {
    if (event->isAutoRepeat())
        return;

    m_inputManager.keyRelease(event->key());
}

void GLView::updateCameraObject() {
    if (m_inputManager.isKeyPressed(Qt::Key_W))
        m_cameraPosition += -CameraSpeed * m_deltaTime * m_cameraObject.transformation().transformVector(Vector3::zAxis());
    if (m_inputManager.isKeyPressed(Qt::Key_S))
        m_cameraPosition += CameraSpeed * m_deltaTime * m_cameraObject.transformation().transformVector(Vector3::zAxis());
    if (m_inputManager.isKeyPressed(Qt::Key_A))
        m_cameraPosition += -CameraSpeed * m_deltaTime * m_cameraObject.transformation().transformVector(Vector3::xAxis());
    if (m_inputManager.isKeyPressed(Qt::Key_D))
        m_cameraPosition += CameraSpeed * m_deltaTime * m_cameraObject.transformation().transformVector(Vector3::xAxis());
    if (m_inputManager.isKeyPressed(Qt::Key_Q))
        m_cameraPosition += -CameraSpeed * m_deltaTime * m_cameraObject.transformation().transformVector(Vector3::yAxis());
    if (m_inputManager.isKeyPressed(Qt::Key_E))
        m_cameraPosition += CameraSpeed * m_deltaTime * m_cameraObject.transformation().transformVector(Vector3::yAxis());

    Quaternion rotation = Quaternion::rotation(Rad{m_phi}, Vector3::zAxis()) *
                          Quaternion::rotation(Rad{0}, Vector3::yAxis()) *
                          Quaternion::rotation(Rad{m_theta}, Vector3::xAxis());
    Matrix4 transfromation = Matrix4::from(rotation.toMatrix(), m_cameraPosition);
    m_cameraObject.setTransformation(transfromation);
}

void GLView::setupScene(const parser::SceneNode& node)
{
    size_t meshIndex = 0;
    setupScene(node, m_manipulator, meshIndex);
}

void GLView::setupScene(const parser::SceneNode& node, Object3D& parent, size_t& meshIndex) {
    auto* object = new Object3D{&parent};

    object->setTransformation(node.computeTransformationMatrix());

    if (node.mesh.has_value()) {
        const parser::Mesh& mesh = *node.mesh;
        for (size_t meshPartIndex = 0; meshPartIndex < mesh.meshParts.size(); ++meshPartIndex) {
            // Geometry
            Trade::MeshData3D meshData = createMeshData(mesh, meshPartIndex);
            if(!meshData.hasNormals() || meshData.primitive() != MeshPrimitive::Triangles) {
                Warning{} << "Mesh isn't solid or doesn't have normals";
            }
            m_meshes[meshIndex] = MeshTools::compile(meshData);

            // Texture
            PluginManager::Manager<Trade::AbstractImporter> manager;
            Containers::Pointer<Trade::AbstractImporter> importer = manager.loadAndInstantiate("PngImporter");
            assert(importer);
            importer->openFile(mesh.meshParts[meshPartIndex].textures[0].string());
            assert(importer->isOpened());

            Containers::Optional<Trade::ImageData2D> imageData = importer->image2D(0);
            GL::TextureFormat format;
            if(imageData && imageData->format() == PixelFormat::RGB8Unorm)
                format = GL::TextureFormat::RGB8;
            else if(imageData && imageData->format() == PixelFormat::RGBA8Unorm)
                format = GL::TextureFormat::RGBA8;

            GL::Texture2D texture;
            texture.setWrapping(GL::SamplerWrapping::MirroredRepeat)
                   .setMagnificationFilter(GL::SamplerFilter::Linear)
                   .setMinificationFilter(GL::SamplerFilter::Linear)
                   .setStorage(1, GL::textureFormat(imageData->format()), imageData->size())
                   .setSubImage(0, {}, *imageData);

            m_textures[meshIndex] = std::move(texture);

            new TexturedDrawable(*object, *m_texturedShader, *m_meshes[meshIndex], *m_textures[meshIndex], m_drawables);
            ++meshIndex;
        }
    }

    if (!node.children.empty()) {
        for (size_t i = 0; i < node.children.size(); ++i)
            setupScene(node.children[i], *object, meshIndex);
    }
}