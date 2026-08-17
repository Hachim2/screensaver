#include "video.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifndef FFMPEG_PATH
#define FFMPEG_PATH "ffmpeg.exe"
#endif

#ifndef FFPROBE_PATH
#define FFPROBE_PATH "ffprobe.exe"
#endif

#define MAX_VIDEO_WIDTH  2560
#define MAX_VIDEO_HEIGHT 1440

bool Video_Init(Video *video, const char *filename, int width, int height) {
    memset(video, 0, sizeof(Video));
    snprintf(video->filename, sizeof(video->filename), "%s", filename);

    char probe_cmd[2048];
    char probe_output[256] = {0};

    snprintf(probe_cmd,sizeof(probe_cmd),"cmd /c \"\"%s\" -v error -select_streams v:0 -show_entries stream=width,height,r_frame_rate -of csv=s=x:p=0 \"%s\"\"",FFPROBE_PATH,filename);

    printf("Commande FFprobe : %s\n", probe_cmd);

    FILE *probe = _popen(probe_cmd, "r");

    if (probe != NULL) {
        if (fgets(probe_output, sizeof(probe_output), probe) != NULL) {
            int source_w = 0;
            int source_h = 0;
            int fps_num = 0;
            int fps_den = 0;

            if (sscanf(probe_output, "%dx%dx%d/%d", &source_w, &source_h, &fps_num, &fps_den) == 4) {
                video->source_width = source_w;
                video->source_height = source_h;

                if (fps_den != 0) {
                    video->fps = (double)fps_num / (double)fps_den;
                }
            }
        }

        _pclose(probe);
    }

    if (video->source_width <= 0)
        video->source_width = width > 0 ? width : 1920;

    if (video->source_height <= 0)
        video->source_height = height > 0 ? height : 1080;

    if (video->fps <= 0.0)
        video->fps = 30.0;

    video->frame_duration = 1.0 / video->fps;
    printf("Video source : %dx%d @ %.2f FPS\n", video->source_width, video->source_height, video->fps);

    int screen_width = width > 0 ? width : 1920;
    int screen_height = height > 0 ? height : 1080;

    // =========================================================
    // Résolution maximale de décodage
    // =========================================================

    int max_width = screen_width;
    int max_height = screen_height;

    // On limite la résolution envoyée par FFmpeg
    if (max_width > MAX_VIDEO_WIDTH)
        max_width = MAX_VIDEO_WIDTH;

    if (max_height > MAX_VIDEO_HEIGHT)
        max_height = MAX_VIDEO_HEIGHT;


    // =========================================================
    // Calcul du scaling en conservant le ratio
    // =========================================================

    double scale_x =
        (double)max_width / (double)video->source_width;

    double scale_y =
        (double)max_height / (double)video->source_height;

    double scale =
        scale_x < scale_y ? scale_x : scale_y;

    // Ne jamais agrandir la vidéo
    if (scale > 1.0)
        scale = 1.0;


    video->width = (int)(video->source_width * scale);

    video->height = (int)(video->source_height * scale);


    // Dimensions paires pour FFmpeg
    video->width &= ~1;
    video->height &= ~1;

    video->width &= ~1;
    video->height &= ~1;

    if (video->width < 2)
        video->width = 2;

    if (video->height < 2)
        video->height = 2;

    printf("Resolution source : %dx%d\n", video->source_width, video->source_height);
    printf("Resolution de sortie : %dx%d\n", video->width, video->height);

    if (video->width < 1)
        video->width = 1;

    if (video->height < 1)
        video->height = 1;

    printf("Resolution de sortie : %dx%d\n", video->width, video->height);

    video->frame_size = video->width * video->height * 4;
    video->buffer = malloc(video->frame_size);

    if (video->buffer == NULL) {
        TraceLog(LOG_ERROR, "Unable to allocate the video buffer");
        return false;
    }

    char command[4096];

    snprintf(
        command,
        sizeof(command),
        "cmd /c \"\"%s\" -stream_loop -1 -hide_banner -loglevel error -i \"%s\" -vf \"scale=%d:%d:force_original_aspect_ratio=decrease\" -f rawvideo -pix_fmt rgba -an -\"",
        FFMPEG_PATH,
        filename,
        video->width,
        video->height
    );

    printf("FFmpeg Command : %s\n", command);

    video->pipe = _popen(command, "rb");

    if (video->pipe == NULL) {
        TraceLog(LOG_ERROR, "Impossible de lancer FFmpeg");
        free(video->buffer);
        video->buffer = NULL;
        return false;
    }

    Image image = {
        .data = video->buffer,
        .width = video->width,
        .height = video->height,
        .mipmaps = 1,
        .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8
    };

    video->texture = LoadTextureFromImage(image);

    if (video->texture.id == 0) {
        TraceLog(LOG_ERROR, "Impossible de créer la texture vidéo");
        _pclose(video->pipe);
        video->pipe = NULL;
        free(video->buffer);
        video->buffer = NULL;
        return false;
    }

    video->playing = true;
    video->next_frame_time = GetTime();
    printf("Video initialisee avec succes !\n");
    return true;
}



