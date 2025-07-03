#include "OpenGLRenderer.h"
#include <iostream>
#include <cmath>
#include <tiny_gltf.h>

OpenGLRenderer::OpenGLRenderer(HWND window) 
    : m_hWnd(window), m_hDC(nullptr), m_hRC(nullptr), 
    m_demoVAO(0), m_demoVBO(0), m_rotationAngle(0.0f),
    m_windowWidth(800), m_windowHeight(600),
    m_currentModel(nullptr), m_isDemo(true), m_isWireframeMode(true) {
}

OpenGLRenderer::~OpenGLRenderer() {
    cleanup();
}

bool OpenGLRenderer::initializeOpenGL() {
    // �f�o�C�X�R���e�L�X�g���擾
    m_hDC = GetDC(m_hWnd);
    if (!m_hDC) {
        std::cerr << "�f�o�C�X�R���e�L�X�g�̎擾�Ɏ��s���܂���" << std::endl;
        return false;
    }

    // �s�N�Z���t�H�[�}�b�g��ݒ�
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
        std::cerr << "�K�؂ȃs�N�Z���t�H�[�}�b�g��������܂���" << std::endl;
        return false;
    }

    if (!SetPixelFormat(m_hDC, pixelFormat, &pfd)) {
        std::cerr << "�s�N�Z���t�H�[�}�b�g�̐ݒ�Ɏ��s���܂���" << std::endl;
        return false;
    }

    // OpenGL�����_�����O�R���e�L�X�g���쐬
    m_hRC = wglCreateContext(m_hDC);
    if (!m_hRC) {
        std::cerr << "OpenGL�R���e�L�X�g�̍쐬�Ɏ��s���܂���" << std::endl;
        return false;
    }

    if (!wglMakeCurrent(m_hDC, m_hRC)) {
        std::cerr << "OpenGL�R���e�L�X�g�̗L�����Ɏ��s���܂���" << std::endl;
        return false;
    }

    // GLEW��������
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "GLEW�̏������Ɏ��s���܂���: " << glewGetErrorString(err) << std::endl;
        return false;
    }

    // OpenGL����\��
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;

    // OpenGL�̐ݒ�
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.2f, 0.3f, 0.4f, 1.0f);

    return true;
}

void OpenGLRenderer::setupTriangle() {
    // �J���t���ȎO�p�`�̒��_�f�[�^�i�ʒu + �F�j
    float vertices[] = {
        // �ʒu           // �F
        -0.5f, -0.5f, 0.0f,   1.0f, 0.0f, 0.0f,  // ���� - ��
        0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,  // �E�� - ��
        0.0f,  0.5f, 0.0f,   0.0f, 0.0f, 1.0f   // ��   - ��
    };

    glGenVertexArrays(1, &m_demoVAO);  // m_VAO �� m_demoVAO
    glGenBuffers(1, &m_demoVBO);       // m_VBO �� m_demoVBO

    glBindVertexArray(m_demoVAO);

    glBindBuffer(GL_ARRAY_BUFFER, m_demoVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // �ʒu���� (location = 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // �F���� (location = 1)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    std::cout << "�O�p�`���b�V�����Z�b�g�A�b�v���܂��� (VAO: " << m_demoVAO << ", VBO: " << m_demoVBO << ")" << std::endl;
}

// glm�s��̏�����
void OpenGLRenderer::initializeMatrices() {
    // ���f���s��i�P�ʍs�񂩂�J�n�j
    m_modelMatrix = glm::mat4(1.0f);

    // �r���[�s��i�J��������������ɉ�����j
    glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
    glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
    m_viewMatrix = glm::lookAt(cameraPos, cameraTarget, cameraUp);

    // ���e�s��i�������e�j
    float aspectRatio = (float)m_windowWidth / (float)m_windowHeight;
    m_projectionMatrix = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 100.0f);

    std::cout << "  glm�s�񂪏���������܂���:" << std::endl;
    std::cout << "  �J�����ʒu: (" << cameraPos.x << ", " << cameraPos.y << ", " << cameraPos.z << ")" << std::endl;
    std::cout << "  �A�X�y�N�g��: " << aspectRatio << std::endl;
    std::cout << "  ����p: 45�x" << std::endl;
}

