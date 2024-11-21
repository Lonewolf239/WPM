#include <iostream>
#include <iomanip>
#include <conio.h>
#include <string>
#include <windows.h>
#include <comdef.h>
#include <Wbemidl.h>
#include <pdh.h>
#pragma comment(lib, "wbemuuid.lib")
#pragma comment(lib, "pdh.lib")
#define clear system("cls");

using namespace std;

class CPUMonitor
{
    
private:
    ULARGE_INTEGER lastCPU, lastSysCPU, lastUserCPU;
    int numProcessors;
    HANDLE self;
    bool firstRun;

public:
    CPUMonitor() : firstRun(true)
    {
        SYSTEM_INFO sysInfo;
        GetSystemInfo(&sysInfo);
        numProcessors = sysInfo.dwNumberOfProcessors;

        self = GetCurrentProcess();
        UpdateValues();
    }

    void UpdateValues()
    {
        FILETIME ftime, fsys, fuser;

        GetSystemTimeAsFileTime(&ftime);
        memcpy(&lastCPU, &ftime, sizeof(FILETIME));

        GetProcessTimes(self, &ftime, &ftime, &fsys, &fuser);
        memcpy(&lastSysCPU, &fsys, sizeof(FILETIME));
        memcpy(&lastUserCPU, &fuser, sizeof(FILETIME));
    }

    double get_cpu_load()
    {
        if (firstRun) {
            firstRun = false;
            UpdateValues();
            Sleep(100);
            return 0.0;
        }
        FILETIME ftime, fsys, fuser;
        ULARGE_INTEGER now, sys, user;
        GetSystemTimeAsFileTime(&ftime);
        memcpy(&now, &ftime, sizeof(FILETIME));
        GetProcessTimes(self, &ftime, &ftime, &fsys, &fuser);
        memcpy(&sys, &fsys, sizeof(FILETIME));
        memcpy(&user, &fuser, sizeof(FILETIME));
        ULARGE_INTEGER sysTime = { sys.QuadPart - lastSysCPU.QuadPart };
        ULARGE_INTEGER userTime = { user.QuadPart - lastUserCPU.QuadPart };
        ULARGE_INTEGER timeDiff = { now.QuadPart - lastCPU.QuadPart };
        if (timeDiff.QuadPart == 0)
            return 0.0;
        double percent = (sysTime.QuadPart + userTime.QuadPart) /
            static_cast<double>(timeDiff.QuadPart);
        percent /= numProcessors;
        lastCPU = now;
        lastUserCPU = user;
        lastSysCPU = sys;
        return max(0.0, min(1.0, percent));
    }
};

static void set_brightness(float brightness);
static void set_power_plan(const string& guid, bool do_duplicate = true);
static float get_screen_brightness();
static void hide_cursor();
static void show_cursor();
static void set_position();
static void font_size_setup();
static void setup_setting();
static void draw_parameter(int pos, const string& name, float value);
static string draw_item(string i, string color = "2");
static void set_cursor_pos(int x, int y);
static void settings();

CPUMonitor monitor;

int main()
{
    setup_setting();
    bool do_clear = true;
    while (true)
    {
        set_cursor_pos(0, 0);
        cout << "\033[36m.---------------------------------.\n"
            << "|\033[5m\033[3m\033[33m      WINDOWS POWER MANAGER      \033[0m\033[36m|\n"
            << "|=================================|\n"
            << "|                                 |\n"
            << "|" << draw_item("1") << "High Performance\033[0m\033[36m          |\n"
            << "|" << draw_item("2") << "Balanced Power\033[0m\033[36m            |\n"
            << "|" << draw_item("3") << "Power Saver\033[0m\033[36m               |\n"
            << "|" << draw_item("4") << "Settings\033[0m\033[36m                  |\n"
            << "|                                 |\n"
            << "|" << draw_item("ESC", "1") << "Exit Program\033[0m\033[36m            |\n"
            << "|                                 |\n"
            << "|---------------------------------|\n"
            << endl
            << endl
            << "'---------------------------------'\033[0m" << flush;
        draw_parameter(13, "BRT", get_screen_brightness());
        draw_parameter(14, "CPU", monitor.get_cpu_load());
        do_clear = true;
        if (_kbhit())
        {
            switch (_getch())
            {
            case 49:
                clear;
                cout << "\033[5m\033[3m\033[33mSetting High Performance mode...\033[0m" << flush;
                set_power_plan("8c5e7fda-e8bf-4a96-9a85-a6e23a8c635c");
                set_brightness(1);
                break;
            case 50:
                clear;
                cout << "\033[5m\033[3m\033[33mSetting Balanced Power mode...\033[0m" << flush;
                set_power_plan("381b4222-f694-41f0-9685-ff5bb260df2e", false);
                set_brightness(0.7f);
                break;
            case 51:
                clear;
                cout << "\033[5m\033[3m\033[33mSetting Power Saver mode...\033[0m" << flush;
                set_power_plan("a1841308-3541-4fab-bc81-f71556f20b4a");
                set_brightness(0.35f);
                break;
            case 52:
                settings();
                break;
            case 27:
                exit(0);
                break;
            default:
                do_clear = false;
            }
        }
        else
            do_clear = false;
    }
    return 0;
}

