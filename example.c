//
// --- Console example (drawing) ---
//

#include <time.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#include "console.h"

#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 40

int main(int argc, char* argv[])
{
    const char* title = "Epic Console Drawing";
    const int titleLen = strlen(title);

    Console console = Console_create(SCREEN_WIDTH, SCREEN_HEIGHT, title);
    
    ConsoleBuffer drawingBuffer = ConsoleBuffer_create(SCREEN_WIDTH, SCREEN_HEIGHT);

    int shapeStartX = 0;
    int shapeStartY = 0;
    bool drawingShape = false;
    uint8_t selectedColour = 14;

    bool running = true;
    while (running)
    {
        Console_refreshEvents(&console);

        ConsoleEvent event;
        while (Console_pollEvent(&console, &event))
        {
            if (event.EventType == KEY_EVENT)
            {
                if (event.Event.KeyEvent.wVirtualKeyCode == VK_ESCAPE)
                {
                    running = false;
                }
                if (event.Event.KeyEvent.wVirtualKeyCode == VK_SPACE)
                {
                    ConsoleBuffer_clear(&drawingBuffer, 0, 0);
                }
                if (event.Event.KeyEvent.bKeyDown)
                {
                    if (event.Event.KeyEvent.wVirtualKeyCode == VK_LEFT)
                    {
                        selectedColour = ((selectedColour - 1) % 15 + 15) % 15;
                    }
                    if (event.Event.KeyEvent.wVirtualKeyCode == VK_RIGHT)
                    {
                        selectedColour = ((selectedColour + 1) % 15 + 15) % 15;
                    }
                }
                
                if (!event.Event.KeyEvent.bKeyDown)
                {
                    if (event.Event.KeyEvent.wVirtualKeyCode == VK_SHIFT || event.Event.KeyEvent.wVirtualKeyCode == VK_CONTROL)
                    {
                        drawingShape = false;
                    }
                }
            }
        }
        
        int mouseX = Console_getMouseX(&console);
        int mouseY = Console_getMouseY(&console);

        if (Console_isKeyPressed(&console, VK_SHIFT) || Console_isKeyPressed(&console, VK_CONTROL))
        {
            if (Console_isLeftMouseJustPressed(&console) || Console_isRightMouseJustPressed(&console))
            {
                shapeStartX = mouseX;
                shapeStartY = mouseY;
                drawingShape = true;
            }
            else if (Console_isLeftMouseJustReleased(&console) || Console_isRightMouseJustReleased(&console))
            {
                char c = ' ';
                uint8_t attrib = (selectedColour + 1) << 4;

                // Delete if right mouse button
                if (Console_isRightMouseJustReleased(&console))
                {
                    c = 0;
                    attrib = 0;
                }

                if (Console_isKeyPressed(&console, VK_SHIFT))
                {
                    ConsoleBuffer_drawLine(&drawingBuffer, shapeStartX, shapeStartY, mouseX, mouseY, c, attrib);
                }
                else if (Console_isKeyPressed(&console, VK_CONTROL))
                {
                    ConsoleBuffer_drawRect(&drawingBuffer, shapeStartX, shapeStartY, mouseX - shapeStartX + 1, mouseY - shapeStartY + 1, c, attrib);
                }

                drawingShape = false;
            }
        }

        if (!drawingShape)
        {
            if (Console_isLeftMousePressed(&console))
            {
                ConsoleBuffer_setChar(&drawingBuffer, mouseX, mouseY, 0);
                ConsoleBuffer_setBackgroundAttrib(&drawingBuffer, mouseX, mouseY, selectedColour + 1);
            }
            else if (Console_isRightMousePressed(&console))
            {
                ConsoleBuffer_setChar(&drawingBuffer, mouseX, mouseY, 0);
                ConsoleBuffer_setBackgroundAttrib(&drawingBuffer, mouseX, mouseY, 0);
            }
        }

        ConsoleBuffer_clear(&console.consoleBuffer, 0, 0);

        memcpy(console.consoleBuffer._buffer, drawingBuffer._buffer, console.consoleBuffer.width * console.consoleBuffer.height * sizeof(CHAR_INFO));

        if (drawingShape)
        {
            char c = ' ';
            uint8_t attrib = (selectedColour + 1) << 4;

            // Delete if holding right mouse button
            if (Console_isRightMousePressed(&console))
            {
                c = 0;
                attrib = 0;
            }

            if (Console_isKeyPressed(&console, VK_SHIFT))
            {
                ConsoleBuffer_drawLine(&console.consoleBuffer, shapeStartX, shapeStartY, mouseX, mouseY, c, attrib);
            }
            else if (Console_isKeyPressed(&console, VK_CONTROL))
            {
                ConsoleBuffer_drawRect(&console.consoleBuffer, shapeStartX, shapeStartY, mouseX - shapeStartX + 1, mouseY - shapeStartY + 1, c, attrib);
            }
        }

        // Draw border
        for (int x = 0; x < SCREEN_WIDTH; x++)
        {
            ConsoleBuffer_setChar(&console.consoleBuffer, x, 0, ' ');
            ConsoleBuffer_setBackgroundAttrib(&console.consoleBuffer, x, 0, 8);
            ConsoleBuffer_setChar(&console.consoleBuffer, x, SCREEN_HEIGHT - 2, ' ');
            ConsoleBuffer_setBackgroundAttrib(&console.consoleBuffer, x, SCREEN_HEIGHT - 2, 8);
            ConsoleBuffer_setChar(&console.consoleBuffer, x, SCREEN_HEIGHT - 1, ' ');
            ConsoleBuffer_setBackgroundAttrib(&console.consoleBuffer, x, SCREEN_HEIGHT - 1, 8);
        }

        for (int y = 0; y < SCREEN_HEIGHT; y++)
        {
            ConsoleBuffer_setChar(&console.consoleBuffer, 0, y, ' ');
            ConsoleBuffer_setBackgroundAttrib(&console.consoleBuffer, 0, y, 8);
            ConsoleBuffer_setChar(&console.consoleBuffer, SCREEN_WIDTH - 1, y, ' ');
            ConsoleBuffer_setBackgroundAttrib(&console.consoleBuffer, SCREEN_WIDTH - 1, y, 8);
        }

        // Title/tooltips
        int startTitle = SCREEN_WIDTH / 2 - titleLen / 2;
        ConsoleBuffer_drawText(&console.consoleBuffer, title, startTitle, 0, 0x8F);

        const char* spaceTooltip = "Space to clear";
        const char* escapeTooltip = "Escape to quit";
        const char* lineTooltip = "Shift for line tool";
        const char* rectTooltip = "Ctrl for rect tool";
        ConsoleBuffer_drawText(&console.consoleBuffer, spaceTooltip, 0, SCREEN_HEIGHT - 2, 0x8F);
        ConsoleBuffer_drawText(&console.consoleBuffer, escapeTooltip, 0, SCREEN_HEIGHT - 1, 0x8F);
        ConsoleBuffer_drawText(&console.consoleBuffer, lineTooltip, 18, SCREEN_HEIGHT - 2, 0x8F);
        ConsoleBuffer_drawText(&console.consoleBuffer, rectTooltip, 18, SCREEN_HEIGHT - 1, 0x8F);

        ConsoleBuffer_setChar(&console.consoleBuffer, 43, SCREEN_HEIGHT - 2, '<');
        ConsoleBuffer_setForegroundAttrib(&console.consoleBuffer, 43, SCREEN_HEIGHT - 2, 0xF);
        ConsoleBuffer_setChar(&console.consoleBuffer, 43 + 33, SCREEN_HEIGHT - 2, '>');
        ConsoleBuffer_setForegroundAttrib(&console.consoleBuffer, 43 + 33, SCREEN_HEIGHT - 2, 0xF);

        for (int i = 1; i < 16; i++)
        {
            ConsoleBuffer_setChar(&console.consoleBuffer, 43 + i * 2, SCREEN_HEIGHT - 2, ' ');
            ConsoleBuffer_setBackgroundAttrib(&console.consoleBuffer, 43 + i * 2, SCREEN_HEIGHT - 2, i);
            ConsoleBuffer_setChar(&console.consoleBuffer, 43 + i * 2 + 1, SCREEN_HEIGHT - 2, ' ');
            ConsoleBuffer_setBackgroundAttrib(&console.consoleBuffer, 43 + i * 2 + 1, SCREEN_HEIGHT - 2, i);
            
            if ((selectedColour + 1) == i)
            {
                ConsoleBuffer_setChar(&console.consoleBuffer, 43 + i * 2, SCREEN_HEIGHT - 1, '^');
                ConsoleBuffer_setForegroundAttrib(&console.consoleBuffer, 43 + i * 2, SCREEN_HEIGHT - 1, 0xF);
            }
        }

        ConsoleBuffer_setChar(&console.consoleBuffer, mouseX, mouseY, ' ');
        int cursorColour = 7;
        if (Console_isLeftMousePressed(&console)) cursorColour = 15;
        else if (Console_isRightMousePressed(&console)) cursorColour = 4;
        ConsoleBuffer_setBackgroundAttrib(&console.consoleBuffer, mouseX, mouseY, cursorColour);

        Console_display(&console);
    }

    Console_destroy(&console);

    return 0;
}