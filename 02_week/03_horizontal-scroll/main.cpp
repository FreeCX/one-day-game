#include <cstdlib>
#include <ctime>
#include <iostream>
#include <vector>
#include <wchar.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "../../00_other/font.h"

typedef struct {
    int x, y;
    int speed;
    int id;
} pixel_star;

typedef struct {
    int x, y;
    int speed;
    int life;
    int damage;
    bool player_flag;
} g_laser;

typedef struct {
    int x, y;
    int life;
    double angle;
    int speed;
    int attack;
    int counter;
} g_enemy;

const char game_name[] = "horizontal-scroll";
const int screen_width = 640;
const int screen_height = 480;
const int star_count = 10;
const int star_speed = 3;
const int star_size = 16;
const int background_size = 256;
const int player_size_x = 48;
const int player_size_y = 64;
const int laser_size_x = 37;
const int laser_size_y = 9;
const int laser_speed = 8;
const int enemy_size = 64;
const int medkit_size = 22;
const int medkit_speed = 3;
bool quit_flag = false;

SDL_Window *window = NULL;
SDL_Renderer *render = NULL;
SDL_Event event;
SDL_Texture *t_background = NULL;
SDL_Texture *t_player = NULL;
SDL_Texture *t_enemy = NULL;
SDL_Texture *t_star = NULL;
SDL_Texture *t_laser_enemy = NULL;
SDL_Texture *t_laser_player = NULL;
SDL_Texture *t_medkit = NULL;
font_table_t *ft = NULL;

pixel_star star[star_count];
unsigned int game_cycle = 0;
unsigned int game_score = 0;
unsigned int x_shift = 0;
int player_x, player_y;
int player_life = 30;
int medkit_x = -1, medkit_y = -1;
double medkit_angle = 0;
std::vector<g_laser> laser;
std::vector<g_enemy> enemy;

void laser_add( int x, int y, int player_flag )
{
    g_laser a;

    a.x = x;  
    a.y = y;
    a.speed = laser_speed;
    a.life = 80;
    a.damage = 3;
    a.player_flag = player_flag;
    laser.push_back( a );
}

void game_add_enemy( void )
{
    g_enemy a;

    a.x = screen_width;
    a.y = rand() % screen_height;
    a.life = 8;
    a.attack = 1;
    a.angle = 0;
    a.speed = rand() % 3 + 1;
    a.counter = 50;
    enemy.push_back( a );
}

void star_index_init( int i, bool init )
{
    if ( init == true ) {
        star[i].x = rand() % screen_width;
    } else {
        star[i].x = screen_width;
    }
    star[i].id = rand() % 2;
    star[i].y = rand() % screen_height;
    star[i].speed = rand() % star_speed + 1;
}

void star_init( void )
{
    for ( int i = 0; i < star_count; i++ ) {
        star_index_init( i, true );
    }
}

bool is_intersect( int x11, int y11, int x21, int y21, int xsize, int ysize )
{
    int x12 = x11 + xsize;
    int y12 = y11 + ysize;
    int x22 = x21 + xsize;
    int y22 = y21 + ysize;

    if ( ( x11 > x21 && x11 < x22 && y11 > y21 && y11 < y22 ) ||
         ( x12 > x21 && x12 < x22 && y11 > y21 && y11 < y22 ) ||
         ( x11 > x21 && x11 < x22 && y12 > y21 && y12 < y22 ) ||
         ( x12 > x21 && x12 < x22 && y12 > y21 && y12 < y22 ) ) {
        return true;
    } else {
        return false;
    }
}

void send_error( int code )
{
    std::cout << SDL_GetError() << std::endl;
    exit( code );
}

void game_restart( void )
{
    star_init();
    laser.clear();
    enemy.clear();
    player_life = 30;
    game_cycle = game_score = 0;
}

