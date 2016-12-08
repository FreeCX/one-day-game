#include "font.h"

std::vector<std::string> tokenize( std::string data, std::string delimeters )
{
    size_t start = 0, counter = 0, length;
    size_t i, del_length = delimeters.length() + 1;
    size_t d_length = data.length();
    std::vector<std::string> a;

    do {
        for ( i = 0; i < del_length; i++ ) {
            if ( data[counter] == delimeters[i] ) {
                length = counter - start;
                if ( length > 0 ) {
                    a.push_back( std::string( data, start, length ) );
                    start = counter + 1;
                    break;
                }
                start = counter + 1;
            }
        }
    } while ( counter++ <= d_length ); 
    return a;
}

int font_load( SDL_Renderer *r, font_table_t **t, const char *font )
{
    SDL_Texture *tex = NULL;
    font_table_t *a = NULL;
    std::string tex_name, data, buf;
    std::vector<std::string> tok;
    char buffer[128];
    wint_t current = 0;
    size_t load = 1, i = 0;
    FILE *f;
    int id = 0;

    a = (font_table_t *) calloc( 1, sizeof(font_table_t) );
    *t = a;
    f = fopen( font, "rb" );
    if ( f == NULL ) {
        return -2;
    }

    do {
        load = fread( &buffer[i], 1, 1, f );
    } while ( buffer[i++] != '\0' );
    data = buffer;
    tok = tokenize( data, " " );
    buf = tok[0];
    a->t_width = atoi( tok[1].c_str() );
    a->t_height = atoi( tok[2].c_str() );
    tok.clear();
    tok = tokenize( font, "/" );
    for ( size_t i = 0; i < tok.size() - 1; i++ ) {
        tex_name += tok[i] + "/";
    }
    tex_name += buf;

    tex = IMG_LoadTexture( r, tex_name.c_str() );
    a->font = tex;
    if ( tex == NULL ) {
        fclose( f );
        return -1;
    }
    SDL_QueryTexture( tex, NULL, NULL, &( a->f_width ), &( a->f_height ) );
    do {
        load = fread( &current, 2, 1, f );
        if ( current != L'\n' && current < 0xFFFF && load != 0 ) {
            a->table[current] = id++;
        }
    } while ( load != 0 );
    fclose( f );
    return 0;
}

void set_color( SDL_Texture *tex, Uint32 color )
{
    SDL_SetTextureColorMod( tex, 
        color >> 16, ( color >> 8 ) & 0xFF, color & 0xFF );
}

void font_draw( SDL_Renderer *r, font_table_t *t, const wchar_t *text, int x, int y )
{
    SDL_Rect wnd = { 0, 0, t->t_width, t->t_height };
    SDL_Rect pos = { 0, 0, t->t_width, t->t_height };
    int dy = 0, i = 0, id = 0, old_x = x;
    wint_t current;

    pos.x = x; pos.y = y;
    while ( ( current = text[i++] ) != '\0' ) {
        switch ( current ) {
            case '\n':
                pos.y += t->t_height;
                pos.x = old_x;
                continue;
            case '\t':
                pos.x += 2 * t->t_width;
                continue;
            // to upper
            // case 'a'...'z':
            case L'а'...L'я':
                current -= 0x20;
                break;
        }
        id = t->table[current];
        while ( id * t->t_width >= t->f_width ) {
            id -= t->f_width / t->t_width;
            dy++;
        }
        wnd.x = id * t->t_width; wnd.y = dy * t->t_height;
        pos.x += t->t_width;
        SDL_RenderCopy( r, t->font, &wnd, &pos );
        dy = 0;
    }
}

void font_destroy( font_table_t *t )
{
    SDL_DestroyTexture( t->font );
    free( t );
}