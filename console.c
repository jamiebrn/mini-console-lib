#include "console.h"

ConsoleBuffer ConsoleBuffer_create(int width, int height)
{
    ConsoleBuffer buffer;
    buffer.width = width;
    buffer.height = height;

    int bufferMemSize = width * height * sizeof(CHAR_INFO);
    buffer._buffer = malloc(bufferMemSize);
    memset(buffer._buffer, 0, bufferMemSize);

    return buffer;
}

void ConsoleBuffer_destroy(ConsoleBuffer* consoleBuffer)
{
    free(consoleBuffer->_buffer);
    consoleBuffer->_buffer = NULL;
}

void ConsoleBuffer_setChar(ConsoleBuffer* consoleBuffer, int x, int y, char c)
{
    consoleBuffer->_buffer[x + y * consoleBuffer->width].Char.AsciiChar = c;
}

void ConsoleBuffer_setAttrib(ConsoleBuffer* consoleBuffer, int x, int y, DWORD attrib)
{
    consoleBuffer->_buffer[x + y * consoleBuffer->width].Attributes = attrib;
}

void ConsoleBuffer_setForegroundAttrib(ConsoleBuffer* consoleBuffer, int x, int y, uint8_t flags)
{
    CHAR_INFO* bufferPtr = &consoleBuffer->_buffer[x + y * consoleBuffer->width];
    bufferPtr->Attributes = (flags & 0xF) | (bufferPtr->Attributes & 0xF0);
}

void ConsoleBuffer_setBackgroundAttrib(ConsoleBuffer* consoleBuffer, int x, int y, uint8_t flags)
{
    CHAR_INFO* bufferPtr = &consoleBuffer->_buffer[x + y * consoleBuffer->width];
    bufferPtr->Attributes = ((flags & 0xF) << 4) | (bufferPtr->Attributes & 0xF);
}

void ConsoleBuffer_drawText(ConsoleBuffer* consoleBuffer, const char* text, int x, int y, WORD attrib)
{
    int i = 0;
    while (1)
    {
        char c = text[i];
        if (c == '\0') break;
        ConsoleBuffer_setChar(consoleBuffer, x + i, y, c);
        ConsoleBuffer_setAttrib(consoleBuffer, x + i, y, attrib);
        i++;
    }
}

void ConsoleBuffer_drawRect(ConsoleBuffer* consoleBuffer, int x, int y, int width, int height, char c, WORD attrib)
{
    int widthSign = 1;
    int heightSign = 1;

    if (width < 0)
    {
        x--;
        widthSign = -1;
    }
    if (height < 0)
    {
        y--;
        heightSign = -1;
    }

    for (int i = 0; abs(i) < abs(width); i += widthSign)
    {
        for (int j = 0; abs(j) < abs(height); j += heightSign)
        {
            ConsoleBuffer_setChar(consoleBuffer, x + i, y + j, c);
            ConsoleBuffer_setAttrib(consoleBuffer, x + i, y + j, attrib);
        }
    }
}

void ConsoleBuffer_drawLine(ConsoleBuffer* consoleBuffer, int x1, int y1, int x2, int y2, char c, WORD attrib)
{
    if (x2 - x1 == 0)
    {
        ConsoleBuffer_drawRect(consoleBuffer, x1, y1, 1, y2 - y1, c, attrib);
        return;
    }

    if (y2 - y1 == 0)
    {
        ConsoleBuffer_drawRect(consoleBuffer, x1, y1, x2 - x1, 1, c, attrib);
        return;
    }

    float grad = (float)(y2 - y1) / (float)(x2 - x1);
    int xSign = (x2 - x1) >= 0 ? 1 : -1;
    float xStep = (float)xSign / (float)max(abs(y2 - y1), abs(x2 - x1));
    for (float i = 0; abs(i) < abs(x2 - x1); i += xStep)
    {
        float x = x1 + i;
        int y = y1 + i * grad;
        ConsoleBuffer_setChar(consoleBuffer, x, y, c);
        ConsoleBuffer_setAttrib(consoleBuffer, x, y, attrib);
    }
}

void ConsoleBuffer_clear(ConsoleBuffer* consoleBuffer, char c, DWORD attrib)
{
    int bufferSize = consoleBuffer->width * consoleBuffer->height;

    if (c == 0 && attrib == 0)
    {
        memset(consoleBuffer->_buffer, 0, bufferSize * sizeof(CHAR_INFO));
        return;
    }

    CHAR_INFO charInfo;
    charInfo.Char.AsciiChar = c;
    charInfo.Attributes = attrib;

    for (int i = 0; i < bufferSize; i++)
    {
        consoleBuffer->_buffer[i] = charInfo;
    }
}

