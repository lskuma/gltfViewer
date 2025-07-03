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

// �O���錾
namespace tinygltf {
    class Model;
    struct Mesh;
    struct Primitive;
}

class Camera;  // Camera �N���X�̑O���錾

// glTF���b�V���f�[�^��ێ�����\����
struct GLTFMeshData {
    GLuint VAO;
    GLuint VBO;
    GLuint EBO;
    GLenum mode;         // GL_TRIANGLES, GL_TRIANGLE_STRIP, etc.
    GLsizei indexCount;  // �C���f�b�N�X��
    GLsizei vertexCount; // ���_��
    bool hasIndices;     // �C���f�b�N�X�����邩�ǂ���

    GLTFMeshData() : VAO(0), VBO(0), EBO(0), mode(GL_TRIANGLES), 
        indexCount(0), vertexCount(0), hasIndices(false) {}
};

class OpenGLRenderer {
private:
    HWND m_hWnd;
    HDC m_hDC;
    HGLRC m_hRC;

    // �f���p�O�p�`�̃��\�[�X
    GLuint m_demoVAO, m_demoVBO;

    // glTF���b�V���f�[�^�̃R���e�i
    std::vector<std::unique_ptr<GLTFMeshData>> m_meshData;

    // ���݃��[�h����Ă���glTF���f���ւ̎Q��
    const tinygltf::Model* m_currentModel;

    // ShaderManager���g�p�����V�����V�F�[�_�[�V�X�e��
    ShaderManager m_shaderManager;

    // glm�s��ϊ��̃e�X�g�p�ϐ�
    glm::mat4 m_modelMatrix;
    glm::mat4 m_viewMatrix;
    glm::mat4 m_projectionMatrix;

    // �J�����֘A
    const Camera* m_camera;  // �J�����ւ̎Q�Ɓi���L���͊O���j

    // �����_�����O�e�X�g�p
    float m_rotationAngle;
    int m_windowWidth, m_windowHeight;
    bool m_isDemo;  // �f�����[�h��glTF���[�h���𔻒�
    bool m_isWireframeMode; // ���C���[�t���[���\�������b�V���\�����𔻒�

    bool initializeOpenGL();
    void setupTriangle();

    // glTF�֘A�̏������E�����֐�
    bool processGLTFModel(const tinygltf::Model& model);
    bool processMesh(const tinygltf::Mesh& mesh, const tinygltf::Model& model);
    bool processPrimitive(const tinygltf::Primitive& primitive, const tinygltf::Model& model, GLTFMeshData& meshData);

    // �A�N�Z�T�[����o�b�t�@�f�[�^���擾����֐�
    bool getAccessorData(const tinygltf::Model& model, int accessorIndex, std::vector<float>& data);
    bool getIndexData(const tinygltf::Model& model, int accessorIndex, std::vector<unsigned int>& indices);

    // OpenGL���\�[�X�̍쐬
    bool createVAO(
        const std::vector<float>& vertices, 
        const std::vector<unsigned int>& indices, 
        GLTFMeshData& meshData);

    // glm�s��̏������֐�
    void initializeMatrices();
    void updateMatrices();

public:
    OpenGLRenderer(HWND window);
    ~OpenGLRenderer();

    bool initialize();
    void render();
    void onResize(int width, int height);
    void cleanup();

    // glTF���f�������[�h���ĕ`�揀��������
    bool loadGLTFModel(const tinygltf::Model& model);

    // �J�����X�V�֐��i�t�F�[�Y5.2�Ŏ����j
    void updateCamera(const Camera* camera);

    // �����_�����O���[�h�̐ݒ�
    void setDemoMode(bool demo) { m_isDemo = demo; }

    // �e�X�g�p�̌��J���\�b�h
    //void testShaderCompilation();
    //void testGLMIntegration();

private:
    // �����w���p�[�֐�
    void renderDemo();
    void renderGLTF();
    void cleanupGLTFResources();
};