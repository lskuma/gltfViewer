#include <Windows.h>
#include <iostream>

#include <tiny_gltf.h>
#include "GLTFModel.h"


bool GLTFModel::loadFromFile(const std::string& filepath)
{
    tinygltf::TinyGLTF loader;
    std::string err;
    std::string warn;

    // ファイル拡張子によって読み込み方法を決定
    bool ret = false;
    if (filepath.substr(filepath.length() - 4) == ".glb") {
        ret = loader.LoadBinaryFromFile(&m_model, &err, &warn, filepath);
    } else {
        ret = loader.LoadASCIIFromFile(&m_model, &err, &warn, filepath);
    }

    // 警告とエラーメッセージを表示
    if (!warn.empty()) {
        std::cout << "警告: " << warn << std::endl;
    }

    if (!err.empty()) {
        std::cerr << "エラー: " << err << std::endl;
    }

    if (!ret) {
        std::cerr << "glTFファイルの読み込みに失敗しました: " << filepath << std::endl;
        return false;
    }

    m_loaded = true;
    std::cout << "glTFファイルの読み込みが成功しました: " << filepath << std::endl;

    // 基本情報を表示
    printModelInfo();

    return true;
}


void GLTFModel::printModelInfo()
{
    if (!m_loaded) {
        std::cout << "モデルが読み込まれていません" << std::endl;
        return;
    }

    std::cout << "\n=== glTFモデル情報 ===" << std::endl;

    // アセット情報
    if (!m_model.asset.version.empty()) {
        std::cout << "glTFバージョン: " << m_model.asset.version << std::endl;
    }
    if (!m_model.asset.generator.empty()) {
        std::cout << "生成ツール: " << m_model.asset.generator << std::endl;
    }

    // シーン情報
    std::cout << "シーン数: " << m_model.scenes.size() << std::endl;
    if (m_model.defaultScene >= 0) {
        std::cout << "デフォルトシーン: " << m_model.defaultScene << std::endl;
    }

    // ノード情報
    std::cout << "ノード数: " << m_model.nodes.size() << std::endl;

    // メッシュ情報
    std::cout << "メッシュ数: " << m_model.meshes.size() << std::endl;
    for (size_t i = 0; i < m_model.meshes.size(); ++i) {
        const auto& mesh = m_model.meshes[i];
        std::cout << "  メッシュ " << i << ": ";
        if (!mesh.name.empty()) {
            std::cout << "\"" << mesh.name << "\" ";
        }
        std::cout << "プリミティブ数: " << mesh.primitives.size() << std::endl;
    }

    // マテリアル情報
    std::cout << "マテリアル数: " << m_model.materials.size() << std::endl;

    // テクスチャ情報
    std::cout << "テクスチャ数: " << m_model.textures.size() << std::endl;

    // イメージ情報
    std::cout << "イメージ数: " << m_model.images.size() << std::endl;

    // アニメーション情報
    std::cout << "アニメーション数: " << m_model.animations.size() << std::endl;

    // バッファー情報
    std::cout << "バッファー数: " << m_model.buffers.size() << std::endl;
    for (size_t i = 0; i < m_model.buffers.size(); ++i) {
        const auto& buffer = m_model.buffers[i];
        std::cout << "  バッファー " << i << ": " << buffer.data.size() << " バイト";
        if (!buffer.uri.empty()) {
            std::cout << " (URI: " << buffer.uri << ")";
        }
        std::cout << std::endl;
    }

    // バッファービュー情報
    std::cout << "バッファービュー数: " << m_model.bufferViews.size() << std::endl;

    // アクセサー情報
    std::cout << "アクセサー数: " << m_model.accessors.size() << std::endl;

    std::cout << "========================\n" << std::endl;
}

// 詳細な構造解析
void GLTFModel::analyzeStructure()
{
    if (!m_loaded) {
        std::cout << "モデルが読み込まれていません" << std::endl;
        return;
    }

    std::cout << "\n=== 詳細構造解析 ===" << std::endl;

    // シーン階層の解析
    analyzeScenes();

    // メッシュの詳細解析
    analyzeMeshes();

    // マテリアルの詳細解析
    analyzeMaterials();

    // アクセサーとバッファーの解析
    analyzeBuffers();

    std::cout << "========================\n" << std::endl;
}

