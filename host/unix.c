#include <SDL2/SDL.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_ttf.h>
#include "SDL_FontCache/SDL_FontCache.h"
#include"vic_font.h"
//#include"sid_tune.h"
#include <c64maze.h>
#include <time.h>
#include <stdbool.h>

#include "port.h"
/* local vars */

SDL_Rect drect;
SDL_Renderer *ren;
SDL_Event event;
SDL_Window *win;

struct font
{
    unsigned char *pDesc;/* Pointer to the character raster array */
    unsigned int pos[256];/* Position of the symbol */
    unsigned char incX;/* Increment in X (-1 means a proportional font) */
    unsigned char incY;/* Increment in Y */
    unsigned char magnification;/* Magnification */
};

struct font f;
FC_Font *fc;

/* local funcs defs */
/*static void port_process_voice(unsigned char **ptr, unsigned char *sid_pointer,
    unsigned char *wsh);*/
/* funcs */
void port_pset(unsigned int x, unsigned int y)
{
    SDL_RenderDrawPoint( ren, x, y );
}

void port_clearHGRpage(void)
{
    SDL_SetRenderDrawColor( ren, 0, 0, 0, 0 );
    SDL_RenderClear( ren );
    SDL_SetRenderDrawColor( ren, 0x00, 0x66, 0x00, 0xFF );
}

void port_clearMazeRegion(void)
{
    SDL_SetRenderDrawColor( ren, 0, 0, 0, 0 );
    SDL_Rect rect =
    {
        .x = 0,
        .y = 0,
        .w = disp_bounds.labyrinthx,
        .h = disp_bounds.labyrinthy
    };

    SDL_RenderFillRect( ren, &rect );
}

/**
 * Switch on the HGR monochrome graphic mode.
 */
int port_graphics_init(void)
{
    if( SDL_Init( SDL_INIT_EVERYTHING ) == -1 )
    {
        return -1;
    }
    TTF_Init();
    win = SDL_CreateWindow( "C64-Maze", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                            SIZEX * 5,
                            SIZEY * 5,
                            SDL_WINDOW_SHOWN );

    if( win == NULL )
    {
        puts( "error when creating window" );
        puts( SDL_GetError() );
        return -1;
    }
    ren = SDL_CreateRenderer( win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC );
    if( ren == NULL )
    {
        puts( "error when creating renderer" );
        puts( SDL_GetError() );
        return -1;
    }

    SDL_Rect rect;
    if( SDL_GetDisplayBounds( 0, &rect ) != 0 )
    {
        SDL_Log( "SDL_GetDisplayBounds failed: %s", SDL_GetError() );
        return -1;
    }

    if( rect.w / SIZEX < rect.h / SIZEY )
    {
        disp_bounds.szx = rect.w;
        disp_bounds.szy = rect.w * SIZEY / SIZEX;
        /* calculate labyrinth size */
        disp_bounds.labyrinthx = rect.w * 2 / 3;
        disp_bounds.labyrinthy = disp_bounds.labyrinthx / SIZEX * SIZEY;
    }
    else
    {
        disp_bounds.szy = rect.h;
        disp_bounds.szx = rect.h * SIZEX / SIZEY;
        disp_bounds.labyrinthy = rect.h * 2 / 3;
        disp_bounds.labyrinthx = disp_bounds.labyrinthy * SIZEY / SIZEX;
    }
    disp_bounds.stepszx = disp_bounds.labyrinthx * STEPSIZEX / SIZEX;
    disp_bounds.stepszy = disp_bounds.labyrinthx * STEPSIZEY / SIZEX;

    /* set banner boundaries */
    disp_bounds.bannerx = disp_bounds.labyrinthx + 1;
    disp_bounds.bannerx_end = disp_bounds.szx;
    disp_bounds.bannery = 1;
    disp_bounds.bannery_end = disp_bounds.szy;

    printf( "display bounds: "
            "szx=%d, szy=%d, "
            "labyrinth=%dx%d "
            "step=%dx%d "
            "banner=%dx%d to %dx%d\n", disp_bounds.szx, disp_bounds.szy, disp_bounds.labyrinthx,
            disp_bounds.labyrinthy, disp_bounds.stepszx, disp_bounds.stepszy, disp_bounds.bannerx,
            disp_bounds.bannery, disp_bounds.bannerx_end, disp_bounds.bannery_end );

    return 0;
}

void port_vert_line(unsigned short x1, unsigned short y1, unsigned short y2)
{
    SDL_SetRenderDrawColor( ren, 0x00, 0xcc, 0x00, 0xFF );
    SDL_RenderDrawLine( ren, x1, y1, x1, y2 );
}

