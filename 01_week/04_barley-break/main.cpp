#include <cstdlib>
#include <ctime>
#include <iostream>
#include <algorithm>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

const char game_name[] = "barley-break";
const int screen_width = 640;
const int screen_height = 480;
const int tile_size = 64;
const int font_size = 32;
const int pole_size = 4;
const int two_pole_size = pole_size * pole_size;
const int tile_shift_x = ( screen_width - pole_size * tile_size ) / 2;
const int tile_shift_y = ( screen_height - pole_size * tile_size ) / 2;
const short moves[8] = { -1, +0, +1, +0, +0, -1, +0, +1 };
bool quit_flag = false;

SDL_Window *window = NULL;
SDL_Renderer *render = NULL;
SDL_Event event;
SDL_Texture *tile = NULL, *font = NULL;

short pole[pole_size][pole_size];

void send_error( int code )
{
    std::cout << SDL_GetError() << std::endl;
    exit( code );
}

void tile_draw( SDL_Renderer *r, SDL_Texture *tex, short id, int p )
{
    SDL_Rect wnd = { 0, 0, tile_size, tile_size };
    SDL_Rect pos = { 0, 0, tile_size, tile_size };
    short x = p % pole_size;
    short y = p / pole_size;

    id = id != 16 ? 1 : 0;
    pos.x = x * tile_size + tile_shift_x; 
    pos.y = y * tile_size + tile_shift_y;
    wnd.x = id * tile_size;
    SDL_RenderCopy( r, tex, &wnd, &pos );
}

void font_color( SDL_Texture *tex, Uint32 color )
{
    SDL_SetTextureColorMod( tex, color >> 16, ( color >> 8 ) & 0xFF, color & 0xFF );
}

void font_draw( SDL_Renderer *r, SDL_Texture *tex, short text, int p )
{
    SDL_Rect wnd = { 0, 0, font_size, font_size };
    SDL_Rect pos = { 0, 0, font_size, font_size };
    short x = p % pole_size;
    short y = p / pole_size;

    if ( text == 16 ) {
        return;
    }
    pos.x = x * tile_size + tile_shift_x + font_size / 2;
    pos.y = y * tile_size + tile_shift_y + font_size / 2;
    wnd.x = (text - 1) * font_size;
    SDL_RenderCopy( r, tex, &wnd, &pos );
}

void game_restart( void )
{
    for ( short i = 0; i < two_pole_size; i++ ) {
        *( *pole + i ) = i+1;
    }
    for ( short i = 0; i < two_pole_size; i++ ) {
        short rnd_1 = rand() % two_pole_size;
        short rnd_2 = rand() % two_pole_size;
        if ( rnd_1 == rnd_2 ) {
            rnd_2 = (rnd_1 + 1) % two_pole_size;
        }
        std::swap( *(*pole + rnd_1), *(*pole + rnd_2) );
    }
    short incorrect = 0;
    for (short i = 0; i < two_pole_size; i++) {
        for (short j = i+1; j < two_pole_size; j++) {
            if (*(*pole + i) > *(*pole + j)) {
                incorrect ^= 1;
            }
        }
    }
    if (incorrect) {
        std::cout << "OMG!!!" << std::endl;
        game_restart();
    }
}

void game_event( SDL_Event *event )
{
    SDL_PollEvent( event );
    switch ( event->type ) {
        case SDL_QUIT:
            quit_flag = true;
            break;
        case SDL_MOUSEBUTTONDOWN:
            switch ( event->button.button ) {
                case SDL_BUTTON_LEFT:
                    short x = ( event->button.x - tile_shift_x ) / tile_size;
                    short y = ( event->button.y - tile_shift_y ) / tile_size;
                    for ( short i = 0; i < 8; i += 2 ) {
                        short it_move = pole[y+moves[i+1]][x+moves[i+0]] == 16 && 
                            x + moves[i+0] < pole_size && x + moves[i+0] >= 0 &&
                            y + moves[i+1] < pole_size && y + moves[i+1] >= 0;
                        if ( it_move ) {
                            pole[y+moves[i+1]][x+moves[i+0]] = pole[y][x];
                            pole[y][x] = 16;
                        } 
                    }
                    break;
            }
            break;
        case SDL_KEYDOWN:
            short id = 0;
            for ( int i = 0; i < two_pole_size; i++ ) {
                if ( *(*pole + i) == 16 ) {
                    id = i;
                }
            }
            short x = id % pole_size;
            short y = id / pole_size;
            switch ( event->key.keysym.sym ) {
                case SDLK_ESCAPE:
                case SDLK_q:
                    quit_flag = true;
                    break;
                case SDLK_UP:
                case SDLK_w:
                    if (y < 3) {
                        pole[y][x] = pole[y+1][x];
                        pole[y+1][x] = 16;
                    }
                    break;
                case SDLK_DOWN:
                case SDLK_s:
                    if (y > 0) {
                        pole[y][x] = pole[y-1][x];
                        pole[y-1][x] = 16;
                    }
                    break;
                case SDLK_LEFT:
                case SDLK_a:
                    if (x < 3) {
                        pole[y][x] = pole[y][x+1];
                        pole[y][x+1] = 16;
                    }
                    break;
                case SDLK_RIGHT:
                case SDLK_d:
                    if (x > 0) {
                        pole[y][x] = pole[y][x-1];
                        pole[y][x-1] = 16;
                    }
                    break;
                case SDLK_r:
                    game_restart();
                    break;
            }
            break;
    }
    event->button.button = 0;
    event->key.keysym.sym = 0;
}

void game_loop( void )
{
    short count = 0;

    for ( short i = 0; i < two_pole_size; i++ ) {
        if ( *( *pole + i ) == i+1 ) {
            count++;
        }
    }
    if ( count == two_pole_size ) {
        std::cout << "GAME OVER: WIN!\n";
        game_restart();
    }
}

void game_render( void )
{
    SDL_RenderClear( render );
    for ( short i = 0; i < two_pole_size; i++ ) {
        tile_draw( render, tile, *( *pole + i ), i );
        font_draw( render, font, *( *pole + i ), i );
    }
    SDL_RenderPresent( render );
}

void game_destroy( void )
{
    SDL_DestroyTexture( tile );
    SDL_DestroyTexture( font );
    SDL_DestroyRenderer( render );
    SDL_DestroyWindow( window );
    SDL_Quit();
}

void game_init( void )
{
    window = SDL_CreateWindow( game_name, SDL_WINDOWPOS_CENTERED, 
        SDL_WINDOWPOS_CENTERED, screen_width, screen_height, SDL_WINDOW_SHOWN );
    if ( window == NULL ) {
        send_error( EXIT_FAILURE );
    }
    render = SDL_CreateRenderer( window, -1, SDL_RENDERER_ACCELERATED | 
        SDL_RENDERER_PRESENTVSYNC );
    if ( render == NULL ) {
        send_error( EXIT_FAILURE );
    }
    tile = IMG_LoadTexture( render, "./images/tiles.png" );
    font = IMG_LoadTexture( render, "./images/font.png" );
    font_color( font, 0xf0c090 );
    srand( time( NULL ) );
    game_restart();
}

int main( int argc, char *argv[] )
{
    Uint32 FPS_MAX = 1000 / 31; // 30 fps

    game_init();
    while ( quit_flag == false ) {
        game_event( &event );
        game_loop();
        game_render();
        SDL_Delay( FPS_MAX );
    }
    game_destroy();
    return EXIT_SUCCESS;
}