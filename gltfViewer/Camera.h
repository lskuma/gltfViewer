#pragma once

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

/**
* @brief OpenGL�p�J�����N���X
* 
* glm���C�u�������g�p����3D�J�����V�X�e�����������܂��B
* �ʒu�A�����A�A�b�v�x�N�g����glm::vec3�ŊǗ����A
* �r���[�s��Ɠ��e�s���񋟂��܂��B
*/
class Camera {
public:
    /**
    * @brief �J�����̓��e�^�C�v
    */
    enum class ProjectionType {
        PERSPECTIVE,    ///< �������e
        ORTHOGRAPHIC    ///< ���ˉe
    };

    /**
    * @brief �R���X�g���N�^
    * @param position �J�����̏����ʒu
    * @param target �J�����������ڕW�_
    * @param up �A�b�v�x�N�g���i�ʏ��(0,1,0)�j
    */
    Camera(const glm::vec3& position = glm::vec3(0.0f, 0.0f, 3.0f),
        const glm::vec3& target = glm::vec3(0.0f, 0.0f, 0.0f),
        const glm::vec3& up = glm::vec3(0.0f, 1.0f, 0.0f));

    /**
    * @brief �f�X�g���N�^
    */
    ~Camera() = default;

    // === �ʒu�E�������� ===

    /**
    * @brief �J�����̈ʒu��ݒ�
    * @param position �V�����ʒu�x�N�g��
    */
    void setPosition(const glm::vec3& position);

    /**
    * @brief �J�����̈ʒu���擾
    * @return ���݂̈ʒu�x�N�g��
    */
    const glm::vec3& getPosition() const { return m_position; }

    /**
    * @brief �J�����̌�����ݒ�i�ڕW�_�w��j
    * @param target �J�����������ڕW�_
    */
    void setTarget(const glm::vec3& target);

    /**
    * @brief �J�����̌������擾
    * @return ���݂̌����x�N�g���i���K���ς݁j
    */
    const glm::vec3& getForward() const { return m_forwardV; }

    /**
    * @brief �J�����̉E�����x�N�g�����擾
    * @return �E�����x�N�g���i���K���ς݁j
    */
    const glm::vec3& getRight() const { return m_rightV; }

    /**
    * @brief �J�����̃A�b�v�x�N�g�����擾
    * @return �A�b�v�x�N�g���i���K���ς݁j
    */
    const glm::vec3& getUp() const { return m_upV; }

    // === �ړ����� ===

    /**
    * @brief �O�����Ɉړ�
    * @param distance �ړ�����
    */
    void moveForward(float distance);

    /**
    * @brief ������Ɉړ�
    * @param distance �ړ�����
    */
    void moveBackward(float distance);

    /**
    * @brief �������Ɉړ�
    * @param distance �ړ�����
    */
    void moveLeft(float distance);

    /**
    * @brief �E�����Ɉړ�
    * @param distance �ړ�����
    */
    void moveRight(float distance);

    /**
    * @brief ������Ɉړ�
    * @param distance �ړ�����
    */
    void moveUp(float distance);

    /**
    * @brief �������Ɉړ�
    * @param distance �ړ�����
    */
    void moveDown(float distance);

    // === ��]���� ===

    /**
    * @brief ���[�iY����]�j��ݒ�
    * @param yaw ���[�p�x�i���W�A���j
    */
    void setYaw(float yaw);

    /**
    * @brief �s�b�`�iX����]�j��ݒ�
    * @param pitch �s�b�`�p�x�i���W�A���j
    */
    void setPitch(float pitch);

    /**
    * @brief ���[�ƃs�b�`�𓯎��ɐݒ�
    * @param yaw ���[�p�x�i���W�A���j
    * @param pitch �s�b�`�p�x�i���W�A���j
    */
    void setYawPitch(float yaw, float pitch);

    /**
    * @brief ���݂̃��[�p�x���擾
    * @return ���[�p�x�i���W�A���j
    */
    float getYaw() const { return m_yaw; }

    /**
    * @brief ���݂̃s�b�`�p�x���擾
    * @return �s�b�`�p�x�i���W�A���j
    */
    float getPitch() const { return m_pitch; }

    /**
    * @brief ���[�p�x�𑊑ΓI�ɕύX
    * @param deltaYaw �ύX�ʁi���W�A���j
    */
    void addYaw(float deltaYaw);

    /**
    * @brief �s�b�`�p�x�𑊑ΓI�ɕύX
    * @param deltaPitch �ύX�ʁi���W�A���j
    */
    void addPitch(float deltaPitch);

