#include <windows.h>
#include <windowsx.h>
#include <cstdlib>
#include <vector>
#include <string>
#include <ctime>
#pragma comment(lib, "Msimg32.lib")

//#include "Events.h"
#include "Logic.h"

#define FIELD_FILE_NAME "Field.bmp"
#define GEM_1_FILE_NAME "Gem_1.bmp"
#define GEM_2_FILE_NAME "Gem_2.bmp"
#define GEM_3_FILE_NAME "Gem_3.bmp"
#define GEM_4_FILE_NAME "Gem_4.bmp"
#define GEM_5_FILE_NAME "Gem_5.bmp"
#define COLOR_NUM 5
#define WND_CLIENT_SIZE 504
#define WND_POS 200
#define TIME_DEVIDER 40000
#define MX_RESOLUTION 8
#define MASK_COLOR RGB(255, 0, 255)
#define GEM_SIZE 23

class Timer {
public:
    Timer() {
        start_time = 0;
        end_time = 0;
    }
    void Reset()
    {
        LARGE_INTEGER li;
        QueryPerformanceCounter(&li);
        start_time = li.LowPart / TIME_DEVIDER;
    }
    int GetElepsed()
    {
        LARGE_INTEGER li;
        QueryPerformanceCounter(&li);
        end_time = li.LowPart / TIME_DEVIDER;
        return end_time - start_time;
    }
private:
    int start_time;
    int end_time;
};


class Gem {
public:
    POINT pos;
    bool is_active;
    //HBITMAP pikcha;
    int type;
    int destruction_phase;
    Gem()
    {
        is_active = FALSE;
        pos.x = 0;
        pos.y = 0;
        type = 0;
        destruction_phase = 0;
    }
    //void Reactivate() { is_active = !is_active; }
};


typedef std::vector<std::vector<Gem*>> TGemMx;
typedef std::vector<HBITMAP> HBITMAPS;

struct StateInfo {
    TGemMx* pGems;
    HBITMAPS pikchas;
    bool* IsProcessing;
};