// �A�j���[�V�����p�̍s��X�V
void OpenGLRenderer::updateMatrices() {
    // Y������̉�]�ŃA�j���[�V����
    m_rotationAngle += 0.01f;  // ��]���x
    if (m_rotationAngle > 2.0f * 3.14159f) {
        m_rotationAngle -= 2.0f * 3.14159f;
    }

    // glm::rotate()���g�p���ă��f���s����X�V
    m_modelMatrix = glm::mat4(1.0f);
    m_modelMatrix = glm::rotate(m_modelMatrix, m_rotationAngle, glm::vec3(0.0f, 1.0f, 0.0f));
}

bool OpenGLRenderer::initialize() {
    if (!initializeOpenGL()) {
        return false;
    }

    // �V�F�[�_�[���R���p�C���E�����N
    if (!m_shaderManager.createShader(
        ShaderManager::getColoredVertexShader(),
        ShaderManager::getColoredFragmentShader())) {
        std::cerr << "�V�F�[�_�[�̍쐬�Ɏ��s���܂���" << std::endl;
        return false;
    }

    // �e�X�g�֐������s
    //testShaderCompilation();
    //testGLMIntegration();

    setupTriangle();
    initializeMatrices();

    // �f�����[�h�ŊJ�n
    setDemoMode(true);

    std::cout << "OpenGL�����_���[������ɏ���������܂���" << std::endl;
    return true;
}

// �f�����[�h�̕`��
void OpenGLRenderer::renderDemo() {
    // �f���p�̎O�p�`��`��
    glBindVertexArray(m_demoVAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);
}

// glTF���[�h�̕`��
void OpenGLRenderer::renderGLTF() {
    if (!m_currentModel || m_meshData.empty()) {
        return;
    }

    // ���C���[�t���[���\���̐؂�ւ��i�I�v�V�����j
    // �f�o�b�O�p�Ƀ��C���[�t���[�����[�h��L���ɂ���ꍇ

    if (m_isWireframeMode) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glLineWidth(1.0f); // ���C���[�t���[���̐���
    } else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    // �e���b�V����`��
    for (const auto& mesh : m_meshData) {
        glBindVertexArray(mesh->VAO);

        if (mesh->hasIndices) {
            glDrawElements(mesh->mode, mesh->indexCount, GL_UNSIGNED_INT, 0);
        } else {
            glDrawArrays(mesh->mode, 0, mesh->vertexCount);
        }

        glBindVertexArray(0);
    }

    // ���C���[�t���[�����[�h�����ɖ߂�
    if (m_isWireframeMode) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
}

// glTF���\�[�X�̃N���[���A�b�v
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

// render()���\�b�h�̍X�V��
void OpenGLRenderer::render() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // �s����X�V
    updateMatrices();

    // �V�F�[�_�[���g�p
    m_shaderManager.use();

    // glm����OpenGL�V�F�[�_�[��MVP�s���]��
    m_shaderManager.setMVPMatrices(m_modelMatrix, m_viewMatrix, m_projectionMatrix);

    // ���[�h�ɉ����ĕ`��
    if (m_isDemo) {
        renderDemo();
    } else {
        renderGLTF();
    }

    // �o�b�N�o�b�t�@�[���t�����g�o�b�t�@�[�ɕ\��
    SwapBuffers(m_hDC);
}

void OpenGLRenderer::onResize(int width, int height) {
    m_windowWidth = width;
    m_windowHeight = height;

    // �r���[�|�[�g���X�V
    glViewport(0, 0, width, height);

    // ���e�s����Čv�Z�i�A�X�y�N�g�䂪�ύX���ꂽ�ꍇ�j
    float aspectRatio = (float)width / (float)height;
    m_projectionMatrix = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 100.0f);

    std::cout << "�E�B���h�E�T�C�Y�ύX: " << width << "x" << height 
        << " (�A�X�y�N�g��: " << aspectRatio << ")" << std::endl;
}

