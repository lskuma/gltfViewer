#pragma once

#include <windows.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <memory>
#include "ShaderManager.h"

// 前方宣言
namespace tinygltf {
    class Model;
    struct Mesh;
    struct Primitive;
}

class Camera;  // Camera クラスの前方宣言

// glTFメッシュデータを保持する構造体
struct GLTFMeshData {
    GLuint VAO;
    GLuint VBO;
    GLuint EBO;
    GLenum mode;         // GL_TRIANGLES, GL_TRIANGLE_STRIP, etc.
    GLsizei indexCount;  // インデックス数
    GLsizei vertexCount; // 頂点数
    bool hasIndices;     // インデックスがあるかどうか

    GLTFMeshData() : VAO(0), VBO(0), EBO(0), mode(GL_TRIANGLES), 
        indexCount(0), vertexCount(0), hasIndices(false) {}
};

class OpenGLRenderer {
private:
    HWND m_hWnd;
    HDC m_hDC;
    HGLRC m_hRC;

    // デモ用三角形のリソース
    GLuint m_demoVAO, m_demoVBO;

    // glTFメッシュデータのコンテナ
    std::vector<std::unique_ptr<GLTFMeshData>> m_meshData;

    // 現在ロードされているglTFモデルへの参照
    const tinygltf::Model* m_currentModel;

    // ShaderManagerを使用した新しいシェーダーシステム
    ShaderManager m_shaderManager;

    // glm行列変換のテスト用変数
    glm::mat4 m_modelMatrix;
    glm::mat4 m_viewMatrix;
    glm::mat4 m_projectionMatrix;

    // カメラ関連
    const Camera* m_camera;  // カメラへの参照（所有権は外部）

    // レンダリングテスト用
    float m_rotationAngle;
    int m_windowWidth, m_windowHeight;
    bool m_isDemo;  // デモモードかglTFモードかを判定
    bool m_isWireframeMode; // ワイヤーフレーム表示かメッシュ表示かを判定

    bool initializeOpenGL();
    void setupTriangle();

    // glTF関連の初期化・処理関数
    bool processGLTFModel(const tinygltf::Model& model);
    bool processMesh(const tinygltf::Mesh& mesh, const tinygltf::Model& model);
    bool processPrimitive(const tinygltf::Primitive& primitive, const tinygltf::Model& model, GLTFMeshData& meshData);

    // アクセサーからバッファデータを取得する関数
    bool getAccessorData(const tinygltf::Model& model, int accessorIndex, std::vector<float>& data);
    bool getIndexData(const tinygltf::Model& model, int accessorIndex, std::vector<unsigned int>& indices);

    // OpenGLリソースの作成
    bool createVAO(
        const std::vector<float>& vertices, 
        const std::vector<unsigned int>& indices, 
        GLTFMeshData& meshData);

    // glm行列の初期化関数
    void initializeMatrices();
    void updateMatrices();

public:
    OpenGLRenderer(HWND window);
    ~OpenGLRenderer();

    bool initialize();
    void render();
    void onResize(int width, int height);
    void cleanup();

    // glTFモデルをロードして描画準備をする
    bool loadGLTFModel(const tinygltf::Model& model);

    // カメラ更新関数（フェーズ5.2で実装）
    void updateCamera(const Camera* camera);

    // レンダリングモードの設定
    void setDemoMode(bool demo) { m_isDemo = demo; }

    // テスト用の公開メソッド
    //void testShaderCompilation();
    //void testGLMIntegration();

private:
    // 内部ヘルパー関数
    void renderDemo();
    void renderGLTF();
    void cleanupGLTFResources();
};