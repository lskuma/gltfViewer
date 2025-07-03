#include "OpenGLRenderer.h"
#include <iostream>
#include <cmath>
#include <tiny_gltf.h>
#include "Camera.h"

OpenGLRenderer::OpenGLRenderer(HWND window) 
    : m_hWnd(window)
    , m_hDC(nullptr)
    , m_hRC(nullptr)
    , m_demoVAO(0)
    , m_demoVBO(0)
    , m_rotationAngle(0.0f)
    ,m_windowWidth(800)
    , m_windowHeight(600)
    ,m_currentModel(nullptr)
    , m_isDemo(true)
    , m_isWireframeMode(true)
    , m_camera(nullptr)
{
}

OpenGLRenderer::~OpenGLRenderer() {
    cleanup();
}

bool OpenGLRenderer::initializeOpenGL() {
    // デバイスコンテキストを取得
    m_hDC = GetDC(m_hWnd);
    if (!m_hDC) {
        std::cerr << "デバイスコンテキストの取得に失敗しました" << std::endl;
        return false;
    }

    // ピクセルフォーマットを設定
    PIXELFORMATDESCRIPTOR pfd = {};
    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cDepthBits = 24;
    pfd.cStencilBits = 8;
    pfd.iLayerType = PFD_MAIN_PLANE;

    int pixelFormat = ChoosePixelFormat(m_hDC, &pfd);
    if (!pixelFormat) {
        std::cerr << "適切なピクセルフォーマットが見つかりません" << std::endl;
        return false;
    }

    if (!SetPixelFormat(m_hDC, pixelFormat, &pfd)) {
        std::cerr << "ピクセルフォーマットの設定に失敗しました" << std::endl;
        return false;
    }

    // OpenGLレンダリングコンテキストを作成
    m_hRC = wglCreateContext(m_hDC);
    if (!m_hRC) {
        std::cerr << "OpenGLコンテキストの作成に失敗しました" << std::endl;
        return false;
    }

    if (!wglMakeCurrent(m_hDC, m_hRC)) {
        std::cerr << "OpenGLコンテキストの有効化に失敗しました" << std::endl;
        return false;
    }

    // GLEWを初期化
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "GLEWの初期化に失敗しました: " << glewGetErrorString(err) << std::endl;
        return false;
    }

    // OpenGL情報を表示
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;

    // OpenGLの設定
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.2f, 0.3f, 0.4f, 1.0f);

    return true;
}

void OpenGLRenderer::setupTriangle() {
    // カラフルな三角形の頂点データ（位置 + 色）
    float vertices[] = {
        // 位置           // 色
        -0.5f, -0.5f, 0.0f,   1.0f, 0.0f, 0.0f,  // 左下 - 赤
        0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,  // 右下 - 緑
        0.0f,  0.5f, 0.0f,   0.0f, 0.0f, 1.0f   // 上   - 青
    };

    glGenVertexArrays(1, &m_demoVAO);  // m_VAO → m_demoVAO
    glGenBuffers(1, &m_demoVBO);       // m_VBO → m_demoVBO

    glBindVertexArray(m_demoVAO);

    glBindBuffer(GL_ARRAY_BUFFER, m_demoVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // 位置属性 (location = 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // 色属性 (location = 1)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    std::cout << "三角形メッシュをセットアップしました (VAO: " << m_demoVAO << ", VBO: " << m_demoVBO << ")" << std::endl;
}

// glm行列の初期化
void OpenGLRenderer::initializeMatrices() {
    // モデル行列（単位行列から開始）
    m_modelMatrix = glm::mat4(1.0f);

    // ビュー行列（カメラを少し上後ろに下げる）
    glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
    glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
    m_viewMatrix = glm::lookAt(cameraPos, cameraTarget, cameraUp);

    // 投影行列（透視投影）
    float aspectRatio = (float)m_windowWidth / (float)m_windowHeight;
    m_projectionMatrix = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 100.0f);

    std::cout << "  glm行列が初期化されました:" << std::endl;
    std::cout << "  カメラ位置: (" << cameraPos.x << ", " << cameraPos.y << ", " << cameraPos.z << ")" << std::endl;
    std::cout << "  アスペクト比: " << aspectRatio << std::endl;
    std::cout << "  視野角: 45度" << std::endl;
}

