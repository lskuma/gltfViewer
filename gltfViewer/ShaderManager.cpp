#include "ShaderManager.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

ShaderManager::ShaderManager() 
    : m_programID(0), m_isCompiled(false) {
}

ShaderManager::~ShaderManager() {
    cleanup();
}

GLuint ShaderManager::compileShader(const std::string& source, GLenum shaderType) {
    GLuint shader = glCreateShader(shaderType);
    if (shader == 0) {
        std::cerr << "�V�F�[�_�[�I�u�W�F�N�g�̍쐬�Ɏ��s���܂���" << std::endl;
        return 0;
    }

    const char* sourcePtr = source.c_str();
    glShaderSource(shader, 1, &sourcePtr, nullptr);
    glCompileShader(shader);

    // �R���p�C���G���[���`�F�b�N
    std::string shaderTypeName = (shaderType == GL_VERTEX_SHADER) ? "VERTEX" : "FRAGMENT";
    checkCompileErrors(shader, shaderTypeName);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

bool ShaderManager::linkProgram(GLuint vertexShader, GLuint fragmentShader) {
    m_programID = glCreateProgram();
    if (m_programID == 0) {
        std::cerr << "�V�F�[�_�[�v���O�����̍쐬�Ɏ��s���܂���" << std::endl;
        return false;
    }

    glAttachShader(m_programID, vertexShader);
    glAttachShader(m_programID, fragmentShader);
    glLinkProgram(m_programID);

    checkLinkErrors(m_programID);

    GLint success;
    glGetProgramiv(m_programID, GL_LINK_STATUS, &success);
    if (!success) {
        glDeleteProgram(m_programID);
        m_programID = 0;
        return false;
    }

    // �V�F�[�_�[�I�u�W�F�N�g�͕s�v�ɂȂ����̂ō폜
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return true;
}

void ShaderManager::checkCompileErrors(GLuint shader, const std::string& type) {
    GLint success;
    GLchar infoLog[1024];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
        std::cerr << "�V�F�[�_�[�R���p�C���G���[ [" << type << "]:\n" << infoLog << std::endl;
    }
}

void ShaderManager::checkLinkErrors(GLuint program) {
    GLint success;
    GLchar infoLog[1024];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 1024, nullptr, infoLog);
        std::cerr << "�V�F�[�_�[�����N�G���[:\n" << infoLog << std::endl;
    }
}

GLint ShaderManager::getUniformLocation(const std::string& name) {
    // �L���b�V������ʒu������
    auto it = m_uniformLocations.find(name);
    if (it != m_uniformLocations.end()) {
        return it->second;
    }

    // �V����Uniform�ʒu���擾���ăL���b�V���ɕۑ�
    GLint location = glGetUniformLocation(m_programID, name.c_str());
    if (location == -1) {
        std::cerr << "�x��: Uniform�ϐ� '" << name << "' ��������܂���" << std::endl;
    }
    m_uniformLocations[name] = location;
    return location;
}

bool ShaderManager::createShader(const std::string& vertexSource, const std::string& fragmentSource) {
    cleanup(); // �����̃V�F�[�_�[���N���[���A�b�v

    // ���_�V�F�[�_�[���R���p�C��
    GLuint vertexShader = compileShader(vertexSource, GL_VERTEX_SHADER);
    if (vertexShader == 0) {
        std::cerr << "���_�V�F�[�_�[�̃R���p�C���Ɏ��s���܂���" << std::endl;
        return false;
    }

    // �t���O�����g�V�F�[�_�[���R���p�C��
    GLuint fragmentShader = compileShader(fragmentSource, GL_FRAGMENT_SHADER);
    if (fragmentShader == 0) {
        std::cerr << "�t���O�����g�V�F�[�_�[�̃R���p�C���Ɏ��s���܂���" << std::endl;
        glDeleteShader(vertexShader);
        return false;
    }

    // �v���O�����������N
    if (!linkProgram(vertexShader, fragmentShader)) {
        std::cerr << "�V�F�[�_�[�v���O�����̃����N�Ɏ��s���܂���" << std::endl;
        return false;
    }

    m_isCompiled = true;
    std::cout << "�V�F�[�_�[������ɃR���p�C���E�����N����܂��� (Program ID: " << m_programID << ")" << std::endl;
    return true;
}

