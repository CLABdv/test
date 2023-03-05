#ifndef DOOM_GENERIC
#define DOOM_GENERIC

#include <stdlib.h>
#include <stdint.h>

#ifdef USE_OG_RES_
#define DOOMGENERIC_RESX 640
#define DOOMGENERIC_RESY 400
#else
#define DOOMGENERIC_RESX 171
#define DOOMGENERIC_RESY 128
#endif

extern uint32_t* DG_ScreenBuffer;
extern void dg_Destroy();

void DG_Init();
void DG_DrawFrame();
void DG_SleepMs(uint32_t ms);
uint32_t DG_GetTicksMs();
int DG_GetKey(int* pressed, unsigned char* key);
void DG_SetWindowTitle(const char * title);

#endif //DOOM_GENERIC