// 行列更新メソッドの更新版（カメラ対応）
void OpenGLRenderer::updateMatrices() {
    //// モデル行列の更新（アニメーション用回転）
    //m_rotationAngle += 0.01f;  // 回転速度
    //if (m_rotationAngle > 2.0f * 3.14159f) {
    //    m_rotationAngle -= 2.0f * 3.14159f;
    //}

    //// glm::rotate()を使用してモデル行列を更新
    //m_modelMatrix = glm::mat4(1.0f);
    //m_modelMatrix = glm::rotate(m_modelMatrix, m_rotationAngle, glm::vec3(0.0f, 1.0f, 0.0f));

    // カメラが設定されている場合、ビューと投影行列を更新
    if (m_camera) {
        m_viewMatrix = m_camera->getViewMatrix();
        m_projectionMatrix = m_camera->getProjectionMatrix();
    } else {
        // デフォルトのカメラ設定（後方互換性のため）
        m_viewMatrix = glm::lookAt(
            glm::vec3(0.0f, 0.0f, 3.0f),  // カメラ位置
            glm::vec3(0.0f, 0.0f, 0.0f),  // 注視点
            glm::vec3(0.0f, 1.0f, 0.0f)   // アップベクトル
        );
        m_projectionMatrix = glm::perspective(
            glm::radians(45.0f),
            (float)m_windowWidth / (float)m_windowHeight,
            0.1f,
            100.0f
        );
    }

    // シェーダーに行列を送信
    //m_shaderManager.setMVPMatrices(m_modelMatrix, m_viewMatrix, m_projectionMatrix);
}

bool OpenGLRenderer::initialize() {
    if (!initializeOpenGL()) {
        return false;
    }

    // シェーダーをコンパイル・リンク
    if (!m_shaderManager.createShader(
        ShaderManager::getColoredVertexShader(),
        ShaderManager::getColoredFragmentShader())) {
        std::cerr << "シェーダーの作成に失敗しました" << std::endl;
        return false;
    }

    // テスト関数を実行
    //testShaderCompilation();
    //testGLMIntegration();

    setupTriangle();
    initializeMatrices();

    // デモモードで開始
    setDemoMode(true);

    std::cout << "OpenGLレンダラーが正常に初期化されました" << std::endl;
    return true;
}

// デモモードの描画
void OpenGLRenderer::renderDemo() {
    // デモ用の三角形を描画
    glBindVertexArray(m_demoVAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);
}

// glTFモードの描画
void OpenGLRenderer::renderGLTF() {
    if (!m_currentModel || m_meshData.empty()) {
        return;
    }

    // ワイヤーフレーム表示の切り替え（オプション）
    // デバッグ用にワイヤーフレームモードを有効にする場合

    if (m_isWireframeMode) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glLineWidth(1.0f); // ワイヤーフレームの線幅
    } else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    // 各メッシュを描画
    for (const auto& mesh : m_meshData) {
        glBindVertexArray(mesh->VAO);

        if (mesh->hasIndices) {
            glDrawElements(mesh->mode, mesh->indexCount, GL_UNSIGNED_INT, 0);
        } else {
            glDrawArrays(mesh->mode, 0, mesh->vertexCount);
        }

        glBindVertexArray(0);
    }

    // ワイヤーフレームモードを元に戻す
    if (m_isWireframeMode) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
}