bool ShaderManager::createShaderFromFiles(const std::string& vertexPath, const std::string& fragmentPath) {
    // �t�@�C�����璸�_�V�F�[�_�[��ǂݍ���
    std::ifstream vertexFile(vertexPath);
    if (!vertexFile.is_open()) {
        std::cerr << "���_�V�F�[�_�[�t�@�C�����J���܂���: " << vertexPath << std::endl;
        return false;
    }
    std::stringstream vertexStream;
    vertexStream << vertexFile.rdbuf();
    std::string vertexSource = vertexStream.str();

    // �t�@�C������t���O�����g�V�F�[�_�[��ǂݍ���
    std::ifstream fragmentFile(fragmentPath);
    if (!fragmentFile.is_open()) {
        std::cerr << "�t���O�����g�V�F�[�_�[�t�@�C�����J���܂���: " << fragmentPath << std::endl;
        return false;
    }
    std::stringstream fragmentStream;
    fragmentStream << fragmentFile.rdbuf();
    std::string fragmentSource = fragmentStream.str();

    return createShader(vertexSource, fragmentSource);
}

void ShaderManager::use() {
    if (m_isCompiled && m_programID != 0) {
        glUseProgram(m_programID);
    } else {
        std::cerr << "�x��: �����ȃV�F�[�_�[�v���O�������g�p���悤�Ƃ��܂���" << std::endl;
    }
}

void ShaderManager::unuse() {
    glUseProgram(0);
}

// ? glm�^��Uniform�ϐ��ݒ�֐��Q - glm�����̊j�S�@�\
void ShaderManager::setUniform(const std::string& name, const glm::mat4& matrix) {
    GLint location = getUniformLocation(name);
    if (location != -1) {
        glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
    }
}

void ShaderManager::setUniform(const std::string& name, const glm::mat3& matrix) {
    GLint location = getUniformLocation(name);
    if (location != -1) {
        glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
    }
}

void ShaderManager::setUniform(const std::string& name, const glm::vec4& vector) {
    GLint location = getUniformLocation(name);
    if (location != -1) {
        glUniform4fv(location, 1, glm::value_ptr(vector));
    }
}

void ShaderManager::setUniform(const std::string& name, const glm::vec3& vector) {
    GLint location = getUniformLocation(name);
    if (location != -1) {
        glUniform3fv(location, 1, glm::value_ptr(vector));
    }
}

void ShaderManager::setUniform(const std::string& name, const glm::vec2& vector) {
    GLint location = getUniformLocation(name);
    if (location != -1) {
        glUniform2fv(location, 1, glm::value_ptr(vector));
    }
}

void ShaderManager::setUniform(const std::string& name, float value) {
    GLint location = getUniformLocation(name);
    if (location != -1) {
        glUniform1f(location, value);
    }
}

void ShaderManager::setUniform(const std::string& name, int value) {
    GLint location = getUniformLocation(name);
    if (location != -1) {
        glUniform1i(location, value);
    }
}

void ShaderManager::setUniform(const std::string& name, bool value) {
    GLint location = getUniformLocation(name);
    if (location != -1) {
        glUniform1i(location, value ? 1 : 0);
    }
}

// ? MVP�s��ݒ�p�֗̕��֐� - Phase 4.3�ŏd�v
void ShaderManager::setMVPMatrices(const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection) {
    setUniform("u_model", model);
    setUniform("u_view", view);
    setUniform("u_projection", projection);

    // MVP�s��̐ς��v�Z���Đݒ�i�p�t�H�[�}���X�œK���j
    glm::mat4 mvp = projection * view * model;
    setUniform("u_mvp", mvp);
}

