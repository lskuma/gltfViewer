#include <windows.h>
#include <tchar.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include <iostream>
#include <string>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#define TINYGLTF_IMPLEMENTATION
#include <tiny_gltf.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include "OpenGLRenderer.h"
#include "GLTFModel.h"

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
    std::cout << "ESCキーで終了" << std::endl;
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

// グローバル変数
OpenGLRenderer* g_renderer = nullptr;
GLTFModel* g_gltfModel = nullptr;
bool g_running = true;

// ウィンドウプロシージャ
LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_CREATE:
        g_renderer = new OpenGLRenderer(hWnd);
        if (!g_renderer->initialize()) {
            std::cerr << "OpenGLレンダラーの初期化に失敗しました" << std::endl;
            return -1;
        } else {
            if(g_gltfModel != nullptr && g_gltfModel->validateModel())
            {
                g_renderer->loadGLTFModel(g_gltfModel->getModel());
            }
        }
        break;

    case WM_PAINT: {
            PAINTSTRUCT ps;
            BeginPaint(hWnd, &ps);
            if (g_renderer) {
                g_renderer->render();
            }
            EndPaint(hWnd, &ps);
            return 0;
    }

    case WM_SIZE: {
        int width = LOWORD(lParam);
        int height = HIWORD(lParam);
        if (g_renderer && width > 0 && height > 0) {
            g_renderer->onResize(width, height);
            InvalidateRect(hWnd, nullptr, FALSE);
        }
        break;
    }

    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE) {
            PostMessage(hWnd, WM_CLOSE, 0, 0);
        }
        break;

    case WM_CLOSE:
        g_running = false;
        DestroyWindow(hWnd);
        break;

    case WM_DESTROY:
        if (g_renderer) {
            delete g_renderer;
            g_renderer = nullptr;
        }
        if(g_gltfModel) {
            delete g_gltfModel;
            g_gltfModel = nullptr;
        }
        g_running = false;
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
    return 0;
}

int main(int argc, char* argv[]) {
    // コマンドライン引数を処理してglTFファイルパスを取得
    std::string gltfFilePath = processCommandLineArgs(argc, argv);

    bool isDemo = gltfFilePath.empty();
    if (isDemo) {
        std::cout << std::endl << "デモモードで実行中 - カラフルな三角形を表示します。" << std::endl;
    } else {
        g_gltfModel = new GLTFModel();
        g_gltfModel->loadFromFile(gltfFilePath);
        g_gltfModel->analyzeStructure();
    }

    // インスタンスハンドルを取得
    HINSTANCE hInstance = GetModuleHandle(nullptr);

    // ウィンドウクラスを登録
    LPCTSTR className = _T("OpenGLWindow");
    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = className;

    if (!RegisterClassEx(&wc)) {
        std::cerr << "ウィンドウクラスの登録に失敗しました" << std::endl;
        return -1;
    }

    // ウィンドウを作成
    HWND hWnd = CreateWindowEx(
        0,
        className,
        _T("gltfViewer"),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        800, 600,
        nullptr,
        nullptr,
        hInstance,
        nullptr
    );

    if (!hWnd) {
        std::cerr << "ウィンドウの作成に失敗しました" << std::endl;
        return -1;
    }

    // ウィンドウを表示
    ShowWindow(hWnd, SW_SHOWDEFAULT);
    UpdateWindow(hWnd);

    // 連続レンダリング付きメッセージループ
    MSG msg = {};
    while (g_running) {
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        // 連続レンダリング
        if (g_renderer && g_running) {
            g_renderer->render();
        }

        // CPU使用率100%を防ぐための短いスリープ
        Sleep(1);
    }

    return 0;
}