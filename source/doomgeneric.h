#ifndef DOOM_GENERIC
#define DOOM_GENERIC

#include <stdlib.h>
#include <stdint.h>


#define DOOMGENERIC_RESX 640
#define DOOMGENERIC_RESY 400

// the screens actual resolution is 178 in x,
// this is the one that gets the closest to a 4:3 aspect ratio
#define DOOM_ACTUAL_RESX 171
#define DOOM_ACTUAL_RESY 128
#define DOOM_ACTUAL_BYTES_PIXEL 4 // 32 bits per pixel / 8

extern uint32_t* DG_ScreenBuffer;
extern void dg_Destroy();

void DG_Init();
void DG_DrawFrame();
void DG_SleepMs(uint32_t ms);
uint32_t DG_GetTicksMs();
int DG_GetKey(int* pressed, unsigned char* key);
void DG_SetWindowTitle(const char * title);

#endif //DOOM_GENERIC
