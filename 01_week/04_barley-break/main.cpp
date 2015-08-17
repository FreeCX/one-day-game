#include <cstdlib>
#include <ctime>
#include <iostream>
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
    std::cout << SDL_GetError() << "\n";
    exit( code );
}

void swap( short *i, short *j )
{
    short t = *i;
    *i = *j;
    *j = t;
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

void font_draw( SDL_Renderer *r, SDL_Texture *tex, char *text, int p )
{
    SDL_Rect wnd = { 0, 0, font_size, font_size };
    SDL_Rect pos = { 0, 0, font_size, font_size };
    int i = 0, current;
    short x = p % pole_size;
    short y = p / pole_size;

    if ( strcmp( text, "16" ) == 0 ) {
        return;
    }
    if ( strlen( text ) > 1 ) {
        pos.x = x * tile_size + tile_shift_x + font_size / 4; 
    } else {
        pos.x = x * tile_size + tile_shift_x + font_size / 2; 
    }
    pos.y = y * tile_size + tile_shift_y + font_size / 2;
    while ( ( current = text[i++] ) != '\0' ) {
        wnd.x = ( current - '0' ) * font_size;
        SDL_RenderCopy( r, tex, &wnd, &pos );
        pos.x += font_size / 2;
    }
}

void game_restart( void )
{
    for ( short i = 0; i < two_pole_size; i++ ) {
        *( *pole + i ) = i+1;
    }
    for ( short i = 0; i < two_pole_size; i++ ) {
        swap( *pole + ( rand() % two_pole_size ), 
              *pole + ( rand() % two_pole_size ) );
    }
}

void game_event( SDL_Event *event )
{
    int x, y;

    SDL_PollEvent( event );
    switch ( event->type ) {
        case SDL_QUIT:
            quit_flag = true;
            break;
        case SDL_MOUSEBUTTONDOWN:
            switch ( event->button.button ) {
                case SDL_BUTTON_LEFT:
                    x = ( event->button.x - tile_shift_x ) / tile_size;
                    y = ( event->button.y - tile_shift_y ) / tile_size;
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
            event->button.button = 0; // button hack
            break;
        default:
            break;
    }
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
    char buff[2];

    SDL_RenderClear( render );
    for ( short i = 0; i < two_pole_size; i++ ) {
        sprintf( buff, "%d", *( *pole + i ) );
        tile_draw( render, tile, *( *pole + i ), i );
        font_draw( render, font, buff, i );
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