// glTFリソースのクリーンアップ
void OpenGLRenderer::cleanupGLTFResources() {
    for (auto& mesh : m_meshData) {
        if (mesh->EBO != 0) {
            glDeleteBuffers(1, &mesh->EBO);
        }
        if (mesh->VBO != 0) {
            glDeleteBuffers(1, &mesh->VBO);
        }
        if (mesh->VAO != 0) {
            glDeleteVertexArrays(1, &mesh->VAO);
        }
    }

    m_meshData.clear();
    m_currentModel = nullptr;
}

// render()メソッドの更新版
void OpenGLRenderer::render() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 行列を更新
    updateMatrices();

    // シェーダーを使用
    m_shaderManager.use();

    // glmからOpenGLシェーダーへMVP行列を転送
    m_shaderManager.setMVPMatrices(m_modelMatrix, m_viewMatrix, m_projectionMatrix);

    // モードに応じて描画
    if (m_isDemo) {
        renderDemo();
    } else {
        renderGLTF();
    }

    // バックバッファーをフロントバッファーに表示
    SwapBuffers(m_hDC);
}

void OpenGLRenderer::onResize(int width, int height) {
    m_windowWidth = width;
    m_windowHeight = height;

    // ビューポートを更新
    glViewport(0, 0, width, height);

    // 投影行列を再計算（アスペクト比が変更された場合）
    float aspectRatio = (float)width / (float)height;
    m_projectionMatrix = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 100.0f);

    std::cout << "ウィンドウサイズ変更: " << width << "x" << height 
        << " (アスペクト比: " << aspectRatio << ")" << std::endl;
}

void OpenGLRenderer::cleanup() {
    // glTFリソースをクリーンアップ
    cleanupGLTFResources();

    // デモ用のVBO/VAOをクリーンアップ
    if (m_demoVBO != 0) {
        glDeleteBuffers(1, &m_demoVBO);
        m_demoVBO = 0;
    }

    if (m_demoVAO != 0) {
        glDeleteVertexArrays(1, &m_demoVAO);
        m_demoVAO = 0;
    }

    // シェーダーをクリーンアップ
    m_shaderManager.cleanup();

    // OpenGLコンテキストをクリーンアップ
    if (m_hRC) {
        wglMakeCurrent(nullptr, nullptr);
        wglDeleteContext(m_hRC);
        m_hRC = nullptr;
    }

    if (m_hDC) {
        ReleaseDC(m_hWnd, m_hDC);
        m_hDC = nullptr;
    }

    std::cout << "OpenGLレンダラーのクリーンアップが完了しました" << std::endl;
}

// glTFモデルのロード
bool OpenGLRenderer::loadGLTFModel(const tinygltf::Model& model) {
    std::cout << "=== glTFモデルロード開始 ===" << std::endl;

    // 既存のglTFリソースをクリーンアップ
    cleanupGLTFResources();

    // モデルの参照を保存
    m_currentModel = &model;

    // glTFモデルを処理
    if (!processGLTFModel(model)) {
        std::cerr << "エラー: glTFモデルの処理に失敗しました" << std::endl;
        return false;
    }

    // レンダリングモードをglTFに設定
    setDemoMode(false);

    std::cout << "? glTFモデルロード完了 (メッシュ数: " << m_meshData.size() << ")" << std::endl;
    return true;
}

// glTFモデル全体の処理
bool OpenGLRenderer::processGLTFModel(const tinygltf::Model& model) {
    std::cout << "glTFモデル処理中..." << std::endl;
    std::cout << "  メッシュ数: " << model.meshes.size() << std::endl;
    std::cout << "  ノード数: " << model.nodes.size() << std::endl;
    std::cout << "  シーン数: " << model.scenes.size() << std::endl;

    // 各メッシュを処理
    for (size_t i = 0; i < model.meshes.size(); ++i) {
        if (!processMesh(model.meshes[i], model)) {
            std::cerr << "エラー: メッシュ " << i << " の処理に失敗しました" << std::endl;
            return false;
        }
    }

    return true;
}