HBITMAPS LoadBMPs(std::vector<std::string> names);
void CreateGemMX(TGemMx& gem_mx, TIntMx vals, POINT cell_size);
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void LParamToPoint(LPARAM lParam, POINT& point);
bool CheckActive(TGemMx Gems, POINTSS& SwapCells);
void SwapGems(TGemMx& SwapMx, POINT c1, POINT c2);
bool SwapGemsVis(TGemMx gems, POINT c1, POINT c2, POINT cell_size, int time);
void ValsToGems(const TIntMx Vals, const TGemMx Gems);
void GemsToVals(TIntMx& Vals, const TGemMx Gems);
void Falling(TGemMx& Gems);
void TPGems(TGemMx Gems, POINT cell_size);
bool BlowVis(TGemMx Gems);
void AddGems(TGemMx Gems);
bool FallingVis(TGemMx Gems, POINT cell_size, int time);

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR pCmdLine, int nCmdShow)
{
    // Register the window class.
    const wchar_t CLASS_NAME[] = L"My Window Class";

    WNDCLASS wc = { };

    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.lpszMenuName = 0;
    wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    RegisterClass(&wc);

    // Create the window.

    StateInfo st_inf;
    RECT rect;
    rect.left = 0;
    rect.top = 0;
    rect.right = WND_CLIENT_SIZE;
    rect.bottom = WND_CLIENT_SIZE;

    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, NULL);

    HWND hwnd = CreateWindowEx(
        0,                              // Optional window styles.
        CLASS_NAME,                     // Window class
        L"Match3",                      // Window text
        WS_OVERLAPPEDWINDOW,            // Window style

        // Size and position
        WND_POS, WND_POS, WND_POS + rect.right - rect.left, WND_POS + rect.bottom - rect.top,

        NULL,       // Parent window    
        NULL,       // Menu
        hInstance,  // Instance handle
        &st_inf     // Additional application data
    );

    if (hwnd == NULL)
    {
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);

    //инициализация поля
    TGemMx Gems;
    TIntMx Vals;
    CreateMx(Vals, 8, COLOR_NUM);
    while (Check(Vals))
    {
        Falling(Vals);
        AddGems(Vals);
    }
    RECT client_rect;
    GetClientRect(hwnd, &client_rect);
    POINT cell_size;
    cell_size.x = (client_rect.right - client_rect.left) / MX_RESOLUTION;
    cell_size.y = (client_rect.bottom - client_rect.top) / MX_RESOLUTION;
    CreateGemMX(Gems, Vals, cell_size);
    std::vector<std::string> file_names;
    file_names.resize(1 + COLOR_NUM);
    file_names[0] = FIELD_FILE_NAME;
    file_names[1] = GEM_1_FILE_NAME;
    file_names[2] = GEM_2_FILE_NAME;
    file_names[3] = GEM_3_FILE_NAME;
    file_names[4] = GEM_4_FILE_NAME;
    file_names[5] = GEM_5_FILE_NAME;
    HBITMAPS pikchas = LoadBMPs(file_names);
    st_inf.pGems = &Gems;
    st_inf.pikchas = pikchas;
    bool IsProcessing, IsBlowing = false, ReSwapping = false, IsFalling = false, IsSwapping = false;
    st_inf.IsProcessing = &IsProcessing;
    POINTSS swap_gems;
    swap_gems.resize(2);
    Timer clock = Timer();
    clock.Reset();
    int elepsed_time;
    int blow_time = 0;
    // Run the message loop.

    MSG msg = { };
    while (TRUE)
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT) break;
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            elepsed_time = clock.GetElepsed();
            clock.Reset();
            IsProcessing = IsSwapping;
            if (!IsProcessing)
            {
                IsSwapping = CheckActive(Gems, swap_gems);
                if (IsSwapping)
                    SwapGems(Gems, swap_gems[0], swap_gems[1]);
                IsProcessing = IsSwapping;
            }
            if (IsSwapping)
            {
                IsSwapping = SwapGemsVis(Gems, swap_gems[0], swap_gems[1], cell_size, elepsed_time);
                if (!IsSwapping && !ReSwapping)
                {
                    GemsToVals(Vals, Gems);
                    IsSwapping = !Check(Vals);
                    ReSwapping = IsSwapping;
                    if (IsSwapping)
                        SwapGems(Gems, swap_gems[0], swap_gems[1]);
                    IsBlowing = !IsSwapping;
                    if (IsBlowing)
                    {
                        ValsToGems(Vals, Gems);
                        //TimeToBlow = TimeToBlow.Zero;
                    }
                }
                if (!IsSwapping && ReSwapping)
                    ReSwapping = false;
                IsProcessing = IsSwapping;
                InvalidateRect(hwnd, NULL, FALSE);
            }
            if (IsBlowing)
            {
                blow_time += elepsed_time;
                if (blow_time > 20)
                {
                    IsBlowing = BlowVis(Gems);
                    blow_time = 0;
                }
                if (!IsBlowing)
                {
                    TPGems(Gems, cell_size);
                    Falling(Gems);
                    AddGems(Gems);
                    IsFalling = true;
                }
                IsProcessing = IsBlowing;
                InvalidateRect(hwnd, NULL, FALSE);
            }
            if(IsFalling)
            {
                IsFalling = FallingVis(Gems, cell_size, elepsed_time);
                IsProcessing = IsFalling;
                InvalidateRect(hwnd, NULL, FALSE);
                if (!IsProcessing)
                {
                    GemsToVals(Vals, Gems);
                    IsBlowing = Check(Vals);
                    ValsToGems(Vals, Gems);
                    IsProcessing = IsBlowing;
                }
            }
        }

    return 0;
}

