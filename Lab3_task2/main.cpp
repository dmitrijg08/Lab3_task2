#include <windows.h>

// Ідентифікатори меню
#define IDM_DRAW 201
#define IDM_COLOR 202
#define IDM_SCALE 203
#define IDM_ABOUT 204
#define IDM_EXIT 205

// Глобальні змінні стану
bool isImageVisible = false;
COLORREF shipColor = RGB(139, 69, 19);
float currentScale = 1.0f;

// --- Графічні класи ---
class Sea {
public:
    void show(HDC dc, int width, int height) {
        HBRUSH brush = CreateSolidBrush(RGB(0, 105, 148));
        HGDIOBJ oldBrush = SelectObject(dc, brush);
        Rectangle(dc, 0, height / 2, width, height);
        SelectObject(dc, oldBrush);
        DeleteObject(brush);
    }
};

class Cloud {
public:
    void show(HDC dc, int X, int Y) {
        HBRUSH brush = CreateSolidBrush(RGB(255, 255, 255));
        HGDIOBJ oldBrush = SelectObject(dc, brush);
        HPEN pen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
        HGDIOBJ oldPen = SelectObject(dc, pen);

        Ellipse(dc, X, Y, X + 60, Y + 40);
        Ellipse(dc, X + 30, Y - 20, X + 90, Y + 30);
        Ellipse(dc, X + 60, Y + 10, X + 120, Y + 50);

        SelectObject(dc, oldBrush); SelectObject(dc, oldPen);
        DeleteObject(brush); DeleteObject(pen);
    }
};

class Ship {
public:
    void show(HDC dc, int X, int Y, COLORREF color) {
        HBRUSH hullBrush = CreateSolidBrush(color);
        HGDIOBJ oldBrush = SelectObject(dc, hullBrush);
        POINT hull[4] = { {X, Y}, {X + 100, Y}, {X + 80, Y + 30}, {X + 20, Y + 30} };
        Polygon(dc, hull, 4);

        HBRUSH mastBrush = CreateSolidBrush(RGB(0, 0, 0));
        SelectObject(dc, mastBrush);
        Rectangle(dc, X + 45, Y - 80, X + 55, Y);

        HBRUSH sailBrush = CreateSolidBrush(RGB(240, 240, 240));
        SelectObject(dc, sailBrush);
        POINT sail[3] = { {X + 55, Y - 70}, {X + 90, Y - 10}, {X + 55, Y - 10} };
        Polygon(dc, sail, 3);

        SelectObject(dc, oldBrush);
        DeleteObject(hullBrush); DeleteObject(mastBrush); DeleteObject(sailBrush);
    }
};

class Lighthouse {
public:
    void show(HDC dc, int X, int Y) {
        HBRUSH whiteBrush = CreateSolidBrush(RGB(255, 255, 255));
        HBRUSH redBrush = CreateSolidBrush(RGB(255, 0, 0));
        HGDIOBJ oldBrush = SelectObject(dc, whiteBrush);

        Rectangle(dc, X, Y, X + 60, Y + 40);
        SelectObject(dc, redBrush);
        Rectangle(dc, X + 5, Y - 40, X + 55, Y);
        SelectObject(dc, whiteBrush);
        Rectangle(dc, X + 10, Y - 80, X + 50, Y - 40);
        SelectObject(dc, redBrush);
        Rectangle(dc, X + 15, Y - 120, X + 45, Y - 80);

        HBRUSH grayBrush = CreateSolidBrush(RGB(100, 100, 100));
        SelectObject(dc, grayBrush);
        Rectangle(dc, X + 10, Y - 140, X + 50, Y - 120);

        HBRUSH yellowBrush = CreateSolidBrush(RGB(255, 255, 0));
        SelectObject(dc, yellowBrush);
        Ellipse(dc, X + 20, Y - 135, X + 40, Y - 115);

        SelectObject(dc, redBrush);
        POINT roof[3] = { {X, Y - 140}, {X + 60, Y - 140}, {X + 30, Y - 170} };
        Polygon(dc, roof, 3);

        SelectObject(dc, oldBrush);
        DeleteObject(whiteBrush); DeleteObject(redBrush);
        DeleteObject(grayBrush); DeleteObject(yellowBrush);
    }
};