// メッシュの処理
bool OpenGLRenderer::processMesh(const tinygltf::Mesh& mesh, const tinygltf::Model& model) {
    std::cout << "  メッシュ処理中: " << mesh.name << " (プリミティブ数: " << mesh.primitives.size() << ")" << std::endl;

    // 各プリミティブを処理
    for (size_t i = 0; i < mesh.primitives.size(); ++i) {
        auto meshData = std::make_unique<GLTFMeshData>();

        if (!processPrimitive(mesh.primitives[i], model, *meshData)) {
            std::cerr << "エラー: プリミティブ " << i << " の処理に失敗しました" << std::endl;
            return false;
        }

        m_meshData.push_back(std::move(meshData));
    }

    return true;
}

// プリミティブの処理
bool OpenGLRenderer::processPrimitive(const tinygltf::Primitive& primitive, const tinygltf::Model& model, GLTFMeshData& meshData) {
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    // 描画モードの設定
    meshData.mode = GL_TRIANGLES;
    if (primitive.mode == TINYGLTF_MODE_TRIANGLES) {
        meshData.mode = GL_TRIANGLES;
    } else if (primitive.mode == TINYGLTF_MODE_TRIANGLE_STRIP) {
        meshData.mode = GL_TRIANGLE_STRIP;
    } else if (primitive.mode == TINYGLTF_MODE_TRIANGLE_FAN) {
        meshData.mode = GL_TRIANGLE_FAN;
    }

    // 位置データの取得
    auto positionIt = primitive.attributes.find("POSITION");
    if (positionIt == primitive.attributes.end()) {
        std::cerr << "エラー: POSITION属性が見つかりません" << std::endl;
        return false;
    }

    if (!getAccessorData(model, positionIt->second, vertices)) {
        std::cerr << "エラー: 位置データの取得に失敗しました" << std::endl;
        return false;
    }

    meshData.vertexCount = static_cast<GLsizei>(vertices.size() / 3);

    // インデックスデータの取得（オプション）
    if (primitive.indices >= 0) {
        if (!getIndexData(model, primitive.indices, indices)) {
            std::cerr << "エラー: インデックスデータの取得に失敗しました" << std::endl;
            return false;
        }
        meshData.hasIndices = true;
        meshData.indexCount = static_cast<GLsizei>(indices.size());
    }

    // マテリアルデータの取得（先ずはベースカラーのみ）
    auto material = model.materials.at(primitive.material);
    auto pbr = material.pbrMetallicRoughness;
    auto color = pbr.baseColorFactor;
    glm::vec3 materialColor(color[0], color[1], color[2]);
    /*m_shaderManager.setUniform("u_materialColor", materialColor);*/

    // VAOの作成
    if (!createVAO(vertices, indices, meshData)) {
        std::cerr << "エラー: VAOの作成に失敗しました" << std::endl;
        return false;
    }

    std::cout << "    プリミティブ処理完了 (頂点数: " << meshData.vertexCount;
    if (meshData.hasIndices) {
        std::cout << ", インデックス数: " << meshData.indexCount;
    }
    std::cout << ")" << std::endl;

    return true;
}

// アクセサーからデータを取得
bool OpenGLRenderer::getAccessorData(const tinygltf::Model& model, int accessorIndex, std::vector<float>& data) {
    if (accessorIndex < 0 || accessorIndex >= static_cast<int>(model.accessors.size())) {
        std::cerr << "エラー: 無効なアクセサーインデックス: " << accessorIndex << std::endl;
        return false;
    }

    const tinygltf::Accessor& accessor = model.accessors[accessorIndex];
    const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
    const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

    // 現在は位置データ（POSITION）のみ対応（3要素のfloat）
    if (accessor.type != TINYGLTF_TYPE_VEC3 || accessor.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT) {
        std::cerr << "エラー: 未対応のアクセサータイプ" << std::endl;
        return false;
    }

    data.resize(accessor.count * 3);

    const unsigned char* src = buffer.data.data() + bufferView.byteOffset + accessor.byteOffset;
    const float* floatData = reinterpret_cast<const float*>(src);

    for (size_t i = 0; i < accessor.count * 3; ++i) {
        data[i] = floatData[i];
    }

    return true;
}

