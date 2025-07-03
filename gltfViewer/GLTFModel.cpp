#include <Windows.h>
#include <iostream>

#include <tiny_gltf.h>
#include "GLTFModel.h"


bool GLTFModel::loadFromFile(const std::string& filepath)
{
    tinygltf::TinyGLTF loader;
    std::string err;
    std::string warn;

    // �t�@�C���g���q�ɂ���ēǂݍ��ݕ��@������
    bool ret = false;
    if (filepath.substr(filepath.length() - 4) == ".glb") {
        ret = loader.LoadBinaryFromFile(&m_model, &err, &warn, filepath);
    } else {
        ret = loader.LoadASCIIFromFile(&m_model, &err, &warn, filepath);
    }

    // �x���ƃG���[���b�Z�[�W��\��
    if (!warn.empty()) {
        std::cout << "�x��: " << warn << std::endl;
    }

    if (!err.empty()) {
        std::cerr << "�G���[: " << err << std::endl;
    }

    if (!ret) {
        std::cerr << "glTF�t�@�C���̓ǂݍ��݂Ɏ��s���܂���: " << filepath << std::endl;
        return false;
    }

    m_loaded = true;
    std::cout << "glTF�t�@�C���̓ǂݍ��݂��������܂���: " << filepath << std::endl;

    // ��{����\��
    printModelInfo();

    return true;
}


void GLTFModel::printModelInfo()
{
    if (!m_loaded) {
        std::cout << "���f�����ǂݍ��܂�Ă��܂���" << std::endl;
        return;
    }

    std::cout << "\n=== glTF���f����� ===" << std::endl;

    // �A�Z�b�g���
    if (!m_model.asset.version.empty()) {
        std::cout << "glTF�o�[�W����: " << m_model.asset.version << std::endl;
    }
    if (!m_model.asset.generator.empty()) {
        std::cout << "�����c�[��: " << m_model.asset.generator << std::endl;
    }

    // �V�[�����
    std::cout << "�V�[����: " << m_model.scenes.size() << std::endl;
    if (m_model.defaultScene >= 0) {
        std::cout << "�f�t�H���g�V�[��: " << m_model.defaultScene << std::endl;
    }

    // �m�[�h���
    std::cout << "�m�[�h��: " << m_model.nodes.size() << std::endl;

    // ���b�V�����
    std::cout << "���b�V����: " << m_model.meshes.size() << std::endl;
    for (size_t i = 0; i < m_model.meshes.size(); ++i) {
        const auto& mesh = m_model.meshes[i];
        std::cout << "  ���b�V�� " << i << ": ";
        if (!mesh.name.empty()) {
            std::cout << "\"" << mesh.name << "\" ";
        }
        std::cout << "�v���~�e�B�u��: " << mesh.primitives.size() << std::endl;
    }

    // �}�e���A�����
    std::cout << "�}�e���A����: " << m_model.materials.size() << std::endl;

    // �e�N�X�`�����
    std::cout << "�e�N�X�`����: " << m_model.textures.size() << std::endl;

    // �C���[�W���
    std::cout << "�C���[�W��: " << m_model.images.size() << std::endl;

    // �A�j���[�V�������
    std::cout << "�A�j���[�V������: " << m_model.animations.size() << std::endl;

    // �o�b�t�@�[���
    std::cout << "�o�b�t�@�[��: " << m_model.buffers.size() << std::endl;
    for (size_t i = 0; i < m_model.buffers.size(); ++i) {
        const auto& buffer = m_model.buffers[i];
        std::cout << "  �o�b�t�@�[ " << i << ": " << buffer.data.size() << " �o�C�g";
        if (!buffer.uri.empty()) {
            std::cout << " (URI: " << buffer.uri << ")";
        }
        std::cout << std::endl;
    }

    // �o�b�t�@�[�r���[���
    std::cout << "�o�b�t�@�[�r���[��: " << m_model.bufferViews.size() << std::endl;

    // �A�N�Z�T�[���
    std::cout << "�A�N�Z�T�[��: " << m_model.accessors.size() << std::endl;

    std::cout << "========================\n" << std::endl;
}

// �ڍׂȍ\�����
void GLTFModel::analyzeStructure()
{
    if (!m_loaded) {
        std::cout << "���f�����ǂݍ��܂�Ă��܂���" << std::endl;
        return;
    }

    std::cout << "\n=== �ڍ׍\����� ===" << std::endl;

    // �V�[���K�w�̉��
    analyzeScenes();

    // ���b�V���̏ڍ׉��
    analyzeMeshes();

    // �}�e���A���̏ڍ׉��
    analyzeMaterials();

    // �A�N�Z�T�[�ƃo�b�t�@�[�̉��
    analyzeBuffers();

    std::cout << "========================\n" << std::endl;
}

