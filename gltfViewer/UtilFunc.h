#pragma once

// ファイルが存在するかをチェックする関数
bool fileExists(const std::string& filename) {
    DWORD dwAttrib = GetFileAttributesA(filename.c_str());
    return (dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

// glTFファイル拡張子を検証する関数
bool isValidGltfFile(const std::string& filename) {
    if (filename.length() < 5) return false;

    std::string extension = filename.substr(filename.length() - 5);
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

    return (extension == ".gltf" || filename.substr(filename.length() - 4) == ".glb");
}

// コマンドライン引数を処理する関数
std::string processCommandLineArgs(int argc, char* argv[]) {
    std::cout << "glTFビューアー - OpenGLレンダラー" << std::endl;
    std::cout << "使用方法: " << argv[0] << " <gltfファイルパス>" << std::endl;
    std::cout << "サポート形式: .gltf, .glb" << std::endl;
    std::cout << "操作方法:" << std::endl;
    std::cout << "  ESC: 終了" << std::endl;
    std::cout << "  WASD: カメラ移動 (前後左右)" << std::endl;
    std::cout << "  QE: カメラ上下移動" << std::endl;
    std::cout << "  マウス左ドラッグ: カメラ回転" << std::endl;
    std::cout << "  マウスホイール: ズーム (未実装)" << std::endl;
    std::cout << std::endl;

    // 引数の数をチェック
    if (argc < 2) {
        std::cout << "glTFファイルが指定されていません。三角形でデモモードを実行します。" << std::endl;
        std::cout << "glTFファイルを読み込むには、ファイルパスを引数として指定してください。" << std::endl;
        return ""; // 空文字列はデモモードを示す
    }

    if (argc > 2) {
        std::cout << "警告: 複数の引数が指定されました。最初の引数をglTFファイルパスとして使用します。" << std::endl;
    }

    std::string gltfFilePath = argv[1];
    std::cout << "glTFファイルの読み込みを試行中: " << gltfFilePath << std::endl;

    // ファイル拡張子を検証
    if (!isValidGltfFile(gltfFilePath)) {
        std::cerr << "エラー: 無効なファイル拡張子です。.gltfまたは.glbファイルが必要です。" << std::endl;
        std::cerr << "指定されたファイル: " << gltfFilePath << std::endl;
        return ""; // デモモードで続行するため空文字列を返す
    }

    // ファイルが存在するかチェック
    if (!fileExists(gltfFilePath)) {
        std::cerr << "エラー: ファイルが存在しないか、アクセスできません。" << std::endl;
        std::cerr << "ファイルパス: " << gltfFilePath << std::endl;

        // 役立つエラー情報を提供する試み
        size_t lastSlash = gltfFilePath.find_last_of("\\/");
        if (lastSlash != std::string::npos) {
            std::string directory = gltfFilePath.substr(0, lastSlash);
            std::string filename = gltfFilePath.substr(lastSlash + 1);
            std::cerr << "ディレクトリ: " << directory << std::endl;
            std::cerr << "ファイル名: " << filename << std::endl;
        }

        return ""; // デモモードで続行するため空文字列を返す
    }

    // 追加検証のためファイルサイズを取得
    HANDLE hFile = CreateFileA(gltfFilePath.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile != INVALID_HANDLE_VALUE) {
        LARGE_INTEGER fileSize;
        if (GetFileSizeEx(hFile, &fileSize)) {
            std::cout << "ファイルサイズ: " << fileSize.QuadPart << " バイト" << std::endl;

            if (fileSize.QuadPart == 0) {
                std::cerr << "エラー: ファイルが空です。" << std::endl;
                CloseHandle(hFile);
                return "";
            }

            if (fileSize.QuadPart > 100 * 1024 * 1024) { // 安全のため100MB制限
                std::cout << "警告: 大きなファイルが検出されました（>100MB）。読み込みに時間がかかる場合があります。" << std::endl;
            }
        }
        CloseHandle(hFile);
    }

    std::cout << "ファイル検証が成功しました。glTFファイルの読み込み準備完了。" << std::endl;
    return gltfFilePath;
}