HBITMAPS LoadBMPs(std::vector<std::string> names)
{
    HBITMAPS result;
    result.resize(names.size());
    for (int i = 0; i < names.size(); i++)
        result[i] = (HBITMAP)LoadImageA(NULL, names[i].c_str(), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    return result;
}

void CreateGemMX(TGemMx & gem_mx, TIntMx vals, POINT cell_size)
{
    gem_mx.resize(vals.size());
    for (int i = 0; i < vals.size(); i++)
        gem_mx[i].resize(vals[i].size());
    for (int i = 0; i < gem_mx.size(); i++)
        for (int j = 0; j < gem_mx[i].size(); j++)
        {
            gem_mx[i][j] = new Gem();
            gem_mx[i][j]->type = vals[i][j];
            gem_mx[i][j]->pos.x = cell_size.x * j;
            gem_mx[i][j]->pos.y = cell_size.y * i;
        }
}

void AddGems(TGemMx Gems) 
{
    srand(time(NULL));
    for (int i = 0; i < Gems.size(); i++)
        for (int j = 0; j < Gems[i].size(); j++)
            if (Gems[i][j]->type < 0)
                Gems[i][j]->type = rand() % COLOR_NUM + 1;
}

bool FallingVis(TGemMx Gems, POINT cell_size, int time)
{
    bool Result = false;
    for (int i = 0; i < Gems.size(); i++)
        for (int j = 0; j < Gems[i].size(); j++)
            if (Gems[i][j]->pos.y < i * cell_size.y)
            {
                Gems[i][j]->pos.y += time;
                Result = true;
            }
    if (!Result)
        for (int i = 0; i < Gems.size(); i++)
            for (int j = 0; j < Gems[i].size(); j++)
                Gems[i][j]->pos.y = i * cell_size.y;
    return Result;
}

bool BlowVis(TGemMx Gems)
{
    for (int i = 0; i < Gems.size(); i++)
        for (int j = 0; j < Gems[i].size(); j++)
            if (Gems[i][j]->type < 0)
                if (Gems[i][j]->destruction_phase < 8)
                    Gems[i][j]->destruction_phase++;
                else
                    return false;
    return true;
}

void TPGems(TGemMx Gems, POINT cell_size)
{
    for (int j = 0; j < Gems[0].size(); j++)
    {
        int counter = 0;
        for (int i = 0; i < Gems.size(); i++)
            if (Gems[i][j]->type < 0)
            {
                counter++;
                Gems[i][j]->destruction_phase = 0;
                Gems[i][j]->pos.y = -counter * cell_size.y;
            }
    }
}

void Falling(TGemMx& Gems)
{
    for (int j = 0; j < Gems[0].size(); j++)
        for (int k = 0; k < Gems.size(); k++)
            for (int i = Gems.size() - 1; i > k; i--)
                if (Gems[i][j]->type < 0)
                {
                    Gem* buff = Gems[i][j];
                    Gems[i][j] = Gems[i - 1][j];
                    Gems[i - 1][j] = buff;
                }
}

bool CheckActive(TGemMx Gems, POINTSS& SwapCells)
{
    SwapCells.resize(2);
    int k = 0;
    for (int i = 0; i < Gems.size(); i++)
        for (int j = 0; j < Gems[i].size(); j++)
            if (Gems[i][j]->is_active)
            {
                SwapCells[k].x = j;
                SwapCells[k].y = i;
                k++;
            }
    k == 2;
    return k == 2;
}

void SwapGems(TGemMx& SwapMx, POINT c1, POINT c2)
{
    Gem* buff = SwapMx[c1.y][c1.x];
    SwapMx[c1.y][c1.x] = SwapMx[c2.y][c2.x];
    SwapMx[c2.y][c2.x] = buff;
    SwapMx[c1.y][c1.x]->is_active = false;
    SwapMx[c2.y][c2.x]->is_active = false;
}

bool SwapGemsVis(TGemMx gems, POINT c1, POINT c2, POINT cell_size, int time)
{
    POINT dir;
    dir.x = c2.x - c1.x;
    dir.y = c2.y - c1.y;
    gems[c1.y][c1.x]->pos.x -= dir.x * time;
    gems[c1.y][c1.x]->pos.y -= dir.y * time;
    gems[c2.y][c2.x]->pos.x += dir.x * time;
    gems[c2.y][c2.x]->pos.y += dir.y * time;
    if ((gems[c1.y][c1.x]->pos.x < cell_size.x * c1.x) || (gems[c1.y][c1.x]->pos.y < cell_size.y * c1.y))
    {
        gems[c1.y][c1.x]->pos.x = cell_size.x * c1.x;
        gems[c1.y][c1.x]->pos.y = cell_size.y * c1.y;
        gems[c2.y][c2.x]->pos.x = cell_size.x * c2.x;
        gems[c2.y][c2.x]->pos.y = cell_size.y * c2.y;
        return false;
    }
    return true;;
}

void ValsToGems(const TIntMx Vals, const TGemMx Gems)
{
    for (int i = 0; i < Vals.size(); i++)
        for (int j = 0; j < Vals[i].size(); j++)
            Gems[i][j]->type = Vals[i][j];
}

void GemsToVals(TIntMx& Vals, const TGemMx Gems)
{
    for (int i = 0; i < Vals.size(); i++)
        for (int j = 0; j < Vals[i].size(); j++)
            Vals[i][j] = Gems[i][j]->type;
}

inline StateInfo* GetAppState(HWND hwnd)
{
    LONG_PTR ptr = GetWindowLongPtr(hwnd, GWLP_USERDATA);
    StateInfo* pState = reinterpret_cast<StateInfo*>(ptr);
    return pState;
}

void OnPaint(HWND hwnd, HBITMAPS pikchas, TGemMx gems)
{
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);
    HDC buff_hdc = CreateCompatibleDC(hdc);

    RECT client_rect;
    GetClientRect(hwnd, &client_rect);
    POINT cell;
    cell.x = (client_rect.right - client_rect.left) / MX_RESOLUTION;
    cell.y = (client_rect.bottom - client_rect.top) / MX_RESOLUTION;

    HBITMAP hbmp = CreateCompatibleBitmap(hdc,
        client_rect.right - client_rect.left,
        client_rect.bottom - client_rect.top);
    SelectObject(buff_hdc, hbmp);

    //рисуем игровое поле на главный буфер
    BITMAP pict_bmp;
    HBITMAP hbmp1 = pikchas[0];
    GetObject(hbmp1, sizeof(pict_bmp), &pict_bmp);
    HDC buff_hdc1 = CreateCompatibleDC(buff_hdc);
    SelectObject(buff_hdc1, hbmp1);
    StretchBlt(buff_hdc, 0, 0, client_rect.right,
        client_rect.bottom, buff_hdc1, 0, 0, pict_bmp.bmWidth, pict_bmp.bmHeight, SRCCOPY);

    //цикл отрисовки каждого кристалла
    for (int i = 0; i < gems.size(); i++)
        for (int j = 0; j < gems[i].size(); j++)
        {
            HBITMAP hbmp2 = pikchas[abs(gems[i][j]->type)];
            GetObject(hbmp2, sizeof(pict_bmp), &pict_bmp);
            HDC buff_hdc2 = CreateCompatibleDC(buff_hdc);
            HDC buff_hdc3 = CreateCompatibleDC(buff_hdc);
            HBITMAP hbmp3 = CreateCompatibleBitmap(buff_hdc, client_rect.right - client_rect.left,
                client_rect.bottom - client_rect.top);
            SelectObject(buff_hdc3, hbmp3);
            SelectObject(buff_hdc2, hbmp2);
            StretchBlt(buff_hdc3, 0, 0, cell.x, cell.y, buff_hdc2,
                GEM_SIZE * gems[i][j]->destruction_phase, 0, GEM_SIZE, GEM_SIZE, SRCCOPY);
            TransparentBlt(buff_hdc, gems[i][j]->pos.x, gems[i][j]->pos.y,
                cell.x, cell.y,
                buff_hdc3, 0, 0, cell.x, cell.y, MASK_COLOR);
            DeleteDC(buff_hdc2);
            DeleteDC(buff_hdc3);
            DeleteObject(hbmp3);
        }
    BitBlt(hdc, 0, 0, client_rect.right - client_rect.left,
        client_rect.bottom - client_rect.top, buff_hdc, 0, 0, SRCCOPY);
    DeleteObject(hbmp);
    DeleteDC(buff_hdc);
    DeleteDC(buff_hdc1);

    EndPaint(hwnd, &ps);
}

