//VITA graphics.c
#include "graphics.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vita2d.h>
#include "../game.h"
#include "../qda.h"
#include "../hero.h"

uint8_t drawing_phase = 0;
vita2d_texture* border = NULL;
extern int in_game;
extern uint8_t use_uma0;
int border_idx = 0;

void vita2d_texture_set_pixel(vita2d_texture* texture, int x, int y, unsigned int color){
	uint32_t* data = vita2d_texture_get_datap(texture);
	uint32_t pitch = vita2d_texture_get_stride(texture)>>2;
	data[x+y*pitch] = color;
}

//One time graphics setup
void PHL_GraphicsInit()
{
	vita2d_init_advanced(0x800000);
	if (use_uma0) border = vita2d_load_PNG_file("uma0:data/HCL/borders/border0.png");
	else border = vita2d_load_PNG_file("ux0:data/HCL/borders/border0.png");
}

void PHL_SwapBorder()
{
	char filename[128];
	vita2d_free_texture(border);
	border_idx++;
	sprintf(filename, "%s:data/HCL/borders/border%d.png", use_uma0 ? "uma0" : "ux0", border_idx);
	border = vita2d_load_PNG_file(filename);
	if (border == NULL){
		border_idx = 0;
		if (use_uma0) border = vita2d_load_PNG_file("uma0:data/HCL/borders/border0.png");
		else border = vita2d_load_PNG_file("ux0:data/HCL/borders/border0.png");
	}
}

void PHL_StartDrawing()
{
	if (drawing_phase) return;
	vita2d_start_drawing();
	if (!in_game) vita2d_clear_screen();
	drawing_phase = 1;
}

void PHL_EndDrawing()
{	
	if (!drawing_phase) return;
	if (in_game) vita2d_draw_texture(border, 0, 0);
	vita2d_end_drawing();
	vita2d_wait_rendering_done();
	vita2d_swap_buffers();
	drawing_phase = 0;
}

void PHL_GraphicsExit()
{
	if (drawing_phase) PHL_EndDrawing();
	vita2d_fini();
}

void PHL_ForceScreenUpdate()
{	
	
}

void PHL_SetDrawbuffer(vita2d_texture* surf)
{

}

void PHL_ResetDrawbuffer()
{

}

uint32_t PHL_NewRGB(uint8_t r, uint8_t g, uint8_t b)
{
	return (r | (g << 8) | (b << 16) | (0xFF << 24));
}

void PHL_SetColorKey(vita2d_texture* surf, uint8_t r, uint8_t g, uint8_t b)
{

}

vita2d_texture* PHL_NewSurface(uint16_t w, uint16_t h)
{	
	return vita2d_create_empty_texture_rendertarget(w, h, SCE_GXM_TEXTURE_FORMAT_A8B8G8R8);
}

void PHL_FreeSurface(vita2d_texture* surf)
{
	if (surf != NULL) {
		vita2d_free_texture(surf);
		surf = NULL;
	}
}

vita2d_texture* PHL_LoadBMP(int index)
{	
	vita2d_texture* result = NULL;
	unsigned char* QDAFile = (unsigned char*)malloc(headers[index].size);
	
	FILE* f = NULL;
	if (use_uma0) fopen("uma0:data/HCL/bmp.qda", "rb");
	else fopen("ux0:data/HCL/bmp.qda", "rb");
	if (f != NULL){
		
		//Load QDA file data
		fseek(f, headers[index].offset, SEEK_SET);
		fread(QDAFile, headers[index].size, 1, f);
		fclose(f);
		
		//Read data from header
		unsigned short w, h;

		memcpy(&w, &QDAFile[18], 2);
		memcpy(&h, &QDAFile[22], 2);

		result = PHL_NewSurface(w, h);
		
		//Load Palette
		uint32_t palette[20][18];
		int dx, dy;
		int count = 0;
		for (dx = 0; dx < 20; dx++) {
			for (dy = 0; dy < 16; dy++) {
				palette[dx][dy] = PHL_NewRGB(QDAFile[54 + count + 2], QDAFile[54 + count + 1], QDAFile[54 + count]);
				count += 4;
			}
		}
		uint32_t alphaKey = palette[0][0];
		
		//Darkness special case
		if (index == 27) alphaKey = PHL_NewRGB(0, 0, 0);
		
		//Edit surface pixels
		for (dx = w; dx > 0; dx--) {
			for (dy = h; dy >= 0; dy--) {		
				int pix = w - dx + w * dy;

				int px = QDAFile[1078 + pix] / 16;
				int py = QDAFile[1078 + pix] % 16;
				
				if (palette[px][py] == alphaKey) {
					vita2d_texture_set_pixel(result, w - dx, h - dy - 1, RGBA8(0, 0, 0, 0));
				}else{
					vita2d_texture_set_pixel(result, w - dx, h - dy - 1, palette[px][py]);
				}			
			}
		}
		
	}
	
	free(QDAFile);
	
	return result;
}

void PHL_DrawRect(int16_t x, int16_t y, uint16_t w, uint16_t h, uint32_t color)
{	
	if (!drawing_phase) return;	
	vita2d_draw_rectangle(160 + x,32 + y, w, h, RGBA8((color) & 0xFF, (color >> 8) & 0xFF, (color >> 16) & 0xFF, (color >> 24) & 0xFF));
}

void PHL_DrawSurface(int16_t x, int16_t y, vita2d_texture* surf)
{
	if (!drawing_phase) return;
	
	//Quake Shake
	if (quakeTimer > 0) {
		int val = quakeTimer % 4;
		if (val == 0) {
			y -= 2;
		}else if (val == 2) {
			y += 2;
		}
	}
	
	vita2d_draw_texture_scale(surf, 160 + x, 32 + y, 2.0, 2.0);
}

void PHL_DrawSurfacePart(int16_t x, int16_t y, int16_t cropx, int16_t cropy, int16_t cropw, int16_t croph, vita2d_texture* surf)
{	
	if (!drawing_phase) return;
	if (surf != NULL)
	{
		//Quake Shake
		if (quakeTimer > 0) {
			int val = quakeTimer % 4;
			if (val == 0) {
				y -= 2;
			}else if (val == 2) {
				y += 2;
			}
		}
		
		vita2d_draw_texture_part_scale(surf,160 + x,32 + y, cropx/2, cropy/2, cropw/2, croph/2, 2.0, 2.0);

	}
}

void PHL_DrawBackground(PHL_Background back, PHL_Background fore)
{
	int xx, yy;
	
	for (yy = 0; yy < 12; yy++)
	{
		for (xx = 0; xx < 16; xx++)
		{
			//Draw Background tiles
			PHL_DrawSurfacePart(xx * 40, yy * 40, back.tileX[xx][yy] * 40, back.tileY[xx][yy] * 40, 40, 40, images[imgTiles]);
			
			//Only draw foreground tile if not a blank tile
			if (fore.tileX[xx][yy] != 0 || fore.tileY[xx][yy] != 0) {
				PHL_DrawSurfacePart(xx * 40, yy * 40, fore.tileX[xx][yy] * 40, fore.tileY[xx][yy] * 40, 40, 40, images[imgTiles]);
			}
		}
	}
}

void PHL_UpdateBackground(PHL_Background back, PHL_Background fore)
{
	
}