// インデックスデータの取得
bool OpenGLRenderer::getIndexData(const tinygltf::Model& model, int accessorIndex, std::vector<unsigned int>& indices) {
    if (accessorIndex < 0 || accessorIndex >= static_cast<int>(model.accessors.size())) {
        std::cerr << "エラー: 無効なアクセサーインデックス: " << accessorIndex << std::endl;
        return false;
    }

    const tinygltf::Accessor& accessor = model.accessors[accessorIndex];
    const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
    const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

    indices.resize(accessor.count);

    const unsigned char* src = buffer.data.data() + bufferView.byteOffset + accessor.byteOffset;

    // コンポーネントタイプに応じてインデックスを読み取り
    if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
        const unsigned short* shortData = reinterpret_cast<const unsigned short*>(src);
        for (size_t i = 0; i < accessor.count; ++i) {
            indices[i] = static_cast<unsigned int>(shortData[i]);
        }
    } else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
        const unsigned int* intData = reinterpret_cast<const unsigned int*>(src);
        for (size_t i = 0; i < accessor.count; ++i) {
            indices[i] = intData[i];
        }
    } else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
        const unsigned char* byteData = src;
        for (size_t i = 0; i < accessor.count; ++i) {
            indices[i] = static_cast<unsigned int>(byteData[i]);
        }
    } else {
        std::cerr << "エラー: 未対応のインデックスコンポーネントタイプ" << std::endl;
        return false;
    }

    return true;
}

// VAOの作成
bool OpenGLRenderer::createVAO(
    const std::vector<float>& vertices, 
    const std::vector<unsigned int>& indices, 
    GLTFMeshData& meshData) 
{
    // VAOとVBOを生成
    glGenVertexArrays(1, &meshData.VAO);
    glGenBuffers(1, &meshData.VBO);

    if (meshData.hasIndices) {
        glGenBuffers(1, &meshData.EBO);
    }

    glBindVertexArray(meshData.VAO);

    // 頂点バッファーの設定
    glBindBuffer(GL_ARRAY_BUFFER, meshData.VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    // 位置属性の設定 (location = 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // インデックスバッファーの設定（存在する場合）
    if (meshData.hasIndices) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshData.EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
    }

    glBindVertexArray(0);

    // OpenGLエラーチェック
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "エラー: VAO作成中にOpenGLエラーが発生しました: " << error << std::endl;
        return false;
    }

    return true;
}

// フェーズ5.2: カメラ更新メソッドの実装
void OpenGLRenderer::updateCamera(const Camera* camera) {
    if (!camera) {
        std::cerr << "警告: カメラがnullptrです" << std::endl;
        return;
    }

    // カメラの参照を保存
    m_camera = camera;

    // カメラからビュー行列と投影行列を取得
    m_viewMatrix = camera->getViewMatrix();
    m_projectionMatrix = camera->getProjectionMatrix();

    // デバッグ出力（glm行列の確認）
#ifdef _DEBUG
    std::cout << "カメラ行列を更新しました:" << std::endl;
    std::cout << "ビュー行列:" << std::endl;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            std::cout << m_viewMatrix[i][j] << " ";
        }
        std::cout << std::endl;
    }
    std::cout << "投影行列:" << std::endl;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            std::cout << m_projectionMatrix[i][j] << " ";
        }
        std::cout << std::endl;
    }
#endif

    // シェーダーに行列を送信（現在のシェーダープログラムがバインドされている前提）
    //m_shaderManager.setMVPMatrices(m_modelMatrix, m_viewMatrix, m_projectionMatrix);
}

