#include <BC.hpp>

using namespace std;

#define MAX_LOADSTRING 100

// 전역 변수
HINSTANCE hInst;
HWND hWnd;
WCHAR szTitle[MAX_LOADSTRING];
WCHAR szWindowClass[MAX_LOADSTRING];

NOTIFYICONDATA niData;
HHOOK hHook;

ControlServer *controlServer = nullptr;
thread *controlThread = nullptr;

// 함수
ATOM                MyRegisterClass(HINSTANCE hInstance);
HWND                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

void                CreateTray();
LRESULT CALLBACK    LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);


int APIENTRY WinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow) {
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // 전역 문자열 초기화
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_BC, szWindowClass, MAX_LOADSTRING);
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
    MSG msg;

    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
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
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_BC));
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
    niData.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_BC));

    niData.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    niData.uCallbackMessage = WM_EX_MESSAGE;

    sprintf_s(niData.szTip, "Browser Controller");

    Shell_NotifyIcon(NIM_ADD, &niData);
}

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (controlServer != nullptr && nCode == HC_ACTION && wParam == WM_KEYDOWN && GetAsyncKeyState(VK_OEM_5)) {
        KBDLLHOOKSTRUCT *p = (KBDLLHOOKSTRUCT *)lParam;

        switch (p->vkCode) {
        case 0x50:          controlServer->sendPrev();          break; // p
        case 0x58:          controlServer->sendSpeedDown();     break; // x
        case 0x43:          controlServer->sendSpeedUp();       break; // c
        case VK_OEM_4:      controlServer->sendPause();         break; // [
        case VK_OEM_6:      controlServer->sendNext();          break; // ]
        case VK_OEM_3:      controlServer->sendStartMix();      break; // ~
        case VK_OEM_MINUS:  controlServer->sendVolumeDown();    break; // -
        case VK_OEM_PLUS:   controlServer->sendVolumeUp();      break; // +
        case 0x49:          controlServer->sendRepeat();               // i
                            controlServer->sendTheatreMode();   break;
        case 0x4F:          controlServer->sendShuffle();              // o
                            controlServer->sendExtendChatting();break;
        case 0xBC:          controlServer->sendBackward();      break; // <
        case 0xBE:          controlServer->sendForward();       break; // >
        default:
            if (0x31 <= p->vkCode && p->vkCode <= 0x39) { // 1 ~ 9
                int index = p->vkCode - 0x30;
                controlServer->sendStart(index);
            }
            break;
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
                        AppendMenuA(hMenu, MF_STRING, ID_TRAY_EXIT_CONTEXT_MENU_ITEM, "종료");

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
