//
// --- Mini Console library
//

#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <string.h>

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

typedef CHAR_INFO ConsolePixel;
typedef INPUT_RECORD ConsoleEvent;

#else

typedef struct ConsolePixel
{
    union { // pointless union to mimic windows API
        char AsciiChar;
    } Char;
    uint16_t Attributes;
} ConsolePixel;

#define FOREGROUND_BLUE 0x0001
#define FOREGROUND_GREEN 0x0002
#define FOREGROUND_RED 0x0004
#define FOREGROUND_INTENSITY 0x0008
#define BACKGROUND_BLUE 0x0010
#define BACKGROUND_GREEN 0x0020
#define BACKGROUND_RED 0x0040
#define BACKGROUND_INTENSITY 0x0080

typedef struct COORD
{
    int16_t X;
    int16_t Y;
} COORD;

typedef struct KEY_EVENT_RECORD
{
    char bKeyDown;
    uint16_t wRepeatCount;
    uint16_t wVirtualKeyCode;
    uint16_t wVirtualScanCode;
    union {
        char AsciiChar;
    } uChar;
    uint64_t dwControlKeyState;
} KEY_EVENT_RECORD;

typedef struct MOUSE_EVENT_RECORD
{
    COORD dwMousePosition;
    uint64_t dwButtonState;
    uint64_t dwControlKeyState;
    uint64_t dwEventFlags;
} MOUSE_EVENT_RECORD;

#define FROM_LEFT_1ST_BUTTON_PRESSED 0x0001
#define RIGHTMOST_BUTTON_PRESSED 0x0002

#define MOUSE_MOVED 0x0001

typedef struct ConsoleEvent
{
    uint16_t EventType;
    union {
        KEY_EVENT_RECORD KeyEvent;
        MOUSE_EVENT_RECORD MouseEvent;
    } Event;
} ConsoleEvent;

#define KEY_EVENT 0x0001
#define MOUSE_EVENT 0x0002

#define MIN(a,b) (((a)<(b)) ? (a) : (b))
#define MAX(a,b) (((a)>(b)) ? (a) : (b))

// Key definitions

#define VK_LBUTTON        0x01
#define VK_RBUTTON        0x02
#define VK_CANCEL         0x03
#define VK_MBUTTON        0x04

#define VK_BACK           0x08
#define VK_TAB            0x09

#define VK_CLEAR          0x0C
#define VK_RETURN         0x0D

#define VK_SHIFT          0x10
#define VK_CONTROL        0x11
#define VK_MENU           0x12
#define VK_PAUSE          0x13
#define VK_CAPITAL        0x14

#define VK_ESCAPE         0x1B

#define VK_CONVERT        0x1C
#define VK_NONCONVERT     0x1D
#define VK_ACCEPT         0x1E
#define VK_MODECHANGE     0x1F

#define VK_SPACE          0x20
#define VK_PRIOR          0x21
#define VK_NEXT           0x22
#define VK_END            0x23
#define VK_HOME           0x24
#define VK_LEFT           0x25
#define VK_UP             0x26
#define VK_RIGHT          0x27
#define VK_DOWN           0x28
#define VK_SELECT         0x29
#define VK_PRINT          0x2A
#define VK_EXECUTE        0x2B
#define VK_SNAPSHOT       0x2C
#define VK_INSERT         0x2D
#define VK_DELETE         0x2E
#define VK_HELP           0x2F

#define VK_LWIN           0x5B
#define VK_RWIN           0x5C
#define VK_APPS           0x5D

#define VK_SLEEP          0x5F

#define VK_NUMPAD0        0x60
#define VK_NUMPAD1        0x61
#define VK_NUMPAD2        0x62
#define VK_NUMPAD3        0x63
#define VK_NUMPAD4        0x64
#define VK_NUMPAD5        0x65
#define VK_NUMPAD6        0x66
#define VK_NUMPAD7        0x67
#define VK_NUMPAD8        0x68
#define VK_NUMPAD9        0x69
#define VK_MULTIPLY       0x6A
#define VK_ADD            0x6B
#define VK_SEPARATOR      0x6C
#define VK_SUBTRACT       0x6D
#define VK_DECIMAL        0x6E
#define VK_DIVIDE         0x6F
#define VK_F1             0x70
#define VK_F2             0x71
#define VK_F3             0x72
#define VK_F4             0x73
#define VK_F5             0x74
#define VK_F6             0x75
#define VK_F7             0x76
#define VK_F8             0x77
#define VK_F9             0x78
#define VK_F10            0x79
#define VK_F11            0x7A
#define VK_F12            0x7B
#define VK_F13            0x7C
#define VK_F14            0x7D
#define VK_F15            0x7E
#define VK_F16            0x7F
#define VK_F17            0x80
#define VK_F18            0x81
#define VK_F19            0x82
#define VK_F20            0x83
#define VK_F21            0x84
#define VK_F22            0x85
#define VK_F23            0x86
#define VK_F24            0x87