//// シェーダーコンパイレーションのテスト
//void OpenGLRenderer::testShaderCompilation() {
//    std::cout << "=== シェーダーコンパイレーションテスト ===" << std::endl;
//
//    if (m_shaderManager.isValid()) {
//        std::cout << "O シェーダーのコンパイルとリンクが成功しました" << std::endl;
//        std::cout << "  プログラムID: " << m_shaderManager.getProgramID() << std::endl;
//
//        // シェーダーの詳細情報を表示
//        m_shaderManager.printActiveUniforms();
//        m_shaderManager.printActiveAttributes();
//    } else {
//        std::cerr << "X シェーダーのコンパイル/リンクに失敗しました" << std::endl;
//    }
//}
//
//// glm統合のテスト
//void OpenGLRenderer::testGLMIntegration() {
//    std::cout << "=== glm統合テスト ===" << std::endl;
//
//    // 基本的なglmベクトル操作のテスト
//    glm::vec3 testVec1(1.0f, 2.0f, 3.0f);
//    glm::vec3 testVec2(4.0f, 5.0f, 6.0f);
//    glm::vec3 testVecSum = testVec1 + testVec2;
//
//    std::cout << "O glm::vec3演算テスト: " 
//        << "(" << testVec1.x << "," << testVec1.y << "," << testVec1.z << ") + "
//        << "(" << testVec2.x << "," << testVec2.y << "," << testVec2.z << ") = "
//        << "(" << testVecSum.x << "," << testVecSum.y << "," << testVecSum.z << ")" << std::endl;
//
//    // 基本的なglm行列操作のテスト
//    glm::mat4 testMatrix = glm::mat4(1.0f);
//    testMatrix = glm::translate(testMatrix, glm::vec3(1.0f, 2.0f, 3.0f));
//    testMatrix = glm::rotate(testMatrix, glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
//    testMatrix = glm::scale(testMatrix, glm::vec3(2.0f, 2.0f, 2.0f));
//
//    std::cout << "O glm::mat4変換テスト: 平行移動 → 回転 → スケール" << std::endl;
//
//    // glm数学関数のテスト
//    float testAngle = glm::radians(90.0f);
//    float testSin = glm::sin(testAngle);
//    float testCos = glm::cos(testAngle);
//
//    std::cout << "O glm数学関数テスト: sin(90°) = " << testSin 
//        << ", cos(90°) = " << testCos << std::endl;
//
//    // ベクトル長さとドット積のテスト
//    float vecLength = glm::length(testVec1);
//    float dotProduct = glm::dot(testVec1, testVec2);
//
//    std::cout << "O glmベクトル演算テスト: length = " << vecLength 
//        << ", dot product = " << dotProduct << std::endl;
//
//    // 正規化テスト
//    glm::vec3 normalizedVec = glm::normalize(testVec1);
//    std::cout << "O glm正規化テスト: (" << testVec1.x << "," << testVec1.y << "," << testVec1.z << ") → "
//        << "(" << normalizedVec.x << "," << normalizedVec.y << "," << normalizedVec.z << ")" << std::endl;
//
//    // MVP行列の計算テスト
//    glm::mat4 model = glm::mat4(1.0f);
//    glm::mat4 view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0));
//    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f/600.0f, 0.1f, 100.0f);
//    glm::mat4 mvp = projection * view * model;
//
//    std::cout << "O MVP行列計算テスト: 行列の乗算が正常に実行されました" << std::endl;
//
//    // glm::value_ptr()のテスト（OpenGL転送用）
//    const float* matrixPtr = glm::value_ptr(mvp);
//    if (matrixPtr != nullptr) {
//        std::cout << "O glm::value_ptr()テスト: OpenGL転送用ポインタの取得に成功" << std::endl;
//        std::cout << "  MVP[0][0] = " << matrixPtr[0] << ", MVP[3][3] = " << matrixPtr[15] << std::endl;
//    }
//
//    std::cout << "=== glm統合テスト完了 ===" << std::endl;
//}