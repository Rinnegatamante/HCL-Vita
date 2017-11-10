#include "system.h"
#include <vitasdk.h>
#include "../PHL.h"
#include <stdlib.h>
#include <stdio.h>

vita2d_pgf* debug_font;
uint32_t white;
int quitGame = 0;

int PHL_MainLoop(){	
	if (quitGame == 1) return 0;
	return 1;
}

void PHL_ConsoleInit(){
	debug_font = vita2d_load_default_pgf();
	white = RGBA8(0xFF, 0xFF, 0xFF, 0xFF);
}

void PHL_GameQuit(){
	quitGame = 1;
}

void PHL_ErrorScreen(char* message){	
	SceCtrlData pad;
	int oldpad = 0;
	
	while (PHL_MainLoop()) {
		
		sceCtrlPeekBufferPositive(0, &pad, 1);
		if (pad.buttons & SCE_CTRL_START) break;
		
		vita2d_start_drawing();
		vita2d_pgf_draw_text(debug_font, 2, 50, white, 1.0, message);
		vita2d_pgf_draw_text(debug_font, 2, 80, white, 1.0, "Press START to close");
		vita2d_end_drawing();
		vita2d_wait_rendering_done();
		vita2d_swap_buffers();
		
		oldpad = pad.buttons;
	}
	
	sceKernelExitDeleteThread(0);
}