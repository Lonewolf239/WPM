#include <iostream>
#include <stdlib.h>
#include <iomanip>
#include <conio.h>
#include <string>
#include <windows.h>
#include <algorithm>
#include <comdef.h>
#include <Wbemidl.h>
#include <pdh.h>
#include <thread>
#include <chrono>
#include <atomic>
#include <memory>
#include <fstream>
#include <sstream>
#include <vector>
#pragma comment(lib, "wbemuuid.lib")
#pragma comment(lib, "pdh.lib")
#define clear system("cls");
#define pause this_thread::sleep_for(chrono::seconds(1));

using namespace std;

static void set_cursor_pos(int x, int y);
static void set_brightness(int brightness);
static bool set_power_plan(const string& guid);
static float get_screen_brightness();
static void hide_cursor();
static void show_cursor();
static void set_position();
static void font_size_setup();
static void setup_setting();
static void draw_parameter(int position, const string& name, const float value);
static string center_pad(const string& str, size_t width);
static string right_pad(string& str, const size_t num, const char paddingChar = ' ');
static void draw_title(string title, const size_t num, const string dark);
static string draw_item(string i, string name, string color = "2");
static int get_value(int y);
static void save_settings();
static void load_settings();
static vector<string> split(string s, char delimiter);
static void settings();

static bool DisplayBrightness = true;
static int HPB = 100;
static int BPB = 70;
static int PSB = 35;

static void set_cursor_pos(int x, int y)
{
    COORD pos = { static_cast<SHORT>(x), static_cast<SHORT>(y) };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

static void set_brightness(int brightness)
{
    string command = "powershell (Get-WmiObject -Namespace root/WMI -Class WmiMonitorBrightnessMethods).WmiSetBrightness(1," + to_string(brightness) + ") > nul 2>&1";
    system(command.c_str());
}

static bool set_power_plan(const string& guid)
{
    string check_command = "powercfg /list | find \"" + guid + "\" > nul 2>&1";
    int exists = system(check_command.c_str());
    if (exists != 0) {
        string create_command = "powercfg -duplicatescheme " + guid + " > nul 2>&1";
        if (system(create_command.c_str()) != 0)
            return false;
    }
    string activate_command = "powercfg /setactive " + guid + " > nul 2>&1";
    return (system(activate_command.c_str()) == 0);
}

static float get_screen_brightness()
{
    float brightness = 0.0f;
    HRESULT hres;
    hres = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hres))
        return 0;
    hres = CoInitializeSecurity(0, -1, 0, 0, 0, 3, 0, EOAC_NONE, 0);
    if (FAILED(hres)) {
        CoUninitialize();
        return 0;
    }
    IWbemLocator* pLoc = NULL;
    hres = CoCreateInstance(
        CLSID_WbemLocator,
        0,
        CLSCTX_INPROC_SERVER,
        IID_IWbemLocator,
        (LPVOID*)&pLoc
    );
    if (FAILED(hres)) {
        CoUninitialize();
        return 0;
    }
    IWbemServices* pSvc = NULL;
    hres = pLoc->ConnectServer(_bstr_t(L"ROOT\\WMI"), 0, 0, 0, 0, 0, 0, &pSvc);
    if (FAILED(hres)) {
        pLoc->Release();
        CoUninitialize();
        return 0;
    }
    hres = CoSetProxyBlanket(
        pSvc,
        RPC_C_AUTHN_WINNT,
        RPC_C_AUTHZ_NONE,
        NULL,
        RPC_C_AUTHN_LEVEL_CALL,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL,
        EOAC_NONE
    );
    if (FAILED(hres)) {
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return 0;
    }
    IEnumWbemClassObject* pEnumerator = NULL;
    hres = pSvc->ExecQuery(
        bstr_t("WQL"),
        bstr_t("SELECT * FROM WmiMonitorBrightness"),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        NULL,
        &pEnumerator
    );
    if (FAILED(hres)) {
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return 0;
    }
    IWbemClassObject* pclsObj = NULL;
    ULONG uReturn = 0;
    while (pEnumerator) {
        hres = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);
        if (0 == uReturn)
            break;
        VARIANT vtProp{};
        hres = pclsObj->Get(L"CurrentBrightness", 0, &vtProp, 0, 0);
        if (SUCCEEDED(hres)) {
            brightness = static_cast<float>(vtProp.uiVal);
            VariantClear(&vtProp);
        }
        pclsObj->Release();
    }
    pEnumerator->Release();
    pSvc->Release();
    pLoc->Release();
    CoUninitialize();
    return brightness;
}

