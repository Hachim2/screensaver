#ifndef VIDEO_H
#define VIDEO_H

#include "raylib.h"
#include <stdbool.h>
#include <stdio.h>

typedef struct
{
    char filename[512];

    // Résolution réelle de la vidéo
    int source_width;
    int source_height;

    // Résolution utilisée par FFmpeg/Raylib
    int width;
    int height;

    // FPS de la vidéo
    double fps;

    // Temps entre deux frames
    double frame_duration;

    // Temps auquel la prochaine frame doit être affichée
    double next_frame_time;

    int frame_size;

    unsigned char *buffer;

    FILE *pipe;

    Texture2D texture;

    bool playing;

} Video;


bool Video_Init(
    Video *video,
    const char *filename,
    int width,
    int height
);

bool Video_Update(Video *video);

void Video_Draw(Video *video);

void Video_Close(Video *video);

#endif