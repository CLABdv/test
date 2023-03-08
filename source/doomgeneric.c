#include "doomgeneric.h"
#include <math.h>

uint32_t* DG_ScreenBuffer = 0;

#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <string.h>

//HACK: Change to not use global vars
int fbfd = 0;
struct fb_var_screeninfo vinfo;
struct fb_fix_screeninfo finfo;
long int screensize = 0;
char *fbp = 0;
long int location = 0;
uint32_t start_time;

//4 is bytes per pixel on target device
uint32_t finScreen[DOOM_ACTUAL_RESX*DOOM_ACTUAL_RESY*4];

#if 0
// the 0.99... is correction for cases where the decimal repeats infinitely, otherwise the algorithm will try to access memory out of bounds
// i have no clue how many paranthesis i need, but i know i use too many
#define x_step ((float)((float)DOOMGENERIC_RESX/(float)DOOM_ACTUAL_RESX))
#define y_step ((float)((float)DOOMGENERIC_RESY/(float)DOOM_ACTUAL_RESY))
#define x_side ((float)0.9999*((float)DOOMGENERIC_RESX/(float)DOOM_ACTUAL_RESX))
#define y_side ((float)0.9999*((float)DOOMGENERIC_RESY/(float)DOOM_ACTUAL_RESY))
float inverse_x_step = 1.0/x_step;
float inverse_y_step = 1.0/y_step;

static void shitty_resample()
{
    int x_out,y_out;
    float dx,dy;
    uint32_t destr,destg,destb;
    uint32_t desta=0xFF;
    for(y_out=0;y_out<DOOM_ACTUAL_RESY;y_out++)
    {
        float y_top=y_out*y_step;
        float y_bot=y_top+y_side;
        int jstart=(int)y_top;
        int jend=(int)y_bot;
        float devy1=jstart+1-y_top;
        float devy2=jend+1-y_bot;

        for(x_out=0;x_out<DOOM_ACTUAL_RESX;x_out++)
        {
            float x_left=x_out*x_step;
            float x_right=x_left+x_side;
            int istart=(int)x_left;
            int iend=(int)x_right;
            float devx1=istart+1-x_left;
            float devx2=iend+1-x_right;

            destr=0;
            destg=0;
            destb=0;
            dx=devx1;
            for(int i=istart;i<=iend;i++)
            {
                //this is inefficient
                //its an easy fix, i cba rn
                if(i==iend)
                    dx-=devx2;

                dy=devy1;
                for(int j=jstart;j<=jend;j++)
                {
                    if(j==jend)
                        dy-=devy2;

                    // coeff
                    float AP = inverse_x_step*inverse_y_step*dx*dy;
                    /* assuming the screen gives the info in AAGGBBRR */
                    uint8_t sourcer=((DG_ScreenBuffer[j*DOOMGENERIC_RESX+i]&0x000000FF)>>0);
                    uint8_t sourceg=((DG_ScreenBuffer[j*DOOMGENERIC_RESX+i]&0x0000FF00)>>8);
                    uint8_t sourceb=((DG_ScreenBuffer[j*DOOMGENERIC_RESX+i]&0x00FF0000)>>16);
                    destr+=AP*sourcer;
                    destg+=AP*sourceg;
                    destb+=AP*sourceb;
                    dy=1;
                }
                dx=1;
            }
        uint32_t destFin=
            (destr<<0)+
            (destg<<8)+
            (destb<<16)+
            (desta<<24);
        finScreen[y_out*DOOM_ACTUAL_RESX+x_out]=destFin;
        /* printf("destfin is %08X and testFin is %08X\n",destFin,testFin); */

        }
    }
}
#endif

// ratios
#define R_X ((float)DOOMGENERIC_RESX-1)/(DOOM_ACTUAL_RESX-1)
#define R_Y ((float)DOOMGENERIC_RESY-1)/(DOOM_ACTUAL_RESY-1)


void set8(uint8_t *cols, uint32_t src, float weight)
{
    // assumes AABBGGRR
    uint8_t r=(src&0x000000FF);
    uint8_t g=(src&0x0000FF00)>>8;
    uint8_t b=(src&0x00FF0000)>>16;
    cols[0]+=(int)r*weight;
    cols[1]+=(int)g*weight;
    cols[2]+=(int)b*weight;
}