void ShaderManager::setModelMatrix(const glm::mat4& model) {
    setUniform("u_model", model);
}

void ShaderManager::setViewMatrix(const glm::mat4& view) {
    setUniform("u_view", view);
}

void ShaderManager::setProjectionMatrix(const glm::mat4& projection) {
    setUniform("u_projection", projection);
}

void ShaderManager::cleanup() {
    if (m_programID != 0) {
        glDeleteProgram(m_programID);
        m_programID = 0;
    }
    m_uniformLocations.clear();
    m_isCompiled = false;
}

void ShaderManager::printActiveUniforms() {
    if (!m_isCompiled) {
        std::cout << "�V�F�[�_�[���R���p�C������Ă��܂���" << std::endl;
        return;
    }

    GLint uniformCount;
    glGetProgramiv(m_programID, GL_ACTIVE_UNIFORMS, &uniformCount);
    std::cout << "�A�N�e�B�u��Uniform�ϐ��̐�: " << uniformCount << std::endl;

    GLint maxLength;
    glGetProgramiv(m_programID, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxLength);
    std::vector<GLchar> nameBuffer(maxLength);

    for (GLint i = 0; i < uniformCount; ++i) {
        GLsizei length;
        GLint size;
        GLenum type;
        glGetActiveUniform(m_programID, i, maxLength, &length, &size, &type, nameBuffer.data());

        std::string name(nameBuffer.data(), length);
        GLint location = glGetUniformLocation(m_programID, name.c_str());
        std::cout << "  [" << i << "] " << name << " (location: " << location << ", type: " << type << ")" << std::endl;
    }
}

void ShaderManager::printActiveAttributes() {
    if (!m_isCompiled) {
        std::cout << "�V�F�[�_�[���R���p�C������Ă��܂���" << std::endl;
        return;
    }

    GLint attributeCount;
    glGetProgramiv(m_programID, GL_ACTIVE_ATTRIBUTES, &attributeCount);
    std::cout << "�A�N�e�B�u�ȑ����̐�: " << attributeCount << std::endl;

    GLint maxLength;
    glGetProgramiv(m_programID, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &maxLength);
    std::vector<GLchar> nameBuffer(maxLength);

    for (GLint i = 0; i < attributeCount; ++i) {
        GLsizei length;
        GLint size;
        GLenum type;
        glGetActiveAttrib(m_programID, i, maxLength, &length, &size, &type, nameBuffer.data());

        std::string name(nameBuffer.data(), length);
        GLint location = glGetAttribLocation(m_programID, name.c_str());
        std::cout << "  [" << i << "] " << name << " (location: " << location << ", type: " << type << ")" << std::endl;
    }
}

// ��{�I�ȃV�F�[�_�[�\�[�X�̒�
std::string ShaderManager::getBasicVertexShader() {
    return R"(
#version 330 core
layout (location = 0) in vec3 a_position;

uniform mat4 u_mvp;

void main() {
    gl_Position = u_mvp * vec4(a_position, 1.0);
}
)";
}

std::string ShaderManager::getBasicFragmentShader() {
    return R"(
#version 330 core
out vec4 FragColor;

uniform vec4 u_color;

void main() {
    FragColor = u_color;
}
)";
}

std::string ShaderManager::getColoredVertexShader() {
    return R"(
#version 330 core
layout (location = 0) in vec3 a_position;
layout (location = 1) in vec3 a_color;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;
uniform mat4 u_mvp;

out vec3 v_color;

void main() {
    v_color = a_color;
    gl_Position = u_mvp * vec4(a_position, 1.0);
}
)";
}

std::string ShaderManager::getColoredFragmentShader() {
    return R"(
#version 330 core
in vec3 v_color;
out vec4 FragColor;

uniform vec3 u_materialColor;

void main() {
    FragColor = vec4(v_color, 1.0);
}
)";
}