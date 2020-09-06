// YMC.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//

#include "framework.h"
#include "YMC.h"

#define MAX_LOADSTRING 100

// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
HWND hWnd;
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.
NOTIFYICONDATA niData;
ControlServer *controlServer = nullptr;
thread *controlThread = nullptr;
HHOOK hHook;

// 이 코드 모듈에 포함된 함수의 선언을 전달합니다:
ATOM                MyRegisterClass(HINSTANCE hInstance);
HWND                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
void                CreateTray();
LRESULT CALLBACK    LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow) {
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // 전역 문자열 초기화
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_YMC, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);
    hInst = hInstance;

    // 애플리케이션 초기화
    if ((hWnd = InitInstance(hInstance, SW_HIDE)) == NULL) {
        return FALSE;
    }

    // 서비스 시작
    CreateTray();

    controlServer = new ControlServer();
    controlThread = new thread(&ControlServer::run, controlServer);

    hHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, hInstance, 0);

    // 기본 메시지 루프
    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_YMC));
    MSG msg;

    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance) {
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_YMC));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = NULL;
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

HWND InitInstance(HINSTANCE hInstance, int nCmdShow) {
   hInst = hInstance;

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (hWnd) {
       ShowWindow(hWnd, nCmdShow);
       UpdateWindow(hWnd);
   }

   return hWnd;
}

void CreateTray() {
    ZeroMemory(&niData, sizeof(NOTIFYICONDATA));

    niData.cbSize = sizeof(NOTIFYICONDATA);
    niData.hWnd = hWnd;

    niData.uID = ID_TRAY_ICON;
    niData.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_YMC));

    niData.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    niData.uCallbackMessage = WM_EX_MESSAGE;

    wsprintf(niData.szTip, L"Youtube Music Controller");

    Shell_NotifyIcon(NIM_ADD, &niData);
}

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (controlServer != nullptr && nCode == HC_ACTION && wParam == WM_KEYDOWN && GetAsyncKeyState(VK_OEM_5)) {
        KBDLLHOOKSTRUCT *p = (KBDLLHOOKSTRUCT *)lParam;

        switch (p->vkCode) {
        case 0x50:      controlServer->sendPrev();   break;
        case VK_OEM_4:  controlServer->sendPause();  break;
        case VK_OEM_6:  controlServer->sendNext();   break;
        }
    }
    return CallNextHookEx(hHook, nCode, wParam, lParam);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
    case WM_EX_MESSAGE:
        {
            if (wParam == ID_TRAY_ICON) {
                switch (lParam) {
                case WM_RBUTTONUP:
                    {
                        HMENU hMenu = CreatePopupMenu();
                        AppendMenu(hMenu, MF_STRING, ID_TRAY_EXIT_CONTEXT_MENU_ITEM, L"종료");

                        POINT pt;
                        GetCursorPos(&pt);

                        SetForegroundWindow(hWnd);
                        TrackPopupMenu(hMenu, TPM_LEFTALIGN, pt.x, pt.y, 0, hWnd, NULL);
                        SetForegroundWindow(hWnd);
                    }
                    break;
                }
            }
        }
        break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            switch (wmId)
            {
            case IDM_EXIT:
            case ID_TRAY_EXIT_CONTEXT_MENU_ITEM:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_DESTROY:
        UnhookWindowsHookEx(hHook);

        if (controlServer != nullptr && controlThread != nullptr) {
            controlServer->stop();
            controlThread->join();

            delete controlThread;
            delete controlServer;

            controlThread = nullptr;
            controlServer = nullptr;
        }

        Shell_NotifyIcon(NIM_DELETE, &niData);
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