// shamelessly stolen from
// https://chao-ji.github.io/jekyll/update/2018/07/19/BilinearResize.html
void bilinear_interpol() // interpol is here, hope you haven't done anything naughty
{
    uint8_t cols[3];
    for(int i=0;i<DOOM_ACTUAL_RESY;i++)
    {
        int y_top=(int)(i*R_Y);
        int y_bot=(int)ceilf(i*R_Y); // cast + 1 won't be accurate if the number is prefectly whole (i think)

        float y_weight=(R_Y*i)-y_top;
        for(int j=0;j<DOOM_ACTUAL_RESX;j++)
        {
            int x_left=(int)(j*R_X);
            int x_right=(int)ceilf(j*R_X);

            float x_weight=(R_X*j)-x_left;

            // pixel to take weights from
            // top left, top right, bot left, bot right
            uint32_t tl=DG_ScreenBuffer[y_top*DOOMGENERIC_RESX+x_left];
            uint32_t tr=DG_ScreenBuffer[y_top*DOOMGENERIC_RESX+x_right];
            uint32_t bl=DG_ScreenBuffer[y_bot*DOOMGENERIC_RESX+x_left];
            uint32_t br=DG_ScreenBuffer[y_bot*DOOMGENERIC_RESX+x_right];

            memset(cols,0,sizeof(cols));
            set8(cols,tl,(1-x_weight)*(1-y_weight));
            set8(cols,tr,x_weight*(1-y_weight));
            set8(cols,bl,y_weight*(1-x_weight));
            set8(cols,br,x_weight*y_weight);
            uint32_t fin=cols[0]+
                         (cols[1]<<8)+
                         (cols[2]<<16)+
                         (0xFF<<24);
            finScreen[i*DOOM_ACTUAL_RESX+j]=fin;
        }
    }
}

void DG_SetWindowTitle(const char *title)
{
	// do nothing
}

static uint64_t get_clock_tick_ms()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)(ts.tv_nsec / 1000000) + ((uint64_t)ts.tv_sec * 1000ull);
}

void DG_Init()
{
    int x,y;
    start_time=(uint32_t)get_clock_tick_ms();

    // Open the file for reading and writing
    fbfd = open("/dev/fb0", O_RDWR);
    if (fbfd == -1) {
        perror("Error: cannot open framebuffer device");
        exit(1);
    }
    printf("The framebuffer device was opened successfully.\n");

    // Get fixed screen information
    if (ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo) == -1) {
        perror("Error reading fixed information");
        exit(2);
    }

    // Get variable screen information
    if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo) == -1) {
        perror("Error reading variable information");
        exit(3);
    }

    printf("%dx%d, %dbpp\n", vinfo.xres, vinfo.yres, vinfo.bits_per_pixel);

    // Figure out the size of the screen in bytes
    screensize = (vinfo.xres * vinfo.yres * vinfo.bits_per_pixel) / 8;

    // Map the device to memory
    fbp = (char *)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED,fbfd, 0);

    if ((int)fbp == -1) {
        perror("Error: failed to map framebuffer device to memory");
     exit(4);
    }
    printf("The framebuffer device was mapped to memory successfully.\n");

    // stuff is freed as main returns.
}

void DG_DrawFrame()
{
    /* shitty_resample(); */
    bilinear_interpol();
    for(int y=0;y<DOOM_ACTUAL_RESY;y++)
    {
            location = (y+vinfo.yoffset) * finfo.line_length;
            memcpy(fbp+location,((char*)finScreen)+DOOM_ACTUAL_RESX*y*4,DOOM_ACTUAL_RESX*4);
    }
}

void DG_SleepMs(uint32_t ms)
{
    usleep(ms*1000);
}

// TODO: Implement function.
int DG_GetKey(int *pressed, unsigned char *key)
{
    *pressed='w';
    *key='w';
    return 0;
}

uint32_t DG_GetTicksMs()
{
    return get_clock_tick_ms()-start_time;
}


void dg_Create()
{
	DG_ScreenBuffer = malloc(DOOMGENERIC_RESX * DOOMGENERIC_RESY * 4);
	DG_Init();
}


extern void dg_Destroy()
{
    munmap(fbp,screensize);
    close(fbfd);
}
