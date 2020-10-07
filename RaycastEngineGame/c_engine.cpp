#include "c_engine.h"


// MIDI
#include "minisdl_audio.h"
#define TSF_IMPLEMENTATION
#include "tsf.h"
#define TML_IMPLEMENTATION
#include "tml.h"



RaycastEngine* RaycastEngine::p_instance = NULL;


// Callback function called by the audio thread
static void AudioCallback(void* data, Uint8* stream, int len)
{
	//Number of samples to process
	int SampleBlock, SampleCount = (len / (2 * sizeof(float))); //2 output channels
	for (SampleBlock = TSF_RENDER_EFFECTSAMPLEBLOCK; SampleCount; SampleCount -= SampleBlock, stream += (SampleBlock * (2 * sizeof(float))))
	{
		//We progress the MIDI playback and then process TSF_RENDER_EFFECTSAMPLEBLOCK samples at once
		if (SampleBlock > SampleCount) SampleBlock = SampleCount;

		//Loop through all MIDI messages which need to be played up until the current playback time
		for (g_Msec += SampleBlock * (1000.0f / 44100.0f); g_MidiMessage && g_Msec >= g_MidiMessage->time; g_MidiMessage = g_MidiMessage->next)
		{
			switch (g_MidiMessage->type)
			{
			case TML_PROGRAM_CHANGE: //channel program (preset) change (special handling for 10th MIDI channel with drums)
				tsf_channel_set_presetnumber(g_TinySoundFont, g_MidiMessage->channel, g_MidiMessage->program, (g_MidiMessage->channel == 9));
				break;
			case TML_NOTE_ON: //play a note
				tsf_channel_note_on(g_TinySoundFont, g_MidiMessage->channel, g_MidiMessage->key, g_MidiMessage->velocity / 127.0f);
				break;
			case TML_NOTE_OFF: //stop a note
				tsf_channel_note_off(g_TinySoundFont, g_MidiMessage->channel, g_MidiMessage->key);
				break;
			case TML_PITCH_BEND: //pitch wheel modification
				tsf_channel_set_pitchwheel(g_TinySoundFont, g_MidiMessage->channel, g_MidiMessage->pitch_bend);
				break;
			case TML_CONTROL_CHANGE: //MIDI controller messages
				tsf_channel_midi_control(g_TinySoundFont, g_MidiMessage->channel, g_MidiMessage->control, g_MidiMessage->control_value);
				break;
			}
		}

		// Render the block of audio samples in float format
		tsf_render_float(g_TinySoundFont, (float*)stream, SampleBlock, 0);
	}
}


void RaycastEngine::StartMIDIEngine() {

	tml_message* TinyMidiLoader = NULL;
	g_Msec = 0;

	// Define the desired audio output format we request
	SDL_AudioSpec OutputAudioSpec;
	OutputAudioSpec.freq = 44100;
	OutputAudioSpec.format = AUDIO_F32;
	OutputAudioSpec.channels = 2;
	OutputAudioSpec.samples = 4096;
	OutputAudioSpec.callback = AudioCallback;


	// Initialize the audio system
	if (SDL_AudioInit(TSF_NULL) < 0)
	{
		fprintf(stderr, "Could not initialize audio hardware or driver\n");
	}

	//TinyMidiLoader = tml_load_filename("SODOM.mid");
	//if (!TinyMidiLoader)
	//{
	//	fprintf(stderr, "Could not load MIDI file\n");
	//}

	//Set up the global MidiMessage pointer to the first MIDI message
	//g_MidiMessage = TinyMidiLoader;

	// Load the SoundFont from a file
	g_TinySoundFont = tsf_load_filename("sounds/midi.sf2");
	if (!g_TinySoundFont)
	{
		fprintf(stderr, "MIDI : Could not load SoundFont\n");
	}

	//Initialize preset on special 10th MIDI channel to use percussion sound bank (128) if available
	tsf_channel_set_bank_preset(g_TinySoundFont, 9, 128, 0);

	// Set the SoundFont rendering output mode
	tsf_set_output(g_TinySoundFont, TSF_STEREO_INTERLEAVED, OutputAudioSpec.freq, 0.0f);

	// Request the desired audio output format
	if (SDL_OpenAudio(&OutputAudioSpec, TSF_NULL) < 0)
	{
		fprintf(stderr, "MIDI : Could not open the audio hardware or the desired audio output format\n");
	}


	tsf_set_volume(g_TinySoundFont, 0.2f);


	// Start the actual audio playback here
	// The audio thread will begin to call our AudioCallback function
	SDL_PauseAudio(0);


	//Wait until the entire MIDI file has been played back (until the end of the linked message list is reached)
	//while (g_MidiMessage != NULL) SDL_Delay(100);

	// We could call tsf_close(g_TinySoundFont) and tml_free(TinyMidiLoader)
	// here to free the memory and resources but we just let the OS clean up
	// because the process ends here.

}

void RaycastEngine::RestartMIDI(char* fileName) {

	tsf_reset(g_TinySoundFont);

	tml_message* TinyMidiLoader = NULL;
	g_Msec = 0;


	TinyMidiLoader = tml_load_filename(fileName);
	if (!TinyMidiLoader)
	{
		fprintf(stderr, "Could not load MIDI file\n");
	}

	//Set up the global MidiMessage pointer to the first MIDI message
	g_MidiMessage = TinyMidiLoader;

}

void RaycastEngine::StopMIDI() {
	tsf_reset(g_TinySoundFont);

	tml_message* TinyMidiLoader = NULL;
	g_Msec = 0;
}