#define VK_NUMLOCK        0x90
#define VK_SCROLL         0x91

#define VK_LSHIFT         0xA0
#define VK_RSHIFT         0xA1
#define VK_LCONTROL       0xA2
#define VK_RCONTROL       0xA3
#define VK_LMENU          0xA4
#define VK_RMENU          0xA5

#define VK_OEM_1          0xBA   // ';:' for US
#define VK_OEM_PLUS       0xBB   // '+' any country
#define VK_OEM_COMMA      0xBC   // ',' any country
#define VK_OEM_MINUS      0xBD   // '-' any country
#define VK_OEM_PERIOD     0xBE   // '.' any country
#define VK_OEM_2          0xBF   // '/?' for US
#define VK_OEM_3          0xC0   // '`~' for US

#define VK_OEM_4          0xDB  //  '[{' for US
#define VK_OEM_5          0xDC  //  '\|' for US
#define VK_OEM_6          0xDD  //  ']}' for US
#define VK_OEM_7          0xDE  //  ''"' for US
#define VK_OEM_8          0xDF

#endif

typedef struct ConsoleBuffer
{
    int width;
    int height;

    ConsolePixel* _buffer;
} ConsoleBuffer;

ConsoleBuffer ConsoleBuffer_create(int width, int height);
ConsoleBuffer ConsoleBuffer_copy(const ConsoleBuffer* consoleBuffer);
void ConsoleBuffer_destroy(ConsoleBuffer* consoleBuffer);

void ConsoleBuffer_setChar(ConsoleBuffer* consoleBuffer, int x, int y, char c);

void ConsoleBuffer_setAttrib(ConsoleBuffer* consoleBuffer, int x, int y, uint16_t attrib);
void ConsoleBuffer_setForegroundAttrib(ConsoleBuffer* consoleBuffer, int x, int y, uint8_t flags);
void ConsoleBuffer_setBackgroundAttrib(ConsoleBuffer* consoleBuffer, int x, int y, uint8_t flags);

ConsolePixel ConsoleBuffer_getPixel(ConsoleBuffer* consoleBuffer, int x, int y);

void ConsoleBuffer_drawText(ConsoleBuffer* consoleBuffer, const char* text, int x, int y, uint16_t attrib);
void ConsoleBuffer_drawRect(ConsoleBuffer* consoleBuffer, int x, int y, int width, int height, char c, uint16_t attrib);
void ConsoleBuffer_drawLine(ConsoleBuffer* consoleBuffer, int x1, int y1, int x2, int y2, char c, uint16_t attrib);

void ConsoleBuffer_clear(ConsoleBuffer* consoleBuffer, char c, uint16_t attrib);

typedef struct Console
{
    #ifdef _WIN32
    HANDLE _writeHandle;
    HANDLE _readHandle;

    DWORD _previousWriteMode;
    DWORD _previousReadMode;
    CONSOLE_CURSOR_INFO _previousCursorInfo;
    SMALL_RECT _previousWindowSize;
    COORD _previousBufferSize;
    #endif

    ConsoleBuffer consoleBuffer;

    ConsoleEvent* _eventBuffer;
    uint16_t _numEvents;
    uint16_t _eventIter;

    COORD _mousePos;
    char _leftMousePressed;
    char _leftMousePressedLastFrame;
    char _rightMousePressed;
    char _rightMousePressedLastFrame;
    char _keysPressed[256];
} Console;

Console Console_create(int width, int height, const char* title);
void Console_destroy(Console* console);

// Call once per frame
void Console_refreshEvents(Console* console);
int Console_pollEvent(Console* console, ConsoleEvent* consoleEvent);

int Console_getMouseX(const Console* console);
int Console_getMouseY(const Console* console);
char Console_isLeftMousePressed(const Console* console);
char Console_isLeftMouseJustPressed(const Console* console);
char Console_isLeftMouseJustReleased(const Console* console);
char Console_isRightMousePressed(const Console* console);
char Console_isRightMouseJustPressed(const Console* console);
char Console_isRightMouseJustReleased(const Console* console);
char Console_isKeyPressed(const Console* console, uint8_t key);

void Console_clearWindow(Console* console, uint16_t attrib);

void Console_display(Console* console);

#ifndef _WIN32
void _linux_getWinColour(uint8_t flag, uint8_t* r, uint8_t* g, uint8_t* b);
#endif