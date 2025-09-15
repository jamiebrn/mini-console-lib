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
#define UNDO_HISTORY_MAX 30

void drawingBuffers_shiftRight(ConsoleBuffer* drawingBuffers)
{
    ConsoleBuffer_destroy(&drawingBuffers[UNDO_HISTORY_MAX - 1]);

    for (int i = UNDO_HISTORY_MAX - 1; i > 0; i--)
    {
        drawingBuffers[i] = drawingBuffers[i - 1];
    }

    drawingBuffers[0] = ConsoleBuffer_copy(&drawingBuffers[1]);
}

void drawingBuffers_shiftLeft(ConsoleBuffer* drawingBuffers)
{
    ConsoleBuffer_destroy(&drawingBuffers[0]);

    for (int i = 0; i < UNDO_HISTORY_MAX - 1; i++)
    {
        drawingBuffers[i] = drawingBuffers[i + 1];
    }

    drawingBuffers[UNDO_HISTORY_MAX - 1] = ConsoleBuffer_copy(&drawingBuffers[UNDO_HISTORY_MAX - 2]);
}

uint8_t getSelectedColour(uint8_t selected_colour)
{
    uint8_t colour = 0;
    colour |= ((selected_colour & 0x1) << 3);
    colour |= ((selected_colour >> 1) & 0x7);
    return colour;
}

