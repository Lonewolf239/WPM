#include "Methods.h"

int main()
{
    HANDLE hMutex = CreateMutex(NULL, TRUE, L"WPM_Unique_Mutex");
    if (GetLastError() == ERROR_ALREADY_EXISTS)
        return 0;
    setup_setting();
    thread worker(async_worker);
    if (!is_admin())
    {
        system("mode con cols=37 lines=9");
        set_position();
        CurrentTitle = "NOT ADMINISTRATOR";
        while (true)
        {
            set_cursor_pos(0, 0);
            if (ColorsSupport)
                cout << "\033[36m.-----------------------------------.\n"
                << endl
                << "|===================================|\n"
                << "|                                   |\n"
                << "| \033[1mPlease run WPM as \033[4m\033[31madministrator\033[0m\033[1m\033[36m!\033[0m\033[36m  |\n"
                << "|                                   |\n"
                << "| \033[33mPress any key to exit...\033[0m\033[36m          |\n"
                << "|                                   |\n"
                << "'-----------------------------------'\033[0m" << flush;
            else
                cout << ".-----------------------------------.\n"
                << endl
                << "|===================================|\n"
                << "|                                   |\n"
                << "| Please run WPM as administrator!  |\n"
                << "|                                   |\n"
                << "| Press any key to exit...          |\n"
                << "|                                   |\n"
                << "'-----------------------------------'" << flush;
            draw_title(CurrentTitle, 35);
            if (_kbhit())
                break;
            this_thread::sleep_for(chrono::milliseconds(10));
        }
        ProgramClosing = true;
        worker.join();
        if (hMutex != NULL)
            CloseHandle(hMutex);
        return 0;
    }
    thread cursor(cursor_blinking);
    load_settings();
    bool do_clear = true;
    while (true)
    {
        CurrentTitle = "WINDOWS POWER MANAGER";
        set_cursor_pos(0, 0);
        if (ColorsSupport)
        {
            cout << "\033[36m.---------------------------------.\n"
                << endl
                << "|=================================|\n"
                << "|                                 |\n"
                << "|" << draw_item("1", "High Performance") << "|\n"
                << "|" << draw_item("2", "Balanced Power") << "|\n"
                << "|" << draw_item("3", "Power Saver") << "|\n"
                << "|" << draw_item("4", "Settings", "2", is_brightness_control_available()) << "|\n"
                << "|                                 |\n"
                << "|" << draw_item("ESC", "Exit Program", "1") << "|\n"
                << "|                 \033[90mBy. Lonewolf239\033[0m\033[36m |\n" << flush;
            if (DisplayBrightness)
                cout << "|---------------------------------|\n" << endl << flush;
            cout << "'---------------------------------'\033[0m" << flush;
        }
        else
        {
            cout << ".---------------------------------.\n"
                << endl
                << "|=================================|\n"
                << "|                                 |\n"
                << "|" << draw_item("1", "High Performance") << "|\n"
                << "|" << draw_item("2", "Balanced Power") << "|\n"
                << "|" << draw_item("3", "Power Saver") << "|\n"
                << "|" << draw_item("4", "Settings", "2", is_brightness_control_available()) << "|\n"
                << "|                                 |\n"
                << "|" << draw_item("ESC", "Exit Program", "1") << "|\n"
                << "|                 By. Lonewolf239 |\n" << flush;
            if (DisplayBrightness)
                cout << "|---------------------------------|\n" << endl << flush;
            cout << "'---------------------------------'" << flush;
        }
        draw_title(CurrentTitle, 33);
        if (DisplayBrightness)
            draw_parameter(12, "BRT", get_screen_brightness());
        do_clear = true;
        if (_kbhit())
        {
            switch (_getch())
            {
            case 49:
                clear;
                if (ColorsSupport)
                    cout << "\033[33mSetting High Performance mode...\033[0m" << flush;
                else
                    cout << "Setting High Performance mode..." << flush;
                if (set_power_plan(HIGH_PERFORMANCE_GUID)) {
                    set_brightness(HPB);
                    if (ColorsSupport)
                        cout << "\033[32m";
                    cout << "\nSuccess!" << endl;
                }
                else
                {
                    if (ColorsSupport)
                        cout << "\033[31m";
                    cout << "\nFailed to set power plan!" << endl;
                }
                this_thread::sleep_for(chrono::seconds(1));
                break;
            case 50:
                clear;
                if (ColorsSupport)
                    cout << "\033[33mSetting Balanced Power mode...\033[0m" << flush;
                else
                    cout << "Setting Balanced Power mode..." << flush;
                if (set_power_plan(BALANCED_GUID)) {
                    set_brightness(BPB);
                    if (ColorsSupport)
                        cout << "\033[32m";
                    cout << "\nSuccess!" << endl;
                }
                else
                {
                    if (ColorsSupport)
                        cout << "\033[31m";
                    cout << "\nFailed to set power plan!" << endl;
                }
                this_thread::sleep_for(chrono::seconds(1));
                break;
            case 51:
                clear;
                if (ColorsSupport)
                    cout << "\033[33mSetting Power Saver mode...\033[0m" << flush;
                else
                    cout << "Setting Power Saver mode..." << flush;
                if (set_power_plan(POWER_SAVER_GUID)) {
                    set_brightness(PSB);
                    if (ColorsSupport)
                        cout << "\033[32m";
                    cout << "\nSuccess!" << endl;
                }
                else
                {
                    if (ColorsSupport)
                        cout << "\033[31m";
                    cout << "\nFailed to set power plan!" << endl;
                }
                this_thread::sleep_for(chrono::seconds(1));
                break;
            case 52:
                if (is_brightness_control_available())
                    settings();
                break;
            case 27:
                ProgramClosing = true;
                worker.join();
                cursor.join();
                if (hMutex != NULL)
                    CloseHandle(hMutex);
                exit(0);
                break;
            default:
                do_clear = false;
            }
        }
        else
            do_clear = false;
        this_thread::sleep_for(chrono::milliseconds(10));
    }
    worker.join();
    cursor.join();
    if (hMutex != NULL)
        CloseHandle(hMutex);
    return 0;
}