extern "C"{
	#include "audio.h"
}
#include "audio_decoder.h"

#define BUFSIZE        8192    // Max dimension of audio buffer size
#define BUFSIZE_MONO   4096    // Dimension of audio buffer files for mono tracks
#define NSAMPLES       2048    // Number of samples for output
#define SOUND_CHANNELS    7    // PSVITA has 8 available audio channels, one is reserved for musics

SceUID AudioThreads[SOUND_CHANNELS + 1], Sound_Mutex, NewSound_Mutex, Music_Mutex, NewMusic_Mutex;
DecodedMusic* new_sound = NULL;
DecodedMusic* new_music = NULL;
bool availThreads[SOUND_CHANNELS];
std::unique_ptr<AudioDecoder> audio_decoder[SOUND_CHANNELS + 1];
volatile bool mustExit = false;
uint8_t ids[] = {0, 1, 2, 3, 4, 5, 6, 7};

// Audio thread code
static int audioThread(unsigned int args, void* arg){

	// Getting thread id
	uint8_t* argv = (uint8_t*)arg;
	uint8_t id = argv[0];
	
	// Initializing audio port
	int ch = sceAudioOutOpenPort(SCE_AUDIO_OUT_PORT_TYPE_MAIN, NSAMPLES, 48000, SCE_AUDIO_OUT_MODE_STEREO);
	sceAudioOutSetConfig(ch, -1, -1, (SceAudioOutMode)-1);
	
	DecodedMusic* mus;
	for (;;){
		
		// Waiting for an audio output request
		sceKernelWaitSema((id == 7) ? Music_Mutex : Sound_Mutex, 1, NULL);
		
		// Setting thread as busy
		if (id < 7) availThreads[id] = false;
		
		// Fetching track
		if (id == 7) mus = new_music;
		else mus = new_sound;
		mus->audioThread = id;
		sceKernelSignalSema(((id == 7) ? NewMusic_Mutex : NewSound_Mutex), 1);
		
		// Checking if a new track is available
		if (mus == NULL){
			
			//If we enter here, we probably are in the exiting procedure
			if (mustExit){
				if (id < 7) sceKernelSignalSema(Sound_Mutex, 1);
				sceAudioOutReleasePort(ch);
				return sceKernelExitDeleteThread(0);
			}
		
		}
		
		// Setting audio channel volume
		int vol_stereo[] = {32767, 32767};
		sceAudioOutSetVolume(ch, (SceAudioOutChannelFlag)(SCE_AUDIO_VOLUME_FLAG_L_CH | SCE_AUDIO_VOLUME_FLAG_R_CH), vol_stereo);
		
		// Initializing audio decoder
		audio_decoder[id] = AudioDecoder::Create(mus->handle, "Track");
		if (audio_decoder[id] == NULL) continue; // TODO: Find why this case apparently can happen
		audio_decoder[id]->Open(mus->handle);
		audio_decoder[id]->SetLooping(mus->loop);
		audio_decoder[id]->SetFormat(48000, AudioDecoder::Format::S16, 2);
		
		// Checking resampler output mode
		int rate, chns;
		AudioDecoder::Format fmt;
		audio_decoder[id]->GetFormat(rate, fmt, chns);
		sceAudioOutSetConfig(ch, -1, -1, (SceAudioOutMode)(chns - 1));
		
		// Initializing audio buffers
		if (mus->audiobuf == NULL){
			mus->audiobuf = (uint8_t*)malloc(BUFSIZE);
			mus->audiobuf2 = (uint8_t*)malloc(BUFSIZE);
		}
		mus->cur_audiobuf = mus->audiobuf;
		
		// Audio playback loop
		for (;;){
		
			// Check if the music must be paused
			if (mus->pauseTrigger || mustExit){
			
				// Check if the music must be closed
				if (mus->closeTrigger){
					audio_decoder[id].reset();
					mus->isPlaying = 0;
					mus->pauseTrigger = 0;
					mus->closeTrigger = 0;
					mus = NULL;
					if (id < 7) availThreads[id] = true;
					if (!mustExit) break;
				}
				
				// Check if the thread must be closed
				if (mustExit){
				
					// Check if the audio stream has already been closed
					if (mus != NULL){
						mus->closeTrigger = 1;
						continue;
					}
					
					// Recursively closing all the threads
					if (id < 7) sceKernelSignalSema(Sound_Mutex, 1);
					sceAudioOutReleasePort(ch);
					return sceKernelExitDeleteThread(0);
					
				}
			
				mus->isPlaying = !mus->isPlaying;
				mus->pauseTrigger = 0;
			}
			
			if (mus->isPlaying){
				
				// Check if audio playback finished
				if ((!mus->loop) && audio_decoder[id]->IsFinished()) mus->isPlaying = 0;
				
				// Update audio output
				if (mus->cur_audiobuf == mus->audiobuf) mus->cur_audiobuf = mus->audiobuf2;
				else mus->cur_audiobuf = mus->audiobuf;
				audio_decoder[id]->Decode(mus->cur_audiobuf, (chns > 1) ? BUFSIZE : BUFSIZE_MONO);
				sceAudioOutOutput(ch, mus->cur_audiobuf);
				
			}else{
				
				// Check if we finished a non-looping audio playback
				if ((!mus->loop) && audio_decoder[id]->IsFinished()){
					
					// Releasing the audio file
					audio_decoder[id].reset();
					mus->isPlaying = 0;
					if (mus->tempBlock){
						free(mus->audiobuf);
						free(mus->audiobuf2);
						free(mus);
						mus = NULL;
					}else{
						mus->audioThread = 0xFF;
					}
					if (id < 7) availThreads[id] = true;
					break;
					
				}else sceKernelDelayThread(1000); // Tricky way to call a re-scheduling
				
			}
			
		}
		
	}
	
}

