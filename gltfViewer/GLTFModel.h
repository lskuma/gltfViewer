#pragma once

// glTFモデルデータを管理するクラス
class GLTFModel {
private:
    tinygltf::Model m_model;
    bool m_loaded;

public:
    GLTFModel() : m_loaded(false) {}

    bool loadFromFile(const std::string& filepath);

    void printModelInfo();

    // 詳細な構造解析
    void analyzeStructure();

private:
    void analyzeScenes();

    void analyzeNodeHierarchy(int nodeIndex, int indent);

    void analyzeMeshes();

    void analyzeAccessor(int accessorIndex, int indent);

    void analyzeMaterials();

    void analyzeBuffers();

public:
    // 検証機能
    bool validateModel();

    bool isLoaded() const { return m_loaded; }
    const tinygltf::Model& getModel() const { return m_model; }
};