#include "../Class/ConsoleMenu.h"
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

#ifdef _WIN32
  #include <windows.h>
  #ifndef ENABLE_QUICK_EDIT_MODE
  #define ENABLE_QUICK_EDIT_MODE 0x0040
  #endif
#endif

using namespace std;

#ifdef _WIN32
struct Rect { SHORT x, y, w, h; };

static void put(HANDLE out, SHORT x, SHORT y, const string& s) {
    COORD c{ x, y };
    SetConsoleCursorPosition(out, c);
    cout << s;
}

static void drawFrame(HANDLE out, SHORT x, SHORT y, SHORT w, const string& title) {
    string top = "+" + string(w - 2, '-') + "+";
    string mid = "|" + string(w - 2, ' ') + "|";
    string bot = "+" + string(w - 2, '-') + "+";
    put(out, x, y,     top);
    put(out, x, y + 1, mid);
    int inner = w - 2;
    int padL = max(0, (inner - (int)title.size()) / 2);
    int padR = max(0, inner - padL - (int)title.size());
    put(out, x + 1, y + 1, string(padL, ' ') + title + string(padR, ' '));
    put(out, x, y + 2, bot);
}

static void drawButton(HANDLE out, const Rect& r, const string& label, bool highlight) {
    // khung hộp
    string top = "+" + string(r.w - 2, '-') + "+";
    string mid = "|" + string(r.w - 2, ' ') + "|";
    string bot = "+" + string(r.w - 2, '-') + "+";
    put(out, r.x, r.y,     top);
    put(out, r.x, r.y + 1, mid);
    put(out, r.x, r.y + 2, bot);

    // nhãn giữa dòng
    int inner = r.w - 2;
    string text = label;
    if ((int)text.size() > inner) text = text.substr(0, inner);
    int padL = max(0, (inner - (int)text.size()) / 2);
    int padR = max(0, inner - padL - (int)text.size());

    WORD normal = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;                // 7
    WORD hl     = BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED | FOREGROUND_INTENSITY; // 112+8

    SetConsoleTextAttribute(out, highlight ? hl : normal);
    put(out, r.x + 1, r.y + 1, string(padL, ' ') + text + string(padR, ' '));
    SetConsoleTextAttribute(out, normal);
}
#endif

int ConsoleMenu::pick(const std::string& title, const std::vector<std::string>& options) {
#ifndef _WIN32
    std::cout << "== " << title << " ==\n";
    for (auto& s : options) std::cout << " • " << s << "\n";
    std::cout << "Chon (1.." << options.size() << "): ";
    int n; std::cin >> n; std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return (n >= 1 && n <= (int)options.size()) ? (n - 1) : -1;
#else
    system("cls");

    HANDLE hin  = GetStdHandle(STD_INPUT_HANDLE);
    HANDLE hout = GetStdHandle(STD_OUTPUT_HANDLE);

    DWORD oldMode; GetConsoleMode(hin, &oldMode);
    DWORD mode = oldMode;
    mode |= ENABLE_EXTENDED_FLAGS | ENABLE_MOUSE_INPUT | ENABLE_WINDOW_INPUT;
    mode &= ~(ENABLE_QUICK_EDIT_MODE | ENABLE_INSERT_MODE);
    SetConsoleMode(hin, mode);

    FlushConsoleInputBuffer(hin);

    size_t maxLabel = title.size();
    for (auto& s : options) maxLabel = max(maxLabel, s.size());
    SHORT frameW = (SHORT)max<int>((int)maxLabel + 8, 38);
    SHORT X = 4, Y = 1;

    drawFrame(hout, X, Y, frameW, title);

    const SHORT btnW = frameW - 4;
    const SHORT btnH = 3;
    const SHORT gapY = 1;
    SHORT startY = Y + 4;

    vector<Rect> rects; rects.reserve(options.size());
    for (size_t i = 0; i < options.size(); ++i) {
        Rect r{ (SHORT)(X + 2), (SHORT)(startY + i * (btnH + gapY)), btnW, btnH };
        rects.push_back(r);
        drawButton(hout, r, options[i], false);
    }

    INPUT_RECORD rec; DWORD read = 0;
    int hover = -1, pressed = -1;
    DWORD prevButtons = 0;

    auto indexAt = [&](SHORT mx, SHORT my) -> int {
        for (int i = 0; i < (int)rects.size(); ++i) {
            const Rect& r = rects[i];
            if (mx >= r.x && mx < r.x + r.w && my >= r.y && my < r.y + r.h) return i;
        }
        return -1;
    };

    while (true) {
        ReadConsoleInput(hin, &rec, 1, &read);

        if (rec.EventType == MOUSE_EVENT) {
            const auto& me = rec.Event.MouseEvent;
            SHORT mx = me.dwMousePosition.X, my = me.dwMousePosition.Y;
            int idx = indexAt(mx, my);

            if (me.dwEventFlags == MOUSE_MOVED) {
                if (idx != hover) {
                    if (hover >= 0) drawButton(hout, rects[hover], options[hover], false);
                    if (idx   >= 0) drawButton(hout, rects[idx],  options[idx],  true);
                    hover = idx;
                }
            } else if (me.dwEventFlags == 0) {
                bool leftNow  = (me.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED) != 0;
                bool leftPrev = (prevButtons     & FROM_LEFT_1ST_BUTTON_PRESSED) != 0;

                if (leftNow && !leftPrev) {
                    pressed = idx;
                } else if (!leftNow && leftPrev) {
                    if (pressed >= 0 && idx == pressed) {
                        SetConsoleMode(hin, oldMode);
                        FlushConsoleInputBuffer(hin);
                        return idx;
                    }
                    pressed = -1;
                }
                prevButtons = me.dwButtonState;
            }
        } else if (rec.EventType == KEY_EVENT && rec.Event.KeyEvent.bKeyDown) {
            if (rec.Event.KeyEvent.wVirtualKeyCode == VK_ESCAPE) {
                SetConsoleMode(hin, oldMode);
                FlushConsoleInputBuffer(hin);
                return -1;
            }
        }
    }
#endif
}
