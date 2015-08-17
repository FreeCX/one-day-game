#include <cstdlib>
#include <ctime>
#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
const char game_name[] = "plumber";
const int screen_width = 640;
const int screen_height = 480;
const int tile_size = 32;
const int pole_size = 12;
const int two_pole_size = pole_size * pole_size;
const int tile_shift_x = ( screen_width - pole_size * tile_size ) / 2;
const int tile_shift_y = ( screen_height - pole_size * tile_size ) / 2;
bool quit_flag = false;

enum {
    E_DRAIN = 0,
    E_UP_DOWN,
    E_LEFT_RIGHT,
    E_UP_LEFT,
    E_UP_RIGHT,
    E_BOTTOM_RIGHT,
    E_BOTTOM_LEFT
};
enum {
    GAME_IDLE = 0,
    GAME_ERROR,
    GAME_WIN
};

SDL_Window *window = NULL;
SDL_Renderer *render = NULL;
SDL_Event event;
SDL_Texture *tiles = NULL;

short pole[pole_size][pole_size];
short color[two_pole_size];
short game_check_init = 0;

void send_error( int code )
{
    std::cout << SDL_GetError() << "\n";
    exit( code );
}

void tile_draw( SDL_Renderer *r, SDL_Texture *tex, short id, int p )
{
    SDL_Rect wnd = { 0, 0, tile_size, tile_size };
    SDL_Rect pos = { 0, 0, tile_size, tile_size };
    short x = p % pole_size;
    short y = p / pole_size;

    pos.x = x * tile_size + tile_shift_x; 
    pos.y = y * tile_size + tile_shift_y;
    wnd.x = id * tile_size;
    SDL_RenderCopy( r, tex, &wnd, &pos );
}

void game_restart( void )
{
    memset( color, 0, sizeof( color ) );
    for ( short i = 0; i < two_pole_size; i++ ) {
        pole[i%pole_size][i/pole_size] = rand() % 6 + 1;
    }
    pole[0][0] = pole[pole_size-1][pole_size-1] = 0;
}

void game_check( void )
{
    static short game_state = 0;
    static short prev_state = 0;
    static short curr_state = pole_size;
    static short step_count = 0;
    short x1, y1, x2, y2;

    if ( game_check_init == 0 ) {
        game_check_init = 1;
    }
    if ( curr_state == two_pole_size-1 ) {
        game_state = GAME_WIN;
        step_count = prev_state = 0;
        curr_state = pole_size;
        game_check_init = 0;
    } 
    if ( step_count >= two_pole_size ) {
        game_state = GAME_ERROR;
        step_count = prev_state = 0;
        curr_state = pole_size;
        game_check_init = 0;
    }
    color[prev_state] = 1;
    color[curr_state] = 1;

    x1 = prev_state % pole_size;
    y1 = prev_state / pole_size;
    x2 = curr_state % pole_size;
    y2 = curr_state / pole_size;
    switch ( pole[x2][y2] ) {
        case E_UP_DOWN:
            if ( y2-1 == y1 ) {
                y2++;
            } else {
                y2--;
            }
            break;
        case E_LEFT_RIGHT:
            if ( x2-1 == x1 ) {
                x2++;
            } else {
                x2--;
            }
            break;
        case E_UP_LEFT:
            if ( y2-1 == y1 ) {
                x2--;
            } else {
                y2--;
            }
            break;
        case E_UP_RIGHT:
            if ( y2-1 == y1 ) {
                x2++;
            } else {
                y2--;
            }
            break;
        case E_BOTTOM_RIGHT:
            if ( y2+1 == y1 ) {
                x2++;
            } else {
                y2++;
            }
            break;
        case E_BOTTOM_LEFT:
            if ( y2+1 == y1 ) {
                x2--;
            } else {
                y2++;
            }
            break;
    }
    prev_state = curr_state;
    curr_state = x2 + y2 * pole_size;
    step_count++;
    switch ( game_state ) {
        case GAME_ERROR:
            std::cout << "WATER PATH ERROR!\n";
            game_state = GAME_IDLE;
            memset( color, 0, sizeof( color ) );
            break;
        case GAME_WIN:
            std::cout << "GAME OVER: WIN!\n";
            game_state = GAME_IDLE;
            game_restart();
            break;
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
                    x += y * pole_size;
                    if ( pole[x%pole_size][x/pole_size] > E_LEFT_RIGHT && pole[x%pole_size][x/pole_size] < E_BOTTOM_LEFT ) {
                        pole[x%pole_size][x/pole_size]++;
                    } else if ( pole[x%pole_size][x/pole_size] == E_BOTTOM_LEFT ) {
                        pole[x%pole_size][x/pole_size] = E_UP_LEFT;
                    }
                    if ( pole[x%pole_size][x/pole_size] == E_UP_DOWN ) {
                        pole[x%pole_size][x/pole_size]++;
                    } else if ( pole[x%pole_size][x/pole_size] == E_LEFT_RIGHT ) {
                        pole[x%pole_size][x/pole_size]--;
                    }
                    break;
                case SDL_BUTTON_RIGHT:
                    game_check();
                    break;
            }
            event->button.button = 0; // button hack
            break;
    }
}

void game_loop( void )
{
    if ( game_check_init == 1 ) {
        game_check();
    }
}

void game_render( void )
{
    SDL_RenderClear( render );
    for ( short i = 0; i < two_pole_size; i++ ) {
        if ( color[i] == 1 ) {
            SDL_SetTextureColorMod( tiles, 0, 100, 255 );
        } else {
            SDL_SetTextureColorMod( tiles, 255, 255, 255 );
        }
        tile_draw( render, tiles, pole[i%pole_size][i/pole_size], i );
    }
    SDL_RenderPresent( render );
}

void game_destroy( void )
{
    SDL_DestroyTexture( tiles );
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
    tiles = IMG_LoadTexture( render, "./images/tiles.png" );
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