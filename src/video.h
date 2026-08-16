#ifndef VIDEO_H
#define VIDEO_H

#include "raylib.h"
#include <stdbool.h>
#include <stdio.h>

typedef struct {
    char filename[512];
    int source_width;
    int source_height;
    int width;
    int height;
    double fps;
    double frame_duration;
    double next_frame_time;
    int frame_size;
    unsigned char *buffer;
    FILE *pipe;
    Texture2D texture;
    bool playing;
} Video;

bool Video_Init(Video *video, const char *filename, int width, int height);
bool Video_Update(Video *video);
void Video_Draw(Video *video, float alpha);
void Video_Close(Video *video);

#endif