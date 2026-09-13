#include "raylib.h"
#include "inactivity.h"
#include "video.h"

#include <process.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifndef VIDEO_DIRECTORY
#define VIDEO_DIRECTORY "videos"
#endif

#define CHILD_ARGUMENT "--screensaver-child"
#define EXIT_ON_ACTIVITY_MS 1000

static char executable_path[1024];

static void RunScreensaver(int monitor) {
    SetConfigFlags(FLAG_FULLSCREEN_MODE | FLAG_VSYNC_HINT | FLAG_WINDOW_HIDDEN);
    InitWindow(0, 0, "Screensaver");
    SetWindowMonitor(monitor);
    SetWindowOpacity(0.0f);
    ClearWindowState(FLAG_WINDOW_HIDDEN);
    SetTargetFPS(60);

    Video video;
    float fade_alpha = 0.0f;
    char video_path[1024];
    snprintf(video_path, sizeof(video_path), "%s/minecraft.mp4", VIDEO_DIRECTORY);

    if (!Video_Init(&video, video_path, GetScreenWidth(), GetScreenHeight())) {
        SetWindowOpacity(1.0f);
        CloseWindow();
        return;
    }

    Vector2 last_mouse_position = GetMousePosition();
    bool input_ready = false;

    while (!WindowShouldClose()) {
        Vector2 mouse_position = GetMousePosition();
        bool mouse_moved = mouse_position.x != last_mouse_position.x ||
                           mouse_position.y != last_mouse_position.y;
        bool mouse_clicked = IsMouseButtonPressed(MOUSE_BUTTON_LEFT) ||
                             IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) ||
                             IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE);
        int key_pressed = GetKeyPressed();
        last_mouse_position = mouse_position;

        if (!input_ready) {
            mouse_moved = false;
            mouse_clicked = false;
            key_pressed = 0;
        }

        // GetLastInputInfo is system-wide, so every child closes when the
        // user interacts with any monitor.
        bool user_is_active = get_inactivity_time() < EXIT_ON_ACTIVITY_MS;
        if (user_is_active || key_pressed != 0 || mouse_clicked || mouse_moved) break;

        fade_alpha += GetFrameTime() * 0.16f;
        if (fade_alpha > 1.0f) fade_alpha = 1.0f;
        SetWindowOpacity(fade_alpha);

        BeginDrawing();
        ClearBackground(BLACK);
        Video_Update(&video);
        Video_Draw(&video, 1.0f);
        EndDrawing();

        last_mouse_position = GetMousePosition();
        input_ready = true;
    }

    SetWindowOpacity(1.0f);
    Video_Close(&video);
    CloseWindow();
}

static void StartScreensaverChild(int monitor) {
    char monitor_argument[32];
    snprintf(monitor_argument, sizeof(monitor_argument), "%d", monitor);

    intptr_t result = _spawnl(
        _P_NOWAIT,
        executable_path,
        executable_path,
        CHILD_ARGUMENT,
        monitor_argument,
        NULL
    );

    if (result == -1) {
        fprintf(stderr, "Unable to start screensaver for monitor %d\n", monitor);
    }
}

static int GetConnectedMonitorCount(void) {
    SetConfigFlags(FLAG_WINDOW_HIDDEN);
    InitWindow(1, 1, "Screensaver monitor detection");
    int monitor_count = GetMonitorCount();
    CloseWindow();

    return monitor_count > 0 ? monitor_count : 1;
}

int main(int argc, char *argv[]) {
    if (_fullpath(executable_path, argv[0], sizeof(executable_path)) == NULL) {
        snprintf(executable_path, sizeof(executable_path), "%s", argv[0]);
    }

    if (argc == 3 && strcmp(argv[1], CHILD_ARGUMENT) == 0) {
        RunScreensaver(atoi(argv[2]));
        return 0;
    }

    while (1) {
        uint32_t inactivity = get_inactivity_time();
        printf("Inactivity : %lu ms\n", (unsigned long)inactivity);
        wait_ms(1000);

        if (inactivity >= 10000) {
            int monitor_count = GetConnectedMonitorCount();

            for (int monitor = 0; monitor < monitor_count; monitor++) {
                StartScreensaverChild(monitor);
            }

            // Do not launch another group while the current screensaver is up.
            while (get_inactivity_time() >= EXIT_ON_ACTIVITY_MS) {
                wait_ms(250);
            }
        }
    }
}