void OpenGLRenderer::cleanup() {
    // glTF���\�[�X���N���[���A�b�v
    cleanupGLTFResources();

    // �f���p��VBO/VAO���N���[���A�b�v
    if (m_demoVBO != 0) {
        glDeleteBuffers(1, &m_demoVBO);
        m_demoVBO = 0;
    }

    if (m_demoVAO != 0) {
        glDeleteVertexArrays(1, &m_demoVAO);
        m_demoVAO = 0;
    }

    // �V�F�[�_�[���N���[���A�b�v
    m_shaderManager.cleanup();

    // OpenGL�R���e�L�X�g���N���[���A�b�v
    if (m_hRC) {
        wglMakeCurrent(nullptr, nullptr);
        wglDeleteContext(m_hRC);
        m_hRC = nullptr;
    }

    if (m_hDC) {
        ReleaseDC(m_hWnd, m_hDC);
        m_hDC = nullptr;
    }

    std::cout << "OpenGL�����_���[�̃N���[���A�b�v���������܂���" << std::endl;
}

// glTF���f���̃��[�h
bool OpenGLRenderer::loadGLTFModel(const tinygltf::Model& model) {
    std::cout << "=== glTF���f�����[�h�J�n ===" << std::endl;

    // ������glTF���\�[�X���N���[���A�b�v
    cleanupGLTFResources();

    // ���f���̎Q�Ƃ�ۑ�
    m_currentModel = &model;

    // glTF���f��������
    if (!processGLTFModel(model)) {
        std::cerr << "�G���[: glTF���f���̏����Ɏ��s���܂���" << std::endl;
        return false;
    }

    // �����_�����O���[�h��glTF�ɐݒ�
    setDemoMode(false);

    std::cout << "? glTF���f�����[�h���� (���b�V����: " << m_meshData.size() << ")" << std::endl;
    return true;
}

// glTF���f���S�̂̏���
bool OpenGLRenderer::processGLTFModel(const tinygltf::Model& model) {
    std::cout << "glTF���f��������..." << std::endl;
    std::cout << "  ���b�V����: " << model.meshes.size() << std::endl;
    std::cout << "  �m�[�h��: " << model.nodes.size() << std::endl;
    std::cout << "  �V�[����: " << model.scenes.size() << std::endl;

    // �e���b�V��������
    for (size_t i = 0; i < model.meshes.size(); ++i) {
        if (!processMesh(model.meshes[i], model)) {
            std::cerr << "�G���[: ���b�V�� " << i << " �̏����Ɏ��s���܂���" << std::endl;
            return false;
        }
    }

    return true;
}

// ���b�V���̏���
bool OpenGLRenderer::processMesh(const tinygltf::Mesh& mesh, const tinygltf::Model& model) {
    std::cout << "  ���b�V��������: " << mesh.name << " (�v���~�e�B�u��: " << mesh.primitives.size() << ")" << std::endl;

    // �e�v���~�e�B�u������
    for (size_t i = 0; i < mesh.primitives.size(); ++i) {
        auto meshData = std::make_unique<GLTFMeshData>();

        if (!processPrimitive(mesh.primitives[i], model, *meshData)) {
            std::cerr << "�G���[: �v���~�e�B�u " << i << " �̏����Ɏ��s���܂���" << std::endl;
            return false;
        }

        m_meshData.push_back(std::move(meshData));
    }

    return true;
}

