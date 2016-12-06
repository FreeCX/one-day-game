#include <cmath>
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
    int life;
    int speed;
    int attack;
} g_enemy;

typedef struct {
    int x, y;
    int life;
    int ammo;
} g_player;

typedef struct {
    int x, y;
    int life;
    int bullet;
} g_ammo;

typedef struct {
    int x, y;
    float dx, dy;
    int life;
    int step;
    int damage;
} g_bullet;

typedef struct {
    int x, y;
    int type;
    int life;
    int alpha;
    Uint32 color;
} g_blood;

const char game_name[] = "survival game";
const int screen_width = 640;
const int screen_height = 480;
const int unit_size = 32;
const int bullet_size = 8;
const int blood_size = 64;
const int max_ammo = 12;
const int player_step = 2;
bool quit_flag = false;

SDL_Window *window = NULL;
SDL_Renderer *render = NULL;
SDL_Event event;
SDL_Texture *unit = NULL;
SDL_Texture *background = NULL;
SDL_Texture *t_bullet = NULL;
SDL_Texture *t_ammo = NULL;
SDL_Texture *t_blood = NULL;
font_table_t *ft = NULL;

unsigned int game_cycle = 0;
unsigned int game_score = 0;
g_player player;
std::vector<g_enemy> enemy;
std::vector<g_bullet> bullet;
std::vector<g_ammo> ammo;
std::vector<g_blood> blood;

void game_add_enemy( void )
{
    g_enemy a;
    int x, y;

    x = rand() % screen_width;
    y = rand() % ( screen_height / 6 );
    y = y < 0 ? screen_height - y : y;
    a.x = x;
    a.y = y;
    a.life = 8;
    a.attack = 1;
    enemy.push_back( a );
}

