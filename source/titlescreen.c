#include "titlescreen.h"
#include "game.h"
#include <stdio.h>
#include "text.h"

int tempsave = 0;
int cursor = 0;

void titleScreenSetup();

int titleScreenStep();
void titleScreenDraw();

void pretitleScreen(){
	char loop = 1;	
	while (PHL_MainLoop())
	{
		//Get input
		PHL_ScanInput();
		
		//Check for button pressing
		if (btnAccept.pressed == 1 || btnStart.pressed == 1) {
			PHL_PlaySound(sounds[sndOk], 1);
			break;
		}
		
		//Draw pre-titlescreen
		PHL_StartDrawing();
		PHL_DrawTextBoldCentered("HYDRA CASTLE LABYRINTH VITA V.1.0", 320, 30, YELLOW);
		PHL_DrawTextBoldCentered("BY RINNEGATAMANTE", 320, 50, YELLOW);
		PHL_DrawTextBoldCentered("THANKS TO ALL DISTINGUISHED PATRONERS", 320, 100, YELLOW);
		PHL_DrawTextBoldCentered("FOR THEIR AWESOME SUPPORT:", 320, 120, YELLOW);
		PHL_DrawTextBoldCentered("XANDRIDFIRE", 320, 150, WHITE);
		PHL_DrawTextBoldCentered("STYDE PREGNY", 320, 170, WHITE);
		PHL_DrawTextBoldCentered("BILLY MCLAUGHIN II", 320, 190, WHITE);
		PHL_DrawTextBoldCentered("CREDITS", 320, 240, YELLOW);
		PHL_DrawTextBoldCentered("E.HASHIMOTO FOR THE ORIGINAL GAME", 320, 270, WHITE);
		PHL_DrawTextBoldCentered("EASYRPG TEAM FOR THE AUDIO DECODER", 320, 290, WHITE);
		PHL_DrawTextBoldCentered("SWITCHBLADEFURY FOR A SCREEN BORDER", 320, 310, WHITE);
		PHL_DrawTextBoldCentered("BRANDONHEAT8 FOR TWO SCREEN BORDERS", 320, 330, WHITE);
		PHL_DrawTextBoldCentered("PRESS START TO CONTINUE", 320, 400, YELLOW);
		PHL_EndDrawing();	
		
	}
}

int titleScreen()
{
	
	#ifdef _PSP2
	pretitleScreen();
	#endif
	
	titleScreenSetup();
	
	char loop = 1;
	int result = -1;
	
	while (PHL_MainLoop() && loop == 1)
	{	
		//Get input
		PHL_ScanInput();
		
		//Titlescreen step
		result = titleScreenStep();
		
		//Draw titlescreen
		PHL_StartDrawing();
			
		titleScreenDraw();

		if (result != -1) {
			loop = 0;
			//Force screen to black
			PHL_DrawRect(0, 0, 640, 480, PHL_NewRGB(0, 0, 0));
		}

		PHL_EndDrawing();		
	}
	
	return result;	
}

void titleScreenSetup()
{	
	cursor = 0;
	
	//Move cursor if save file exists
	if ( fileExists("map/018.map") ) {
		cursor = 1;
	}	
	
	//Check if temp save file exists
	tempsave = 0;
	if ( fileExists("data/save.tmp") ) {
		tempsave = 1;
		cursor = 1;		
	}
}

int titleScreenStep()
{
	//Move cursor
	if (btnDown.pressed == 1 || btnSelect.pressed == 1) {
		cursor += 1;
		if (cursor > 2) {
			cursor = 0;
		}
		PHL_PlaySound(sounds[sndPi01], 1);
	}
	
	if (btnUp.pressed == 1) {
		cursor -= 1;
		if (cursor < 0) {
			cursor = 2;
		}
		PHL_PlaySound(sounds[sndPi01], 1);
	}
	
	//Selection
	if (btnAccept.pressed == 1 || btnStart.pressed == 1) {
		PHL_PlaySound(sounds[sndOk], 1);
		return cursor;
	}
	
	return -1;
}

void titleScreenDraw()
{
	//Blackdrop
	PHL_DrawRect(0, 0, 640, 480, PHL_NewRGB(0, 0, 0));
	
	//if (tempsave == 0) {
		//Title image
		PHL_DrawSurfacePart(168, 72, 0, 0, 304, 168, images[imgTitle01]);
	/*}else{
		//Save error message
		drawTextCentered(saveError[0], 320, 80);
		drawTextCentered(saveError[1], 320, 80 + 50);
		drawTextCentered(saveError[2], 320, 80 + 96);
	}*/
	
	//Cursor
	PHL_DrawSurfacePart(228, 264 + (cursor * 32), 4, 176, 184, 32, images[imgTitle01]);
	
	//Text
	PHL_DrawTextBold("NEW GAME", 256, 272, YELLOW);
	PHL_DrawTextBold("LOAD GAME", 248, 304, YELLOW);
	PHL_DrawTextBold("EXIT", 288, 336, YELLOW);
	PHL_DrawTextBold("(C) 2011 E.HASHIMOTO", 160, 400, WHITE);
}