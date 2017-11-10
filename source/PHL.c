#include "PHL.h"
#include <stdio.h>
#include <string.h>
#include "qda.h"
#include "game.h"

void PHL_Init()
{	
	PHL_GraphicsInit();
	PHL_AudioInit();

	#ifdef _3DS
		Result rc = romfsInit();
		/*if (rc) {
			printf("romfsInit: %08lX\n", rc);
			//while(1){}
		}
		else
		{
			printf("\nromfs Init Successful!\n");
		}*/
	#endif

	WHITE = 0;
	RED = 1;
	YELLOW = 2;
}

void PHL_Deinit()
{
	#ifndef _PSP2
		PHL_AudioClose();
	#endif
	PHL_GraphicsExit();
	
	#ifdef _3DS
		romfsExit();
	#endif
}

extern void LOG(char* format, ...);

//Extracts bmps from the bmp.qda archive file
PHL_Surface PHL_LoadQDA(char* fname)
{	
	PHL_Surface surf = NULL;
	
	int numofsheets = 29;
	int i;
	
	for (i = 0; i < numofsheets; i++)
	{
		if (strcmp(fname, (char*)headers[i].fileName) == 0) { //Match found
			//printf("\nMatch Found: %s", fname);
			surf = PHL_LoadBMP(i);
			i = numofsheets; //End search
		}
	}
	
	return surf;
}

void PHL_DrawTextBold(char* txt, int dx, int dy, int col)
{
	int i, cx, cy;
	
	for (i = 0; i < strlen(txt); i++)
	{
		cx = (txt[i] - 32) * 16;
		cy = 32 * col;
		
		while (cx >= 512) {
			cx -= 512;
			cy += 16;
		}
		
		PHL_DrawSurfacePart(dx + (16 * i), dy, cx, cy, 16, 16, images[imgBoldFont]);
	}
}

void PHL_DrawTextBoldCentered(char* txt, int dx, int dy, int col)
{
	if (dy < 640 && dy > -16) {
		int stringW = strlen(txt) * 16;
		
		PHL_DrawTextBold(txt, dx - (stringW / 2), dy, col);
	}
}