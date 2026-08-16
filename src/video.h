#ifndef VIDEO_H
#define VIDEO_H

#include "raylib.h"
#include <stdio.h>
#include <stdint.h>

typedef struct {
    FILE *pipe;
    Texture2D texture;

    uint8_t *buffer;

    int width;
    int height;
    int frame_size;

    char filename[512];
    bool playing;
} Video;

bool Video_Init(Video *video, const char *filename, int width, int height);
bool Video_Update(Video *video);
void Video_Draw(Video *video);
void Video_Close(Video *video);

#endif