bool Video_Update(Video *video) {
    if (!video->playing)
        return false;

    double current_time = GetTime();

    if (current_time < video->next_frame_time)
        return false;

    double read_start = GetTime();
    size_t total_read = 0;

    while (total_read < (size_t)video->frame_size) {
        size_t result = fread(video->buffer + total_read, 1, video->frame_size - total_read, video->pipe);

        if (result == 0) {
            video->playing = false;
            return false;
        }

        total_read += result;
    }

    double read_time = (GetTime() - read_start) * 1000.0;
    double texture_start = GetTime();
    UpdateTexture(video->texture, video->buffer);
    double texture_time = (GetTime() - texture_start) * 1000.0;

    video->next_frame_time += video->frame_duration;

    if (video->next_frame_time < current_time - 1.0) {
        video->next_frame_time = current_time + video->frame_duration;
    }

    static int frame_counter = 0;
    frame_counter++;

    if (frame_counter >= 60) {
        printf(
            "\n--- PROFILING VIDEO ---\n"
            "Resolution : %dx%d\n"
            "FPS video : %.2f\n"
            "Frame size : %.2f MB\n"
            "fread : %.3f ms\n"
            "UpdateTexture : %.3f ms\n"
            "-----------------------\n",
            video->width,
            video->height,
            video->fps,
            (double)video->frame_size / (1024.0 * 1024.0),
            read_time,
            texture_time
        );
        frame_counter = 0;
    }

    return true;
}



void Video_Draw(Video *video, float alpha) {
    float screen_width = (float)GetScreenWidth();
    float screen_height = (float)GetScreenHeight();
    float video_width = (float)video->width;
    float video_height = (float)video->height;
    float video_ratio = video_width / video_height;
    float screen_ratio = screen_width / screen_height;

    Rectangle source;

    if (video_ratio > screen_ratio) {
        float visible_width = video_height * screen_ratio;
        source = (Rectangle){
            (video_width - visible_width) / 2.0f,
            0,
            visible_width,
            video_height
        };
    } else {
        float visible_height = video_width / screen_ratio;
        source = (Rectangle){
            0,
            (video_height - visible_height) / 2.0f,
            video_width,
            visible_height
        };
    }

    Rectangle destination = {0, 0, screen_width, screen_height};
    DrawTexturePro(video->texture, source, destination, (Vector2){0, 0}, 0.0f, Fade(WHITE, alpha));
}

void Video_Close(Video *video) {
    if (video->pipe != NULL) {
        _pclose(video->pipe);
        video->pipe = NULL;
    }

    if (video->texture.id != 0) {
        UnloadTexture(video->texture);
        video->texture.id = 0;
    }

    if (video->buffer != NULL) {
        free(video->buffer);
        video->buffer = NULL;
    }

    video->playing = false;
}