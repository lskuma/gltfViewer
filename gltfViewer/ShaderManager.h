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

    // シェーダーコンパイル用の内部関数
    GLuint compileShader(const std::string& source, GLenum shaderType);
    bool linkProgram(GLuint vertexShader, GLuint fragmentShader);
    void checkCompileErrors(GLuint shader, const std::string& type);
    void checkLinkErrors(GLuint program);

    // Uniform位置のキャッシュ機能
    GLint getUniformLocation(const std::string& name);

public:
    ShaderManager();
    ~ShaderManager();

    // シェーダーの作成・コンパイル
    bool createShader(const std::string& vertexSource, const std::string& fragmentSource);
    bool createShaderFromFiles(const std::string& vertexPath, const std::string& fragmentPath);

    // シェーダーの使用開始・終了
    void use();
    void unuse();

    // glm型のUniform変数設定 (glm統合の重要機能)
    void setUniform(const std::string& name, const glm::mat4& matrix);
    void setUniform(const std::string& name, const glm::mat3& matrix);
    void setUniform(const std::string& name, const glm::vec4& vector);
    void setUniform(const std::string& name, const glm::vec3& vector);
    void setUniform(const std::string& name, const glm::vec2& vector);
    void setUniform(const std::string& name, float value);
    void setUniform(const std::string& name, int value);
    void setUniform(const std::string& name, bool value);

    // MVP行列設定用の便利関数 (Phase 4.3で重要)
    void setMVPMatrices(const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection);
    void setModelMatrix(const glm::mat4& model);
    void setViewMatrix(const glm::mat4& view);
    void setProjectionMatrix(const glm::mat4& projection);

    // 状態取得
    bool isValid() const { return m_isCompiled && m_programID != 0; }
    GLuint getProgramID() const { return m_programID; }

    // リソース管理
    void cleanup();

    // デバッグ用ユーティリティ
    void printActiveUniforms();
    void printActiveAttributes();

    // 基本的なシェーダーソースの提供
    static std::string getBasicVertexShader();
    static std::string getBasicFragmentShader();
    static std::string getColoredVertexShader();
    static std::string getColoredFragmentShader();
};