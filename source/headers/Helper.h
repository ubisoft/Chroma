#pragma once
#include <shlobj.h>
#include "pch.h"
#include <sstream>
#include <gdiplus.h>

//Screen capture helper

using namespace Gdiplus;
using namespace winrt;
using namespace Windows::UI;
using namespace Windows::UI::Composition;
using namespace Windows::UI::Composition::Desktop;

extern std::string g_screenshotPath;
extern std::map<int, std::string> globalList;
extern INT g_appliedFilter;
#define INVALID_SELECTION -1
#define MENU_OFFSET 1000
#define SLEEP_CONST 1000

auto CreateDispatcherQueueController()
{
    namespace abi = ABI::Windows::System;

    DispatcherQueueOptions options
    {
        sizeof(DispatcherQueueOptions),
        DQTYPE_THREAD_CURRENT,
        DQTAT_COM_STA
    };

    Windows::System::DispatcherQueueController controller{ nullptr };
    check_hresult(CreateDispatcherQueueController(options, reinterpret_cast<abi::IDispatcherQueueController**>(put_abi(controller))));
    return controller;
}

DesktopWindowTarget CreateDesktopWindowTarget(Compositor const& compositor, HWND window)
{
    namespace abi = ABI::Windows::UI::Composition::Desktop;

    auto interop = compositor.as<abi::ICompositorDesktopInterop>();
    DesktopWindowTarget target{ nullptr };
    check_hresult(interop->CreateDesktopWindowTarget(window, true, reinterpret_cast<abi::IDesktopWindowTarget**>(put_abi(target))));
    return target;
}

// Get dll version number
ULONGLONG GetDllVersion(LPCTSTR lpszDllName)
{
    ULONGLONG ullVersion = 0;
    HINSTANCE hinstDll;
    hinstDll = LoadLibrary(lpszDllName);
    if (hinstDll)
    {
        DLLGETVERSIONPROC pDllGetVersion;
        pDllGetVersion = (DLLGETVERSIONPROC)GetProcAddress(hinstDll, "DllGetVersion");
        if (pDllGetVersion)
        {
            DLLVERSIONINFO dvi;
            HRESULT hr;
            ZeroMemory(&dvi, sizeof(dvi));
            dvi.cbSize = sizeof(dvi);
            hr = (*pDllGetVersion)(&dvi);
            if (SUCCEEDED(hr))
                ullVersion = MAKEDLLVERULL(dvi.dwMajorVersion, dvi.dwMinorVersion, 0, 0);
        }
        FreeLibrary(hinstDll);
    }
    return ullVersion;
}
static int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
    if (uMsg == BFFM_INITIALIZED)
    {
        std::string tmp = (const char*)lpData;
        SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
    }
    return 0;
}

static std::string BrowseFolder(const HWND& hwnd)
{
    CHAR path[MAX_PATH];

    const char* path_param = g_screenshotPath.c_str();

    BROWSEINFOA bi = { 0 };
    bi.hwndOwner = hwnd;
    bi.lpszTitle = ("Browse for folder...");
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE | BIF_USENEWUI;
    bi.lpfn = BrowseCallbackProc;
    bi.lParam = (LPARAM)path_param;

    LPITEMIDLIST pidl = SHBrowseForFolderA(&bi);

    if (pidl != 0)
    {
        //get the name of the folder and put it in path
        SHGetPathFromIDListA(pidl, path);

        //free memory used
        IMalloc* imalloc = 0;
        if (SUCCEEDED(SHGetMalloc(&imalloc)))
        {
            imalloc->Free(pidl);
            imalloc->Release();
        }

        return path;
    }
    return g_screenshotPath;
}