void game_add_ammo( void )
{
    g_ammo a;
    int x, y;

    x = rand() % screen_width;
    y = rand() % screen_height;
    a.x = x; a.y = y;
    a.life = 300;
    a.bullet = rand() % max_ammo + 1;
    ammo.push_back( a );
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

void game_add_blood( int x, int y, Uint32 color, int type )
{
    g_blood a;

    a.x = x; a.y = y;
    a.type = type;
    a.life = 400;
    a.alpha = 250;
    a.color = color;
    blood.push_back( a );
}

void game_struct_init( void )
{
    enemy.clear();
    bullet.clear();
    ammo.clear();
    blood.clear();
    game_add_enemy();
    player.x = ( screen_width - unit_size ) / 2;
    player.y = ( screen_height - unit_size ) / 2;
    player.life = 5;
    player.ammo = max_ammo;
}

void tile_draw( SDL_Renderer *r, SDL_Texture *tex, int id, int size, int x, int y )
{
    SDL_Rect wnd = { 0, 0, size, size };
    SDL_Rect pos = { 0, 0, size, size };

    pos.x = x;
    pos.y = y;
    wnd.x = id * size;
    SDL_RenderCopy( r, tex, &wnd, &pos );
}

void send_error( int code )
{
    std::cout << SDL_GetError() << std::endl;
    exit( code );
}

void game_restart( void )
{
    std::cout << "GAME OVER: YOU'RE DEAD!" << std::endl;
    game_cycle = game_score = 0;
    game_struct_init();
}

void game_event( SDL_Event *event )
{
    static short keycode_repeat = 5;
    static int last_keycode = 0;
    int x, y;
    float dx, dy, norm;

    SDL_PollEvent( event );
    switch ( event->type ) {
        case SDL_QUIT:
            quit_flag = true;
            break;
        case SDL_KEYDOWN:
            switch ( event->key.keysym.sym ) {
                case SDLK_UP:
                    player.y -= player_step;
                    last_keycode = SDLK_UP;
                    keycode_repeat = 5;
                    break;
                case SDLK_DOWN:
                    player.y += player_step;
                    last_keycode = SDLK_DOWN;
                    keycode_repeat = 5;
                    break;
                case SDLK_LEFT:
                    player.x -= player_step;
                    last_keycode = SDLK_LEFT;
                    keycode_repeat = 5;
                    break;
                case SDLK_RIGHT:
                    player.x += player_step;
                    last_keycode = SDLK_RIGHT;
                    keycode_repeat = 5;
                    break;
                case SDLK_ESCAPE:
                case SDLK_q:
                    quit_flag = true;
                    break;
                case SDLK_r:
                    game_restart();
                    break;
            }
            break;
        case SDL_MOUSEBUTTONDOWN:
            switch ( event->button.button ) {
                case SDL_BUTTON_LEFT:
                    if ( player.ammo > 0 ) {
                        x = event->button.x;
                        y = event->button.y;
                        norm = sqrt( pow( x - player.x, 2 ) + pow( y - player.y, 2 ) );
                        dx = ( x - player.x ) / norm;
                        dy = ( y - player.y ) / norm;
                        game_add_bullet( player.x + unit_size / 2 - bullet_size / 2, 
                            player.y + unit_size / 2 - bullet_size / 2, dx, dy );
                        player.ammo--;
                    }
                    break;
            }
            break;
        default:
            break;
    }
    if (keycode_repeat) {
        switch (last_keycode) {
            case SDLK_UP:
                player.y -= player_step;
                break;
            case SDLK_DOWN:
                player.y += player_step;
                break;
            case SDLK_LEFT:
                player.x -= player_step;
                break;
            case SDLK_RIGHT:
                player.x += player_step;
                break;
        }
        keycode_repeat -= 1;
    }
    event->button.button = 0;
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

void game_loop( void )
{
    for ( size_t i = 0; i < bullet.size(); i++ ) {
        bullet[i].x = bullet[i].x + ceil( bullet[i].dx * bullet[i].step );
        bullet[i].y = bullet[i].y + ceil( bullet[i].dy * bullet[i].step );
        bullet[i].step++;
        bullet[i].life--;
        if ( bullet[i].life == 0 ) {
            bullet.erase( bullet.begin() + i );
            continue;
        }
        for ( size_t j = 0; j < enemy.size(); j++ ) {
            if ( is_intersect( bullet[i].x, bullet[i].y, enemy[j].x, enemy[j].y ) ) {
                game_add_blood( enemy[j].x - unit_size/2, 
                    enemy[j].y - unit_size/2, 0xA00000, rand()%3 );
                enemy[j].life -= bullet[i].damage;
                bullet.erase( bullet.begin() + i );
                game_score += 5;
                break;
            }
        }
    }
    for ( size_t i = 0; i < ammo.size(); i++ ) {
        ammo[i].life--;
        if ( ammo[i].life == 0 ) {
            ammo.erase( ammo.begin() + i );
            continue;
        }
        for ( size_t j = 0; j < ammo.size(); j++ ) {
            if ( is_intersect( ammo[i].x, ammo[i].y, player.x, player.y ) ) {
                player.ammo += ammo[i].bullet;
                ammo.erase( ammo.begin() + i );
                break;
            }
        }
    }
    for ( size_t i = 0; i < enemy.size(); i++ ) {
        float norm = sqrt( pow( player.x - enemy[i].x, 2 ) + pow( player.y - 16 - enemy[i].y, 2 ) );
        float dx, dy;
        if ( norm != 0 ) {
            dx = ( player.x - enemy[i].x ) / norm;
            dy = ( player.y - 16 - enemy[i].y ) / norm;
        } else {
            dx = dy = 0;
        }
        enemy[i].x += ceil( dx );
        enemy[i].y += ceil( dy );
        if ( enemy[i].life < 0 ) {
            enemy.erase( enemy.begin() + i );
            game_score += 50;
            continue;
        }
        if ( is_intersect( enemy[i].x, enemy[i].y, player.x, player.y ) && game_cycle % 20 == 0 ) {
            player.life -= enemy[i].attack;
            game_add_blood( player.x - unit_size/2, 
                player.y - unit_size/2, 0xFFFFFF, rand()%3  );
        }
    }
    for ( size_t i = 0; i < blood.size(); i++ ) {
        blood[i].life--;
        if ( blood[i].life < 25 ) {
            blood[i].alpha -= 10;
        }
        if ( blood[i].life == 0 ) {
            blood.erase( blood.begin() + i );
            continue;
        }
    }
    if ( player.life <= 0 ) {
        game_restart();
    }
    if ( game_cycle % 200 == 0 ) {
        game_add_enemy();
    }
    if ( game_cycle % 400 == 0 ) {
        game_add_ammo();
    }
    game_cycle++;
}

void game_render( void )
{
    wchar_t buffer[32];

    SDL_RenderClear( render );
    SDL_RenderCopy( render, background, NULL, NULL );
    for ( size_t i = 0; i < blood.size(); i++ ) {
        set_color( t_blood, blood[i].color );
        SDL_SetTextureAlphaMod( t_blood, blood[i].alpha );
        tile_draw( render, t_blood, blood[i].type, blood_size, blood[i].x, blood[i].y );
    }
    set_color( unit, 0xFFFFFF );
    tile_draw( render, unit, 0, unit_size, player.x, player.y );
    for ( size_t i = 0; i < bullet.size(); i++ ) {
        tile_draw( render, t_bullet, 0, bullet_size, bullet[i].x, bullet[i].y );
    }
    for ( size_t i = 0; i < enemy.size(); i++ ) {
        set_color( unit, 0x00FF00 );
        tile_draw( render, unit, 0, unit_size, enemy[i].x, enemy[i].y );
    }
    for ( size_t i = 0; i < ammo.size(); i++ ) {
        tile_draw( render, t_ammo, 0, unit_size, ammo[i].x, ammo[i].y );
    }
    swprintf( buffer, 32, L"  жизнь: %02d", player.life );
    font_draw( render, ft, buffer, 8, 440 );
    swprintf( buffer, 32, L"патроны: %02d", player.ammo );
    font_draw( render, ft, buffer, 8, 450 );
    swprintf( buffer, 32, L"   очки: %u", game_score );
    font_draw( render, ft, buffer, 8, 460 );
    SDL_RenderPresent( render );
}

void game_destroy( void )
{
    font_destroy( ft );
    SDL_DestroyTexture( unit );
    SDL_DestroyTexture( background );
    SDL_DestroyTexture( t_bullet );
    SDL_DestroyTexture( t_ammo );
    SDL_DestroyTexture( t_blood );
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
    unit = IMG_LoadTexture( render, "./images/unit.png" );
    background = IMG_LoadTexture( render, "./images/background.png" );
    t_bullet = IMG_LoadTexture( render, "./images/bullet.png" );
    t_ammo = IMG_LoadTexture( render, "./images/ammo.png" );
    t_blood = IMG_LoadTexture( render, "./images/blood.png" );
    font_load( render, &ft, "../../00_other/font.cfg" );
    set_color( ft->font, 0x0 ); // black color
    game_struct_init();
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