#include "Camera.h"
#include <iostream>
#include <algorithm>
#include <cmath>

// === コンストラクタ ===

Camera::Camera(const glm::vec3& position, const glm::vec3& target, const glm::vec3& up)
    : m_position(position)
    , m_worldUp(glm::normalize(up))
    , m_projectionType(ProjectionType::PERSPECTIVE)
    , m_fov(glm::radians(45.0f))
    , m_aspectRatio(16.0f / 9.0f)
    , m_nearPlane(0.1f)
    , m_farPlane(100.0f)
    , m_left(-1.0f)
    , m_right(1.0f)
    , m_bottom(-1.0f)
    , m_top(1.0f)
    , m_viewMatrixDirty(true)
    , m_projectionMatrixDirty(true)
{
    // 目標点から初期角度を計算
    glm::vec3 direction = glm::normalize(target - position);

    // ヨー角度を計算（XZ平面での角度）
    m_yaw = std::atan2(direction.x, direction.z);

    // ピッチ角度を計算（Y軸での角度）
    float horizontalDistance = std::sqrt(direction.x * direction.x + direction.z * direction.z);
    m_pitch = std::atan2(-direction.y, horizontalDistance);

    // 内部ベクトルを更新
    updateVectors();
    updateViewMatrix();
    updateProjectionMatrix();

    std::cout << "Camera initialized at position: (" 
        << position.x << ", " << position.y << ", " << position.z << ")" << std::endl;
    std::cout << "Initial yaw: " << glm::degrees(m_yaw) << " degrees, pitch: " 
        << glm::degrees(m_pitch) << " degrees" << std::endl;
}

// === 位置・向き操作 ===

void Camera::setPosition(const glm::vec3& position) {
    m_position = position;
    m_viewMatrixDirty = true;
}

void Camera::setTarget(const glm::vec3& target) {
    glm::vec3 direction = glm::normalize(target - m_position);

    // ヨー角度を計算
    m_yaw = std::atan2(direction.x, direction.z);

    // ピッチ角度を計算
    float horizontalDistance = std::sqrt(direction.x * direction.x + direction.z * direction.z);
    m_pitch = constrainPitch(std::atan2(-direction.y, horizontalDistance));

    updateVectors();
}

// === 移動操作 ===

void Camera::moveForward(float distance) {
    m_position += m_forwardV * distance;
    m_viewMatrixDirty = true;
    updateViewMatrix();
}

void Camera::moveBackward(float distance) {
    m_position -= m_forwardV * distance;
    m_viewMatrixDirty = true;
    updateViewMatrix();
}

void Camera::moveLeft(float distance) {
    m_position -= m_rightV * distance;
    m_viewMatrixDirty = true;
    updateViewMatrix();
}

void Camera::moveRight(float distance) {
    m_position += m_rightV * distance;
    m_viewMatrixDirty = true;
    updateViewMatrix();
}

void Camera::moveUp(float distance) {
    m_position += m_upV * distance;  // カメラのローカルアップベクトルを使用
    m_viewMatrixDirty = true;
    updateViewMatrix();
}

void Camera::moveDown(float distance) {
    m_position -= m_upV * distance;  // カメラのローカルアップベクトルを使用
    m_viewMatrixDirty = true;
    updateViewMatrix();
}

// === 回転操作 ===

void Camera::setYaw(float yaw) {
    m_yaw = yaw;
    updateVectors();
}

void Camera::setPitch(float pitch) {
    m_pitch = constrainPitch(pitch);
    updateVectors();
}

void Camera::setYawPitch(float yaw, float pitch) {
    m_yaw = yaw;
    m_pitch = constrainPitch(pitch);
    updateVectors();
}

void Camera::addYaw(float deltaYaw) {
    m_yaw += deltaYaw;
    updateVectors();
}

void Camera::addPitch(float deltaPitch) {
    m_pitch = constrainPitch(m_pitch + deltaPitch);
    updateVectors();
}

// === 投影設定 ===