bool DirectoryExist(LPCSTR szPath)
{
    DWORD dwAttrib = GetFileAttributesA(szPath);

    return (dwAttrib != INVALID_FILE_ATTRIBUTES && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

std::string GetFilePath(bool isCombineMode)
{    
    if (g_screenshotPath.empty() || !DirectoryExist(g_screenshotPath.c_str()))
    {
        CHAR my_documents[MAX_PATH];
        SHGetFolderPathA(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, my_documents);
        g_screenshotPath = my_documents;
    }  

    SYSTEMTIME st;
    GetLocalTime(&st);
    std::stringstream strStream;
    strStream << st.wDay << "_" << st.wMonth << "_" << st.wYear << "_" << st.wHour << "_" << st.wMinute << "_" << st.wSecond << ".bmp";
    std::string finalPath;
    finalPath = g_screenshotPath + "\\";
    if (isCombineMode)
    {
        finalPath = finalPath + globalList[ID_FILTER_DEFAULT] + "_";
    }
    finalPath = finalPath + globalList[g_appliedFilter] + "_";
    finalPath = finalPath + strStream.str();
    return finalPath;
}

BOOL WINAPI SaveBitmap(LONG clWidth, LONG clHeight, int x1, int y1, std::string &fileName)
{
    BITMAPFILEHEADER bfHeader;
    BITMAPINFOHEADER biHeader;
    BITMAPINFO bInfo;
    HGDIOBJ hTempBitmap;
    HBITMAP hBitmap{};
    BITMAP bAllDesktops;
    HDC hDC, hMemDC{};
    HFONT hFont;
    BYTE* bBits = NULL;
    HANDLE hHeap = GetProcessHeap();
    DWORD cbBits{}, dwWritten = 0;

    ZeroMemory(&bfHeader, sizeof(BITMAPFILEHEADER));
    ZeroMemory(&biHeader, sizeof(BITMAPINFOHEADER));
    ZeroMemory(&bInfo, sizeof(BITMAPINFO));
    ZeroMemory(&bAllDesktops, sizeof(BITMAP));

    hDC = GetDC(NULL);
    hTempBitmap = GetCurrentObject(hDC, OBJ_BITMAP);
    GetObjectW(hTempBitmap, sizeof(BITMAP), &bAllDesktops);

    bfHeader.bfType = (WORD)('B' | ('M' << 8));
    bfHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    biHeader.biSize = sizeof(BITMAPINFOHEADER);
    biHeader.biBitCount = 24;
    biHeader.biCompression = BI_RGB;
    biHeader.biPlanes = 1;

    biHeader.biWidth = clWidth;
    biHeader.biHeight = clHeight;

    bInfo.bmiHeader = biHeader;

    cbBits = (((24 * biHeader.biWidth + 31) & ~31) / 8) * biHeader.biHeight;

    hMemDC = CreateCompatibleDC(hDC);
    hBitmap = CreateDIBSection(hDC, &bInfo, DIB_RGB_COLORS, (VOID**)&bBits, NULL, 0);
    SelectObject(hMemDC, hBitmap);
    BitBlt(hMemDC, 0, 0, clWidth, clHeight, hDC, x1, y1, SRCCOPY);
    // filter label on screenshot
    hFont = CreateFontA(20, 0, 0, 0, FW_DONTCARE, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, "Roboto");
    SelectObject(hMemDC, hFont);
    SetBkMode(hMemDC, OPAQUE);
    SetBkColor(hMemDC, 0x00000000);
    SetTextColor(hMemDC, 0x00FFFFFF);
    RECT rect;
    SetRect(&rect, 0, clHeight - 20, clWidth, clHeight);
    DrawTextA(hMemDC, globalList[g_appliedFilter].c_str(), -1, &rect, DT_LEFT);

    fileName = GetFilePath(false);
    HANDLE hFile = CreateFileA(fileName.c_str(), GENERIC_WRITE | GENERIC_READ, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    WriteFile(hFile, &bfHeader, sizeof(BITMAPFILEHEADER), &dwWritten, NULL);
    WriteFile(hFile, &biHeader, sizeof(BITMAPINFOHEADER), &dwWritten, NULL);
    WriteFile(hFile, bBits, cbBits, &dwWritten, NULL);

    CloseHandle(hFile);

    DeleteDC(hMemDC);
    ReleaseDC(NULL, hDC);
    DeleteObject(hBitmap);

    return TRUE;
}

void FindOverlappingRegion(HWND const hwnd, HWND const srcHwnd, RECT& overlapRect)
{
    // tool client area size
    RECT toolClientRect;
    GetClientRect(hwnd, &toolClientRect);
    LONG clWidth = toolClientRect.right - toolClientRect.left;
    LONG clHeight = toolClientRect.bottom - toolClientRect.top;
    POINT clRegionPoint = { toolClientRect.left, toolClientRect.top };
    ClientToScreen(hwnd, &clRegionPoint);
    // game client area size
    RECT gameClientRect;
    GetClientRect(srcHwnd, &gameClientRect);
    LONG srcWidth = gameClientRect.right - gameClientRect.left;
    LONG srcHeight = gameClientRect.bottom - gameClientRect.top;
    POINT srcRegionPoint = { gameClientRect.left, gameClientRect.top };
    ClientToScreen(srcHwnd, &srcRegionPoint);

    if (clRegionPoint.x + clWidth <= srcRegionPoint.x || clRegionPoint.x >= srcRegionPoint.x + srcWidth)
    {
        return;
    }
    if (clRegionPoint.y + clHeight <= srcRegionPoint.y || clRegionPoint.y >= srcRegionPoint.y + srcHeight)
    {
        return;
    }

    overlapRect.left = (clRegionPoint.x >= srcRegionPoint.x) ? clRegionPoint.x : srcRegionPoint.x;
    overlapRect.top = (clRegionPoint.y >= srcRegionPoint.y) ? clRegionPoint.y : srcRegionPoint.y;
    overlapRect.right = (clRegionPoint.x + clWidth >= srcRegionPoint.x + srcWidth) ? srcRegionPoint.x + srcWidth : clRegionPoint.x + clWidth;
    overlapRect.bottom = (clRegionPoint.y + clHeight >= srcRegionPoint.y + srcHeight) ? srcRegionPoint.y + srcHeight : clRegionPoint.y + clHeight;
}

int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
    GdiplusStartupInput gpStartupInput;
    ULONG_PTR gpToken;
    int val = GdiplusStartup(&gpToken, &gpStartupInput, NULL);
    if (val != Ok)
        return NULL;

    UINT  num = 0;
    UINT  size = 0;
    GetImageEncodersSize(&num, &size);
    if (size == 0)
        return -1;
    
    ImageCodecInfo* pImageCodecInfo = NULL;
    pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
    if (pImageCodecInfo == NULL)
        return -1;

    GetImageEncoders(num, size, pImageCodecInfo);

    for (UINT j = 0; j < num; ++j)
    {
        if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
        {
            *pClsid = pImageCodecInfo[j].Clsid;
            free(pImageCodecInfo);
            return j;
        }
    }

    free(pImageCodecInfo);
    GdiplusShutdown(gpToken);
    return -1;
}

void CombineTwoImages(const std::string &filterFileName, const std::string & defaultFileName)
{
    Status result;
    CLSID bmpClsid;
    if (GetEncoderClsid(L"image/bmp", &bmpClsid) != -1)
    {
        Bitmap* bmpDefaultImg = Bitmap::FromFile(std::wstring(defaultFileName.begin(), defaultFileName.end()).c_str());
        Bitmap* bmpFilterImg = Bitmap::FromFile(std::wstring(filterFileName.begin(), filterFileName.end()).c_str());

        if (bmpDefaultImg->GetLastStatus() == Ok && bmpFilterImg->GetLastStatus() == Ok)
        {
            UINT outputWidth = bmpDefaultImg->GetWidth() + bmpFilterImg->GetWidth();
            UINT outputHeight = (bmpDefaultImg->GetHeight() >= bmpFilterImg->GetHeight()) ? bmpDefaultImg->GetHeight() : bmpFilterImg->GetHeight();
            Bitmap outputBitmap(outputWidth, outputHeight);
            if (outputBitmap.GetLastStatus() == Ok)
            {
                Graphics* graphic = Graphics::FromImage(&outputBitmap);
                result = graphic->DrawImage(bmpDefaultImg, 0, 0);
                assert(result == Ok);
                result = graphic->DrawImage(bmpFilterImg, bmpDefaultImg->GetWidth(), 0);
                assert(result == Ok);

                std::string filePath = GetFilePath(true);
                result = outputBitmap.Save(std::wstring(filePath.begin(), filePath.end()).c_str(), &bmpClsid, NULL);
                assert(result == Ok);
                delete graphic;
            }
        }
        delete bmpDefaultImg;
        delete bmpFilterImg;
    }
}


BOOL ScreenshotHelper(HWND const hwnd, HWND const srcHwnd, const int& hotkeyID)
{
    MONITORINFOEX clientMonitorinfo;
    MONITORINFOEX srcMonitorinfo;
    clientMonitorinfo.cbSize = sizeof(MONITORINFOEX);
    srcMonitorinfo.cbSize = sizeof(MONITORINFOEX);

    // get monitor info
    auto clientMonitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY);
    auto srcMonitor = MonitorFromWindow(srcHwnd, MONITOR_DEFAULTTONULL);

    if (!GetMonitorInfoW(clientMonitor, &clientMonitorinfo) || !GetMonitorInfoW(srcMonitor, &srcMonitorinfo))
    {
        return FALSE;
    }

    WCHAR* srcDeviceName = srcMonitorinfo.szDevice;
    DEVMODE srcDevMode;
    srcDevMode.dmSize = sizeof(DEVMODE);
    EnumDisplaySettings(srcDeviceName, ENUM_REGISTRY_SETTINGS, &srcDevMode);

    WCHAR* clientDeviceName = clientMonitorinfo.szDevice;
    DEVMODE clientDevMode;
    clientDevMode.dmSize = sizeof(DEVMODE);
    EnumDisplaySettings(clientDeviceName, ENUM_REGISTRY_SETTINGS, &clientDevMode);

    // game(source) monitor scale factor
    FLOAT srcMonitorWidthScale = (float)srcDevMode.dmPelsWidth / (srcMonitorinfo.rcMonitor.right - srcMonitorinfo.rcMonitor.left);
    FLOAT srcMonitorHeightScale = (float)srcDevMode.dmPelsHeight / (srcMonitorinfo.rcMonitor.bottom - srcMonitorinfo.rcMonitor.top);
    // client monitor scale factor
    FLOAT clientMonitorWidthScale = (float)clientDevMode.dmPelsWidth / (clientMonitorinfo.rcMonitor.right - clientMonitorinfo.rcMonitor.left);
    FLOAT clientMonitorHeightScale = (float)clientDevMode.dmPelsHeight / (clientMonitorinfo.rcMonitor.bottom - clientMonitorinfo.rcMonitor.top);
    
    RECT overlapRect;
    FindOverlappingRegion(hwnd, srcHwnd, overlapRect);
    LONG overlapRegionWidth = overlapRect.right - overlapRect.left;
    LONG overlapRegionHeight = overlapRect.bottom - overlapRect.top;

    std::string filterFileName;
    std::string defaultFileName;

    if (wcscmp(srcDeviceName, clientDeviceName) == 0)
    {
        if (overlapRegionHeight != 0 && overlapRegionWidth != 0)
        {            
            SaveBitmap((LONG)(overlapRegionWidth * srcMonitorWidthScale), (LONG)(overlapRegionHeight * srcMonitorHeightScale), (int)(overlapRect.left * srcMonitorWidthScale), (int)(overlapRect.top * srcMonitorWidthScale), filterFileName);
        }
    }
    else
    {
        // tool client area size
        RECT toolClientRect;
        GetClientRect(hwnd, &toolClientRect);
        LONG clWidth = toolClientRect.right - toolClientRect.left;
        LONG clHeight = toolClientRect.bottom - toolClientRect.top;
        POINT clRegionPoint = { toolClientRect.left, toolClientRect.top };
        ClientToScreen(hwnd, &clRegionPoint);
        SaveBitmap((LONG)(clWidth * clientMonitorWidthScale), (LONG)(clHeight * clientMonitorHeightScale), (int)(clRegionPoint.x * clientMonitorWidthScale), (int)(clRegionPoint.y * clientMonitorHeightScale), filterFileName);
    }
    
    if (hotkeyID == ID_HOTKEY_TOOLGAME_SCREENSHOT && g_appliedFilter != ID_FILTER_DEFAULT)
    {
        if (wcscmp(srcDeviceName, clientDeviceName) == 0)
        {                                    
            ShowWindow(hwnd, SW_HIDE);
            INT back_appliedFilter = g_appliedFilter;
            g_appliedFilter = ID_FILTER_DEFAULT;
            Sleep(SLEEP_CONST);
            if (overlapRegionHeight != 0 && overlapRegionWidth != 0)
            {
                SaveBitmap((LONG)(overlapRegionWidth * srcMonitorWidthScale), (LONG)(overlapRegionHeight * srcMonitorHeightScale), (int)(overlapRect.left * srcMonitorWidthScale), (int)(overlapRect.top * srcMonitorWidthScale), defaultFileName);                
            }
            ShowWindow(hwnd, SW_SHOW);
            g_appliedFilter = back_appliedFilter;
        }
        else
        {            
            Sleep(SLEEP_CONST);
            INT back_appliedFilter = g_appliedFilter;
            g_appliedFilter = ID_FILTER_DEFAULT;     
            // game client area size
            RECT gameClientRect;
            GetClientRect(srcHwnd, &gameClientRect);
            LONG srcWidth = gameClientRect.right - gameClientRect.left;
            LONG srcHeight = gameClientRect.bottom - gameClientRect.top;
            POINT srcRegionPoint = { gameClientRect.left, gameClientRect.top };
            ClientToScreen(srcHwnd, &srcRegionPoint);
            SaveBitmap((LONG)(srcWidth * srcMonitorWidthScale), (LONG)(srcHeight * srcMonitorHeightScale), (int)(srcRegionPoint.x * srcMonitorWidthScale), (int)(srcRegionPoint.y * srcMonitorHeightScale), defaultFileName);
            g_appliedFilter = back_appliedFilter;
        }
        CombineTwoImages(filterFileName, defaultFileName);
    }
    return TRUE;
}

void CustomMessageBox(HWND const hwnd, MSGBOXPARAMSA& msgbox,  LPCSTR msgboxText)
{
    msgbox.cbSize = sizeof(MSGBOXPARAMS);
    msgbox.hwndOwner = hwnd;
    msgbox.hInstance = GetModuleHandle(NULL);
    msgbox.lpszText = msgboxText;
    msgbox.lpszCaption = "Chroma";
    msgbox.dwStyle = MB_OK | MB_USERICON | MB_SYSTEMMODAL;
    msgbox.lpszIcon = MAKEINTRESOURCEA(IDI_AUTOMATION_ICON);
}