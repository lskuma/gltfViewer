#pragma once

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

/**
* @brief OpenGL用カメラクラス
* 
* glmライブラリを使用して3Dカメラシステムを実装します。
* 位置、向き、アップベクトルをglm::vec3で管理し、
* ビュー行列と投影行列を提供します。
*/
class Camera {
public:
    /**
    * @brief カメラの投影タイプ
    */
    enum class ProjectionType {
        PERSPECTIVE,    ///< 透視投影
        ORTHOGRAPHIC    ///< 正射影
    };

    /**
    * @brief コンストラクタ
    * @param position カメラの初期位置
    * @param target カメラが向く目標点
    * @param up アップベクトル（通常は(0,1,0)）
    */
    Camera(const glm::vec3& position = glm::vec3(0.0f, 0.0f, 3.0f),
        const glm::vec3& target = glm::vec3(0.0f, 0.0f, 0.0f),
        const glm::vec3& up = glm::vec3(0.0f, 1.0f, 0.0f));

    /**
    * @brief デストラクタ
    */
    ~Camera() = default;

    // === 位置・向き操作 ===

    /**
    * @brief カメラの位置を設定
    * @param position 新しい位置ベクトル
    */
    void setPosition(const glm::vec3& position);

    /**
    * @brief カメラの位置を取得
    * @return 現在の位置ベクトル
    */
    const glm::vec3& getPosition() const { return m_position; }

    /**
    * @brief カメラの向きを設定（目標点指定）
    * @param target カメラが向く目標点
    */
    void setTarget(const glm::vec3& target);

    /**
    * @brief カメラの向きを取得
    * @return 現在の向きベクトル（正規化済み）
    */
    const glm::vec3& getForward() const { return m_forwardV; }

    /**
    * @brief カメラの右方向ベクトルを取得
    * @return 右方向ベクトル（正規化済み）
    */
    const glm::vec3& getRight() const { return m_rightV; }

    /**
    * @brief カメラのアップベクトルを取得
    * @return アップベクトル（正規化済み）
    */
    const glm::vec3& getUp() const { return m_upV; }

    // === 移動操作 ===

    /**
    * @brief 前方向に移動
    * @param distance 移動距離
    */
    void moveForward(float distance);

    /**
    * @brief 後方向に移動
    * @param distance 移動距離
    */
    void moveBackward(float distance);

    /**
    * @brief 左方向に移動
    * @param distance 移動距離
    */
    void moveLeft(float distance);

    /**
    * @brief 右方向に移動
    * @param distance 移動距離
    */
    void moveRight(float distance);

    /**
    * @brief 上方向に移動
    * @param distance 移動距離
    */
    void moveUp(float distance);

    /**
    * @brief 下方向に移動
    * @param distance 移動距離
    */
    void moveDown(float distance);

    // === 回転操作 ===

    /**
    * @brief ヨー（Y軸回転）を設定
    * @param yaw ヨー角度（ラジアン）
    */
    void setYaw(float yaw);

    /**
    * @brief ピッチ（X軸回転）を設定
    * @param pitch ピッチ角度（ラジアン）
    */
    void setPitch(float pitch);

    /**
    * @brief ヨーとピッチを同時に設定
    * @param yaw ヨー角度（ラジアン）
    * @param pitch ピッチ角度（ラジアン）
    */
    void setYawPitch(float yaw, float pitch);

    /**
    * @brief 現在のヨー角度を取得
    * @return ヨー角度（ラジアン）
    */
    float getYaw() const { return m_yaw; }

    /**
    * @brief 現在のピッチ角度を取得
    * @return ピッチ角度（ラジアン）
    */
    float getPitch() const { return m_pitch; }

    /**
    * @brief ヨー角度を相対的に変更
    * @param deltaYaw 変更量（ラジアン）
    */
    void addYaw(float deltaYaw);

    /**
    * @brief ピッチ角度を相対的に変更
    * @param deltaPitch 変更量（ラジアン）
    */
    void addPitch(float deltaPitch);