    // === ���e�ݒ� ===

    /**
    * @brief �������e��ݒ�
    * @param fov ����p�i���W�A���j
    * @param aspectRatio �A�X�y�N�g��i��/�����j
    * @param nearPlane �߃N���b�v�ʋ���
    * @param farPlane ���N���b�v�ʋ���
    */
    void setPerspective(float fov, float aspectRatio, float nearPlane, float farPlane);

    /**
    * @brief ���ˉe��ݒ�
    * @param left ���[
    * @param right �E�[
    * @param bottom ���[
    * @param top ��[
    * @param nearPlane �߃N���b�v�ʋ���
    * @param farPlane ���N���b�v�ʋ���
    */
    void setOrthographic(float left, float right, float bottom, float top, float nearPlane, float farPlane);

    /**
    * @brief �A�X�y�N�g���ݒ�i�������e�̏ꍇ�̂݁j
    * @param aspectRatio �V�����A�X�y�N�g��
    */
    void setAspectRatio(float aspectRatio);

    // === �s��擾 ===

    /**
    * @brief �r���[�s����擾
    * @return �r���[�s��iglm::mat4�j
    */
    const glm::mat4& getViewMatrix() const { return m_viewMatrix; }

    /**
    * @brief ���e�s����擾
    * @return ���e�s��iglm::mat4�j
    */
    const glm::mat4& getProjectionMatrix() const { return m_projectionMatrix; }

    /**
    * @brief �r���[�E���e�s����擾
    * @return �r���[�E���e�s��iglm::mat4�j
    */
    glm::mat4 getViewProjectionMatrix() const;

    // === �����t�B�b�e�B���O ===

    /**
    * @brief �o�E���f�B���O�{�b�N�X�Ɋ�Â��ăJ�����������t�B�b�g
    * @param boundingBoxMin �o�E���f�B���O�{�b�N�X�̍ŏ����W
    * @param boundingBoxMax �o�E���f�B���O�{�b�N�X�̍ő���W
    * @param padding �p�f�B���O�W���i1.0�ōœK�t�B�b�g�A�傫���قǗ]�T�����j
    */
    void fitToBoundingBox(const glm::vec3& boundingBoxMin, const glm::vec3& boundingBoxMax, float padding = 1.2f);

    // === �f�o�b�O ===

    /**
    * @brief �J�����̏�Ԃ��f�o�b�O�o��
    */
    void debugPrint() const;

private:
    // === �v���C�x�[�g�����o�ϐ� ===

    // �ʒu�E����
    glm::vec3 m_position;       ///< �J�����̈ʒu
    glm::vec3 m_forwardV;       ///< �O�����x�N�g���i���K���ς݁j
    glm::vec3 m_rightV;         ///< �E�����x�N�g���i���K���ς݁j
    glm::vec3 m_upV;            ///< �A�b�v�x�N�g���i���K���ς݁j
    glm::vec3 m_worldUp;        ///< ���[���h�A�b�v�x�N�g���i�ʏ��(0,1,0)�j

    // ��]�p�x
    float m_yaw;                ///< ���[�p�x�i���W�A���j
    float m_pitch;              ///< �s�b�`�p�x�i���W�A���j

    // ���e�ݒ�
    ProjectionType m_projectionType;  ///< ���e�^�C�v
    float m_fov;                      ///< ����p�i���W�A���A�������e�p�j
    float m_aspectRatio;              ///< �A�X�y�N�g��
    float m_nearPlane;                ///< �߃N���b�v�ʋ���
    float m_farPlane;                 ///< ���N���b�v�ʋ���

    // ���ˉe�p�p�����[�^
    float m_left, m_right, m_bottom, m_top;

    // �s��
    mutable glm::mat4 m_viewMatrix;        ///< �r���[�s��
    mutable glm::mat4 m_projectionMatrix;  ///< ���e�s��
    mutable bool m_viewMatrixDirty;        ///< �r���[�s��̍X�V�t���O
    mutable bool m_projectionMatrixDirty;  ///< ���e�s��̍X�V�t���O

    // === �v���C�x�[�g���\�b�h ===

    /**
    * @brief �����x�N�g�����X�V
    */
    void updateVectors();

    /**
    * @brief �r���[�s����X�V
    */
    void updateViewMatrix() const;

    /**
    * @brief ���e�s����X�V
    */
    void updateProjectionMatrix() const;

    /**
    * @brief �s�b�`�p�x�𐧌�
    * @param pitch �����O�̃s�b�`�p�x
    * @return ������̃s�b�`�p�x
    */
    float constrainPitch(float pitch) const;
};