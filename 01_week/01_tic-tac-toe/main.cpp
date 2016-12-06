#include <cstdlib>
#include <ctime>
#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

const char game_name[] = "tic-tac-toe";
const int screen_width = 640;
const int screen_height = 480;
const short element_size = 64;
const short psize = 3;
const int pole_width_shift = ( screen_width - element_size * psize ) / 2;
const int pole_height_shift = ( screen_height - element_size * psize ) / 2;

int player_step = 1;
bool quit_flag = false;

SDL_Window *window = NULL;
SDL_Renderer *render = NULL;
SDL_Event event;
SDL_Texture *elements = NULL;

short pole_shift[psize] = { -4, 0, 4 };
short game_pole[psize*psize] = { 0 };
short av[] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 0, 3, 6, 
    1, 4, 7, 2, 5, 8, 0, 4, 8, 2, 4, 6
};

void send_error( int code )
{
    std::cout << SDL_GetError() << "\n";
    exit( code );
}

void element_draw( SDL_Renderer *r, SDL_Texture *tex, short id, int p )
{
    SDL_Rect wnd = { 0, 0, element_size, element_size };
    SDL_Rect pos = { 0, 0, element_size, element_size };
    short x = p % psize;
    short y = p / psize;

    pos.x = x * element_size + pole_width_shift + pole_shift[x%3]; 
    pos.y = y * element_size + pole_height_shift + pole_shift[y%3];
    wnd.x = id * element_size;
    SDL_RenderCopy( r, tex, &wnd, &pos );
}

void game_restart( void )
{
    memset( game_pole, 0, sizeof( game_pole ) );
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
                    x = ( event->button.x - pole_width_shift ) / element_size;
                    y = ( event->button.y - pole_height_shift ) / element_size;
                    x += y * psize;
                    if ( x >= 0 && x < psize * psize && player_step && game_pole[x] == 0 ) {
                        game_pole[x] = 1;
                        player_step = 0;
                    }
                    break;
            }
        case SDL_KEYDOWN:
            switch ( event->key.keysym.sym ) {
                case SDLK_ESCAPE:
                case SDLK_q:
                    quit_flag = true;
                    break;
                case SDLK_r:
                    game_restart();
                    break;
            }
            break;
        default:
            break;
    }
    // SDL2 button hack
    event->button.button = 0;
    event->key.keysym.sym = 0;
}

void pc_step( void )
{
    short i, set_bit = 0, counter = 0;

    if ( player_step == 0 ) {
        do {
            i = rand() % ( psize * psize );
            if ( game_pole[i] == 0 ) {
                game_pole[i] = 2;
                set_bit = 1;
            }
            counter++;
        } while ( set_bit == 0 && counter < 10 );
        player_step = 1;
    }
    if ( counter >= 10 ) {
        game_restart();
        std::cout << "GAME OVER: Nobody WIN!\n";
    }
}

void find_winner( void )
{
    short is_winner = 0, counter = 0, a = 0;

    for ( short i = 0; i < 24; i += 3 ) {
        a = ( game_pole[av[i+0]] == game_pole[av[i+1]] ) && 
            ( game_pole[av[i+1]] == game_pole[av[i+2]] );
        if ( i < psize * psize && game_pole[i] != 0 ) {
            counter++;
        }
        if ( a ) {
            is_winner = game_pole[av[i+1]];
            break;
        }
    } 
    if ( counter == psize * psize ) {
        is_winner = 3;
    }
    if ( is_winner > 0 ) {
        player_step = 1;
        std::cout << "GAME OVER: ";
        if ( is_winner == 1 ) {
            std::cout << "Player";
        } else if ( is_winner == 2 ) {
            std::cout << "Computer";
        } else if ( is_winner == 3 ) {
            std::cout << "Nobody";
        }
        std::cout << " WIN!\n";
        game_restart();
    }
}

void game_render( void )
{
    SDL_RenderClear( render );
    for ( short i = 0; i < psize * psize; i++ ) {
        element_draw( render, elements, game_pole[i], i );
    }
    SDL_RenderPresent( render );
}

void game_loop( void )
{
    find_winner();
    pc_step();
    find_winner();
}

void game_destroy( void )
{
    SDL_DestroyTexture( elements );
    SDL_DestroyRenderer( render );
    SDL_DestroyWindow( window );
    SDL_Quit();
}

void game_init( void )
{
    srand( time( NULL ) );
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
    elements = IMG_LoadTexture( render, "./images/elements.png" );
}

int main( int argc, char *argv[] )
{
    Uint32 FPS_MAX = 1000 / 31; // 30 fps

    game_init();
    while ( quit_flag == false ) {
        game_event( &event );
        game_render();
        game_loop();
        SDL_Delay( FPS_MAX );
    }
    game_destroy();
    return EXIT_SUCCESS;
}