void Camera::setPerspective(float fov, float aspectRatio, float nearPlane, float farPlane) {
    m_projectionType = ProjectionType::PERSPECTIVE;
    m_fov = fov;
    m_aspectRatio = aspectRatio;
    m_nearPlane = nearPlane;
    m_farPlane = farPlane;
    m_projectionMatrixDirty = true;

    updateProjectionMatrix();

    std::cout << "Perspective projection set - FOV: " << glm::degrees(fov) 
        << " degrees, Aspect: " << aspectRatio 
        << ", Near: " << nearPlane << ", Far: " << farPlane << std::endl;
}

void Camera::setOrthographic(float left, float right, float bottom, float top, float nearPlane, float farPlane) {
    m_projectionType = ProjectionType::ORTHOGRAPHIC;
    m_left = left;
    m_right = right;
    m_bottom = bottom;
    m_top = top;
    m_nearPlane = nearPlane;
    m_farPlane = farPlane;
    m_projectionMatrixDirty = true;

    std::cout << "Orthographic projection set - Left: " << left << ", Right: " << right 
        << ", Bottom: " << bottom << ", Top: " << top 
        << ", Near: " << nearPlane << ", Far: " << farPlane << std::endl;
}

void Camera::setAspectRatio(float aspectRatio) {
    if (m_projectionType == ProjectionType::PERSPECTIVE) {
        m_aspectRatio = aspectRatio;
        m_projectionMatrixDirty = true;
    }
}

// === 行列取得 ===

glm::mat4 Camera::getViewProjectionMatrix() const {
    return getProjectionMatrix() * getViewMatrix();
}

// === 自動フィッティング ===

void Camera::fitToBoundingBox(const glm::vec3& boundingBoxMin, const glm::vec3& boundingBoxMax, float padding) {
    // バウンディングボックスの中心を計算
    glm::vec3 center = (boundingBoxMin + boundingBoxMax) * 0.5f;

    // バウンディングボックスのサイズを計算
    glm::vec3 size = boundingBoxMax - boundingBoxMin;
    float maxDimension = std::max({size.x, size.y, size.z});

    // 安全チェック
    if (maxDimension <= 0.0f) {
        std::cerr << "Warning: Invalid bounding box size, using default camera position" << std::endl;
        setPosition(glm::vec3(0.0f, 0.0f, 3.0f));
        setTarget(glm::vec3(0.0f, 0.0f, 0.0f));
        return;
    }

    // 透視投影の場合、視野角に基づいて適切な距離を計算
    float distance;
    if (m_projectionType == ProjectionType::PERSPECTIVE) {
        // 視野角の半分のタンジェントを使用して距離を計算
        float halfFov = m_fov * 0.5f;
        distance = (maxDimension * padding) / (2.0f * std::tan(halfFov));

        // 最小距離を保証
        distance = std::max(distance, maxDimension * 0.5f);
    } else {
        // 正射影の場合は固定距離
        distance = maxDimension * padding;
    }

    // カメラの位置を設定（少し上と後ろから見下ろす角度）
    glm::vec3 offset = glm::normalize(glm::vec3(1.0f, 1.0f, 1.0f)) * distance;
    setPosition(center + offset);
    setTarget(center);

    // 近・遠クリップ面を適切に設定
    float newNear = std::max(0.01f, distance - maxDimension * padding);
    float newFar = distance + maxDimension * padding * 2.0f;

    if (m_projectionType == ProjectionType::PERSPECTIVE) {
        setPerspective(m_fov, m_aspectRatio, newNear, newFar);
    } else {
        float orthoSize = maxDimension * padding;
        setOrthographic(-orthoSize, orthoSize, -orthoSize, orthoSize, newNear, newFar);
    }

    std::cout << "Camera fitted to bounding box:" << std::endl;
    std::cout << "  Center: (" << center.x << ", " << center.y << ", " << center.z << ")" << std::endl;
    std::cout << "  Size: (" << size.x << ", " << size.y << ", " << size.z << ")" << std::endl;
    std::cout << "  Max dimension: " << maxDimension << std::endl;
    std::cout << "  Distance: " << distance << std::endl;
    std::cout << "  Near: " << newNear << ", Far: " << newFar << std::endl;
}

// === デバッグ ===

