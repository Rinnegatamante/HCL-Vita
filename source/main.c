#include "PHL.h"
#include "game.h"
#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>

#ifdef _PSP2
extern int enterButton;
#endif

void createSaveLocations()
{	
	//Force create save data folders	
	#ifdef _3DS
		//3DS builds
		mkdir("sdmc:/3ds", 0777);
		mkdir("sdmc:/3ds/appdata", 0777);
		mkdir("sdmc:/3ds/appdata/HydraCastleLabyrinth", 0777);
		mkdir("sdmc:/3ds/appdata/HydraCastleLabyrinth/data", 0777);
		mkdir("sdmc:/3ds/appdata/HydraCastleLabyrinth/map", 0777);
	#else
		#ifdef _PSP2
			//vita
			sceIoMkdir("ux0:data/HCL", 0777);
			sceIoMkdir("ux0:data/HCL/data", 0777);
			sceIoMkdir("ux0:data/HCL/map", 0777);
		#else
			//psp, wii
			mkdir("/data", 0777);
			mkdir("/map", 0777);
		#endif
	#endif
}


int main(int argc, char **argv)
{	
	//Setup
	#ifdef _3DS
		sdmcInit();
		osSetSpeedupEnable(false);
	#endif
	
	#ifdef _PSP2
		SceAppUtilInitParam appUtilParam;
		SceAppUtilBootParam appUtilBootParam;
		memset(&appUtilParam, 0, sizeof(SceAppUtilInitParam));
		memset(&appUtilBootParam, 0, sizeof(SceAppUtilBootParam));
		sceAppUtilInit(&appUtilParam, &appUtilBootParam);
		sceAppUtilSystemParamGetInt(SCE_SYSTEM_PARAM_ID_ENTER_BUTTON, (int *)&enterButton);
	#endif
	
	srand(time(NULL));
	createSaveLocations();
	
	game();

	//System specific cleanup	
	#ifdef _PSP
		sceKernelExitGame();
	#endif
	
	#ifdef _3DS
		sdmcExit();
	#endif
	
	#ifdef _PSP2
		sceAppUtilShutdown();
	#endif
	
	return 0;
}