void DrawScene(HDC dc, int width, int height) {
    // Режим розширеної графіки для масштабування
    SetGraphicsMode(dc, GM_ADVANCED);
    XFORM xForm = { 0 };
    xForm.eM11 = currentScale;
    xForm.eM22 = currentScale;
    SetWorldTransform(dc, &xForm);

    HBRUSH skyBrush = CreateSolidBrush(RGB(135, 206, 235));
    HGDIOBJ oldBrush = SelectObject(dc, skyBrush);
    Rectangle(dc, 0, 0, (int)(width / currentScale), (int)(height / 2 / currentScale));
    SelectObject(dc, oldBrush); DeleteObject(skyBrush);

    Sea sea; sea.show(dc, (int)(width / currentScale), (int)(height / currentScale));
    Cloud cloud; cloud.show(dc, 100, 50); cloud.show(dc, 400, 80); cloud.show(dc, 650, 40);
    Lighthouse lighthouse; lighthouse.show(dc, 80, 260);

    Ship ship;
    ship.show(dc, 300, 350, shipColor);
    ship.show(dc, 600, 450, shipColor);
}

// --- Віконна процедура ---
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_CREATE: {
        // Створення меню
        HMENU hMenu = CreateMenu();

        HMENU hMenuImage = CreatePopupMenu();
        AppendMenuW(hMenuImage, MF_STRING, IDM_DRAW, L"Рисунок...");

        HMENU hMenuTrans = CreatePopupMenu();
        AppendMenuW(hMenuTrans, MF_STRING, IDM_COLOR, L"Зміна кольору");
        AppendMenuW(hMenuTrans, MF_STRING, IDM_SCALE, L"Збільшення");

        HMENU hMenuInfo = CreatePopupMenu();
        AppendMenuW(hMenuInfo, MF_STRING, IDM_ABOUT, L"Про програму");
        AppendMenuW(hMenuInfo, MF_STRING, IDM_EXIT, L"Вихід");

        AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hMenuImage, L"Зображення");
        AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hMenuTrans, L"Трансформації");
        AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hMenuInfo, L"Інформація");

        SetMenu(hwnd, hMenu);
        break;
    }

    case WM_COMMAND: {
        int commandId = LOWORD(wParam);
        switch (commandId) {
        case IDM_DRAW:
            isImageVisible = true;
            InvalidateRect(hwnd, nullptr, TRUE);
            break;
        case IDM_COLOR:
            shipColor = RGB(255, 0, 0);
            InvalidateRect(hwnd, nullptr, TRUE);
            break;
        case IDM_SCALE:
            currentScale += 0.3f;
            if (currentScale > 2.5f) currentScale = 1.0f; // Скидання масштабу
            InvalidateRect(hwnd, nullptr, TRUE);
            break;
        case IDM_ABOUT:
            MessageBoxW(hwnd, L"Лабораторна робота №3\nВаріант 2", L"Про програму", MB_OK | MB_ICONINFORMATION);
            break;
        case IDM_EXIT:
            DestroyWindow(hwnd);
            break;
        }
        break;
    }

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC dc = BeginPaint(hwnd, &ps);

        if (isImageVisible) {
            RECT rect;
            GetClientRect(hwnd, &rect);
            DrawScene(dc, rect.right, rect.bottom);
        }

        EndPaint(hwnd, &ps);
        break;
    }

    case WM_DESTROY: {
        PostQuitMessage(0);
        break;
    }
    }
    return DefWindowProc(hwnd, message, wParam, lParam);
}

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow) {
    const wchar_t CLASS_NAME[] = L"Lab3Task2";

    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = CreateSolidBrush(RGB(255, 255, 255));

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0, CLASS_NAME, L"Лабораторна робота 3 - Завдання 2 (Меню)",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 900, 600,
        nullptr, nullptr, hInstance, nullptr);

    if (hwnd == nullptr) return 0;

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg = { 0 };
    while (GetMessage(&msg, nullptr, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}