//VITA graphics.h
#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <vitasdk.h>
#include <vita2d.h>

#define PHL_Surface vita2d_texture*

#define PHL_RGB uint32_t

typedef struct {
	int tileX[16][12];
	int tileY[16][12];
} PHL_Background;

vita2d_texture* backBuffer;

void PHL_GraphicsInit();
void PHL_GraphicsExit();

void PHL_StartDrawing();
void PHL_EndDrawing();

void PHL_ForceScreenUpdate();

void PHL_SetDrawbuffer(vita2d_texture* surf);
void PHL_ResetDrawbuffer();

uint32_t PHL_NewRGB(uint8_t r, uint8_t g, uint8_t b);
void PHL_SetColorKey(vita2d_texture* surf, uint8_t r, uint8_t g, uint8_t b);

vita2d_texture* PHL_NewSurface(uint16_t w, uint16_t h);
void PHL_FreeSurface(vita2d_texture* surf);

vita2d_texture* PHL_LoadBMP(int index);

void PHL_DrawRect(int16_t x, int16_t y, uint16_t w, uint16_t h, uint32_t col);

void PHL_DrawSurface(int16_t x, int16_t y, vita2d_texture* surf);
void PHL_DrawSurfacePart(int16_t x, int16_t y, int16_t cropx, int16_t cropy, int16_t cropw, int16_t croph, vita2d_texture*);

void PHL_DrawBackground(PHL_Background back, PHL_Background fore);
void PHL_UpdateBackground(PHL_Background back, PHL_Background fore);

void PHL_SwapBorder();

#endif