void game_event( SDL_Event *event )
{
    SDL_PollEvent( event );
    switch ( event->type ) {
        case SDL_QUIT:
            quit_flag = true;
            break;
        case SDL_MOUSEMOTION:
            player_y = event->motion.y;
            break;
        case SDL_MOUSEBUTTONDOWN:
            switch ( event->button.button ) {
                case SDL_BUTTON_LEFT:
                    laser_add( player_x, event->button.y, true );
                    laser_add( player_x, event->button.y + player_size_y - 8, 
                               true );
                    break;
            }
            break;
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
    event->button.button = 0; // button hack
}

void game_loop( void )
{
    for ( int i = 0; i < star_count; i++ ) {
        star[i].x -= star[i].speed;
        if ( star[i].x < 0 ) {
            star_index_init( i, false );
        }
    }
    x_shift++;
    if ( x_shift == 256 ) {
        x_shift = 0;
    }
    for ( size_t i = 0; i < laser.size(); i++ ) {
        laser[i].life--;
        if ( laser[i].life == 0 ) {
            laser.erase( laser.begin() + i );
            continue;
        }
        if ( laser[i].player_flag ) {
            laser[i].x += laser[i].speed;
        } else {
            laser[i].x -= laser[i].speed;
            if ( is_intersect( player_x, player_y, laser[i].x, laser[i].y, 
                               player_size_x, player_size_y ) ) {
                player_life -= laser[i].damage;
                laser[i].life = 1;
                continue;
            }
        }
        for ( size_t j = 0; j < enemy.size(); j++ ) {
            if ( is_intersect( laser[i].x, laser[i].y, enemy[j].x, enemy[j].y, 
                               enemy_size, enemy_size ) ) {
                enemy[j].life -= laser[i].damage;
                laser.erase( laser.begin() + i );
                game_score += 5;
                break;
            }
        }
    }
    for ( size_t i = 0; i < enemy.size(); i++ ) {
        enemy[i].x -= enemy[i].speed;
        enemy[i].angle += 1.0;
        enemy[i].counter--;
        if ( is_intersect( player_x, player_y, enemy[i].x, enemy[i].y, 
                           player_size_x, player_size_y ) ) {
            player_life -= enemy[i].life;
            enemy[i].life = -1;
        }
        if ( enemy[i].angle == 360 ) {
            enemy[i].angle = 0.0;
        }
        if ( enemy[i].x < 0 ) {
            enemy.erase( enemy.begin() + i );
            continue;
        }
        if ( enemy[i].life < 0 ) {
            enemy.erase( enemy.begin() + i );
            game_score += 50;
        }
        if ( enemy[i].counter < 0 ) {
            laser_add( enemy[i].x, enemy[i].y, false );
            enemy[i].counter = 50;
        }
    }
    if ( player_life <= 0 ) {
        std::cout << "GAME OVER!\nScore: " << game_score << "\n"; 
        game_restart();
    }
    if ( game_cycle % 100 == 0 ) {
        game_add_enemy();
    }
    if ( game_cycle % 2000 == 0 ) {
        medkit_x = screen_width;
        medkit_y = rand() % screen_height;
        medkit_angle = 0;
    }
    if ( is_intersect( player_x, player_y, medkit_x, medkit_y, 
                       player_size_x, player_size_y ) ) {
        player_life += 15;
        medkit_x = -100;
        medkit_y = -100;
    }
    game_cycle++;
    if ( medkit_x > -medkit_size ) {
        medkit_angle += 1.0;
        medkit_x -= medkit_speed;
    }
}

void tile_draw( SDL_Renderer *r, SDL_Texture *tex, int id, int s_x, int s_y, 
                int x, int y )
{
    SDL_Rect wnd = { 0, 0, s_x, s_y };
    SDL_Rect pos = { 0, 0, s_x, s_y };

    pos.x = x;
    pos.y = y;
    wnd.x = id * s_x;
    SDL_RenderCopy( r, tex, &wnd, &pos );
}

void tile_angle_draw( SDL_Renderer *r, SDL_Texture *tex, double angle, int id, 
                      int s_x, int s_y, int x, int y )
{
    SDL_Rect wnd = { 0, 0, s_x, s_y };
    SDL_Rect pos = { 0, 0, s_x, s_y };

    pos.x = x - s_x / 2;
    pos.y = y - s_y / 2;
    wnd.x = id * s_x;
    SDL_RenderCopyEx( r, tex, &wnd, &pos, angle, NULL, SDL_FLIP_NONE );
}

void draw_point( int x, int y, Uint32 color )
{
    Uint8 r, g, b, a;

    SDL_GetRenderDrawColor( render, &r, &g, &b, &a );
    SDL_SetRenderDrawColor( render, color >> 16, ( color >> 8 ) & 0xFF, 
                            ( color & 0xFF ), 255 );
    SDL_RenderDrawPoint( render, x, y );
    SDL_SetRenderDrawColor( render, r, g, b, a );
} 

void game_render( void )
{
    SDL_Texture *select = NULL;
    wchar_t buffer[32];

    SDL_RenderClear( render );
    for ( int i = 0; i < 4; i++ ) {
        for ( int j = 0; j < 2; j++ ) {
            tile_draw( render, t_background, 0, background_size, 
                       background_size, background_size * i - x_shift, 
                       background_size * j );
        }
    }
    for ( int i = 0; i < star_count; i++ ) {
        tile_draw( render, t_star, star[i].id, star_size, star_size, 
                   star[i].x, star[i].y );
    }
    for ( size_t i = 0; i < laser.size(); i++ ) {
        if ( laser[i].player_flag ) {
            select = t_laser_player;
        } else {
            select = t_laser_enemy;
        }
        tile_draw( render, select, 0, laser_size_x, laser_size_y, 
                   laser[i].x, laser[i].y );
    }
    for ( size_t i = 0; i < enemy.size(); i++ ) {
        tile_angle_draw( render, t_enemy, enemy[i].angle, 0, 
                         enemy_size, enemy_size, enemy[i].x, enemy[i].y );
    }
    if ( medkit_x > -medkit_size ) {
        tile_angle_draw( render, t_medkit, medkit_angle, 0, medkit_size, 
                         medkit_size, medkit_x, medkit_y );
    }
    tile_draw( render, t_player, 0, player_size_x, player_size_y, 
               player_x, player_y );
    swprintf( buffer, 32, L"  жизнь: %02d", player_life );
    font_draw( render, ft, buffer, 8, 440 );
    swprintf( buffer, 32, L"   очки: %u", game_score );
    font_draw( render, ft, buffer, 8, 450 );
    SDL_RenderPresent( render );
}

void game_destroy( void )
{
    SDL_DestroyTexture( t_star );
    SDL_DestroyTexture( t_background );
    SDL_DestroyTexture( t_player );
    SDL_DestroyTexture( t_laser_enemy );
    SDL_DestroyTexture( t_laser_player );
    SDL_DestroyTexture( t_enemy );
    SDL_DestroyTexture( t_medkit );
    SDL_DestroyRenderer( render );
    SDL_DestroyWindow( window );
    SDL_Quit();
}

void game_init( void )
{
    window = SDL_CreateWindow( game_name, SDL_WINDOWPOS_CENTERED, 
                               SDL_WINDOWPOS_CENTERED, screen_width, 
                               screen_height, SDL_WINDOW_SHOWN );
    if ( window == NULL ) {
        send_error( EXIT_FAILURE );
    }
    render = SDL_CreateRenderer( window, -1, SDL_RENDERER_ACCELERATED | 
                                 SDL_RENDERER_PRESENTVSYNC );
    if ( render == NULL ) {
        send_error( EXIT_FAILURE );
    }
    srand( time( NULL ) );
    font_load( render, &ft, "../../00_other/font.cfg" );
    set_color( ft->font, 0xFFFFFF );
    t_star = IMG_LoadTexture( render, "./images/star.png" );
    t_background = IMG_LoadTexture( render, "./images/background.png" );
    t_player = IMG_LoadTexture( render, "./images/player.png" );
    t_laser_enemy = IMG_LoadTexture( render, "./images/laser_enemy.png" );
    t_laser_player = IMG_LoadTexture( render, "./images/laser_player.png" );
    t_enemy = IMG_LoadTexture( render, "./images/enemy.png" );
    t_medkit = IMG_LoadTexture( render, "./images/medkit.png" );
    game_restart();
}

int main( int argc, char *argv[] )
{
    Uint32 FPS_MAX = 1000 / 63; // 60 fps

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