// �v���~�e�B�u�̏���
bool OpenGLRenderer::processPrimitive(const tinygltf::Primitive& primitive, const tinygltf::Model& model, GLTFMeshData& meshData) {
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    // �`�惂�[�h�̐ݒ�
    meshData.mode = GL_TRIANGLES;
    if (primitive.mode == TINYGLTF_MODE_TRIANGLES) {
        meshData.mode = GL_TRIANGLES;
    } else if (primitive.mode == TINYGLTF_MODE_TRIANGLE_STRIP) {
        meshData.mode = GL_TRIANGLE_STRIP;
    } else if (primitive.mode == TINYGLTF_MODE_TRIANGLE_FAN) {
        meshData.mode = GL_TRIANGLE_FAN;
    }

    // �ʒu�f�[�^�̎擾
    auto positionIt = primitive.attributes.find("POSITION");
    if (positionIt == primitive.attributes.end()) {
        std::cerr << "�G���[: POSITION������������܂���" << std::endl;
        return false;
    }

    if (!getAccessorData(model, positionIt->second, vertices)) {
        std::cerr << "�G���[: �ʒu�f�[�^�̎擾�Ɏ��s���܂���" << std::endl;
        return false;
    }

    meshData.vertexCount = static_cast<GLsizei>(vertices.size() / 3);

    // �C���f�b�N�X�f�[�^�̎擾�i�I�v�V�����j
    if (primitive.indices >= 0) {
        if (!getIndexData(model, primitive.indices, indices)) {
            std::cerr << "�G���[: �C���f�b�N�X�f�[�^�̎擾�Ɏ��s���܂���" << std::endl;
            return false;
        }
        meshData.hasIndices = true;
        meshData.indexCount = static_cast<GLsizei>(indices.size());
    }

    // VAO�̍쐬
    if (!createVAO(vertices, indices, meshData)) {
        std::cerr << "�G���[: VAO�̍쐬�Ɏ��s���܂���" << std::endl;
        return false;
    }

    std::cout << "    �v���~�e�B�u�������� (���_��: " << meshData.vertexCount;
    if (meshData.hasIndices) {
        std::cout << ", �C���f�b�N�X��: " << meshData.indexCount;
    }
    std::cout << ")" << std::endl;

    return true;
}

// �A�N�Z�T�[����f�[�^���擾
bool OpenGLRenderer::getAccessorData(const tinygltf::Model& model, int accessorIndex, std::vector<float>& data) {
    if (accessorIndex < 0 || accessorIndex >= static_cast<int>(model.accessors.size())) {
        std::cerr << "�G���[: �����ȃA�N�Z�T�[�C���f�b�N�X: " << accessorIndex << std::endl;
        return false;
    }

    const tinygltf::Accessor& accessor = model.accessors[accessorIndex];
    const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
    const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

    // ���݂͈ʒu�f�[�^�iPOSITION�j�̂ݑΉ��i3�v�f��float�j
    if (accessor.type != TINYGLTF_TYPE_VEC3 || accessor.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT) {
        std::cerr << "�G���[: ���Ή��̃A�N�Z�T�[�^�C�v" << std::endl;
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

// �C���f�b�N�X�f�[�^�̎擾
bool OpenGLRenderer::getIndexData(const tinygltf::Model& model, int accessorIndex, std::vector<unsigned int>& indices) {
    if (accessorIndex < 0 || accessorIndex >= static_cast<int>(model.accessors.size())) {
        std::cerr << "�G���[: �����ȃA�N�Z�T�[�C���f�b�N�X: " << accessorIndex << std::endl;
        return false;
    }

    const tinygltf::Accessor& accessor = model.accessors[accessorIndex];
    const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
    const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

    indices.resize(accessor.count);

    const unsigned char* src = buffer.data.data() + bufferView.byteOffset + accessor.byteOffset;

    // �R���|�[�l���g�^�C�v�ɉ����ăC���f�b�N�X��ǂݎ��
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
        std::cerr << "�G���[: ���Ή��̃C���f�b�N�X�R���|�[�l���g�^�C�v" << std::endl;
        return false;
    }

    return true;
}

// VAO�̍쐬
bool OpenGLRenderer::createVAO(const std::vector<float>& vertices, const std::vector<unsigned int>& indices, GLTFMeshData& meshData) {
    // VAO��VBO�𐶐�
    glGenVertexArrays(1, &meshData.VAO);
    glGenBuffers(1, &meshData.VBO);

    if (meshData.hasIndices) {
        glGenBuffers(1, &meshData.EBO);
    }

    glBindVertexArray(meshData.VAO);

    // ���_�o�b�t�@�[�̐ݒ�
    glBindBuffer(GL_ARRAY_BUFFER, meshData.VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    // �ʒu�����̐ݒ� (location = 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // �C���f�b�N�X�o�b�t�@�[�̐ݒ�i���݂���ꍇ�j
    if (meshData.hasIndices) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshData.EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
    }

    glBindVertexArray(0);

    // OpenGL�G���[�`�F�b�N
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "�G���[: VAO�쐬����OpenGL�G���[���������܂���: " << error << std::endl;
        return false;
    }

    return true;
}