void Camera::debugPrint() const {
    std::cout << "=== Camera Debug Info ===" << std::endl;
    std::cout << "Position: (" << m_position.x << ", " << m_position.y << ", " << m_position.z << ")" << std::endl;
    std::cout << "Forward: (" << m_forwardV.x << ", " << m_forwardV.y << ", " << m_forwardV.z << ")" << std::endl;
    std::cout << "Right: (" << m_rightV.x << ", " << m_rightV.y << ", " << m_rightV.z << ")" << std::endl;
    std::cout << "Up: (" << m_upV.x << ", " << m_upV.y << ", " << m_upV.z << ")" << std::endl;
    std::cout << "Yaw: " << glm::degrees(m_yaw) << " degrees" << std::endl;
    std::cout << "Pitch: " << glm::degrees(m_pitch) << " degrees" << std::endl;

    if (m_projectionType == ProjectionType::PERSPECTIVE) {
        std::cout << "Projection: Perspective" << std::endl;
        std::cout << "FOV: " << glm::degrees(m_fov) << " degrees" << std::endl;
        std::cout << "Aspect Ratio: " << m_aspectRatio << std::endl;
    } else {
        std::cout << "Projection: Orthographic" << std::endl;
        std::cout << "Left: " << m_left << ", Right: " << m_right << std::endl;
        std::cout << "Bottom: " << m_bottom << ", Top: " << m_top << std::endl;
    }
    std::cout << "Near: " << m_nearPlane << ", Far: " << m_farPlane << std::endl;

    // 行列の有効性をチェック
    const glm::mat4& viewMatrix = getViewMatrix();
    const glm::mat4& projMatrix = getProjectionMatrix();

    // NaN/Infチェック
    bool viewMatrixValid = true;
    bool projMatrixValid = true;

    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            if (!std::isfinite(viewMatrix[i][j])) {
                viewMatrixValid = false;
            }
            if (!std::isfinite(projMatrix[i][j])) {
                projMatrixValid = false;
            }
        }
    }

    std::cout << "View Matrix Valid: " << (viewMatrixValid ? "Yes" : "No") << std::endl;
    std::cout << "Projection Matrix Valid: " << (projMatrixValid ? "Yes" : "No") << std::endl;
    std::cout << "=========================" << std::endl;
}

// === プライベートメソッド ===

void Camera::updateVectors() {
    // オイラー角から方向ベクトルを計算（正しい数学的変換）
    glm::vec3 front;
    front.x = std::cos(m_yaw) * std::cos(m_pitch);
    front.y = std::sin(m_pitch);
    front.z = std::sin(m_yaw) * std::cos(m_pitch);

    m_forwardV = glm::normalize(front);

    // 右ベクトルを計算（外積）
    m_rightV = glm::normalize(glm::cross(m_forwardV, m_worldUp));

    // アップベクトルを計算（外積）
    m_upV = glm::normalize(glm::cross(m_rightV, m_forwardV));

    // ビュー行列の更新フラグを設定
    m_viewMatrixDirty = true;
    updateViewMatrix();
}

void Camera::updateViewMatrix() const {
    if (m_viewMatrixDirty) {
        // glm::lookAt()を使用してビュー行列を作成
        m_viewMatrix = glm::lookAt(m_position, m_position + m_forwardV, m_upV);
        m_viewMatrixDirty = false;
    }
}

void Camera::updateProjectionMatrix() const {
    if (m_projectionMatrixDirty) {
        if (m_projectionType == ProjectionType::PERSPECTIVE) {
            // glm::perspective()を使用して透視投影行列を作成
            m_projectionMatrix = glm::perspective(m_fov, m_aspectRatio, m_nearPlane, m_farPlane);
        } else {
            // glm::ortho()を使用して正射影行列を作成
            m_projectionMatrix = glm::ortho(m_left, m_right, m_bottom, m_top, m_nearPlane, m_farPlane);
        }
        m_projectionMatrixDirty = false;
    }
}

float Camera::constrainPitch(float pitch) const {
    // ピッチを-89度から+89度の範囲に制限（ジンバルロックを防ぐ）
    const float maxPitch = glm::radians(89.0f);
    return std::max(-maxPitch, std::min(maxPitch, pitch));
}