//
// --- Mini Console library
//

#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>

typedef struct ConsoleBuffer
{
    int width;
    int height;

    CHAR_INFO* _buffer;
} ConsoleBuffer;

ConsoleBuffer ConsoleBuffer_create(int width, int height);
void ConsoleBuffer_destroy(ConsoleBuffer* consoleBuffer);

void ConsoleBuffer_setChar(ConsoleBuffer* consoleBuffer, int x, int y, char c);

void ConsoleBuffer_setAttrib(ConsoleBuffer* consoleBuffer, int x, int y, DWORD attrib);
void ConsoleBuffer_setForegroundAttrib(ConsoleBuffer* consoleBuffer, int x, int y, uint8_t flags);
void ConsoleBuffer_setBackgroundAttrib(ConsoleBuffer* consoleBuffer, int x, int y, uint8_t flags);

void ConsoleBuffer_drawText(ConsoleBuffer* consoleBuffer, const char* text, int x, int y, WORD attrib);
void ConsoleBuffer_drawRect(ConsoleBuffer* consoleBuffer, int x, int y, int width, int height, char c, WORD attrib);
void ConsoleBuffer_drawLine(ConsoleBuffer* consoleBuffer, int x1, int y1, int x2, int y2, char c, WORD attrib);

void ConsoleBuffer_clear(ConsoleBuffer* consoleBuffer, char c, DWORD attrib);

typedef struct Console
{
    HANDLE _writeHandle;
    HANDLE _readHandle;

    DWORD _previousWriteMode;
    DWORD _previousReadMode;
    CONSOLE_CURSOR_INFO _previousCursorInfo;

    ConsoleBuffer consoleBuffer;

    INPUT_RECORD* _eventBuffer;
    DWORD _numEvents;
    DWORD _eventIter;

    COORD _mousePos;
    char _leftMousePressed;
    char _leftMousePressedLastFrame;
    char _rightMousePressed;
    char _rightMousePressedLastFrame;
    char _keysPressed[256];
} Console;

typedef INPUT_RECORD ConsoleEvent;
typedef CHAR_INFO ConsolePixel;

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

void Console_clearWindow(Console* console, WORD attrib);

void Console_display(Console* console);