void GLTFModel::analyzeScenes()
{
    std::cout << "\n--- �V�[���K�w��� ---" << std::endl;

    for (size_t i = 0; i < m_model.scenes.size(); ++i) {
        const auto& scene = m_model.scenes[i];
        std::cout << "�V�[�� " << i;
        if (!scene.name.empty()) {
            std::cout << " (\"" << scene.name << "\")";
        }
        std::cout << ":" << std::endl;

        std::cout << "  ���[�g�m�[�h��: " << scene.nodes.size() << std::endl;
        for (int nodeIdx : scene.nodes) {
            analyzeNodeHierarchy(nodeIdx, 1);
        }
    }
}

void GLTFModel::analyzeNodeHierarchy(int nodeIndex, int indent)
{
    if (nodeIndex < 0 || nodeIndex >= static_cast<int>(m_model.nodes.size())) {
        return;
    }

    const auto& node = m_model.nodes[nodeIndex];
    std::string indentStr(indent * 2, ' ');

    std::cout << indentStr << "�m�[�h " << nodeIndex;
    if (!node.name.empty()) {
        std::cout << " (\"" << node.name << "\")";
    }
    std::cout << ":" << std::endl;

    // �ϊ����
    if (!node.translation.empty()) {
        std::cout << indentStr << "  ���s�ړ�: (" 
            << node.translation[0] << ", " 
            << node.translation[1] << ", " 
            << node.translation[2] << ")" << std::endl;
    }
    if (!node.rotation.empty()) {
        std::cout << indentStr << "  ��](quat): (" 
            << node.rotation[0] << ", " 
            << node.rotation[1] << ", " 
            << node.rotation[2] << ", " 
            << node.rotation[3] << ")" << std::endl;
    }
    if (!node.scale.empty()) {
        std::cout << indentStr << "  �X�P�[��: (" 
            << node.scale[0] << ", " 
            << node.scale[1] << ", " 
            << node.scale[2] << ")" << std::endl;
    }
    if (!node.matrix.empty()) {
        std::cout << indentStr << "  �s��ϊ�: ���� (4x4)" << std::endl;
    }

    // ���b�V���Q��
    if (node.mesh >= 0) {
        std::cout << indentStr << "  ���b�V���Q��: " << node.mesh << std::endl;
    }

    // �q�m�[�h
    if (!node.children.empty()) {
        std::cout << indentStr << "  �q�m�[�h��: " << node.children.size() << std::endl;
        for (int childIdx : node.children) {
            analyzeNodeHierarchy(childIdx, indent + 1);
        }
    }
}

void GLTFModel::analyzeMeshes()
{
    std::cout << "\n--- ���b�V���ڍ׉�� ---" << std::endl;

    for (size_t i = 0; i < m_model.meshes.size(); ++i) {
        const auto& mesh = m_model.meshes[i];
        std::cout << "���b�V�� " << i;
        if (!mesh.name.empty()) {
            std::cout << " (\"" << mesh.name << "\")";
        }
        std::cout << ":" << std::endl;

        for (size_t j = 0; j < mesh.primitives.size(); ++j) {
            const auto& primitive = mesh.primitives[j];
            std::cout << "  �v���~�e�B�u " << j << ":" << std::endl;

            // �`�惂�[�h
            std::cout << "    �`�惂�[�h: ";
            switch (primitive.mode) {
            case TINYGLTF_MODE_POINTS: std::cout << "POINTS"; break;
            case TINYGLTF_MODE_LINE: std::cout << "LINES"; break;
            case TINYGLTF_MODE_LINE_LOOP: std::cout << "LINE_LOOP"; break;
            case TINYGLTF_MODE_LINE_STRIP: std::cout << "LINE_STRIP"; break;
            case TINYGLTF_MODE_TRIANGLES: std::cout << "TRIANGLES"; break;
            case TINYGLTF_MODE_TRIANGLE_STRIP: std::cout << "TRIANGLE_STRIP"; break;
            case TINYGLTF_MODE_TRIANGLE_FAN: std::cout << "TRIANGLE_FAN"; break;
            default: std::cout << "�s�� (" << primitive.mode << ")"; break;
            }
            std::cout << std::endl;

            // ���_����
            std::cout << "    ���_����:" << std::endl;
            for (const auto& attr : primitive.attributes) {
                std::cout << "      " << attr.first << " -> �A�N�Z�T�[ " << attr.second << std::endl;
                analyzeAccessor(attr.second, 6);
            }

            // �C���f�b�N�X
            if (primitive.indices >= 0) {
                std::cout << "    �C���f�b�N�X -> �A�N�Z�T�[ " << primitive.indices << std::endl;
                analyzeAccessor(primitive.indices, 6);
            }

            // �}�e���A��
            if (primitive.material >= 0) {
                std::cout << "    �}�e���A��: " << primitive.material << std::endl;
            }
        }
    }
}

