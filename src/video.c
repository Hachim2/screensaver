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


// =============================================================
// Initialisation
// =============================================================

bool Video_Init(Video *video,const char *filename,int width,int height){
    memset(video, 0, sizeof(Video));

    snprintf(
        video->filename,
        sizeof(video->filename),
        "%s",
        filename
    );


    // =========================================================
    // FFprobe
    // =========================================================

    char probe_cmd[2048];
    char probe_output[256] = {0};

    snprintf(
        probe_cmd,
        sizeof(probe_cmd),

        "cmd /c \"\"%s\" "
        "-v error "
        "-select_streams v:0 "
        "-show_entries stream=width,height,r_frame_rate "
        "-of csv=s=x:p=0 "
        "\"%s\"\"",

        FFPROBE_PATH,
        filename
    );

    printf(
        "Commande FFprobe : %s\n",
        probe_cmd
    );


    FILE *probe =
        _popen(probe_cmd, "r");


    if (probe != NULL)
    {
        if (
            fgets(
                probe_output,
                sizeof(probe_output),
                probe
            ) != NULL
        )
        {
            int source_w = 0;
            int source_h = 0;

            int fps_num = 0;
            int fps_den = 0;


            /*
             * Exemple :
             *
             * 3840x2160x60/1
             */

            if (
                sscanf(
                    probe_output,
                    "%dx%dx%d/%d",
                    &source_w,
                    &source_h,
                    &fps_num,
                    &fps_den
                ) == 4
            )
            {
                video->source_width =
                    source_w;

                video->source_height =
                    source_h;


                if (fps_den != 0)
                {
                    video->fps =
                        (double)fps_num /
                        (double)fps_den;
                }
            }
        }

        _pclose(probe);
    }


    // =========================================================
    // Valeurs de secours
    // =========================================================

    if (video->source_width <= 0)
        video->source_width = width > 0 ? width : 1920;

    if (video->source_height <= 0)
        video->source_height = height > 0 ? height : 1080;

    if (video->fps <= 0.0)
        video->fps = 30.0;


    video->frame_duration =
        1.0 / video->fps;


    printf(
        "Video source : %dx%d @ %.2f FPS\n",
        video->source_width,
        video->source_height,
        video->fps
    );


    // =========================================================
    // Calcul de la résolution de sortie
    // =========================================================

    /*
     * On limite le décodage à 1920x1080 maximum.
     *
     * Cela évite de décoder une vidéo 4K en 3840x2160 RGBA.
     */

    // =========================================================
    // Résolution de sortie adaptée à l'écran
    // =========================================================

    int screen_width =
        width > 0 ? width : 1920;

    int screen_height =
        height > 0 ? height : 1080;


    double source_width =
        (double)video->source_width;

    double source_height =
        (double)video->source_height;

    double scale_x =
        (double)screen_width / source_width;

    double scale_y =
        (double)screen_height / source_height;


    // On ne dépasse jamais la résolution source.
    // On réduit seulement si la vidéo est plus grande
    // que l'écran.
    double scale = scale_x < scale_y
                ? scale_x
                : scale_y;

    if (scale > 1.0)
        scale = 1.0;


    video->width =
        (int)(source_width * scale);

    video->height =
        (int)(source_height * scale);


    // Certaines opérations vidéo préfèrent des dimensions paires.
    video->width &= ~1;
    video->height &= ~1;


    if (video->width < 2)
        video->width = 2;

    if (video->height < 2)
        video->height = 2;


    printf(
        "Resolution source : %dx%d\n",
        video->source_width,
        video->source_height
    );

    printf(
        "Resolution de sortie : %dx%d\n",
        video->width,
        video->height
    );


    if (video->width < 1)
        video->width = 1;

    if (video->height < 1)
        video->height = 1;


    printf(
        "Resolution de sortie : %dx%d\n",
        video->width,
        video->height
    );


    // =========================================================
    // Buffer RGBA
    // =========================================================

    video->frame_size =
        video->width *
        video->height *
        4;


    video->buffer =
        malloc(video->frame_size);


    if (video->buffer == NULL)
    {
        TraceLog(
            LOG_ERROR,
            "Impossible d'allouer le buffer vidéo"
        );

        return false;
    }


    // =========================================================
    // FFmpeg
    // =========================================================

    char command[4096];


    snprintf(
        command,
        sizeof(command),

        "cmd /c \"\"%s\" "
        "-stream_loop -1 "
        "-hide_banner "
        "-loglevel error "
        "-i \"%s\" "
        "-vf \"scale=%d:%d:force_original_aspect_ratio=decrease\" "
        "-f rawvideo "
        "-pix_fmt rgba "
        "-an "
        "-\"",

        FFMPEG_PATH,
        filename,

        video->width,
        video->height
    );


    printf(
        "Commande FFmpeg : %s\n",
        command
    );


    video->pipe =
        _popen(command, "rb");


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


    // =========================================================
    // Texture
    // =========================================================

    Image image =
    {
        .data = video->buffer,

        .width = video->width,

        .height = video->height,

        .mipmaps = 1,

        .format =
            PIXELFORMAT_UNCOMPRESSED_R8G8B8A8
    };


    video->texture =
        LoadTextureFromImage(image);


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

    video->next_frame_time =
        GetTime();


    printf(
        "Video initialisee avec succes !\n"
    );

    return true;
}


