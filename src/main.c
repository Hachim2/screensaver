#include "raylib.h"
#include "inactivity.h"
#include "video.h"

#include <stdio.h>
#include <stdint.h>

int main() {
    while (1) {
        uint32_t inactivity = get_inactivity_time();
        printf("Inactivité : %lu ms\n", (unsigned long)inactivity);
        wait_ms(1000);

        if (inactivity >= 10000) {
            SetConfigFlags(FLAG_WINDOW_UNDECORATED);
            InitWindow(GetMonitorWidth(0), GetMonitorHeight(0), "Screensaver - Ma fenêtre Raylib");
            SetTargetFPS(60);

            Video video;
            float fade_alpha = 0.0f;

            if (!Video_Init(&video, "..\\videos\\the_passage.mp4", GetMonitorWidth(0), GetMonitorHeight(0))) {
                CloseWindow();
                return 1;
            }

            while (!WindowShouldClose()){
                if (GetKeyPressed() == 0){
                    // Le fondu dure environ 0,5 seconde
                    fade_alpha += GetFrameTime() * 0.16f;

                    if (fade_alpha > 1.0f)
                        fade_alpha = 1.0f;

                    BeginDrawing();

                    ClearBackground(BLACK);

                    Video_Update(&video);

                    Video_Draw(&video, fade_alpha);

                    EndDrawing();
                }
                else{
                    break;
                }
            }

            Video_Close(&video);
            CloseWindow();
        }
    }

    return 0;
}
