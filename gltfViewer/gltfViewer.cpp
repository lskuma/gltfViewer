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

#include "Camera.h"
#include "OpenGLRenderer.h"
#include "GLTFModel.h"
#include "UtilFunc.h"

// グローバル変数
OpenGLRenderer* g_renderer = nullptr;
GLTFModel* g_gltfModel = nullptr;
Camera* g_camera = nullptr;
bool g_running = true;

// マウス入力制御用グローバル変数
bool g_mousePressed = false;
int g_lastMouseX = 0;
int g_lastMouseY = 0;
const float g_mouseSensitivity = 0.005f;  // マウス感度
const float g_movementSpeed = 0.1f;        // カメラ移動速度

// キーボード入力状態
bool g_keyStates[256] = { false };

// キーボード入力処理関数
void processKeyboardInput() {
    if (!g_camera) return;

    // WASD移動
    if (g_keyStates['W'] || g_keyStates['w']) {
        g_camera->moveForward(g_movementSpeed);
    }
    if (g_keyStates['S'] || g_keyStates['s']) {
        g_camera->moveBackward(g_movementSpeed);
    }
    if (g_keyStates['A'] || g_keyStates['a']) {
        g_camera->moveLeft(g_movementSpeed);
    }
    if (g_keyStates['D'] || g_keyStates['d']) {
        g_camera->moveRight(g_movementSpeed);
    }

    // QE上下移動
    if (g_keyStates['Q'] || g_keyStates['q']) {
        g_camera->moveUp(g_movementSpeed);
    }
    if (g_keyStates['E'] || g_keyStates['e']) {
        g_camera->moveDown(g_movementSpeed);
    }

    // カメラ更新をレンダラーに反映
    if (g_renderer) {
        g_renderer->updateCamera(g_camera);
    }
}

// ウィンドウプロシージャ
LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_CREATE:
        // カメラを初期化
        g_camera = new Camera();
        g_camera->setPerspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

        g_renderer = new OpenGLRenderer(hWnd);
        if (!g_renderer->initialize()) {
            std::cerr << "OpenGLレンダラーの初期化に失敗しました" << std::endl;
            return -1;
        } else {
            // カメラをレンダラーに設定
            g_renderer->updateCamera(g_camera);

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

            // カメラのアスペクト比も更新
            if (g_camera) {
                g_camera->setAspectRatio((float)width / (float)height);
                g_renderer->updateCamera(g_camera);
            }

            InvalidateRect(hWnd, nullptr, FALSE);
        }
        break;
    }

                // キーボード入力処理
    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE) {
            PostMessage(hWnd, WM_CLOSE, 0, 0);
        } else if (wParam < 256) {
            g_keyStates[wParam] = true;
        }
        break;

    case WM_KEYUP:
        if (wParam < 256) {
            g_keyStates[wParam] = false;
        }
        break;

        // マウス入力処理
    case WM_LBUTTONDOWN: {
            g_mousePressed = true;
            g_lastMouseX = LOWORD(lParam);
            g_lastMouseY = HIWORD(lParam);
            SetCapture(hWnd);  // マウスキャプチャを開始
            break;
    }

    case WM_LBUTTONUP: {
        g_mousePressed = false;
        ReleaseCapture();  // マウスキャプチャを終了
        break;
    }

    case WM_MOUSEMOVE: {
        int mouseX = LOWORD(lParam);
        int mouseY = HIWORD(lParam);

        if (g_mousePressed && g_camera) {
            // マウスの移動量を計算
            int deltaX = mouseX - g_lastMouseX;
            int deltaY = mouseY - g_lastMouseY;

            // カメラの回転に反映（glm::rotate()を使用した回転制御）
            g_camera->addYaw(-deltaX * g_mouseSensitivity);   // 水平回転（ヨー）
            g_camera->addPitch(-deltaY * g_mouseSensitivity); // 垂直回転（ピッチ）

            // カメラ更新をレンダラーに反映
            if (g_renderer) {
                g_renderer->updateCamera(g_camera);
            }

            // 再描画をトリガー
            InvalidateRect(hWnd, nullptr, FALSE);
        }

        g_lastMouseX = mouseX;
        g_lastMouseY = mouseY;
        break;
    }

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
        if(g_camera) {
            delete g_camera;
            g_camera = nullptr;
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
        _T("gltfViewer - WASD移動, マウスドラッグ回転, ESC終了"),
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

        // キーボード入力処理（連続入力対応）
        processKeyboardInput();

        // 連続レンダリング
        if (g_renderer && g_running) {
            g_renderer->render();
        }

        // CPU使用率100%を防ぐための短いスリープ
        Sleep(1);
    }

    return 0;
}