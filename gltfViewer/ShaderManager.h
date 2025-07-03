#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <unordered_map>

class ShaderManager {
private:
    GLuint m_programID;
    std::unordered_map<std::string, GLint> m_uniformLocations;
    bool m_isCompiled;

    // �V�F�[�_�[�R���p�C���p�̓����֐�
    GLuint compileShader(const std::string& source, GLenum shaderType);
    bool linkProgram(GLuint vertexShader, GLuint fragmentShader);
    void checkCompileErrors(GLuint shader, const std::string& type);
    void checkLinkErrors(GLuint program);

    // Uniform�ʒu�̃L���b�V���@�\
    GLint getUniformLocation(const std::string& name);

public:
    ShaderManager();
    ~ShaderManager();

    // �V�F�[�_�[�̍쐬�E�R���p�C��
    bool createShader(const std::string& vertexSource, const std::string& fragmentSource);
    bool createShaderFromFiles(const std::string& vertexPath, const std::string& fragmentPath);

    // �V�F�[�_�[�̎g�p�J�n�E�I��
    void use();
    void unuse();

    // glm�^��Uniform�ϐ��ݒ� (glm�����̏d�v�@�\)
    void setUniform(const std::string& name, const glm::mat4& matrix);
    void setUniform(const std::string& name, const glm::mat3& matrix);
    void setUniform(const std::string& name, const glm::vec4& vector);
    void setUniform(const std::string& name, const glm::vec3& vector);
    void setUniform(const std::string& name, const glm::vec2& vector);
    void setUniform(const std::string& name, float value);
    void setUniform(const std::string& name, int value);
    void setUniform(const std::string& name, bool value);

    // MVP�s��ݒ�p�֗̕��֐� (Phase 4.3�ŏd�v)
    void setMVPMatrices(const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection);
    void setModelMatrix(const glm::mat4& model);
    void setViewMatrix(const glm::mat4& view);
    void setProjectionMatrix(const glm::mat4& projection);

    // ��Ԏ擾
    bool isValid() const { return m_isCompiled && m_programID != 0; }
    GLuint getProgramID() const { return m_programID; }

    // ���\�[�X�Ǘ�
    void cleanup();

    // �f�o�b�O�p���[�e�B���e�B
    void printActiveUniforms();
    void printActiveAttributes();

    // ��{�I�ȃV�F�[�_�[�\�[�X�̒�
    static std::string getBasicVertexShader();
    static std::string getBasicFragmentShader();
    static std::string getColoredVertexShader();
    static std::string getColoredFragmentShader();
};