#include "../Class/ConsoleMenu.h"
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <sstream>
#include <limits>

#ifdef _WIN32
  #include <windows.h>
  #ifndef ENABLE_QUICK_EDIT_MODE
  #define ENABLE_QUICK_EDIT_MODE 0x0040
  #endif
#endif

using namespace std;

#ifdef _WIN32
struct Rect { SHORT x, y, w, h; };

static int fallbackPick(const std::string& title,
                       const std::vector<std::string>& options,
                       DWORD oldMode,
                       HANDLE hin) {
    // restore mode before textual prompt
    SetConsoleMode(hin, oldMode);
    FlushConsoleInputBuffer(hin);
    std::cout << "== " << title << " ==\n";
    for (size_t i = 0; i < options.size(); ++i) {
        std::cout << " " << (i+1) << ") " << options[i] << "\n";
    }
    std::cout << "Choose (1.." << options.size() << ") or 0 to exit: ";
    int n; std::cin >> n; std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    if (n <= 0 || n > (int)options.size()) return -1;
    return n-1;
}

static void put(HANDLE out, SHORT x, SHORT y, const string& s) {
    COORD c{ x, y };
    SetConsoleCursorPosition(out, c);
    cout << s;
}
static void setAttr(HANDLE out, WORD attr) {
    SetConsoleTextAttribute(out, attr);
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
    string top = "+" + string(r.w - 2, '-') + "+";
    string mid = "|" + string(r.w - 2, ' ') + "|";
    string bot = "+" + string(r.w - 2, '-') + "+";
    put(out, r.x, r.y,     top);
    put(out, r.x, r.y + 1, mid);
    put(out, r.x, r.y + 2, bot);

    int inner = r.w - 2;
    string text = label;
    if ((int)text.size() > inner) text = text.substr(0, inner);
    int padL = max(0, (inner - (int)text.size()) / 2);
    int padR = max(0, inner - padL - (int)text.size());

    WORD normal = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE; // 7
    WORD hl     = BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED | FOREGROUND_INTENSITY; // 112+8

    setAttr(out, highlight ? hl : normal);
    put(out, r.x + 1, r.y + 1, string(padL, ' ') + text + string(padR, ' '));
    setAttr(out, normal);
}
static vector<string> wrap(const string& s, int width) {
    vector<string> out;
    istringstream iss(s);
    string w, line;
    while (iss >> w) {
        if ((int)(line.size() + (line.empty()?0:1) + w.size()) > width) {
            out.push_back(line); line = w;
        } else {
            if (!line.empty()) line += ' ';
            line += w;
        }
    }
    if (!line.empty()) out.push_back(line);
    return out;
}
static void drawInfoBox(HANDLE out, SHORT x, SHORT y, SHORT w, SHORT h, const string& title) {
    // frame
    put(out, x, y, "+" + string(w-2, '-') + "+");
    for (int r=1; r<h-1; ++r) put(out, x, y+r, "|" + string(w-2,' ') + "|");
    put(out, x, y+h-1, "+" + string(w-2, '-') + "+");

    // title (center)
    int pad = max(0, (w-2 - (int)title.size())/2);
    put(out, x+1, y, string(pad,' ') + title + string(w-2 - pad - (int)title.size(), ' '));
}
static void renderInfoText(HANDLE out, SHORT x, SHORT y, SHORT w, SHORT h, const string& text) {
    // clear inside
    for (int r=1; r<h-1; ++r) put(out, x+1, y+r, string(w-2, ' '));
    // draw text
    auto lines = wrap(text, w-4);
    for (int i=0; i<(int)lines.size() && i<h-2; ++i)
        put(out, x+2, y+1+i, lines[i]);
}
#endif

int ConsoleMenu::pick(const std::string& title,
                      const std::vector<std::string>& options,
                      const std::vector<std::string>& infos) {
#ifndef _WIN32
    std::cout << "== " << title << " ==\n";
    for (auto& s : options) std::cout << " • " << s << "\n";
    std::cout << "(Mouse khong ho tro)\nChon (1.." << options.size() << "): ";
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
    SHORT X = 2, Y = 1;

    drawFrame(hout, X, Y, frameW, title);

    const SHORT btnW = frameW - 4;
    const SHORT btnH = 3;
    const SHORT gapY = 1;
    SHORT startY = Y + 4;

    vector<Rect> rects; rects.reserve(options.size());
    for (size_t i = 0; i < options.size(); ++i) {
        Rect r{ (SHORT)(X + 2), (SHORT)(startY + (SHORT)i * (btnH + gapY)), btnW, btnH };
        rects.push_back(r);
    }

    // ----- INFO PANEL -----
    SHORT leftBottom = startY + (SHORT)options.size() * (btnH + gapY) - gapY + 1;
    SHORT infoW = 50;
    SHORT infoX = X + frameW + 3;
    SHORT infoY = Y;
    SHORT infoH = max<SHORT>(leftBottom - Y + 1, 7);

    // check console size; fall back to text if not enough space
    CONSOLE_SCREEN_BUFFER_INFO cs; GetConsoleScreenBufferInfo(hout, &cs);
    SHORT winW = cs.srWindow.Right - cs.srWindow.Left + 1;
    SHORT winH = cs.srWindow.Bottom - cs.srWindow.Top + 1;
    SHORT reqW = infoX + infoW + 2;
    SHORT reqH = max<SHORT>(infoY + infoH + 2, leftBottom + 2);
    if (winW < reqW || winH < reqH) {
        return fallbackPick(title, options, oldMode, hin);
    }

    // draw now that we know space is sufficient
    drawFrame(hout, X, Y, frameW, title);
    for (size_t i = 0; i < rects.size(); ++i) {
        drawButton(hout, rects[i], options[i], false);
    }
    drawInfoBox(hout, infoX, infoY, infoW, infoH, "INFO");
    if (!options.empty()) {
        string tip = (!infos.empty() && (int)infos.size() > 0) ? infos[0] : "Left-click de chon nua.";
        renderInfoText(hout, infoX, infoY, infoW, infoH, tip);
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
                    // update info
                    string tip = (idx >=0 && idx < (int)infos.size() && !infos[idx].empty())
                                 ? infos[idx]
                                 : "Left-click to select.";
                    renderInfoText(hout, infoX, infoY, infoW, infoH, tip);
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

int ConsoleMenu::pick(const std::string& title, const std::vector<std::string>& options) {
    static const std::vector<std::string> empty;
    return pick(title, options, empty);
}