int main(int argc, char* argv[])
{
    const char* title = "Epic Console Drawing";
    const int titleLen = strlen(title);

    Console console = Console_create(SCREEN_WIDTH, SCREEN_HEIGHT, title);

    ConsoleBuffer drawingBuffers[UNDO_HISTORY_MAX];
    for (int i = 0; i < UNDO_HISTORY_MAX; i++)
    {
        drawingBuffers[i] = ConsoleBuffer_create(SCREEN_WIDTH, SCREEN_HEIGHT);
    }

    ConsoleBuffer* drawingBuffer = &drawingBuffers[UNDO_HISTORY_MAX - 1];

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
                if (event.Event.KeyEvent.bKeyDown)
                {
                    if (event.Event.KeyEvent.wVirtualKeyCode == VK_ESCAPE)
                    {
                        running = false;
                    }
                    if (event.Event.KeyEvent.wVirtualKeyCode == VK_SPACE)
                    {
                        drawingBuffers_shiftLeft(drawingBuffers);
                        ConsoleBuffer_clear(drawingBuffer, 0, 0);
                    }

                    if (event.Event.KeyEvent.wVirtualKeyCode == VK_LEFT)
                    {
                        selectedColour = ((selectedColour - 1) % 15 + 15) % 15;
                    }
                    if (event.Event.KeyEvent.wVirtualKeyCode == VK_RIGHT)
                    {
                        selectedColour = ((selectedColour + 1) % 15 + 15) % 15;
                    }

                    if (event.Event.KeyEvent.wVirtualKeyCode == 0x5A && Console_isKeyPressed(&console, VK_CONTROL)) // ctrl z
                    {
                        drawingBuffers_shiftRight(drawingBuffers);
                    }
                }
                else
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
                uint8_t attrib = getSelectedColour(selectedColour + 1) << 4;

                // Delete if right mouse button
                if (Console_isRightMouseJustReleased(&console))
                {
                    c = 0;
                    attrib = 0;
                }

                if (Console_isKeyPressed(&console, VK_SHIFT))
                {
                    drawingBuffers_shiftLeft(drawingBuffers);
                    ConsoleBuffer_drawLine(drawingBuffer, shapeStartX, shapeStartY, mouseX, mouseY, c, attrib);
                }
                else if (Console_isKeyPressed(&console, VK_CONTROL))
                {
                    drawingBuffers_shiftLeft(drawingBuffers);
                    ConsoleBuffer_drawRect(drawingBuffer, shapeStartX, shapeStartY, mouseX - shapeStartX + 1, mouseY - shapeStartY + 1, c, attrib);
                }

                drawingShape = false;
            }
        }

        if (!drawingShape)
        {
            if (Console_isLeftMousePressed(&console) && ConsoleBuffer_getPixel(drawingBuffer, mouseX, mouseY).Attributes != getSelectedColour(selectedColour + 1) << 4)
            {
                drawingBuffers_shiftLeft(drawingBuffers);
                ConsoleBuffer_setChar(drawingBuffer, mouseX, mouseY, 0);
                ConsoleBuffer_setBackgroundAttrib(drawingBuffer, mouseX, mouseY, getSelectedColour(selectedColour + 1));
            }
            else if (Console_isRightMousePressed(&console) && ConsoleBuffer_getPixel(drawingBuffer, mouseX, mouseY).Attributes != 0)
            {
                drawingBuffers_shiftLeft(drawingBuffers);
                ConsoleBuffer_setChar(drawingBuffer, mouseX, mouseY, 0);
                ConsoleBuffer_setBackgroundAttrib(drawingBuffer, mouseX, mouseY, 0);
            }
        }

        ConsoleBuffer_clear(&console.consoleBuffer, 0, 0);

        memcpy(console.consoleBuffer._buffer, drawingBuffer->_buffer, console.consoleBuffer.width * console.consoleBuffer.height * sizeof(CHAR_INFO));

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
        ConsoleBuffer_drawLine(&console.consoleBuffer, 0, 0, 0, console.consoleBuffer.height, ' ', 8 << 4);
        ConsoleBuffer_drawLine(&console.consoleBuffer, 0, 0, console.consoleBuffer.width, 0, ' ', 8 << 4);
        ConsoleBuffer_drawLine(&console.consoleBuffer, console.consoleBuffer.width - 1, 0, console.consoleBuffer.width - 1, console.consoleBuffer.height, ' ', 8 << 4);
        ConsoleBuffer_drawRect(&console.consoleBuffer, 0, console.consoleBuffer.height - 2, console.consoleBuffer.width, 2, ' ', 8 << 4);

        // Title/tooltips
        int startTitle = SCREEN_WIDTH / 2 - titleLen / 2;
        ConsoleBuffer_drawText(&console.consoleBuffer, title, startTitle, 0, 0x8F);

        ConsoleBuffer_drawText(&console.consoleBuffer, "Space to clear", 0, SCREEN_HEIGHT - 2, 0x8F);
        ConsoleBuffer_drawText(&console.consoleBuffer, "Escape to quit", 0, SCREEN_HEIGHT - 1, 0x8F);
        ConsoleBuffer_drawText(&console.consoleBuffer, "Shift for line tool", 18, SCREEN_HEIGHT - 2, 0x8F);
        ConsoleBuffer_drawText(&console.consoleBuffer, "Ctrl for rect tool", 18, SCREEN_HEIGHT - 1, 0x8F);

        // Colour selection
        const int colourSelectionX = 43;
        const int colourWidth = 2;
        ConsoleBuffer_setChar(&console.consoleBuffer, colourSelectionX, SCREEN_HEIGHT - 2, '<');
        ConsoleBuffer_setForegroundAttrib(&console.consoleBuffer, colourSelectionX, SCREEN_HEIGHT - 2, 0xF);
        ConsoleBuffer_setChar(&console.consoleBuffer, colourSelectionX + 16 * colourWidth + 1, SCREEN_HEIGHT - 2, '>');
        ConsoleBuffer_setForegroundAttrib(&console.consoleBuffer, colourSelectionX + 16 * colourWidth + 1, SCREEN_HEIGHT - 2, 0xF);

        for (int i = 1; i < 16; i++)
        {
            uint8_t colour = getSelectedColour(i);

            for (int j = 0; j < colourWidth; j++)
            {
                ConsoleBuffer_setChar(&console.consoleBuffer, colourSelectionX + i * colourWidth + j, SCREEN_HEIGHT - 2, ' ');
                ConsoleBuffer_setBackgroundAttrib(&console.consoleBuffer, colourSelectionX + i * colourWidth + j, SCREEN_HEIGHT - 2, colour);
            }
            
            if ((selectedColour + 1) == i)
            {
                ConsoleBuffer_setChar(&console.consoleBuffer, colourSelectionX + i * colourWidth, SCREEN_HEIGHT - 1, '^');
                ConsoleBuffer_setForegroundAttrib(&console.consoleBuffer, colourSelectionX + i * colourWidth, SCREEN_HEIGHT - 1, 0xF);
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