static void hide_cursor()
{
    HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO structCursorInfo;
    GetConsoleCursorInfo(handle, &structCursorInfo);
    structCursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(handle, &structCursorInfo);
}

static void show_cursor()
{
    HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO structCursorInfo;
    GetConsoleCursorInfo(handle, &structCursorInfo);
    structCursorInfo.bVisible = TRUE;
    SetConsoleCursorInfo(handle, &structCursorInfo);
}

static void set_position()
{
    RECT r;
    HWND consoleWindow = GetConsoleWindow();
    GetWindowRect(consoleWindow, &r);
    int x = GetSystemMetrics(SM_CXSCREEN) / 2 - (r.right - r.left) / 2;
    int y = GetSystemMetrics(SM_CYSCREEN) / 2 - (r.bottom - r.top) / 2;
    SetWindowPos(consoleWindow, 0, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}

static void font_size_setup()
{
    wstring font_name = L"Consolas";
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_FONT_INFOEX fontInfo{};
    fontInfo.cbSize = sizeof(CONSOLE_FONT_INFOEX);
    GetCurrentConsoleFontEx(hConsole, FALSE, &fontInfo);
    wcscpy_s(fontInfo.FaceName, font_name.c_str());
    fontInfo.dwFontSize.X = 10;
    fontInfo.dwFontSize.Y = 20;
    fontInfo.FontWeight = FW_NORMAL;
    SetCurrentConsoleFontEx(hConsole, FALSE, &fontInfo);
    set_position();
}

static void setup_setting()
{
    SetConsoleTitleW(L"WPM v1.0 [By.Lonewolf239]");
    if (DisplayBrightness) system("mode con cols=35 lines=14");
    else system("mode con cols=35 lines=12");
    HWND console = GetConsoleWindow();
    HDC hdc = GetDC(console);
    HBRUSH brush = CreateSolidBrush(RGB(12, 12, 50));
    RECT rect;
    GetClientRect(console, &rect);
    FillRect(hdc, &rect, brush);
    DeleteObject(brush);
    ReleaseDC(console, hdc);
    SetLayeredWindowAttributes(console, 0, 255 * 90 / 100, LWA_ALPHA);
    SetWindowLong(console, GWL_EXSTYLE, GetWindowLong(console, GWL_EXSTYLE) | WS_EX_LAYERED);
    SetWindowLong(console, GWL_STYLE,
        GetWindowLong(console, GWL_STYLE) & ~WS_MAXIMIZEBOX & ~WS_SIZEBOX & ~WS_MAXIMIZE | WS_BORDER);
    hide_cursor();
    font_size_setup();
    set_position();
    HANDLE consoleHandle = GetStdHandle(STD_INPUT_HANDLE);
    DWORD consoleMode;
    GetConsoleMode(consoleHandle, &consoleMode);
    SetConsoleMode(consoleHandle, consoleMode & ~(ENABLE_QUICK_EDIT_MODE | ENABLE_EXTENDED_FLAGS | ENABLE_INSERT_MODE));
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
    srand(static_cast<unsigned int>(time(NULL)));
    clear;
}

static void draw_parameter(int position, const string& name, const float value)
{
    const int barWidth = 20;
    int pos = static_cast<int>(barWidth * (value / 100.0));
    string progressBar = "\033[36m| \033[33m\033[3m" + name + " \033[33m[";
    for (int i = 0; i < barWidth; ++i)
    {
        if (i < pos) {
            if (i < barWidth / 3) progressBar += "\033[32m#";
            else if (i < 2 * barWidth / 3) progressBar += "\033[33m#";
            else progressBar += "\033[31m#";
        }
        else progressBar += " ";
    }
    progressBar += "\033[33m]";
    set_cursor_pos(0, position);
    cout << progressBar << " \033[32m" << fixed << setprecision(0) << setw(3) << value << "%\033[0m \033[36m|" << flush;
}

static string center_pad(const string& str, size_t width)
{
    if (str.length() >= width) return str;
    size_t padding = (width - str.length()) / 2;
    return string(padding, ' ') + str + string(width - padding - str.length(), ' ');
}

static string right_pad(string& str, const size_t num, const char paddingChar)
{
    if (num > str.size())
        str.append(num - str.size(), paddingChar);
    return str;
}

static void draw_title(string title, const size_t num, const string dark)
{
    set_cursor_pos(0, 1);
    cout << "\033[0m\033[36m|\033[" + dark + "m\033[33m" + center_pad(title, num) + "\033[0m\033[36m|" << flush;
}

static string draw_item(string i, string name, string color)
{
    string result = "   \033[36m[\033[3" + color + "m" + i + "\033[36m] \033[1m";
    return result +  right_pad(name, 52 - result.size()) + "\033[0m\033[36m";
}

static int get_value(int y)
{
    char value[4] = { -1, -1, -1, 0 };
    int key = 0, index = 0;
    while (true) {
        set_cursor_pos(0, y);
        cout << "|   \033[1mInput value: "
            << (value[0] == -1 ? ' ' : value[0])
            << (value[1] == -1 ? ' ' : value[1])
            << (value[2] == -1 ? ' ' : value[2])
            << "              |" << flush;
        set_cursor_pos(17 + index, y);
        key = _getch();
        if (key == 13)
        {
            if (index > 0)
            {
                value[index] = '\0';
                int num = atoi(value);
                if (num >= 0 && num <= 100)
                    return num;
            }
        }
        else if (key == 8)
        {
            if (index > 0)
                value[--index] = -1;
        }
        else if (key >= 48 && key <= 57)
        {
            if (index < 3)
            {
                if (index == 0)
                    value[index++] = key;
                else if (index == 1)
                {
                    if (value[0] == 48)
                        continue;
                        int num = (value[0] - '0') * 10 + (key - '0');
                    if (num <= 100)
                        value[index++] = key;
                }
                else if (index == 2)
                {
                    int num = (value[0] - '0') * 100 + (value[1] - '0') * 10 + (key - '0');
                    if (num <= 100)
                        value[index++] = key;
                }
            }
        }
    }
}

static void save_settings()
{
    ofstream settings("settings.txt");
    if (settings.is_open()) {
        settings << to_string(DisplayBrightness) + ":" +
            to_string(HPB) + ":" +
            to_string(BPB) + ":" +
            to_string(PSB);
        settings.close();
    }
}

static void load_settings()
{
    ifstream settings("settings.txt");
    if (settings.is_open()) {
        string data;
        settings >> data;
        settings.close();

        try {
            vector<string> sd = split(data, ':');
            if (sd.size() >= 4) {
                DisplayBrightness = stoi(sd[0]);
                HPB = stoi(sd[1]);
                BPB = stoi(sd[2]);
                PSB = stoi(sd[3]);
            }
        }
        catch (const exception& e)
        {
            DisplayBrightness = true;
            HPB = 100;
            BPB = 70;
            PSB = 35;
        }
    }
}

static vector<string> split(string s, char delimiter)
{
    size_t pos_start = 0, pos_end;
    string token;
    vector<string> res;
    while ((pos_end = s.find(delimiter, pos_start)) != string::npos)
    {
        token = s.substr(pos_start, pos_end - pos_start);
        pos_start = pos_end + 1;
        res.push_back(token);
    }
    res.push_back(s.substr(pos_start));
    return res;
}