Console Console_create(int width, int height, const char* title)
{
    Console console;
    memset(&console, 0, sizeof(console));

    console.consoleBuffer = ConsoleBuffer_create(width, height);

    console._writeHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    console._readHandle = GetStdHandle(STD_INPUT_HANDLE);

    GetConsoleMode(console._writeHandle, &console._previousWriteMode);
    GetConsoleMode(console._readHandle, &console._previousReadMode);
    SetConsoleMode(console._writeHandle, console._previousWriteMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    SetConsoleMode(console._readHandle, (console._previousReadMode | ENABLE_MOUSE_INPUT | ENABLE_EXTENDED_FLAGS) & ~(ENABLE_QUICK_EDIT_MODE));

    GetConsoleCursorInfo(console._writeHandle, &console._previousCursorInfo);
    CONSOLE_CURSOR_INFO cursorInfo = console._previousCursorInfo;
    cursorInfo.bVisible = 0;
    SetConsoleCursorInfo(console._writeHandle, &cursorInfo);

    SetConsoleTitle(title);

    COORD bufferSize = {width, height};
    SetConsoleScreenBufferSize(console._writeHandle, bufferSize);

    Console_clearWindow(&console, 0);

    return console;
}

void Console_destroy(Console* console)
{
    Console_clearWindow(console, 0);

    ConsoleBuffer_destroy(&console->consoleBuffer);

    if (console->_eventBuffer)
    {
        free(console->_eventBuffer);
    }

    SetConsoleMode(console->_writeHandle, console->_previousWriteMode);
    SetConsoleMode(console->_readHandle, console->_previousReadMode);
    SetConsoleCursorInfo(console->_writeHandle, &console->_previousCursorInfo);
}

void Console_refreshEvents(Console* console)
{
    DWORD numEvents;
    GetNumberOfConsoleInputEvents(console->_readHandle, &numEvents);
    if (numEvents > 0)
    {
        if (console->_eventBuffer)
        {
            console->_eventBuffer = realloc(console->_eventBuffer, numEvents * sizeof(INPUT_RECORD));
        }
        else
        {
            console->_eventBuffer = malloc(numEvents * sizeof(INPUT_RECORD));
        }

        ReadConsoleInput(console->_readHandle, console->_eventBuffer, numEvents, &console->_numEvents);
        console->_eventIter = 0;
    }

    console->_leftMousePressedLastFrame = console->_leftMousePressed;
    console->_rightMousePressedLastFrame = console->_rightMousePressed;
}

int Console_pollEvent(Console* console, ConsoleEvent* consoleEvent)
{
    if (console->_eventIter >= console->_numEvents)
    {
        return 0;
    }

    *consoleEvent= console->_eventBuffer[console->_eventIter];
    console->_eventIter++;
    
    if (consoleEvent->EventType == KEY_EVENT)
    {
        console->_keysPressed[consoleEvent->Event.KeyEvent.wVirtualKeyCode] = consoleEvent->Event.KeyEvent.bKeyDown;
    }

    if (consoleEvent->EventType == MOUSE_EVENT)
    {
        console->_mousePos.X = min(consoleEvent->Event.MouseEvent.dwMousePosition.X, console->consoleBuffer.width - 1);
        console->_mousePos.Y = min(consoleEvent->Event.MouseEvent.dwMousePosition.Y, console->consoleBuffer.height - 1);
        console->_leftMousePressed = consoleEvent->Event.MouseEvent.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED;
        console->_rightMousePressed = consoleEvent->Event.MouseEvent.dwButtonState & RIGHTMOST_BUTTON_PRESSED;
    }

    return 1;
}

int Console_getMouseX(const Console* console)
{
    return console->_mousePos.X;
}

int Console_getMouseY(const Console* console)
{
    return console->_mousePos.Y;
}

char Console_isLeftMousePressed(const Console* console)
{
    return console->_leftMousePressed;
}

char Console_isLeftMouseJustPressed(const Console* console)
{
    return console->_leftMousePressed && !console->_leftMousePressedLastFrame;
}

char Console_isLeftMouseJustReleased(const Console* console)
{
    return !console->_leftMousePressed && console->_leftMousePressedLastFrame;
}

char Console_isRightMousePressed(const Console* console)
{
    return console->_rightMousePressed;
}

char Console_isRightMouseJustPressed(const Console* console)
{
    return console->_rightMousePressed && !console->_rightMousePressedLastFrame;
}

char Console_isRightMouseJustReleased(const Console* console)
{
    return !console->_rightMousePressed && console->_rightMousePressedLastFrame;
}

char Console_isKeyPressed(const Console* console, uint8_t key)
{
    return console->_keysPressed[key];
}

void Console_clearWindow(Console* console, WORD attrib)
{
    CONSOLE_SCREEN_BUFFER_INFO bufferInfo;
    DWORD cellCount;
    DWORD written;
    COORD home = {0, 0};
    
    GetConsoleScreenBufferInfo(console->_writeHandle, &bufferInfo);
    cellCount = bufferInfo.dwSize.X * bufferInfo.dwSize.Y;
    FillConsoleOutputCharacter(console->_writeHandle, ' ', cellCount, home, &written);
    FillConsoleOutputAttribute(console->_writeHandle, attrib, cellCount, home, &written);
    SetConsoleCursorPosition(console->_writeHandle, home);
}

void Console_display(Console* console)
{
    COORD charBufSize = {console->consoleBuffer.width, console->consoleBuffer.height};
    COORD characterPos = {0, 0};
    SMALL_RECT writeArea = {0, 0, console->consoleBuffer.width - 1, console->consoleBuffer.height - 1}; 

    WriteConsoleOutputA(console->_writeHandle, console->consoleBuffer._buffer, charBufSize, characterPos, &writeArea);
}