void GLTFModel::analyzeAccessor(int accessorIndex, int indent)
{
    if (accessorIndex < 0 || accessorIndex >= static_cast<int>(m_model.accessors.size())) {
        return;
    }

    const auto& accessor = m_model.accessors[accessorIndex];
    std::string indentStr(indent, ' ');

    std::cout << indentStr << "�^�C�v: ";
    switch (accessor.type) {
    case TINYGLTF_TYPE_SCALAR: std::cout << "SCALAR"; break;
    case TINYGLTF_TYPE_VEC2: std::cout << "VEC2"; break;
    case TINYGLTF_TYPE_VEC3: std::cout << "VEC3"; break;
    case TINYGLTF_TYPE_VEC4: std::cout << "VEC4"; break;
    case TINYGLTF_TYPE_MAT2: std::cout << "MAT2"; break;
    case TINYGLTF_TYPE_MAT3: std::cout << "MAT3"; break;
    case TINYGLTF_TYPE_MAT4: std::cout << "MAT4"; break;
    default: std::cout << "�s��"; break;
    }
    std::cout << ", �����^�C�v: ";
    switch (accessor.componentType) {
    case TINYGLTF_COMPONENT_TYPE_BYTE: std::cout << "BYTE"; break;
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE: std::cout << "UNSIGNED_BYTE"; break;
    case TINYGLTF_COMPONENT_TYPE_SHORT: std::cout << "SHORT"; break;
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: std::cout << "UNSIGNED_SHORT"; break;
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT: std::cout << "UNSIGNED_INT"; break;
    case TINYGLTF_COMPONENT_TYPE_FLOAT: std::cout << "FLOAT"; break;
    default: std::cout << "�s��"; break;
    }
    std::cout << ", �v�f��: " << accessor.count << std::endl;

    // �o�E���f�B���O���
    if (!accessor.minValues.empty() && !accessor.maxValues.empty()) {
        std::cout << indentStr << "�͈�: min(";
        for (size_t k = 0; k < accessor.minValues.size(); ++k) {
            if (k > 0) std::cout << ", ";
            std::cout << accessor.minValues[k];
        }
        std::cout << ") max(";
        for (size_t k = 0; k < accessor.maxValues.size(); ++k) {
            if (k > 0) std::cout << ", ";
            std::cout << accessor.maxValues[k];
        }
        std::cout << ")" << std::endl;
    }
}

void GLTFModel::analyzeMaterials()
{
    std::cout << "\n--- �}�e���A���ڍ׉�� ---" << std::endl;

    for (size_t i = 0; i < m_model.materials.size(); ++i) {
        const auto& material = m_model.materials[i];
        std::cout << "�}�e���A�� " << i;
        if (!material.name.empty()) {
            std::cout << " (\"" << material.name << "\")";
        }
        std::cout << ":" << std::endl;

        // PBR���^���b�N���t�l�X
        const auto& pbr = material.pbrMetallicRoughness;
        std::cout << "  PBR���^���b�N���t�l�X:" << std::endl;
        std::cout << "    �x�[�X�J���[: (" 
            << pbr.baseColorFactor[0] << ", " 
            << pbr.baseColorFactor[1] << ", " 
            << pbr.baseColorFactor[2] << ", " 
            << pbr.baseColorFactor[3] << ")" << std::endl;
        std::cout << "    ���^���b�N�W��: " << pbr.metallicFactor << std::endl;
        std::cout << "    ���t�l�X�W��: " << pbr.roughnessFactor << std::endl;

        if (pbr.baseColorTexture.index >= 0) {
            std::cout << "    �x�[�X�J���[�e�N�X�`��: " << pbr.baseColorTexture.index << std::endl;
        }
        if (pbr.metallicRoughnessTexture.index >= 0) {
            std::cout << "    ���^���b�N/���t�l�X�e�N�X�`��: " << pbr.metallicRoughnessTexture.index << std::endl;
        }

        // ���̑��̓���
        if (material.normalTexture.index >= 0) {
            std::cout << "  �@���e�N�X�`��: " << material.normalTexture.index << std::endl;
        }
        if (material.emissiveTexture.index >= 0) {
            std::cout << "  �G�~�b�V�u�e�N�X�`��: " << material.emissiveTexture.index << std::endl;
        }

        std::cout << "  �G�~�b�V�u�W��: (" 
            << material.emissiveFactor[0] << ", " 
            << material.emissiveFactor[1] << ", " 
            << material.emissiveFactor[2] << ")" << std::endl;

        std::cout << "  �A���t�@���[�h: " << material.alphaMode << std::endl;
        if (material.alphaMode == "MASK") {
            std::cout << "  �A���t�@�J�b�g�I�t: " << material.alphaCutoff << std::endl;
        }

        std::cout << "  ���ʕ`��: " << (material.doubleSided ? "�L��" : "����") << std::endl;
    }
}