static void set_cursor_pos(int x, int y)
{
    cout << "\033[" + to_string(y) + ";" + to_string(x) + "H" << flush;
}

static void set_brightness(float brightness)
{
    int brightnessInt = static_cast<int>(brightness * 100);
    string command = "powershell (Get-WmiObject -Namespace root/WMI -Class WmiMonitorBrightnessMethods).WmiSetBrightness(1," + to_string(brightnessInt) + ") > nul 2>&1";
    system(command.c_str());
}

static void set_power_plan(const string& guid, bool do_duplicate)
{
    string command;
    if (do_duplicate)
    {
        command = "powercfg -duplicatescheme " + guid + " > nul 2>&1";
        system(command.c_str());
    }
    command = "powercfg.exe /setactive " + guid + " > nul 2>&1";
    system(command.c_str());
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
            brightness = static_cast<float>(vtProp.uiVal) / 100.0f;
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
    SetConsoleTitleA("WPM v1.0 [By.Lonewolf239]");
    system("mode con cols=35 lines=15");
    hide_cursor();
    font_size_setup();
    set_position();
    HANDLE consoleHandle = GetStdHandle(STD_INPUT_HANDLE);
    DWORD consoleMode;
    GetConsoleMode(consoleHandle, &consoleMode);
    SetConsoleMode(consoleHandle, consoleMode & ~(ENABLE_QUICK_EDIT_MODE | ENABLE_EXTENDED_FLAGS | ENABLE_INSERT_MODE));
    HWND console = GetConsoleWindow();
    SetWindowLong(console, GWL_STYLE, GetWindowLong(console, GWL_STYLE) & ~WS_MAXIMIZEBOX & ~WS_SIZEBOX & ~WS_MAXIMIZE);
    srand(static_cast<unsigned int>(time(NULL)));
    setlocale(LC_CTYPE, "rus");
    clear;
}

static void draw_parameter(int position, const string& name, float value)
{
    const int barWidth = 20;
    int pos = static_cast<int>(barWidth * value);
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
    cout << progressBar << " \033[32m" << fixed << setprecision(0) << setw(3) << (value * 100) << "%\033[0m \033[36m|" << flush;
}

static string draw_item(string i, string color)
{
    return "   \033[36m[\033[3" + color + "m" + i + "\033[36m] \033[1m";
}

static void settings()
{
    bool do_clear = true;
    while (true)
    {
        set_cursor_pos(0, 0);
        cout << "\033[36m.---------------------------------.\n"
            << "|\033[5m\033[3m\033[33m          WPM SETTINGS           \033[0m\033[36m|\n"
            << "|=================================|\n"
            << "|                                 |\n"
            << "|                                 |\n"
            << "|                                 |\n"
            << "|                                 |\n"
            << "|                                 |\n"
            << "|                                 |\n"
            << "|" << draw_item("ESC", "1") << "Return\033[0m\033[36m                  |\n"
            << "|                                 |\n"
            << "|---------------------------------|\n"
            << endl
            << endl
            << "'---------------------------------'\033[0m" << flush;
        draw_parameter(13, "BRT", get_screen_brightness());
        draw_parameter(14, "CPU", monitor.get_cpu_load());
        do_clear = true;
        if (_kbhit()) {
            switch (_getch())
            {
            case 27:
                return;
                break;
            default:
                do_clear = false;
            }
        }
        else
            do_clear = false;
    }
}