//// �V�F�[�_�[�R���p�C���[�V�����̃e�X�g
//void OpenGLRenderer::testShaderCompilation() {
//    std::cout << "=== �V�F�[�_�[�R���p�C���[�V�����e�X�g ===" << std::endl;
//
//    if (m_shaderManager.isValid()) {
//        std::cout << "O �V�F�[�_�[�̃R���p�C���ƃ����N���������܂���" << std::endl;
//        std::cout << "  �v���O����ID: " << m_shaderManager.getProgramID() << std::endl;
//
//        // �V�F�[�_�[�̏ڍ׏���\��
//        m_shaderManager.printActiveUniforms();
//        m_shaderManager.printActiveAttributes();
//    } else {
//        std::cerr << "X �V�F�[�_�[�̃R���p�C��/�����N�Ɏ��s���܂���" << std::endl;
//    }
//}
//
//// glm�����̃e�X�g
//void OpenGLRenderer::testGLMIntegration() {
//    std::cout << "=== glm�����e�X�g ===" << std::endl;
//
//    // ��{�I��glm�x�N�g������̃e�X�g
//    glm::vec3 testVec1(1.0f, 2.0f, 3.0f);
//    glm::vec3 testVec2(4.0f, 5.0f, 6.0f);
//    glm::vec3 testVecSum = testVec1 + testVec2;
//
//    std::cout << "O glm::vec3���Z�e�X�g: " 
//        << "(" << testVec1.x << "," << testVec1.y << "," << testVec1.z << ") + "
//        << "(" << testVec2.x << "," << testVec2.y << "," << testVec2.z << ") = "
//        << "(" << testVecSum.x << "," << testVecSum.y << "," << testVecSum.z << ")" << std::endl;
//
//    // ��{�I��glm�s�񑀍�̃e�X�g
//    glm::mat4 testMatrix = glm::mat4(1.0f);
//    testMatrix = glm::translate(testMatrix, glm::vec3(1.0f, 2.0f, 3.0f));
//    testMatrix = glm::rotate(testMatrix, glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
//    testMatrix = glm::scale(testMatrix, glm::vec3(2.0f, 2.0f, 2.0f));
//
//    std::cout << "O glm::mat4�ϊ��e�X�g: ���s�ړ� �� ��] �� �X�P�[��" << std::endl;
//
//    // glm���w�֐��̃e�X�g
//    float testAngle = glm::radians(90.0f);
//    float testSin = glm::sin(testAngle);
//    float testCos = glm::cos(testAngle);
//
//    std::cout << "O glm���w�֐��e�X�g: sin(90��) = " << testSin 
//        << ", cos(90��) = " << testCos << std::endl;
//
//    // �x�N�g�������ƃh�b�g�ς̃e�X�g
//    float vecLength = glm::length(testVec1);
//    float dotProduct = glm::dot(testVec1, testVec2);
//
//    std::cout << "O glm�x�N�g�����Z�e�X�g: length = " << vecLength 
//        << ", dot product = " << dotProduct << std::endl;
//
//    // ���K���e�X�g
//    glm::vec3 normalizedVec = glm::normalize(testVec1);
//    std::cout << "O glm���K���e�X�g: (" << testVec1.x << "," << testVec1.y << "," << testVec1.z << ") �� "
//        << "(" << normalizedVec.x << "," << normalizedVec.y << "," << normalizedVec.z << ")" << std::endl;
//
//    // MVP�s��̌v�Z�e�X�g
//    glm::mat4 model = glm::mat4(1.0f);
//    glm::mat4 view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0));
//    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f/600.0f, 0.1f, 100.0f);
//    glm::mat4 mvp = projection * view * model;
//
//    std::cout << "O MVP�s��v�Z�e�X�g: �s��̏�Z������Ɏ��s����܂���" << std::endl;
//
//    // glm::value_ptr()�̃e�X�g�iOpenGL�]���p�j
//    const float* matrixPtr = glm::value_ptr(mvp);
//    if (matrixPtr != nullptr) {
//        std::cout << "O glm::value_ptr()�e�X�g: OpenGL�]���p�|�C���^�̎擾�ɐ���" << std::endl;
//        std::cout << "  MVP[0][0] = " << matrixPtr[0] << ", MVP[3][3] = " << matrixPtr[15] << std::endl;
//    }
//
//    std::cout << "=== glm�����e�X�g���� ===" << std::endl;
//}