void GLTFModel::analyzeBuffers()
{
    std::cout << "\n--- �o�b�t�@�[/�A�N�Z�T�[��� ---" << std::endl;

    // �o�b�t�@�[�r���[�̏ڍ�
    std::cout << "�o�b�t�@�[�r���[�ڍ�:" << std::endl;
    for (size_t i = 0; i < m_model.bufferViews.size(); ++i) {
        const auto& bufferView = m_model.bufferViews[i];
        std::cout << "  �r���[ " << i << ": �o�b�t�@�[ " << bufferView.buffer 
            << ", �I�t�Z�b�g " << bufferView.byteOffset 
            << ", ���� " << bufferView.byteLength << " �o�C�g";
        if (bufferView.byteStride > 0) {
            std::cout << ", �X�g���C�h " << bufferView.byteStride;
        }
        if (bufferView.target != 0) {
            std::cout << ", �^�[�Q�b�g ";
            switch (bufferView.target) {
            case TINYGLTF_TARGET_ARRAY_BUFFER: std::cout << "ARRAY_BUFFER"; break;
            case TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER: std::cout << "ELEMENT_ARRAY_BUFFER"; break;
            default: std::cout << bufferView.target; break;
            }
        }
        std::cout << std::endl;
    }
}

// glTF Model�̌���
bool GLTFModel::validateModel()
{
    if (!m_loaded) {
        std::cout << "���f�����ǂݍ��܂�Ă��܂���" << std::endl;
        return false;
    }

    std::cout << "\n=== ���f������ ===" << std::endl;
    bool isValid = true;

    // ��{�\���̌���
    if (m_model.scenes.empty()) {
        std::cerr << "�G���[: �V�[������`����Ă��܂���" << std::endl;
        isValid = false;
    }

    if (m_model.defaultScene >= static_cast<int>(m_model.scenes.size())) {
        std::cerr << "�G���[: �f�t�H���g�V�[���̃C���f�b�N�X�������ł�" << std::endl;
        isValid = false;
    }

    // �A�N�Z�T�[�̌���
    for (size_t i = 0; i < m_model.accessors.size(); ++i) {
        const auto& accessor = m_model.accessors[i];
        if (accessor.bufferView >= static_cast<int>(m_model.bufferViews.size())) {
            std::cerr << "�G���[: �A�N�Z�T�[ " << i << " �� bufferView �C���f�b�N�X�������ł�" << std::endl;
            isValid = false;
        }
    }

    // �o�b�t�@�[�r���[�̌���
    for (size_t i = 0; i < m_model.bufferViews.size(); ++i) {
        const auto& bufferView = m_model.bufferViews[i];
        if (bufferView.buffer >= static_cast<int>(m_model.buffers.size())) {
            std::cerr << "�G���[: �o�b�t�@�[�r���[ " << i << " �� buffer �C���f�b�N�X�������ł�" << std::endl;
            isValid = false;
        } else {
            const auto& buffer = m_model.buffers[bufferView.buffer];
            if (bufferView.byteOffset + bufferView.byteLength > buffer.data.size()) {
                std::cerr << "�G���[: �o�b�t�@�[�r���[ " << i << " ���o�b�t�@�[�͈͂𒴂��Ă��܂�" << std::endl;
                isValid = false;
            }
        }
    }

    // ���b�V���̌���
    for (size_t i = 0; i < m_model.meshes.size(); ++i) {
        const auto& mesh = m_model.meshes[i];
        for (size_t j = 0; j < mesh.primitives.size(); ++j) {
            const auto& primitive = mesh.primitives[j];

            // �K�{�����̊m�F
            if (primitive.attributes.find("POSITION") == primitive.attributes.end()) {
                std::cerr << "�G���[: ���b�V�� " << i << " �v���~�e�B�u " << j << " ��POSITION����������܂���" << std::endl;
                isValid = false;
            }

            // �C���f�b�N�X�̌���
            if (primitive.indices >= static_cast<int>(m_model.accessors.size())) {
                std::cerr << "�G���[: ���b�V�� " << i << " �v���~�e�B�u " << j << " �̃C���f�b�N�X�������ł�" << std::endl;
                isValid = false;
            }
        }
    }

    if (isValid) {
        std::cout << "O ���f�����؂��������܂���" << std::endl;
    } else {
        std::cout << "X ���f�����؂ŃG���[��������܂���" << std::endl;
    }

    std::cout << "========================\n" << std::endl;
    return isValid;
}