void OnLeftButtonDown(HWND hwnd, TGemMx Gems, POINT m_pos, bool IsProcessing)
{
    if (!IsProcessing)
    {
        POINT BeforeActive;
        BeforeActive.x = -1;
        for (int i = 0; i < Gems.size(); i++)
            for (int j = 0; j < Gems[i].size(); j++)
            {
                if (Gems[i][j]->is_active)
                {
                    BeforeActive.x = j;
                    BeforeActive.y = i;
                }
            }
        RECT client_rect;
        GetClientRect(hwnd, &client_rect);
        POINT cell_size;
        cell_size.x = (client_rect.right - client_rect.left) / MX_RESOLUTION;
        cell_size.y = (client_rect.bottom - client_rect.top) / MX_RESOLUTION;
        POINT gem_cell;
        gem_cell.x = m_pos.x / cell_size.x;
        gem_cell.y = m_pos.y / cell_size.y;
        Gems[gem_cell.y][gem_cell.x]->is_active = !(Gems[gem_cell.y][gem_cell.x]->is_active);
        if ((BeforeActive.x >= 0) && (Gems[gem_cell.y][gem_cell.x]->is_active))
            if (!((BeforeActive.x == gem_cell.x) && (abs(BeforeActive.y - gem_cell.y) == 1)) &&
                !((BeforeActive.y == gem_cell.y) && (abs(BeforeActive.x - gem_cell.x) == 1)))
                Gems[BeforeActive.y][BeforeActive.x]->is_active = false;
    }

}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    StateInfo* pState;
    if (uMsg == WM_CREATE)
    {
        CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
        pState = reinterpret_cast<StateInfo*>(pCreate->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pState);
    }
    else
    {
        pState = GetAppState(hwnd);
    }

    switch (uMsg)
    {
    case WM_DESTROY:
    {
        PostQuitMessage(0);
        return 0;
    }

    case WM_PAINT:
    {
        OnPaint(hwnd, pState->pikchas, *(pState->pGems));
        return 0;
    }

    case WM_GETMINMAXINFO:
    {
        LPMINMAXINFO lpMMI = (LPMINMAXINFO)lParam;
        RECT rect;
        rect.left = 0;
        rect.top = 0;
        rect.right = WND_CLIENT_SIZE;
        rect.bottom = WND_CLIENT_SIZE;
        AdjustWindowRect
        (
            &rect,		// указатель на структуру
                        // прямоугольника пользователя
            WS_OVERLAPPEDWINDOW,		// стиль окон
            NULL 		// флажок показа меню
        );
        lpMMI->ptMinTrackSize.x = rect.right - rect.left;
        lpMMI->ptMinTrackSize.y = rect.bottom - rect.top;
        lpMMI->ptMaxTrackSize.x = rect.right - rect.left;
        lpMMI->ptMaxTrackSize.y = rect.bottom - rect.top;
        return 0;
    }

    case WM_LBUTTONDOWN:
    {
        POINT m_pos;
        LParamToPoint(lParam, m_pos);
        OnLeftButtonDown(hwnd, *(pState->pGems), m_pos,*(pState->IsProcessing));
        InvalidateRect(hwnd, NULL, FALSE);
        return 0;
    }
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void LParamToPoint(LPARAM lParam, POINT& point)
{
    point.x = GET_X_LPARAM(lParam);
    point.y = GET_Y_LPARAM(lParam);
}