    // === 投影設定 ===

    /**
    * @brief 透視投影を設定
    * @param fov 視野角（ラジアン）
    * @param aspectRatio アスペクト比（幅/高さ）
    * @param nearPlane 近クリップ面距離
    * @param farPlane 遠クリップ面距離
    */
    void setPerspective(float fov, float aspectRatio, float nearPlane, float farPlane);

    /**
    * @brief 正射影を設定
    * @param left 左端
    * @param right 右端
    * @param bottom 下端
    * @param top 上端
    * @param nearPlane 近クリップ面距離
    * @param farPlane 遠クリップ面距離
    */
    void setOrthographic(float left, float right, float bottom, float top, float nearPlane, float farPlane);

    /**
    * @brief アスペクト比を設定（透視投影の場合のみ）
    * @param aspectRatio 新しいアスペクト比
    */
    void setAspectRatio(float aspectRatio);

    // === 行列取得 ===

    /**
    * @brief ビュー行列を取得
    * @return ビュー行列（glm::mat4）
    */
    const glm::mat4& getViewMatrix() const { return m_viewMatrix; }

    /**
    * @brief 投影行列を取得
    * @return 投影行列（glm::mat4）
    */
    const glm::mat4& getProjectionMatrix() const { return m_projectionMatrix; }

    /**
    * @brief ビュー・投影行列を取得
    * @return ビュー・投影行列（glm::mat4）
    */
    glm::mat4 getViewProjectionMatrix() const;

    // === 自動フィッティング ===

    /**
    * @brief バウンディングボックスに基づいてカメラを自動フィット
    * @param boundingBoxMin バウンディングボックスの最小座標
    * @param boundingBoxMax バウンディングボックスの最大座標
    * @param padding パディング係数（1.0で最適フィット、大きいほど余裕を持つ）
    */
    void fitToBoundingBox(const glm::vec3& boundingBoxMin, const glm::vec3& boundingBoxMax, float padding = 1.2f);

    // === デバッグ ===

    /**
    * @brief カメラの状態をデバッグ出力
    */
    void debugPrint() const;

private:
    // === プライベートメンバ変数 ===

    // 位置・向き
    glm::vec3 m_position;       ///< カメラの位置
    glm::vec3 m_forwardV;       ///< 前方向ベクトル（正規化済み）
    glm::vec3 m_rightV;         ///< 右方向ベクトル（正規化済み）
    glm::vec3 m_upV;            ///< アップベクトル（正規化済み）
    glm::vec3 m_worldUp;        ///< ワールドアップベクトル（通常は(0,1,0)）

    // 回転角度
    float m_yaw;                ///< ヨー角度（ラジアン）
    float m_pitch;              ///< ピッチ角度（ラジアン）

    // 投影設定
    ProjectionType m_projectionType;  ///< 投影タイプ
    float m_fov;                      ///< 視野角（ラジアン、透視投影用）
    float m_aspectRatio;              ///< アスペクト比
    float m_nearPlane;                ///< 近クリップ面距離
    float m_farPlane;                 ///< 遠クリップ面距離

    // 正射影用パラメータ
    float m_left, m_right, m_bottom, m_top;

    // 行列
    mutable glm::mat4 m_viewMatrix;        ///< ビュー行列
    mutable glm::mat4 m_projectionMatrix;  ///< 投影行列
    mutable bool m_viewMatrixDirty;        ///< ビュー行列の更新フラグ
    mutable bool m_projectionMatrixDirty;  ///< 投影行列の更新フラグ

    // === プライベートメソッド ===

    /**
    * @brief 内部ベクトルを更新
    */
    void updateVectors();

    /**
    * @brief ビュー行列を更新
    */
    void updateViewMatrix() const;

    /**
    * @brief 投影行列を更新
    */
    void updateProjectionMatrix() const;

    /**
    * @brief ピッチ角度を制限
    * @param pitch 制限前のピッチ角度
    * @return 制限後のピッチ角度
    */
    float constrainPitch(float pitch) const;
};