void GLTFModel::analyzeScenes()
{
    std::cout << "\n--- シーン階層解析 ---" << std::endl;

    for (size_t i = 0; i < m_model.scenes.size(); ++i) {
        const auto& scene = m_model.scenes[i];
        std::cout << "シーン " << i;
        if (!scene.name.empty()) {
            std::cout << " (\"" << scene.name << "\")";
        }
        std::cout << ":" << std::endl;

        std::cout << "  ルートノード数: " << scene.nodes.size() << std::endl;
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

    std::cout << indentStr << "ノード " << nodeIndex;
    if (!node.name.empty()) {
        std::cout << " (\"" << node.name << "\")";
    }
    std::cout << ":" << std::endl;

    // 変換情報
    if (!node.translation.empty()) {
        std::cout << indentStr << "  平行移動: (" 
            << node.translation[0] << ", " 
            << node.translation[1] << ", " 
            << node.translation[2] << ")" << std::endl;
    }
    if (!node.rotation.empty()) {
        std::cout << indentStr << "  回転(quat): (" 
            << node.rotation[0] << ", " 
            << node.rotation[1] << ", " 
            << node.rotation[2] << ", " 
            << node.rotation[3] << ")" << std::endl;
    }
    if (!node.scale.empty()) {
        std::cout << indentStr << "  スケール: (" 
            << node.scale[0] << ", " 
            << node.scale[1] << ", " 
            << node.scale[2] << ")" << std::endl;
    }
    if (!node.matrix.empty()) {
        std::cout << indentStr << "  行列変換: あり (4x4)" << std::endl;
    }

    // メッシュ参照
    if (node.mesh >= 0) {
        std::cout << indentStr << "  メッシュ参照: " << node.mesh << std::endl;
    }

    // 子ノード
    if (!node.children.empty()) {
        std::cout << indentStr << "  子ノード数: " << node.children.size() << std::endl;
        for (int childIdx : node.children) {
            analyzeNodeHierarchy(childIdx, indent + 1);
        }
    }
}

void GLTFModel::analyzeMeshes()
{
    std::cout << "\n--- メッシュ詳細解析 ---" << std::endl;

    for (size_t i = 0; i < m_model.meshes.size(); ++i) {
        const auto& mesh = m_model.meshes[i];
        std::cout << "メッシュ " << i;
        if (!mesh.name.empty()) {
            std::cout << " (\"" << mesh.name << "\")";
        }
        std::cout << ":" << std::endl;

        for (size_t j = 0; j < mesh.primitives.size(); ++j) {
            const auto& primitive = mesh.primitives[j];
            std::cout << "  プリミティブ " << j << ":" << std::endl;

            // 描画モード
            std::cout << "    描画モード: ";
            switch (primitive.mode) {
            case TINYGLTF_MODE_POINTS: std::cout << "POINTS"; break;
            case TINYGLTF_MODE_LINE: std::cout << "LINES"; break;
            case TINYGLTF_MODE_LINE_LOOP: std::cout << "LINE_LOOP"; break;
            case TINYGLTF_MODE_LINE_STRIP: std::cout << "LINE_STRIP"; break;
            case TINYGLTF_MODE_TRIANGLES: std::cout << "TRIANGLES"; break;
            case TINYGLTF_MODE_TRIANGLE_STRIP: std::cout << "TRIANGLE_STRIP"; break;
            case TINYGLTF_MODE_TRIANGLE_FAN: std::cout << "TRIANGLE_FAN"; break;
            default: std::cout << "不明 (" << primitive.mode << ")"; break;
            }
            std::cout << std::endl;

            // 頂点属性
            std::cout << "    頂点属性:" << std::endl;
            for (const auto& attr : primitive.attributes) {
                std::cout << "      " << attr.first << " -> アクセサー " << attr.second << std::endl;
                analyzeAccessor(attr.second, 6);
            }

            // インデックス
            if (primitive.indices >= 0) {
                std::cout << "    インデックス -> アクセサー " << primitive.indices << std::endl;
                analyzeAccessor(primitive.indices, 6);
            }

            // マテリアル
            if (primitive.material >= 0) {
                std::cout << "    マテリアル: " << primitive.material << std::endl;
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

    std::cout << indentStr << "タイプ: ";
    switch (accessor.type) {
    case TINYGLTF_TYPE_SCALAR: std::cout << "SCALAR"; break;
    case TINYGLTF_TYPE_VEC2: std::cout << "VEC2"; break;
    case TINYGLTF_TYPE_VEC3: std::cout << "VEC3"; break;
    case TINYGLTF_TYPE_VEC4: std::cout << "VEC4"; break;
    case TINYGLTF_TYPE_MAT2: std::cout << "MAT2"; break;
    case TINYGLTF_TYPE_MAT3: std::cout << "MAT3"; break;
    case TINYGLTF_TYPE_MAT4: std::cout << "MAT4"; break;
    default: std::cout << "不明"; break;
    }
    std::cout << ", 成分タイプ: ";
    switch (accessor.componentType) {
    case TINYGLTF_COMPONENT_TYPE_BYTE: std::cout << "BYTE"; break;
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE: std::cout << "UNSIGNED_BYTE"; break;
    case TINYGLTF_COMPONENT_TYPE_SHORT: std::cout << "SHORT"; break;
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: std::cout << "UNSIGNED_SHORT"; break;
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT: std::cout << "UNSIGNED_INT"; break;
    case TINYGLTF_COMPONENT_TYPE_FLOAT: std::cout << "FLOAT"; break;
    default: std::cout << "不明"; break;
    }
    std::cout << ", 要素数: " << accessor.count << std::endl;

    // バウンディング情報
    if (!accessor.minValues.empty() && !accessor.maxValues.empty()) {
        std::cout << indentStr << "範囲: min(";
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
    std::cout << "\n--- マテリアル詳細解析 ---" << std::endl;

    for (size_t i = 0; i < m_model.materials.size(); ++i) {
        const auto& material = m_model.materials[i];
        std::cout << "マテリアル " << i;
        if (!material.name.empty()) {
            std::cout << " (\"" << material.name << "\")";
        }
        std::cout << ":" << std::endl;

        // PBRメタリックラフネス
        const auto& pbr = material.pbrMetallicRoughness;
        std::cout << "  PBRメタリックラフネス:" << std::endl;
        std::cout << "    ベースカラー: (" 
            << pbr.baseColorFactor[0] << ", " 
            << pbr.baseColorFactor[1] << ", " 
            << pbr.baseColorFactor[2] << ", " 
            << pbr.baseColorFactor[3] << ")" << std::endl;
        std::cout << "    メタリック係数: " << pbr.metallicFactor << std::endl;
        std::cout << "    ラフネス係数: " << pbr.roughnessFactor << std::endl;

        if (pbr.baseColorTexture.index >= 0) {
            std::cout << "    ベースカラーテクスチャ: " << pbr.baseColorTexture.index << std::endl;
        }
        if (pbr.metallicRoughnessTexture.index >= 0) {
            std::cout << "    メタリック/ラフネステクスチャ: " << pbr.metallicRoughnessTexture.index << std::endl;
        }

        // その他の特性
        if (material.normalTexture.index >= 0) {
            std::cout << "  法線テクスチャ: " << material.normalTexture.index << std::endl;
        }
        if (material.emissiveTexture.index >= 0) {
            std::cout << "  エミッシブテクスチャ: " << material.emissiveTexture.index << std::endl;
        }

        std::cout << "  エミッシブ係数: (" 
            << material.emissiveFactor[0] << ", " 
            << material.emissiveFactor[1] << ", " 
            << material.emissiveFactor[2] << ")" << std::endl;

        std::cout << "  アルファモード: " << material.alphaMode << std::endl;
        if (material.alphaMode == "MASK") {
            std::cout << "  アルファカットオフ: " << material.alphaCutoff << std::endl;
        }

        std::cout << "  両面描画: " << (material.doubleSided ? "有効" : "無効") << std::endl;
    }
}

void GLTFModel::analyzeBuffers()
{
    std::cout << "\n--- バッファー/アクセサー解析 ---" << std::endl;

    // バッファービューの詳細
    std::cout << "バッファービュー詳細:" << std::endl;
    for (size_t i = 0; i < m_model.bufferViews.size(); ++i) {
        const auto& bufferView = m_model.bufferViews[i];
        std::cout << "  ビュー " << i << ": バッファー " << bufferView.buffer 
            << ", オフセット " << bufferView.byteOffset 
            << ", 長さ " << bufferView.byteLength << " バイト";
        if (bufferView.byteStride > 0) {
            std::cout << ", ストライド " << bufferView.byteStride;
        }
        if (bufferView.target != 0) {
            std::cout << ", ターゲット ";
            switch (bufferView.target) {
            case TINYGLTF_TARGET_ARRAY_BUFFER: std::cout << "ARRAY_BUFFER"; break;
            case TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER: std::cout << "ELEMENT_ARRAY_BUFFER"; break;
            default: std::cout << bufferView.target; break;
            }
        }
        std::cout << std::endl;
    }
}

// glTF Modelの検証
bool GLTFModel::validateModel()
{
    if (!m_loaded) {
        std::cout << "モデルが読み込まれていません" << std::endl;
        return false;
    }

    std::cout << "\n=== モデル検証 ===" << std::endl;
    bool isValid = true;

    // 基本構造の検証
    if (m_model.scenes.empty()) {
        std::cerr << "エラー: シーンが定義されていません" << std::endl;
        isValid = false;
    }

    if (m_model.defaultScene >= static_cast<int>(m_model.scenes.size())) {
        std::cerr << "エラー: デフォルトシーンのインデックスが無効です" << std::endl;
        isValid = false;
    }

    // アクセサーの検証
    for (size_t i = 0; i < m_model.accessors.size(); ++i) {
        const auto& accessor = m_model.accessors[i];
        if (accessor.bufferView >= static_cast<int>(m_model.bufferViews.size())) {
            std::cerr << "エラー: アクセサー " << i << " の bufferView インデックスが無効です" << std::endl;
            isValid = false;
        }
    }

    // バッファービューの検証
    for (size_t i = 0; i < m_model.bufferViews.size(); ++i) {
        const auto& bufferView = m_model.bufferViews[i];
        if (bufferView.buffer >= static_cast<int>(m_model.buffers.size())) {
            std::cerr << "エラー: バッファービュー " << i << " の buffer インデックスが無効です" << std::endl;
            isValid = false;
        } else {
            const auto& buffer = m_model.buffers[bufferView.buffer];
            if (bufferView.byteOffset + bufferView.byteLength > buffer.data.size()) {
                std::cerr << "エラー: バッファービュー " << i << " がバッファー範囲を超えています" << std::endl;
                isValid = false;
            }
        }
    }

    // メッシュの検証
    for (size_t i = 0; i < m_model.meshes.size(); ++i) {
        const auto& mesh = m_model.meshes[i];
        for (size_t j = 0; j < mesh.primitives.size(); ++j) {
            const auto& primitive = mesh.primitives[j];

            // 必須属性の確認
            if (primitive.attributes.find("POSITION") == primitive.attributes.end()) {
                std::cerr << "エラー: メッシュ " << i << " プリミティブ " << j << " にPOSITION属性がありません" << std::endl;
                isValid = false;
            }

            // インデックスの検証
            if (primitive.indices >= static_cast<int>(m_model.accessors.size())) {
                std::cerr << "エラー: メッシュ " << i << " プリミティブ " << j << " のインデックスが無効です" << std::endl;
                isValid = false;
            }
        }
    }

    if (isValid) {
        std::cout << "O モデル検証が成功しました" << std::endl;
    } else {
        std::cout << "X モデル検証でエラーが見つかりました" << std::endl;
    }

    std::cout << "========================\n" << std::endl;
    return isValid;
}