#include "input.h"

void updateKey(Button* btn, int state);

int enterButton = 0;

void PHL_ScanInput()
{	
	SceCtrlData pad;
	sceCtrlPeekBufferPositive(0, &pad, 1);
	uint32_t kDown = pad.buttons;
		
	updateKey(&btnUp, (kDown & SCE_CTRL_UP) || (pad.ly < 70));
	updateKey(&btnDown, kDown & SCE_CTRL_DOWN || (pad.ly > 170));
	updateKey(&btnLeft, kDown & SCE_CTRL_LEFT || (pad.lx < 70));
	updateKey(&btnRight, kDown & SCE_CTRL_RIGHT || (pad.lx > 170));
	
	updateKey(&btnStart, kDown & SCE_CTRL_START);
	updateKey(&btnSelect, kDown & SCE_CTRL_SELECT);
	
	updateKey(&btnFaceRight, kDown & SCE_CTRL_CIRCLE);
	updateKey(&btnFaceDown, kDown & SCE_CTRL_CROSS);
	updateKey(&btnFaceLeft, kDown & SCE_CTRL_SQUARE);
	
	updateKey(&btnL, kDown & SCE_CTRL_LTRIGGER);
	updateKey(&btnR, kDown & SCE_CTRL_RTRIGGER);
	
	if (enterButton == 0){
		updateKey(&btnAccept, kDown & SCE_CTRL_CIRCLE);
		updateKey(&btnDecline, kDown & SCE_CTRL_CROSS);
	}else{
		updateKey(&btnAccept, kDown & SCE_CTRL_CROSS);
		updateKey(&btnDecline, kDown & SCE_CTRL_CIRCLE);
	}
	
	updateKey(&btnSwap, kDown & SCE_CTRL_TRIANGLE);
	
}

void updateKey(Button* btn, int state)
{
	if (state) {
		if (btn->held == 1) {
			btn->pressed = 0;
		}else{
			btn->pressed = 1;
		}
		btn->held = 1;
		btn->released = 0;
	}else{
		if (btn->held == 1) {
			btn->released = 1;
		}else{
			btn->released = 0;
		}
		btn->held = 0;
		btn->pressed = 0;
	}
}