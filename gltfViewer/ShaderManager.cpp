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
        std::cerr << "シェーダーオブジェクトの作成に失敗しました" << std::endl;
        return 0;
    }

    const char* sourcePtr = source.c_str();
    glShaderSource(shader, 1, &sourcePtr, nullptr);
    glCompileShader(shader);

    // コンパイルエラーをチェック
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
        std::cerr << "シェーダープログラムの作成に失敗しました" << std::endl;
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

    // シェーダーオブジェクトは不要になったので削除
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
        std::cerr << "シェーダーコンパイルエラー [" << type << "]:\n" << infoLog << std::endl;
    }
}

void ShaderManager::checkLinkErrors(GLuint program) {
    GLint success;
    GLchar infoLog[1024];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 1024, nullptr, infoLog);
        std::cerr << "シェーダーリンクエラー:\n" << infoLog << std::endl;
    }
}

GLint ShaderManager::getUniformLocation(const std::string& name) {
    // キャッシュから位置を検索
    auto it = m_uniformLocations.find(name);
    if (it != m_uniformLocations.end()) {
        return it->second;
    }

    // 新しくUniform位置を取得してキャッシュに保存
    GLint location = glGetUniformLocation(m_programID, name.c_str());
    if (location == -1) {
        std::cerr << "警告: Uniform変数 '" << name << "' が見つかりません" << std::endl;
    }
    m_uniformLocations[name] = location;
    return location;
}

bool ShaderManager::createShader(const std::string& vertexSource, const std::string& fragmentSource) {
    cleanup(); // 既存のシェーダーをクリーンアップ

    // 頂点シェーダーをコンパイル
    GLuint vertexShader = compileShader(vertexSource, GL_VERTEX_SHADER);
    if (vertexShader == 0) {
        std::cerr << "頂点シェーダーのコンパイルに失敗しました" << std::endl;
        return false;
    }

    // フラグメントシェーダーをコンパイル
    GLuint fragmentShader = compileShader(fragmentSource, GL_FRAGMENT_SHADER);
    if (fragmentShader == 0) {
        std::cerr << "フラグメントシェーダーのコンパイルに失敗しました" << std::endl;
        glDeleteShader(vertexShader);
        return false;
    }

    // プログラムをリンク
    if (!linkProgram(vertexShader, fragmentShader)) {
        std::cerr << "シェーダープログラムのリンクに失敗しました" << std::endl;
        return false;
    }

    m_isCompiled = true;
    std::cout << "シェーダーが正常にコンパイル・リンクされました (Program ID: " << m_programID << ")" << std::endl;
    return true;
}

bool ShaderManager::createShaderFromFiles(const std::string& vertexPath, const std::string& fragmentPath) {
    // ファイルから頂点シェーダーを読み込み
    std::ifstream vertexFile(vertexPath);
    if (!vertexFile.is_open()) {
        std::cerr << "頂点シェーダーファイルが開けません: " << vertexPath << std::endl;
        return false;
    }
    std::stringstream vertexStream;
    vertexStream << vertexFile.rdbuf();
    std::string vertexSource = vertexStream.str();

    // ファイルからフラグメントシェーダーを読み込み
    std::ifstream fragmentFile(fragmentPath);
    if (!fragmentFile.is_open()) {
        std::cerr << "フラグメントシェーダーファイルが開けません: " << fragmentPath << std::endl;
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
        std::cerr << "警告: 無効なシェーダープログラムを使用しようとしました" << std::endl;
    }
}

void ShaderManager::unuse() {
    glUseProgram(0);
}

// ? glm型のUniform変数設定関数群 - glm統合の核心機能
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

// ? MVP行列設定用の便利関数 - Phase 4.3で重要
void ShaderManager::setMVPMatrices(const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection) {
    setUniform("u_model", model);
    setUniform("u_view", view);
    setUniform("u_projection", projection);

    // MVP行列の積も計算して設定（パフォーマンス最適化）
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
        std::cout << "シェーダーがコンパイルされていません" << std::endl;
        return;
    }

    GLint uniformCount;
    glGetProgramiv(m_programID, GL_ACTIVE_UNIFORMS, &uniformCount);
    std::cout << "アクティブなUniform変数の数: " << uniformCount << std::endl;

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
        std::cout << "シェーダーがコンパイルされていません" << std::endl;
        return;
    }

    GLint attributeCount;
    glGetProgramiv(m_programID, GL_ACTIVE_ATTRIBUTES, &attributeCount);
    std::cout << "アクティブな属性の数: " << attributeCount << std::endl;

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

// 基本的なシェーダーソースの提供
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