void PHL_AudioInit()
{
	// Creating audio mutexs
	Sound_Mutex = sceKernelCreateSema("Sound Mutex", 0, 0, 1, NULL);
	NewSound_Mutex = sceKernelCreateSema("NewSound Mutex", 0, 1, 1, NULL);
	Music_Mutex = sceKernelCreateSema("Music Mutex", 0, 0, 1, NULL);
	NewMusic_Mutex = sceKernelCreateSema("NewMusic Mutex", 0, 1, 1, NULL);

	// Starting audio threads
	for (int i=0;i < (SOUND_CHANNELS + 1); i++){
		if (i < SOUND_CHANNELS) availThreads[i] = true;
		AudioThreads[i] = sceKernelCreateThread("Audio Thread", &audioThread, 0x10000100, 0x10000, 0, 0, NULL);
		sceKernelStartThread(AudioThreads[i], sizeof(ids[i]), &ids[i]);
	}
}

void PHL_AudioClose()
{
	// Starting exit procedure for audio threads
	mustExit = true;
	sceKernelSignalSema(Sound_Mutex, 1);
	for (int i=0;i<SOUND_CHANNELS;i++){
		sceKernelWaitThreadEnd(AudioThreads[i], NULL, NULL);
	}
	sceKernelSignalSema(Music_Mutex, 1);
	sceKernelWaitThreadEnd(AudioThreads[SOUND_CHANNELS], NULL, NULL);
	mustExit = false;
		
	// Deleting audio mutex
	sceKernelDeleteSema(Sound_Mutex);
	sceKernelDeleteSema(NewSound_Mutex);
	sceKernelDeleteSema(Music_Mutex);
	sceKernelDeleteSema(NewMusic_Mutex);
}

//Each system can use a custom music file format
PHL_Music PHL_LoadMusic(char* fname, int loop)
{	
	PHL_Music mus;
	sprintf(mus.filepath, "ux0:data/HCL/%s.mid", fname);
	mus.loop = loop;
	mus.audiobuf = NULL;
	mus.audioThread = 0xFF;
	return mus;
}

PHL_Sound PHL_LoadSound(char* fname)
{
	PHL_Sound snd;
	sprintf(snd.filepath, "ux0:data/HCL/%s", fname);
	snd.loop = 0;
	snd.audiobuf = NULL;
	snd.audioThread = 0xFF;
	return snd;
}

void PHL_PlayMusic(PHL_Music snd)
{
	DecodedMusic* mus = &snd;
	
	if (strlen(mus->filepath) <= 1) return;
	
	// We create a temporary duplicated and play it instead of the original one
	DecodedMusic* dup = (DecodedMusic*)malloc(sizeof(DecodedMusic));
	memcpy(dup, mus, sizeof(DecodedMusic));
	dup->tempBlock = 1;
	dup->audiobuf = NULL;
	mus = dup;
	
	mus->handle = fopen(mus->filepath, "rb");
	if (mus->handle == NULL) return;
	mus->isPlaying = 1;
	mus->audioThread = 0xFF;
	mus->pauseTrigger = 0;
	mus->closeTrigger = 0;
	
	// Waiting till track slot is free
	sceKernelWaitSema(NewMusic_Mutex, 1, NULL);
	
	// Passing music to the audio thread
	new_music = mus;
	sceKernelSignalSema(Music_Mutex, 1);
}

void PHL_PlaySound(PHL_Sound snd, int channel)
{
	DecodedMusic* mus = &snd;
	
	if (strlen(mus->filepath) <= 1) return;
	
	// Wait till a thread is available
	bool found = false;
	for (int i=0; i<SOUND_CHANNELS; i++){
		found = availThreads[i];
		if (found) break;
	}
	if (!found) return;
	
	
	// We create a temporary duplicated and play it instead of the original one
	DecodedMusic* dup = (DecodedMusic*)malloc(sizeof(DecodedMusic));
	memcpy(dup, mus, sizeof(DecodedMusic));
	dup->tempBlock = 1;
	dup->audiobuf = NULL;
	mus = dup;
	
	mus->handle = fopen(mus->filepath, "rb");
	if (mus->handle == NULL) return;
	mus->isPlaying = 1;
	mus->audioThread = 0xFF;
	mus->pauseTrigger = 0;
	mus->closeTrigger = 0;
	
	// Waiting till track slot is free
	sceKernelWaitSema(NewSound_Mutex, 1, NULL);
	
	// Passing sound to the audio thread
	new_sound = mus;
	sceKernelSignalSema(Sound_Mutex, 1);
}

void PHL_StopMusic()
{
	if ((new_music != NULL) && new_music->isPlaying){
		new_music->closeTrigger = 1;
		new_music->pauseTrigger = 1;
	}
}

void PHL_StopSound(PHL_Sound snd, int channel)
{
	if ((strlen(snd.filepath) > 1) && snd.isPlaying){
		snd.closeTrigger = 1;
		snd.pauseTrigger = 1;
	}
}

void PHL_FreeMusic(PHL_Music snd)
{
	if (strlen(snd.filepath) > 1){
		sprintf(snd.filepath, "");
		if (snd.audiobuf != NULL){
			free(snd.audiobuf);
			free(snd.audiobuf2);
			snd.audiobuf = NULL;
		}
	}
}

void PHL_FreeSound(PHL_Sound snd)
{
	if (strlen(snd.filepath) > 1) sprintf(snd.filepath, "");
}