#include <cstdlib>
#include <ctime>
#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

const char game_name[] = "memory-game";
const int screen_width = 640;
const int screen_height = 480;
const int card_size = 128;
const int pole_w = 4;
const int pole_h = 3;
const int pole_size = pole_w * pole_h;
const int card_shift_x = ( screen_width - pole_w * card_size ) / 2;
const int card_shift_y = ( screen_height - pole_h * card_size ) / 2;
bool quit_flag = false;

SDL_Window *window = NULL;
SDL_Renderer *render = NULL;
SDL_Event event;
SDL_Texture *cards = NULL;

short card[pole_size]; 
short card_a[2] = { -1, -1 };
short card_s = 0;

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

void game_restart( void )
{
    card_a[0] = card_a[1] = -1;
    for ( short i = 0; i < pole_size; i += 2 ) {
        card[i+0] = ( i / 2 ) + 1;
        card[i+1] = ( i / 2 ) + 1;
    }
    for ( short i = 0; i < pole_size; i++ ) {
        swap( &card[rand() % pole_size], &card[rand() % pole_size] );
    }
}

void card_draw( SDL_Renderer *r, SDL_Texture *tex, short id, int p )
{
    SDL_Rect wnd = { 0, 0, card_size, card_size };
    SDL_Rect pos = { 0, 0, card_size, card_size };
    short x = p % pole_w;
    short y = p / pole_w;

    pos.x = x * card_size + card_shift_x; 
    pos.y = y * card_size + card_shift_y;
    wnd.x = id * card_size;
    SDL_RenderCopy( r, tex, &wnd, &pos );
}

void game_event( SDL_Event *event )
{
    int x, y;

    SDL_PollEvent( event );
    switch ( event->type ) {
        case SDL_QUIT:
            quit_flag = 1;
            break;
        case SDL_MOUSEBUTTONDOWN:
            switch ( event->button.button ) {
                case SDL_BUTTON_LEFT:
                    x = ( event->button.x - card_shift_x ) / card_size;
                    y = ( event->button.y - card_shift_y ) / card_size;
                    x += y * pole_w;
                    if ( card_s >= 2 ) {
                        card_s = 0;
                        card_a[0] = card_a[1] = -1;
                    } else if ( card_a[card_s] == -1 && card[x] != -2 ) {
                        card_a[card_s++] = x;
                    }
                    break;
            }
            event->button.button = 0; // button hack
            break;
    }
}

void game_loop( void )
{
    short counter = 0;
    if ( card_a[1] != -1 && card[card_a[0]] == card[card_a[1]] && card_a[0] != card_a[1] ) {
        card[card_a[0]] = card[card_a[1]] = -2;
    }
    for ( short i = 0; i < pole_size; i++ ) {
        if ( card[i] == -2 ) {
            counter++;
        }
    }
    if ( counter == pole_size ) {
        std::cout << "GAME OVER!\n";
        game_restart();
    }
}

void game_render( void )
{
    short what_draw = 0;

    SDL_RenderClear( render );
    for ( short i = 0; i < pole_size; i++ ) {
        if ( i == card_a[0] || i == card_a[1] ) {
            what_draw = card[i];
        } else if ( card[i] == -2 ) {
            continue;
        } else {
            what_draw = 0;
        }
        card_draw( render, cards, what_draw, i );
    }
    SDL_RenderPresent( render );
}

void game_destroy( void )
{
    SDL_DestroyTexture( cards );
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
    cards = IMG_LoadTexture( render, "./images/card.png" );
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