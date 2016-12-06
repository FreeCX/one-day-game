#include <cmath>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iostream>
#include <vector>
#include <wchar.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "../../00_other/font.h"

typedef struct {
    int x, y;
    int life;
    int speed;
    int attack;
    double angle;
} g_enemy;

typedef struct {
    int x, y;
    float dx, dy;
    int life;
    int step;
    int damage;
} g_bullet;

const char game_name[] = "time-attack";
const int screen_width = 640;
const int screen_height = 480;
const int unit_size = 32;
const int bullet_size = 8;
const int player_x = ( screen_width - unit_size ) / 2;
const int player_y = ( screen_height - unit_size ) / 2;
bool quit_flag = false;

SDL_Window *window = NULL;
SDL_Renderer *render = NULL;
SDL_Event event;
SDL_Texture *background = NULL;
SDL_Texture *t_player = NULL;
SDL_Texture *t_bullet = NULL;
SDL_Texture *t_enemy = NULL;
font_table_t *ft = NULL;

unsigned int game_cycle = 0;
unsigned int game_score = 0;
unsigned int game_time = 0;
double player_angle = 0.0;
int player_life = 10;
std::vector<g_enemy> enemy;
std::vector<g_bullet> bullet;

void game_add_enemy( void )
{
    g_enemy a;
    int x, y;

    x = rand() % screen_width;
    y = rand() % screen_height;
    a.x = x;
    a.y = y;
    a.life = 8;
    a.attack = 1;
    a.angle = 0;
    enemy.push_back( a );
}

void game_add_bullet( int x, int y, float dx, float dy )
{
    g_bullet a;

    a.x  = x;  a.y  = y;
    a.dx = dx; a.dy = dy;
    a.life = 25;
    a.damage = 3;
    a.step = 0;
    bullet.push_back( a );
}

bool is_intersect( int x11, int y11, int x21, int y21 )
{
    int x12 = x11 + unit_size;
    int y12 = y11 + unit_size;
    int x22 = x21 + unit_size;
    int y22 = y21 + unit_size;

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
    enemy.clear();
    bullet.clear();
    player_life = 10;
    game_cycle = game_score = game_time = 0;
}

void game_event( SDL_Event *event )
{
    double x, y, ab, a_b, sign = 1.0, norm, dx, dy;

    SDL_PollEvent( event );
    switch ( event->type ) {
        case SDL_QUIT:
            quit_flag = true;
            break;
        case SDL_MOUSEMOTION:
            x = event->motion.x - player_x;
            y = event->motion.y - player_y;
            a_b = x * x;
            ab = sqrt( x * x + y * y ) * x;
            sign = event->motion.y > player_y ? +1.0 : -1.0;
            player_angle = 90.0 + ( 180.0 / M_PI ) * acos( a_b / ab ) * sign;
            break;
        case SDL_MOUSEBUTTONDOWN:
            switch ( event->button.button ) {
                case SDL_BUTTON_LEFT:
                    x = event->button.x;
                    y = event->button.y;
                    norm = sqrt( pow( x - player_x, 2 ) + pow( y - player_y, 2 ) );
                    dx = ( x - player_x ) / norm;
                    dy = ( y - player_y ) / norm;
                    game_add_bullet( player_x, player_y, dx, dy );
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
    event->button.button = 0;
}

void game_loop( void )
{
    for ( size_t i = 0; i < bullet.size(); i++ ) {
        bullet[i].x = bullet[i].x + floor( bullet[i].dx * bullet[i].step );
        bullet[i].y = bullet[i].y + floor( bullet[i].dy * bullet[i].step );
        bullet[i].step++;
        bullet[i].life--;
        if ( bullet[i].life == 0 ) {
            bullet.erase( bullet.begin() + i );
            continue;
        }
        for ( size_t j = 0; j < enemy.size(); j++ ) {
            if ( is_intersect( bullet[i].x, bullet[i].y, enemy[j].x, enemy[j].y ) ) {
                enemy[j].life -= bullet[i].damage;
                bullet.erase( bullet.begin() + i );
                game_score += 5;
                break;
            }
        }
    }
    for ( size_t i = 0; i < enemy.size(); i++ ) {
        float norm = sqrt( pow( player_x - enemy[i].x, 2 ) + pow( player_y - enemy[i].y, 2 ) );
        float dx, dy;
        enemy[i].angle += 1.0;
        if ( enemy[i].angle == 360 ) {
            enemy[i].angle = 0.0;
        }
        if ( norm > 1E-2 ) {
            dx = ( player_x - enemy[i].x ) / norm;
            dy = ( player_y - enemy[i].y ) / norm;
            enemy[i].x += floor( dx );
            enemy[i].y += floor( dy );
        } else {
            dx = dy = 0;
            player_life -= enemy[i].attack;
            enemy[i].life = -1;
        }
        if ( enemy[i].life < 0 ) {
            enemy.erase( enemy.begin() + i );
            game_score += 50;
        }
    }
    if ( player_life <= 0 ) {
        std::cout << "GAME OVER!\nScore: " << game_score << "\nTime: " <<
            game_time << "\n"; 
        game_restart();
    }
    if ( game_cycle % ( 100 - ( game_time != 0 ? game_time : 99 ) ) == 0 ) {
        game_add_enemy();
    }
    if ( game_cycle % 60 == 0 ) {
        game_time++;
    }
    game_cycle++;
}

void tile_draw( SDL_Renderer *r, SDL_Texture *tex, double angle, int id, int size, int x, int y )
{
    SDL_Rect wnd = { 0, 0, size, size };
    SDL_Rect pos = { 0, 0, size, size };

    pos.x = x - size / 2;
    pos.y = y - size / 2;
    wnd.x = id * size;
    SDL_RenderCopyEx( r, tex, &wnd, &pos, angle, NULL, SDL_FLIP_NONE );
}

void game_render( void )
{
    wchar_t buffer[32];

    SDL_RenderClear( render );
    SDL_RenderCopy( render, background, NULL, NULL );
    tile_draw( render, t_player, player_angle, 0, unit_size, player_x, player_y );
    for ( size_t i = 0; i < bullet.size(); i++ ) {
        tile_draw( render, t_bullet, 0.0, 0, bullet_size, bullet[i].x, bullet[i].y );
    }
    for ( size_t i = 0; i < enemy.size(); i++ ) {
        tile_draw( render, t_enemy, enemy[i].angle, 0, unit_size, enemy[i].x, enemy[i].y );
    }
    swprintf( buffer, 32, L"  жизнь: %02d", player_life );
    font_draw( render, ft, buffer, 8, 440 );
    swprintf( buffer, 32, L"   очки: %u", game_score );
    font_draw( render, ft, buffer, 8, 450 );
    swprintf( buffer, 32, L"  время: %u", game_time );
    font_draw( render, ft, buffer, 8, 460 );
    SDL_RenderPresent( render );
}

void game_destroy( void )
{
    font_destroy( ft );
    SDL_DestroyTexture( background );
    SDL_DestroyTexture( t_player );
    SDL_DestroyTexture( t_enemy );
    SDL_DestroyTexture( t_bullet );
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
    srand( time( NULL ) );
    background = IMG_LoadTexture( render, "./images/background.png" );
    t_player = IMG_LoadTexture( render, "./images/player.png" );
    t_enemy = IMG_LoadTexture( render, "./images/enemy.png" );
    t_bullet = IMG_LoadTexture( render, "./images/bullet.png" );
    font_load( render, &ft, "../../00_other/font.cfg" );
    set_color( ft->font, 0x0 ); // black color
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