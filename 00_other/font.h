#ifndef __FONT_H__
#define __FONT_H__

#include <cstring>
#include <iostream>
#include <string>
#include <vector>
#include <wchar.h>
#include <SDL2/SDL_image.h>

struct font_table {
    int f_height;
    int f_width;
    int t_height;
    int t_width;
    int table[0xFFFF];
    SDL_Texture *font;
};
typedef struct font_table font_table_t;

int font_load( SDL_Renderer *r, font_table_t **t, const char *font );
void set_color( SDL_Texture *tex, Uint32 color );
void font_draw( SDL_Renderer *r, font_table_t *t, const wchar_t *text, int x, int y );
void font_destroy( font_table_t *t );

#endif