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


bool Video_Init(Video *video, const char *filename, int width, int height)
{
    memset(video, 0, sizeof(Video));

    snprintf(
        video->filename,
        sizeof(video->filename),
        "%s",
        filename
    );

    // ---------------------------------------------------------
    // Déterminer la résolution de la vidéo avec FFprobe
    // ---------------------------------------------------------

    char probe_cmd[1024];
    char probe_output[64] = {0};

    snprintf(
        probe_cmd,
        sizeof(probe_cmd),
        "cmd /c \"\"%s\" "
        "-v error "
        "-select_streams v:0 "
        "-show_entries stream=width,height "
        "-of csv=s=x:p=0 "
        "\"%s\"\"",
        FFPROBE_PATH,
        filename
    );
    
    printf("Commande FFprobe : %s\n", probe_cmd);
    FILE *probe = _popen(probe_cmd, "r");

    if (probe != NULL)
    {
        if (fgets(probe_output, sizeof(probe_output), probe) != NULL)
        {
            int source_w = 0;
            int source_h = 0;

            // FFprobe retourne par exemple : 1920x1080
            if (sscanf(probe_output, "%dx%d", &source_w, &source_h) == 2)
            {
                video->width = source_w;
                video->height = source_h;
            }
        }

        _pclose(probe);
    }

    // Si FFprobe n'a pas fonctionné
    if (video->width <= 0 || video->height <= 0)
    {
        video->width = width > 0 ? width : 1920;
        video->height = height > 0 ? height : 1080;
    }

    printf(
        "Resolution video : %dx%d\n",
        video->width,
        video->height
    );

    // ---------------------------------------------------------
    // Allocation du buffer
    // ---------------------------------------------------------

    // RGBA = 4 octets par pixel
    video->frame_size =
        video->width *
        video->height *
        4;

    video->buffer = malloc(video->frame_size);

    if (video->buffer == NULL)
    {
        TraceLog(
            LOG_ERROR,
            "Impossible d'allouer le buffer vidéo"
        );

        return false;
    }

    // ---------------------------------------------------------
    // Lancer FFmpeg
    // ---------------------------------------------------------

    char command[2048];

    snprintf(
        command,
        sizeof(command),
        "cmd /c \"\"%s\" "
        "-stream_loop -1 "
        "-hide_banner "
        "-loglevel error "
        "-i \"%s\" "
        "-f rawvideo "
        "-pix_fmt rgba "
        "-an "
        "-\"",
        FFMPEG_PATH,
        filename
    );

    printf("Lancement de FFmpeg...\n");

    printf("Commande FFmpeg : %s\n", command);
    video->pipe = _popen(command, "rb");

    if (video->pipe == NULL)
    {
        TraceLog(
            LOG_ERROR,
            "Impossible de lancer FFmpeg"
        );

        free(video->buffer);
        video->buffer = NULL;

        return false;
    }

    // ---------------------------------------------------------
    // Créer la texture Raylib
    // ---------------------------------------------------------

    Image image = {
        .data = video->buffer,
        .width = video->width,
        .height = video->height,
        .mipmaps = 1,
        .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8
    };

    video->texture = LoadTextureFromImage(image);

    if (video->texture.id == 0)
    {
        TraceLog(
            LOG_ERROR,
            "Impossible de créer la texture vidéo"
        );

        _pclose(video->pipe);
        video->pipe = NULL;

        free(video->buffer);
        video->buffer = NULL;

        return false;
    }

    video->playing = true;

    printf("Video initialisee avec succes !\n");

    return true;
}


// =============================================================
// Récupérer la prochaine frame
// =============================================================

bool Video_Update(Video *video)
{
    if (!video->playing)
        return false;

    size_t total_read = 0;

    while (total_read < (size_t)video->frame_size)
    {
        size_t result = fread(
            video->buffer + total_read,
            1,
            video->frame_size - total_read,
            video->pipe
        );

        if (result == 0)
        {
            video->playing = false;
            return false;
        }

        total_read += result;
    }

    UpdateTexture(
        video->texture,
        video->buffer
    );

    return true;
}


// =============================================================
// Affichage
// =============================================================

void Video_Draw(Video *video)
{
    DrawTexturePro(
        video->texture,

        (Rectangle){
            0,
            0,
            (float)video->width,
            (float)video->height
        },

        (Rectangle){
            0,
            0,
            (float)GetScreenWidth(),
            (float)GetScreenHeight()
        },

        (Vector2){
            0,
            0
        },

        0.0f,

        WHITE
    );
}


// =============================================================
// Fermeture
// =============================================================

void Video_Close(Video *video)
{
    if (video->pipe != NULL)
    {
        _pclose(video->pipe);
        video->pipe = NULL;
    }

    if (video->texture.id != 0)
    {
        UnloadTexture(video->texture);
        video->texture.id = 0;
    }

    if (video->buffer != NULL)
    {
        free(video->buffer);
        video->buffer = NULL;
    }

    video->playing = false;
}