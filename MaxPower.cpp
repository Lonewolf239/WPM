#include "Methods.h"

const string HIGH_PERFORMANCE_GUID = "8c5e7fda-e8bf-4a96-9a85-a6e23a8c635c";
const string BALANCED_GUID = "381b4222-f694-41f0-9685-ff5bb260df2e";
const string POWER_SAVER_GUID = "a1841308-3541-4fab-bc81-f71556f20b4a";

int main()
{
    thread worker(async_worker);
    thread cursor(cursor_blinking);
    load_settings();
    setup_setting();
    bool do_clear = true;
    while (true)
    {
        CurrentTitle = make_shared<string>("WINDOWS POWER MANAGER");
        set_cursor_pos(0, 0);
        cout << "\033[36m.---------------------------------.\n"
            << endl
            << "|=================================|\n"
            << "|                                 |\n"
            << "|" << draw_item("1", "Hight Performance") << "|\n"
            << "|" << draw_item("2", "Balanced Power") << "|\n"
            << "|" << draw_item("3", "Power Saver") << "|\n"
            << "|" << draw_item("4", "Settings") << "|\n"
            << "|                                 |\n"
            << "|" << draw_item("ESC", "Exit Program", "1") << "|\n"
            << "|                                 |\n" << flush;
        if (DisplayBrightness)
            cout << "|---------------------------------|\n" << endl << flush;
        cout << "'---------------------------------'\033[0m" << flush;
        draw_title(*CurrentTitle, 33, to_string(Darken));
        if (DisplayBrightness)
            draw_parameter(12, "BRT", get_screen_brightness());
        do_clear = true;
        if (_kbhit())
        {
            switch (_getch())
            {
            case 49:
                clear;
                cout << "\033[33mSetting High Performance mode...\033[0m" << flush;
                if (set_power_plan(HIGH_PERFORMANCE_GUID)) {
                    set_brightness(HPB);
                    cout << "\n\033[32mSuccess!" << endl;
                }
                else cout << "\n\033[31mFailed to set power plan!" << endl;
                this_thread::sleep_for(chrono::seconds(1));
                break;

            case 50:
                clear;
                cout << "\033[33mSetting Balanced Power mode...\033[0m" << flush;
                if (set_power_plan(BALANCED_GUID)) {
                    set_brightness(BPB);
                    cout << "\n\033[32mSuccess!" << endl;
                }
                else cout << "\n\033[31mFailed to set power plan!" << endl;
                this_thread::sleep_for(chrono::seconds(1));
                break;

            case 51:
                clear;
                cout << "\033[33mSetting Power Saver mode...\033[0m" << flush;
                if (set_power_plan(POWER_SAVER_GUID)) {
                    set_brightness(PSB);
                    cout << "\n\033[32mSuccess!" << endl;
                }
                else cout << "\n\033[31mFailed to set power plan!" << endl;
                this_thread::sleep_for(chrono::seconds(1));
                break;
            case 52:
                settings();
                break;
            case 27:
                ProgramClosing = true;
                worker.join();
                cursor.join();
                exit(0);
                break;
            default:
                do_clear = false;
            }
        }
        else
            do_clear = false;
    }
    worker.join();
    cursor.join();
    return 0;
}