// =============================================================
// Mise à jour
// =============================================================

bool Video_Update(Video *video)
{
    if (!video->playing)
        return false;

    double current_time = GetTime();

    if (current_time < video->next_frame_time)
        return false;

    // ---------------------------------------------------------
    // Mesure lecture FFmpeg -> RAM
    // ---------------------------------------------------------

    double read_start = GetTime();

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

    double read_time =
        (GetTime() - read_start) * 1000.0;


    // ---------------------------------------------------------
    // Mesure RAM -> GPU
    // ---------------------------------------------------------

    double texture_start = GetTime();

    UpdateTexture(
        video->texture,
        video->buffer
    );

    double texture_time =
        (GetTime() - texture_start) * 1000.0;


    // ---------------------------------------------------------
    // Timing vidéo
    // ---------------------------------------------------------

    video->next_frame_time +=
        video->frame_duration;


    if (video->next_frame_time <
        current_time - 1.0)
    {
        video->next_frame_time =
            current_time +
            video->frame_duration;
    }


    // ---------------------------------------------------------
    // Affichage des statistiques
    // ---------------------------------------------------------

    static int frame_counter = 0;

    frame_counter++;

    if (frame_counter >= 60)
    {
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

            (double)video->frame_size /
            (1024.0 * 1024.0),

            read_time,

            texture_time
        );

        frame_counter = 0;
    }

    return true;
}


// =============================================================
// Affichage
// =============================================================

void Video_Draw(Video *video)
{
    if (
        !video->playing ||
        video->texture.id == 0
    )
    {
        return;
    }


    float screen_width =
        (float)GetScreenWidth();

    float screen_height =
        (float)GetScreenHeight();


    float video_width =
        (float)video->width;

    float video_height =
        (float)video->height;


    float video_ratio =
        video_width /
        video_height;


    float screen_ratio =
        screen_width /
        screen_height;


    Rectangle destination;


    if (video_ratio > screen_ratio)
    {
        destination.width =
            screen_width;

        destination.height =
            screen_width /
            video_ratio;

        destination.x =
            0;

        destination.y =
            (screen_height -
             destination.height) /
            2.0f;
    }
    else
    {
        destination.height =
            screen_height;

        destination.width =
            screen_height *
            video_ratio;

        destination.x =
            (screen_width -
             destination.width) /
            2.0f;

        destination.y =
            0;
    }


    DrawTexturePro(
        video->texture,

        (Rectangle)
        {
            0,
            0,
            video_width,
            video_height
        },

        destination,

        (Vector2)
        {
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