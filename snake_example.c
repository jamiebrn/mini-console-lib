//
// -- Basic snake game console example
//

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>

#include "console.h"

#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 40

#define SNAKE_LENGTH_MAX 256
#define SNAKE_LENGTH_INIT 4

typedef struct Vec2
{
    int x;
    int y;
} Vec2;

int snake_init(Vec2* snake)
{
    memset(snake, 0, SNAKE_LENGTH_MAX * sizeof(Vec2));

    for (int i = 0; i < SNAKE_LENGTH_INIT; i++)
    {
        snake[i].x = (SCREEN_WIDTH / 4) - i;
        snake[i].y = SCREEN_HEIGHT / 2;
    }

    return SNAKE_LENGTH_INIT;
}

void snake_shift(Vec2* snake, int length)
{
    for (int i = length - 1; i > 0; i--)
    {
        snake[i] = snake[i - 1];
    }
}

bool snake_is_part_in_body(Vec2* snake, int length, Vec2 part)
{
    for (int i = 0; i < length; i++)
    {
        if (snake[i].x == part.x && snake[i].y == part.y) return true;
    }

    return false;
}

int sign(int value)
{
    if (value == 0) return 0;
    return value > 0 ? 1 : -1;
}

int main()
{
    srand(time(NULL));

    Console console = Console_create(SCREEN_WIDTH, SCREEN_HEIGHT, "Snake");

    Vec2 snake[SNAKE_LENGTH_MAX];
    int snakeLen = snake_init(snake);
    int dir = 0; // right, down, left, up

    Vec2 apple;
    apple.x = rand() % (SCREEN_WIDTH / 2);
    apple.y = rand() % SCREEN_HEIGHT;

    int moveTick = 0;
    const int MOVE_TICK_MAX = 230;

    bool gameOver = false;

    bool running = true;
    while (running)
    {
        Console_refreshEvents(&console);

        ConsoleEvent event;
        while (Console_pollEvent(&console, &event))
        {
            if (event.Event.KeyEvent.bKeyDown)
            {
                if (event.Event.KeyEvent.wVirtualKeyCode == VK_ESCAPE)
                {
                    running = false;
                }

                if (event.Event.KeyEvent.wVirtualKeyCode == 0x57 && dir != 1)
                {
                    dir = 3;
                }
                else if (event.Event.KeyEvent.wVirtualKeyCode == 0x41 && dir != 0)
                {
                    dir = 2;
                }
                else if (event.Event.KeyEvent.wVirtualKeyCode == 0x53 && dir != 3)
                {
                    dir = 1;
                }
                else if (event.Event.KeyEvent.wVirtualKeyCode == 0x44 && dir != 2)
                {
                    dir = 0;
                }
            }
        }

        moveTick++;
        if (moveTick >= MOVE_TICK_MAX && !gameOver)
        {
            moveTick = 0;
            snake_shift(snake, snakeLen);
            if (dir == 0)
            {
                snake[0].x = (snake[1].x + 1) % (SCREEN_WIDTH / 2);
            }
            else if (dir == 1)
            {
                snake[0].y = (snake[1].y + 1) % SCREEN_HEIGHT;
            }
            else if (dir == 2)
            {
                snake[0].x = ((snake[1].x - 1) % (SCREEN_WIDTH / 2) + SCREEN_WIDTH / 2) % (SCREEN_WIDTH / 2);
            }
            else if (dir == 3)
            {
                snake[0].y = ((snake[1].y - 1) % SCREEN_HEIGHT + SCREEN_HEIGHT) % SCREEN_HEIGHT;
            }

            if (snake_is_part_in_body(&snake[1], snakeLen - 1, snake[0]))
            {
                gameOver = true;
            }

            if (snake[0].x == apple.x && snake[0].y == apple.y)
            {
                apple.x = rand() % (SCREEN_WIDTH / 2);
                apple.y = rand() % SCREEN_HEIGHT;

                Vec2 newBody = snake[snakeLen - 1];

                int diffX = sign(snake[snakeLen - 1].x - snake[snakeLen - 2].x);
                int diffY = sign(snake[snakeLen - 1].y - snake[snakeLen - 2].y);

                newBody.x = ((newBody.x + diffX) % (SCREEN_WIDTH / 2) + SCREEN_WIDTH / 2) % (SCREEN_WIDTH / 2);
                newBody.y = ((newBody.y + diffY) % SCREEN_HEIGHT + SCREEN_HEIGHT) % SCREEN_HEIGHT;

                snake[snakeLen] = newBody;
                snakeLen++;
            }
        }

        ConsoleBuffer_clear(&console.consoleBuffer, 0, 0);

        ConsoleBuffer_drawRect(&console.consoleBuffer, apple.x * 2, apple.y, 2, 1, ' ', BACKGROUND_RED);

        for (int i = 0; i < snakeLen; i++)
        {
            ConsoleBuffer_drawRect(&console.consoleBuffer, snake[i].x * 2, snake[i].y, 2, 1, ' ', BACKGROUND_GREEN);
        }

        ConsoleBuffer_drawText(&console.consoleBuffer, "SCORE:", 0, 0, 15);
        char scoreStr[20];
        itoa(snakeLen - SNAKE_LENGTH_INIT, scoreStr, 10);
        ConsoleBuffer_drawText(&console.consoleBuffer, scoreStr, 8, 0, 15);
        
        if (gameOver)
        {
            ConsoleBuffer_drawText(&console.consoleBuffer, "Game Over!", SCREEN_WIDTH / 2 - 5, SCREEN_HEIGHT / 2, 15);
        }

        Console_display(&console);
    }

    Console_destroy(&console);
}