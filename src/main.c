// Inclure raylib uniquement (pas de conflits !)
#include "raylib.h"
#include "inactivity.h"
#include "video.h"
#include <stdio.h>

int main()
{
    while (1)
    {
        uint32_t inactivity = get_inactivity_time();
        printf("Inactivité : %lu ms\n", inactivity);
        wait_ms(1000);

        if (inactivity >= 10000)
        {
            SetConfigFlags(FLAG_WINDOW_UNDECORATED);
            InitWindow(GetMonitorWidth(0), GetMonitorHeight(0), "Screensaver - Ma fenêtre Raylib");

            SetTargetFPS(60);

            Video video;

            if (!Video_Init(&video, "..\\videos\\logos.mp4", 0, 0))
            {
                CloseWindow();
                return 1;
            }

            while (!WindowShouldClose())
            {
                if (GetKeyPressed() == 0)
                {
                    BeginDrawing();
                    ClearBackground(BLACK);
                    Video_Update(&video);
                    Video_Draw(&video);
                    EndDrawing();
                }
                else
                {
                    break;
                }
            }

            Video_Close(&video);
            CloseWindow();
        }
    }

    return 0;
}