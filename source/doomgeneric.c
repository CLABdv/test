#include "doomgeneric.h"

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

int x = 0, y = 0;
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

    // make the background clean
    for(y=0;y<vinfo.yres;y++)
    {
        for(x=0;x<vinfo.xres;x++)
        {
            location = (x+vinfo.xoffset) * (vinfo.bits_per_pixel/8) + (y+vinfo.yoffset) * finfo.line_length;

                *(fbp + location) = 0xFF;
                *(fbp + location + 1) = 0xFF;
                *(fbp + location + 2) = 0xFF;
                *(fbp + location + 3) = 0xFF; // no transparency
        }
    }
    // stuff is freed as main returns.
}

void DG_DrawFrame()
{
    for(y=0;y<DOOMGENERIC_RESY;y++)
    {
            location = (y+vinfo.yoffset) * finfo.line_length;
            memcpy(fbp+location,((char*)DG_ScreenBuffer)+DOOMGENERIC_RESX*y*4,DOOMGENERIC_RESX*4);
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