void port_diag_line(unsigned short x1, unsigned short y1, unsigned short ix, short incx, short incy)
{
    (void) ix;
    SDL_SetRenderDrawColor( ren, 0x00, 0x66, 0x00, 0xFF );
    SDL_RenderDrawLine( ren, x1, y1, x1 + incx, y1 + incy );
}

void port_hor_line(unsigned short x1, unsigned short x2, unsigned short y1)
{
    SDL_SetRenderDrawColor( ren, 0x00, 0x66, 0x00, 0xFF );
    SDL_RenderDrawLine( ren, x1, y1, x2, y1 );
}

void port_printat(unsigned short x, unsigned short y, char *s)
{
    static bool firsttime = true;
    if( firsttime )
    {
        fc = FC_CreateFont();
        FC_LoadFont( fc, ren, "DejaVuSerif.ttf", 20, FC_MakeColor( 255, 255, 255, 255 ), TTF_STYLE_NORMAL );
        firsttime = false;
    }
    FC_Draw( fc, ren, x, y, s );
}

/* Plot a line using the Bresenham algorithm
   from Nelson Johnson, "Advanced Graphics in C"
   ed. Osborne, McGraw-Hill.
   Horizontal and vertical lines need to be considerably
   speeded up by using a direct access to video RAM.
*/
void port_line(unsigned short x1, unsigned short y1, unsigned short x2, unsigned short y2)
{
    SDL_SetRenderDrawColor( ren, 0x00, 0xcc, 0x00, 0xFF );
    SDL_RenderDrawLine( ren, x1, y1, x2, y2 );
}

unsigned long port_get_time(void)
{
    return time( NULL );
}

void port_colour_banner(void)
{
    SDL_Rect rect =
    {
        .x = disp_bounds.bannerx,
        .y = disp_bounds.bannery,
        .w = disp_bounds.bannerx_end - disp_bounds.bannerx,
        .h = disp_bounds.bannery_end - disp_bounds.bannery
    };

    SDL_SetRenderDrawColor( ren, 210, 10, 10, 255 );
    SDL_RenderFillRect( ren, &rect );
    SDL_RenderPresent( ren );
}

long port_get_current_time(void)
{
    return time( NULL );
}

#if 0
static void port_process_voice(unsigned char **ptr, unsigned char *sid_pointer,
    unsigned char *wsh)
{
    (void)ptr;
    (void)sid_pointer;
    (void)wsh;
    /* TODO */
}
#endif

unsigned char port_sound_irq(void)
{
/* TODO */
    return 0;
}

void port_start_sound(unsigned char *l1, unsigned char *l2, unsigned char *l3)
{
    (void) l1;
    (void) l2;
    (void) l3;
/* TODO */
}

void port_loadVICFont(unsigned char magnification)
{
    unsigned int i;
    /* Load the font tables */
    for( i = 0; i < 256; ++i )
    {
        f.pos[i] = 0;
    }

    for( i = ' '; i <= '~'; ++i )
    {
        f.pos[i] = (i - ' ' + 1) * 8;
    }

    f.pDesc = vic_font;
    f.incX = 8;/* Increment in X (-1 would mean a proportional font) */
    f.incY = 8;/* Increment in Y */
    f.magnification = magnification;
}

char port_getch(void)
{
    SDL_WaitEvent( &event );
    SDL_PollEvent( &event );

    if( event.type == SDL_KEYDOWN )
    {
        switch( event.key.keysym.sym )
        {
        case SDLK_RIGHT:
            event.key.keysym.sym = 'g';
            break;

        case SDLK_LEFT:
            event.key.keysym.sym = 'f';
            break;

        case SDLK_UP:
            event.key.keysym.sym = 't';
            break;

        case SDLK_DOWN:
            event.key.keysym.sym = 'v';
            break;

        case SDLK_ESCAPE:
            exit( 1 );
            break;
        }
    }

    return event.key.keysym.sym;
}

void port_fflushMazeRegion(void)
{
    SDL_SetRenderDrawColor( ren, 0x00, 0x66, 0x00, 0xFF );
    SDL_RenderPresent( ren );
}
void port_music_on(void)
{
}
void port_music_off(void)
{
}
void port_font_magnification(unsigned char magnification)
{
    (void) magnification;
}
void port_exit(void)
{
    SDL_DestroyRenderer( ren );
    SDL_DestroyWindow( win );
    TTF_Quit();
    SDL_Quit();
    exit( 0 );
}
