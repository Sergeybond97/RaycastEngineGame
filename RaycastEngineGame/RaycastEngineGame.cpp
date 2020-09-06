
#define _CRT_SECURE_NO_WARNINGS

#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"


#include <regex>
#include <iostream>
#include <fstream>
#include <filesystem>

#include "irrKlang.h"

#pragma comment(lib, "irrKlang.lib") // link with irrKlang.dll


// MIDI =============================================================

#include "minisdl_audio.h"

#define TSF_IMPLEMENTATION
#include "tsf.h"

#define TML_IMPLEMENTATION
#include "tml.h"

// Holds the global instance pointer
static tsf* g_TinySoundFont;

// Holds global MIDI playback state
static float g_Msec;               //current playback time
static tml_message* g_MidiMessage;  //next message to be played


// Callback function called by the audio thread
static void AudioCallback(void* data, Uint8 *stream, int len)
{
	//Number of samples to process
	int SampleBlock, SampleCount = (len / (2 * sizeof(float))); //2 output channels
	for (SampleBlock = TSF_RENDER_EFFECTSAMPLEBLOCK; SampleCount; SampleCount -= SampleBlock, stream += (SampleBlock * (2 * sizeof(float))))
	{
		//We progress the MIDI playback and then process TSF_RENDER_EFFECTSAMPLEBLOCK samples at once
		if (SampleBlock > SampleCount) SampleBlock = SampleCount;

		//Loop through all MIDI messages which need to be played up until the current playback time
		for (g_Msec += SampleBlock * (1000.0 / 44100.0); g_MidiMessage && g_Msec >= g_MidiMessage->time; g_MidiMessage = g_MidiMessage->next)
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


void StartMIDIEngine() {

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


	tsf_set_volume(g_TinySoundFont, 0.2);


	// Start the actual audio playback here
	// The audio thread will begin to call our AudioCallback function
	SDL_PauseAudio(0);


	//Wait until the entire MIDI file has been played back (until the end of the linked message list is reached)
	//while (g_MidiMessage != NULL) SDL_Delay(100);

	// We could call tsf_close(g_TinySoundFont) and tml_free(TinyMidiLoader)
	// here to free the memory and resources but we just let the OS clean up
	// because the process ends here.

}


void RestartMIDI(char* fileName) {

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


void StopMIDI() {
	tsf_reset(g_TinySoundFont);

	tml_message* TinyMidiLoader = NULL;
	g_Msec = 0;
}

// WAV  =============================================================


irrklang::ISoundEngine* soundEngine;


void StartWAVEngine() {

	// start the sound engine with default parameters
	soundEngine = irrklang::createIrrKlangDevice();

	if (!soundEngine)
	{
		printf("SOUND : Could not startup FX sound engine\n");
	}

	soundEngine->setSoundVolume(0.2);

}


// =====================================================================







//===============================================================================================================================
// ENGINE 
//===============================================================================================================================



// Override base class with your custom functionality
class RaycastEngine : public olc::PixelGameEngine
{

public:

	RaycastEngine()
	{
		// Name you application
		sAppName = "RaycastEngine";
	}


	enum GameState { STATE_GAMEPLAY, STATE_MENU, STATE_EDITOR, STATE_TITLESCREEN, STATE_ENDGAME, STATE_LORESCREEN};
	GameState gameState;


	

	// Some internal stuff ---------------------------

	bool isMenuOpen = false;


	// Stuff for rendering

	bool* drawnPixels;
	//1D Zbuffer
	float* ZBuffer;



	// UI

	bool showCursor = false;

	std::string infoLog0 = "";
	std::string infoLog1 = "";
	std::string infoLog2 = "";
	float infoLogTimer0 = 0;
	float infoLogTimer1 = 0;
	float infoLogTimer2 = 0;

	float infoEffectTimer = 0;


	// Sound

	std::string midiFileName;

	// Controls

	float mouseSensitivity = 1.0;
	

	// Game

	bool isNewGameStarted = false;
	int bossDefeated = 0;



	//===============================================================================================================================
	// MAP
	//===============================================================================================================================



	uint32_t newId;




	std::string worldMapName;
	std::string worldMapFile;
	std::string worldMapNextMapFile;

	int worldMapWidth;
	int worldMapHeight;

	int worldMapFloorTxtr = 0;
	int worldMapCeilTxtr = 1;

	std::vector<int> worldMap;




	void LoadDefaultMap() {

		worldMapName = "DEFAULT";
		worldMapFile = "DEFAULT";
		worldMapNextMapFile = "";

		worldMapWidth = 20;
		worldMapHeight = 20;

		worldMapFloorTxtr = 0;
		worldMapCeilTxtr = 1;


		// Clear arrays
		enemiesArray.clear();
		decorationsArray.clear();
		itemsArray.clear();
		thingsArray.clear();
		interactbleWallsArray.clear();

		// Starting with id = 1
		newId = 1; // 0 is reserved !!!

		worldMap.clear();


		// First row (11111111111111111111)
		for (int i = 0; i < worldMapWidth; i++) {
			worldMap.push_back(1);
		}
		// Center (10000000000000001)
		for (int i = 0; i < worldMapWidth * worldMapHeight - 20; i++) {
			if (i % 20 == 0 || i % 19 == 0) {
				worldMap.push_back(1);
			}
			else {
				worldMap.push_back(0);
			}
		}
		// Last row (11111111111111111111)
		for (int i = 0; i < worldMapWidth; i++) {
			worldMap.push_back(1);
		}

		midiFileName = "NAILS";

		player.position = olc::vf2d(1.5, 1.5);

	}



	void LoadMap(std::string mapName) {
		
		std::cout << "Loading map : " << mapName << "  ....." << std::endl;


		// Clear arrays
		enemiesArray.clear();
		decorationsArray.clear();
		itemsArray.clear();
		thingsArray.clear();
		interactbleWallsArray.clear();


		// Starting with id = 1
		newId = 1; // 0 is reserved !!!


		std::string line;
		std::ifstream myfile(mapName);


		bool readMapName = false;
		std::string paramMapName = "";

		bool readMapWidth = false;
		int paramMapWidth = 0;

		bool readMapHeight = false;
		int paramMapHeight = 0;

		bool readMapFloorTxtr = false;
		int paramMapFloorTxtr = 0;

		bool readMapCeilTxtr = false;
		int paramMapCeilTxtr = 0;

		bool readNextMap = false;
		std::string paramNextMap = "";

		bool readMapMidi = false;
		std::string paramMapMidi = "";

		bool readingMapLayout = false;
		bool readMapLayout = false;
		std::vector<int> mapLayout;


		std::vector<Enemy> readEnemyArray;


		if (myfile.is_open())
		{
			while (std::getline(myfile, line))
			{

				// Check for parameters ---------------------------------

				if (std::regex_match(line, std::regex("(param:)(.*)"))) {
					std::string param;
					param = line.substr(6);
					//std::cout << "-> Param found : " << param << '\n';

					if (std::regex_match(param, std::regex("(name=\")(.*)(\")"))) {
						param = param.substr(6, param.length() - 7);
						//std::cout << "-> Name found : " << param << '\n';
						paramMapName = param;
						readMapName = true;
					}

					if (std::regex_match(param, std::regex("(width=\")(.*)(\")"))) {
						param = param.substr(7, param.length() - 8);
						//std::cout << "-> Width found : " << param << '\n';
						paramMapWidth = std::stoi(param);
						readMapWidth = true;
					}

					if (std::regex_match(param, std::regex("(height=\")(.*)(\")"))) {
						param = param.substr(8, param.length() - 9);
						//std::cout << "-> Height found : " << param << '\n';
						paramMapHeight = std::stoi(param);
						readMapHeight = true;
					}

					if (std::regex_match(param, std::regex("(nextmap=\")(.*)(\")"))) {
						param = param.substr(9, param.length() - 10);
						//std::cout << "-> Next map found : " << param << '\n';
						paramNextMap = param;
						readNextMap = true;
					}

					if (std::regex_match(param, std::regex("(floortxtr=\")(.*)(\")"))) {
						param = param.substr(11, param.length() - 12);
						//std::cout << "-> Floor texture found : " << param << '\n';
						paramMapFloorTxtr = std::stoi(param);
						readMapFloorTxtr = true;
					}

					if (std::regex_match(param, std::regex("(ceiltxtr=\")(.*)(\")"))) {
						param = param.substr(10, param.length() - 11);
						//std::cout << "-> Ceiling texture found : " << param << '\n';
						paramMapCeilTxtr = std::stoi(param);
						readMapCeilTxtr = true;
					}

					if (std::regex_match(param, std::regex("(midi=\")(.*)(\")"))) {
						param = param.substr(6, param.length() - 7);
						//std::cout << "-> Midi found : " << param << '\n';
						paramMapMidi = param;
						readMapMidi = true;
					}
				}

				// Check for labels ---------------------------------

				if (readMapName && readMapWidth && readMapHeight && readNextMap) {
					if (std::regex_match(line, std::regex("(label:)(.*)"))) {
						std::string label;
						label = line.substr(6);
						//std::cout << "-> Label found : " << label << '\n';

						if (std::regex_match(label, std::regex("(MAPSTART)"))) {
							readingMapLayout = true;
							continue;
							//std::cout << "-> MAPSTART found : " << '\n';
						}

						if (std::regex_match(label, std::regex("(MAPEND)"))) {
							readingMapLayout = false;
							readMapLayout = true;
							continue;
							//std::cout << "-> MAPSTART found" << '\n';
						}
					}
				}


				// Reading map ---------------------------------

				if (readingMapLayout) {
					//std::cout << "Map row : " << line << '\n';
					std::string number = "";
					for (int i = 0; i < line.length(); i++) {
						if (line[i] == ',') {
							if (number != "") {
								mapLayout.push_back(std::stoi(number));
								//std::cout << "Map ID : " << number << std::endl;
								number = "";
							}
						}
						else {
							number += line[i];
						}
					}
				}



				// Reading map objects ---------------------------------

				if (readMapLayout) {

					// Player ---------------------------------

					if (std::regex_match(line, std::regex("(player:)(.*)"))) {
						std::string playerLine;
						playerLine = line.substr(7);
						//std::cout << "-> Player found : " << playerLine << '\n';


						olc::vf2d readPosition{ -1, -1 };

						std::smatch match;


						// Position
						std::regex_search(playerLine.cbegin(), playerLine.cend(), match, std::regex("(position=\")(.*)(\":)"));
						std::string foundPosition = match.str();
						if (foundPosition != "")
						{
							foundPosition = foundPosition.substr(0, foundPosition.length() - 1); // remove ":" in the end
							//std::cout << "-> Position found : " << foundPosition << '\n';
							foundPosition = foundPosition.substr(10, foundPosition.length() - 11);
							//std::cout << "-> Position read : " << foundPosition << '\n';

							bool readingX = true;
							std::string posX = "";
							std::string posY = "";

							// Parcing floats
							for (int i = 0; i < foundPosition.length(); i++) {
								if (!readingX) {
									posY += foundPosition[i];
								}

								if (readingX && foundPosition[i] != ',') {
									posX += foundPosition[i];
								}
								else {
									readingX = false;
								}
							}
							//std::cout << "-> Position read X : " << posX << "  Y : " << posY << '\n';

							readPosition = olc::vf2d(std::stof(posX), std::stof(posY));

							//std::cout << "-> Position read V : " << readPosition << '\n';
						}
						else {
							std::cout << "-> ERROR : Player position reading fail" << std::endl;
						}


						if (readPosition != olc::vf2d(-1, -1)) {
							player.position = readPosition;
						}
						else {
							std::cout << "-> ERROR : Player reading fail" << std::endl;
							player.position = olc::vf2d(1.5, 1.5);
						}

					}


					// End of level ---------------------------------

					if (std::regex_match(line, std::regex("(endlevel:)(.*)"))) {
						std::string endLevelLine;
						endLevelLine = line.substr(9);
						//std::cout << "-> Level end found : " << endLevelLine << '\n';


						olc::vf2d readPosition{ -1, -1 };

						std::smatch match;


						// Position
						std::regex_search(endLevelLine.cbegin(), endLevelLine.cend(), match, std::regex("(position=\")(.*)(\":)"));
						std::string foundPosition = match.str();
						if (foundPosition != "")
						{
							foundPosition = foundPosition.substr(0, foundPosition.length() - 1); // remove ":" in the end
							//std::cout << "-> Position found : " << foundPosition << '\n';
							foundPosition = foundPosition.substr(10, foundPosition.length() - 11);
							//std::cout << "-> Position read : " << foundPosition << '\n';

							bool readingX = true;
							std::string posX = "";
							std::string posY = "";

							// Parcing floats
							for (int i = 0; i < foundPosition.length(); i++) {
								if (!readingX) {
									posY += foundPosition[i];
								}

								if (readingX && foundPosition[i] != ',') {
									posX += foundPosition[i];
								}
								else {
									readingX = false;
								}
							}
							//std::cout << "-> Position read X : " << posX << "  Y : " << posY << '\n';

							readPosition = olc::vf2d(std::stof(posX), std::stof(posY));

							//std::cout << "-> Position read V : " << readPosition << '\n';
						}
						else {
							std::cout << "-> ERROR : Level end position reading fail" << std::endl;
						}

						if (readPosition != olc::vf2d(-1, -1)) {
							InteractibleWall newWall = InteractibleWall(this, readPosition, InteractibleWall::InteractionType::ENDLEVEL);
							interactbleWallsArray.push_back(newWall);
						}
						else {
							std::cout << "-> ERROR : Level end reading fail" << std::endl;
						}

					}



					// Enemies ---------------------------------

					if (std::regex_match(line, std::regex("(enemy:)(.*)"))) {
						std::string enemyLine;
						enemyLine = line.substr(6);
						//std::cout << "-> Enemy found : " << enemyLine << '\n';


						int readType = -1;
						olc::vf2d readPosition{ -1, -1 };

						std::smatch match;



						// Type
						std::regex_search(enemyLine.cbegin(), enemyLine.cend(), match, std::regex("(type=\")([0-9]*)(\":)"));
						std::string foundType = match.str();
						if (foundType != "") {
							foundType = foundType.substr(0, foundType.length() - 1); // remove ":" in the end
							//std::cout << "-> Type found : " << foundType << '\n';
							readType = std::stoi(foundType.substr(6, foundType.length() - 7));
							//std::cout << "-> Type read : " << readType << '\n';
						} else {
							std::cout << "-> ERROR : Enemy type reading fail" << std::endl;
						}

						// Position
						std::regex_search(enemyLine.cbegin(), enemyLine.cend(), match, std::regex("(position=\")(.*)(\":)"));
						std::string foundPosition = match.str();
						if (foundPosition != "") 
						{
							foundPosition = foundPosition.substr(0, foundPosition.length() - 1); // remove ":" in the end
							//std::cout << "-> Position found : " << foundPosition << '\n';
							foundPosition = foundPosition.substr(10, foundPosition.length() - 11);
							//std::cout << "-> Position read : " << foundPosition << '\n';

							bool readingX = true;
							std::string posX = "";
							std::string posY = "";

							// Parcing floats
							for (int i = 0; i < foundPosition.length(); i++) {				
								if (!readingX) {
									posY += foundPosition[i];
								}

								if (readingX && foundPosition[i] != ',') {
									posX += foundPosition[i];
								}
								else {
									readingX = false;
								}
							}
							//std::cout << "-> Position read X : " << posX << "  Y : " << posY << '\n';

							readPosition = olc::vf2d(std::stof(posX), std::stof(posY));

							//std::cout << "-> Position read V : " << readPosition << '\n';
						} else {
							std::cout << "-> ERROR : Enemy position reading fail" << std::endl;
						}

						if (readType != -1 && readPosition != olc::vf2d(-1, -1)) {
							Enemy newEnemy = Enemy(this, newId, readType, readPosition);
							enemiesArray.push_back(newEnemy); newId++;
						} else {
							std::cout << "Enemy reading fail" << std::endl;
						}

					}




					// Decorations ---------------------------------

					if (std::regex_match(line, std::regex("(decoration:)(.*)"))) {
						std::string decorationLine;
						decorationLine = line.substr(11);
						//std::cout << "-> Decoration found : " << decorationLine << '\n';


						int readType = -1;
						olc::vf2d readPosition{ -1, -1 };

						std::smatch match;



						// Type
						std::regex_search(decorationLine.cbegin(), decorationLine.cend(), match, std::regex("(type=\")([0-9]*)(\":)"));
						std::string foundType = match.str();
						if (foundType != "") {
							foundType = foundType.substr(0, foundType.length() - 1); // remove ":" in the end
							//std::cout << "-> Type found : " << foundType << '\n';
							readType = std::stoi(foundType.substr(6, foundType.length() - 7));
							//std::cout << "-> Type read : " << readType << '\n';
						} else {
							//std::cout << "-> ERROR : Decoration type reading fail" << std::endl;
						}

						// Position
						std::regex_search(decorationLine.cbegin(), decorationLine.cend(), match, std::regex("(position=\")(.*)(\":)"));
						std::string foundPosition = match.str();
						if (foundPosition != "")
						{
							foundPosition = foundPosition.substr(0, foundPosition.length() - 1); // remove ":" in the end
							//std::cout << "-> Position found : " << foundPosition << '\n';
							foundPosition = foundPosition.substr(10, foundPosition.length() - 11);
							//std::cout << "-> Position read : " << foundPosition << '\n';

							bool readingX = true;
							std::string posX = "";
							std::string posY = "";

							// Parcing floats
							for (int i = 0; i < foundPosition.length(); i++) {
								if (!readingX) {
									posY += foundPosition[i];
								}

								if (readingX && foundPosition[i] != ',') {
									posX += foundPosition[i];
								}
								else {
									readingX = false;
								}
							}
							//std::cout << "-> Position read X : " << posX << "  Y : " << posY << '\n';

							readPosition = olc::vf2d(std::stof(posX), std::stof(posY));

							//std::cout << "-> Position read V : " << readPosition << '\n';
						} else {
							std::cout << "-> ERROR : Decoration position reading fail" << std::endl;
						}


						if (readType != -1 && readPosition != olc::vf2d(-1, -1)) {
							Decoration newDecoration = Decoration(this, newId, readType, readPosition);
							decorationsArray.push_back(newDecoration); newId++;
						} else {
							std::cout << "-> ERROR : Decoration reading fail" << std::endl;
						}
					}





					// Items ---------------------------------

					if (std::regex_match(line, std::regex("(item:)(.*)"))) {
						std::string itemLine;
						itemLine = line.substr(5);
						//std::cout << "-> Item found : " << itemLine << '\n';


						int readType = -1;
						olc::vf2d readPosition{ -1, -1 };

						std::smatch match;



						// Type
						std::regex_search(itemLine.cbegin(), itemLine.cend(), match, std::regex("(type=\")([0-9]*)(\":)"));
						std::string foundType = match.str();
						if (foundType != "") {
							foundType = foundType.substr(0, foundType.length() - 1); // remove ":" in the end
							//std::cout << "-> Type found : " << foundType << '\n';
							readType = std::stoi(foundType.substr(6, foundType.length() - 7));
							//std::cout << "-> Type read : " << readType << '\n';
						}
						else {
							std::cout << "-> ERROR : Item type reading fail" << std::endl;
						}

						// Position
						std::regex_search(itemLine.cbegin(), itemLine.cend(), match, std::regex("(position=\")(.*)(\":)"));
						std::string foundPosition = match.str();
						if (foundPosition != "")
						{
							foundPosition = foundPosition.substr(0, foundPosition.length() - 1); // remove ":" in the end
							//std::cout << "-> Position found : " << foundPosition << '\n';
							foundPosition = foundPosition.substr(10, foundPosition.length() - 11);
							//std::cout << "-> Position read : " << foundPosition << '\n';

							bool readingX = true;
							std::string posX = "";
							std::string posY = "";

							// Parcing floats
							for (int i = 0; i < foundPosition.length(); i++) {
								if (!readingX) {
									posY += foundPosition[i];
								}

								if (readingX && foundPosition[i] != ',') {
									posX += foundPosition[i];
								}
								else {
									readingX = false;
								}
							}
							//std::cout << "-> Position read X : " << posX << "  Y : " << posY << '\n';

							readPosition = olc::vf2d(std::stof(posX), std::stof(posY));

							//std::cout << "-> Position read V : " << readPosition << '\n';
						}
						else {
							std::cout << "-> ERROR : Item position reading fail" << std::endl;
						}


						if (readType != -1 && readPosition != olc::vf2d(-1, -1)) {
							Item newItem = Item(this, newId, readType, readPosition);
							itemsArray.push_back(newItem); newId++;
						}
						else {
							std::cout << "-> ERROR : Item reading fail" << std::endl;
						}
					}





					thingsArray.clear();

					// Create things array -----------------------------

					// Add decorations
					for (int i = 0; i < decorationsArray.size(); i++) {
						thingsArray.push_back(decorationsArray[i].ToThing());
					}
					// Add items
					for (int i = 0; i < itemsArray.size(); i++) {
						thingsArray.push_back(itemsArray[i].ToThing());
					}
					// Add enemies
					for (int i = 0; i < enemiesArray.size(); i++) {
						thingsArray.push_back(enemiesArray[i].ToThing());
					}


					spriteOrder = new int[thingsArray.size()];
					spriteDistance = new float[thingsArray.size()];


				}


			}
			myfile.close();

			if (readMapLayout) {
				// Apply parameters
				worldMap = mapLayout;
				worldMapFile = mapName.substr(5, mapName.length() - 9);
				worldMapWidth = paramMapWidth;
				worldMapHeight = paramMapHeight;
				worldMapName = paramMapName;
				worldMapNextMapFile = paramNextMap;
				worldMapFloorTxtr = paramMapFloorTxtr;
				worldMapCeilTxtr = paramMapCeilTxtr;

				midiFileName = paramMapMidi;


				std::cout << "=====================================" << std::endl;
				std::cout << "Map : " << worldMapName << std::endl;
				std::cout << "Width : " << worldMapWidth << "   Height : " << worldMapHeight << std::endl;
				std::cout << "=====================================" << std::endl;
			}
			else {
				std::cout << "MAP READ ERROR" << std::endl;
			}
			


		}
		else std::cout << "Unable to open file" << std::endl;
	}


	void SaveMap(std::string mapName) {

		std::ofstream myfile(mapName);
		if (myfile.is_open())
		{
			myfile << "param:name=\"" << worldMapName << "\"" << std::endl;
			myfile << "param:width=\"" << std::to_string(worldMapWidth) << "\"" << std::endl;
			myfile << "param:height=\"" << std::to_string(worldMapHeight) << "\"" << std::endl;
			myfile << "param:nextmap=\"" << worldMapNextMapFile << "\"" << std::endl;
			myfile << "param:floortxtr=\"" << worldMapFloorTxtr << "\"" << std::endl;
			myfile << "param:ceiltxtr=\"" << worldMapCeilTxtr << "\"" << std::endl;
			myfile << "param:midi=\"" << midiFileName << "\"" << std::endl;

			myfile << "label:MAPSTART" << std::endl;

			for (int i = 0; i < worldMapHeight; i++) {
				for (int j = 0; j < worldMapWidth; j++) {
					myfile << worldMap[i * worldMapWidth + j] << ",";
				}
				myfile << std::endl;
			}

			myfile << "label:MAPEND" << std::endl;

			myfile << "player:position=\"" << player.position.x << "," << player.position.y << "\":" << std::endl;



			for (int intWall = 0; intWall < interactbleWallsArray.size(); intWall++) {
				if (interactbleWallsArray[intWall].interactionType == InteractibleWall::InteractionType::ENDLEVEL) {
					myfile << "endlevel:position=\"" << interactbleWallsArray[intWall].wallPosition.x << "," << interactbleWallsArray[intWall].wallPosition.y << "\":" << std::endl;
				}
			}

			for (int i = 0; i < enemiesArray.size(); i++) {
				myfile << "enemy:type=\"" << enemiesArray[i].texture << "\":position=\"" << enemiesArray[i].position.x << "," << enemiesArray[i].position.y << "\":" << std::endl;
			}

			for (int i = 0; i < decorationsArray.size(); i++) {
				myfile << "decoration:type=\"" << decorationsArray[i].texture << "\":position=\"" << decorationsArray[i].position.x << "," << decorationsArray[i].position.y << "\":" << std::endl;
			}

			for (int i = 0; i < itemsArray.size(); i++) {
				myfile << "item:type=\"" << itemsArray[i].texture << "\":position=\"" << itemsArray[i].position.x << "," << itemsArray[i].position.y << "\":" << std::endl;
			}


			myfile.close();

			std::cout << "Map \"" << worldMapName << "\" saved : " << mapName << std::endl;
		}
		else std::cout << "Unable to open file";

	}



	void ChangeMapSize(int w, int h) {

		std::vector<int> newWorldMap;

		for (int y = 0; y < h; y++) {
			for (int x = 0; x < w; x++) {

				
				if (x < worldMapWidth && y < worldMapHeight) {
					//std::cout << worldMap[y * worldMapWidth + x];
					newWorldMap.push_back(worldMap[y*worldMapWidth + x]);
				}
				else {
					std::cout << 0;
					newWorldMap.push_back(0);
				}

			}
			//std::cout << std::endl;
		}
		


		worldMap = newWorldMap;

		worldMapWidth = w;
		worldMapHeight = h;
	}




	//===============================================================================================================================
	// GAME OBJECTS
	//===============================================================================================================================


	struct Weapon {
		float damage;
		float fireRate;

		bool onPlayer;

		int currentAmmo;
		int maxAmmo;
	};

	Weapon weapons[3] = {
		// Pistol
		{10, 0.4, true, 40, 400},
		// Rifle
		{5.0, 0.1, true, 40, 400},
		// Shotgun
		{5.0, 1.2, true, 50, 50},
	};





	class Thing
	{

	public:

		RaycastEngine* engineReference;

		uint32_t id;

		// Sprite info
		int texture = 0;
		int spritePartIndex = 0;
		bool enableRender = true;

		// Position
		olc::vf2d position = olc::vf2d{ 10.0, 10.0 };

		// Collision
		bool enableCollision = true;
		float collisionSize = 0.5;

		int thingType = 0; // 0 - decoration, 1 - enemy, 2 - item


		Thing(RaycastEngine* engine) {
			engineReference = engine;
			id = 0;
			thingType = 0;
			texture = 0;
			spritePartIndex = 0;
			position = olc::vf2d{ 10.0, 10.0 };
			enableCollision = true;
			collisionSize = 0.5;
		}
		Thing(RaycastEngine* engine, uint32_t globId, int type, int tex, olc::vf2d pos) {
			engineReference = engine;
			id = globId;
			thingType = type;
			texture = tex;
			spritePartIndex = 0;
			position = pos;
			enableCollision = true;
			collisionSize = 0.5;
		}
		bool operator == (const Thing& rhs) const { return (this->id == rhs.id && this->position == rhs.position && this->texture == rhs.texture); }
		bool operator != (Thing& rhs) const { return (this->id != rhs.id || this->position != rhs.position || this->texture != rhs.texture); }


		bool CheckRayCollision(olc::vf2d start, olc::vf2d direction, float length) {
			if (enableCollision) {
				olc::vf2d spriteColPnt1 = olc::vf2d(position.x, position.y) + direction.perp() * 0.5 * collisionSize;
				olc::vf2d spriteColPnt2 = olc::vf2d(position.x, position.y) - direction.perp() * 0.5 * collisionSize;

				bool interc = engineReference->LineIntersection
				(
					start,
					start + direction * length,
					spriteColPnt1,
					spriteColPnt2
				);
				return interc;
			}
			else {
				return false;
			}
		}
	};


	class Enemy : public Thing {

	public:
		Enemy(RaycastEngine* engine) : Thing(engine) {
			//std::cout << "Created Enemy | ID : " << id << std::endl;
		};
		Enemy(RaycastEngine* engine, uint32_t globId, int tex, olc::vf2d pos) : Thing(engine, globId, 1 , tex, pos) {
			//std::cout << "Created Enemy, param | ID : " << id << std::endl;

			if (tex == 0) {
				health = 70;
				damage = 5;
				moveSpeed = 5;
				collisionSize = 0.8;
			}
			else if (tex == 1) {
				health = 200;
				damage = 10;
				moveSpeed = 3;
				collisionSize = 0.8;
			}
			else if (tex == 2) {
				health = 35;
				damage = 2;
				moveSpeed = 12;
				collisionSize = 0.8;
			}
			else if (tex == 3) {
				health = 50;
				damage = 0;
				moveSpeed = 0;
				collisionSize = 1.0;
			}

		}


		Thing ToThing() {
			Thing thing = Thing(engineReference, id, thingType, texture, position);
			thing.spritePartIndex = spritePartIndex;
			thing.enableCollision = enableCollision;
			thing.enableRender = enableRender;
			thing.collisionSize = collisionSize;
			return thing;
		}


		int enemyState = 0; // 0 - idle0,    1 - idle1,    2 - atack,    3 - dying0,    4 - dying1,    5 - dying3
		float spriteTimer;
		
		int enemyAIstate = 0; // 0 - calm,   1 - folowing,    2 - Running back
		float AItimer;


		int health = 100;
		bool isAlive = true;
		bool isUpdating = true;

		int damage = 5;


		olc::vf2d velocity{ 0,0 };
		olc::vf2d moveVector{ 1, 1 };
		olc::vf2d randMoveVector{ 1, 1 };
		float moveSpeed = 5;




		void Update(float fElapsedTime) {

			// Update things info ----------------------------------

			if (isUpdating) {
				engineReference->UpdateThings();
			}


			// Sprite states -------------------------------------

			spriteTimer -= fElapsedTime;

			// Change idle states

			if (spriteTimer <= 0 && enemyState == 0) {
				enemyState = 1;
				spritePartIndex = enemyState; // Change sprite part to render
				spriteTimer = 0.5;
			}

			if (spriteTimer <= 0 && enemyState == 1) {
				enemyState = 0;
				spritePartIndex = enemyState; // Change sprite part to render
				spriteTimer = 0.5;
			}

			// Shooting state

			if (spriteTimer <= 0 && enemyState == 2) {
				enemyState = 0;
				spritePartIndex = enemyState; // Change sprite part to render
				spriteTimer = 0.2;
			}


			// Change dying states

			if (spriteTimer <= 0 && enemyState == 3) {
				enemyState = 4;
				spritePartIndex = enemyState; // Change sprite part to render
				spriteTimer = 0.3;
			}

			if (spriteTimer <= 0 && enemyState == 4) {
				enemyState = 5;
				spritePartIndex = enemyState; // Change sprite part to render
				engineReference->UpdateThings();
				isUpdating = false;
			}



			// AI -------------------------------------------

			if (isAlive) {

				AItimer -= fElapsedTime;
				//std::cout << AItimer << std::endl;

				olc::vf2d playerRelativePos = olc::vf2d(position - engineReference->player.position) * (-1);
				bool playerInSight = false;

				// Calm state
				if (enemyAIstate == 0) {
					moveVector = olc::vf2d(0, 0);

					// Search for player
					if (AItimer <= 0) {
						playerInSight = ShootRay(playerRelativePos.norm());
						AItimer = 0.5;
					}

					if (playerInSight && engineReference->player.isAlive) {
						//std::cout << "Player is found" << std::endl;
						enemyAIstate = 1;
						AItimer = 1.0;
					}
				}

				// Angry state
				if (enemyAIstate == 1) {
					moveVector = playerRelativePos.norm();


					if (AItimer <= 0) {

						//std::cout << "Try to shoot" << std::endl;

						int randAction = rand() % 1000;
						// Shoot
						if (randAction > 500) {
							playerInSight = ShootRay(playerRelativePos.norm());
							AItimer = 1.0;
						}
						if (randAction > 800) {
							enemyAIstate = 2;
							randMoveVector = olc::vf2d(1.0 - rand() % 2000 / 1000.0, 1.0 - rand() % 2000 / 1000.0);
							AItimer = 1.0;
						}


						if (playerInSight) {
							//std::cout << "Shoot" << std::endl;
							enemyState = 2;
							spritePartIndex = enemyState; // Change sprite part to render
							spriteTimer = 0.2;
							if (texture != 3) { // Boss dont make damege
								engineReference->player.TakeDamage(damage + (rand() % 4 - 2));
							}
						}

					}


				}

				// Running state
				if (enemyAIstate == 2) {
					moveVector = randMoveVector;

					if (AItimer <= 0) {
						enemyAIstate = 0;
						AItimer = 1.0;
					}
				}




				if (!engineReference->player.isAlive) {
					enemyAIstate = 0;
				}





				// Apply motion ----------------------------------

				velocity += moveSpeed * 0.001 * moveVector * fElapsedTime;

				// Clamp speed
				if (velocity.mag() > 0.006) {
					velocity -= moveSpeed * 0.001 * moveVector * fElapsedTime;
				}


				if (engineReference->worldMap[int(position.y) * engineReference->worldMapWidth + int(position.x + (velocity.x * fElapsedTime * 600) + (velocity.norm().x * 0.5))] == false)
					position.x += velocity.x * fElapsedTime * 600;

				if (engineReference->worldMap[int(position.y + (velocity.y * fElapsedTime * 600) + (velocity.norm().y * 0.5)) * engineReference->worldMapWidth + int(position.x)] == false)
					position.y += velocity.y * fElapsedTime * 600;


				// Apply friction ----------------------------------

				velocity = velocity * (1.0 - 0.1 * (fElapsedTime * 1000 / 33));

			}



		}


		void TakeDamage(int amount) {
			health -= amount;
			//std::cout << "Damage, health remains : " << health << std::endl;

			spriteTimer = 0.1;
			if (rand() % 2 == 1) {
				spritePartIndex = 6;
			}
			else {
				spritePartIndex = 7;
			}


			if (health <= 0) {
				isAlive = false;
				health = 0;
				enableCollision = false;

				enemyState = 3;
				spriteTimer = 0.3;
				spritePartIndex = enemyState;

				//engineReference->isNewGameStarted
				if (texture == 3) { // Boss
					engineReference->bossDefeated++;
				}

				engineReference->UpdateThings();
			}
		}


		bool ShootRay(olc::vf2d rayDir) {

			float rayDistance = engineReference->ShootRaycastRay(position, rayDir);
			//std::cout << "RD : " << rayDistance << std::endl;

			bool collide = false;
			for (int i = 0; i < engineReference->thingsArray.size(); i++) {

				Thing* spriteToTest = &engineReference->thingsArray[engineReference->spriteOrder[engineReference->thingsArray.size() - 1 - i]];

				collide = spriteToTest->CheckRayCollision(position, rayDir, rayDistance);
				if (collide)
				{
					//std::cout << "Hit : " << spriteToTest->position.x << "  " << spriteToTest->position.y << "   " << spriteToTest->texture << std::endl;
					break;
				}
			}

			if (!collide && (position - engineReference->player.position).mag() < rayDistance) {
				//std::cout << "HIT PLAYER : " << (position - engineReference->player.position).mag() << std::endl;
				return true;
			}
			else {
				return false;
			}

		}





	};


	class Item : public Thing {

	public:

		Item(RaycastEngine* engine) : Thing(engine) {
			//std::cout << "Created Item | ID : " << id << std::endl;
		};
		Item(RaycastEngine* engine, uint32_t globId, int tex, olc::vf2d pos) : Thing(engine, globId, 2 ,tex, pos) {
			//std::cout << "Created Item, param  | ID : " << id << std::endl;

			enableCollision = false;
		}


		Thing ToThing() {
			Thing thing = Thing(engineReference, id, thingType, texture, position);
			thing.spritePartIndex = spritePartIndex;
			thing.enableCollision = enableCollision;
			thing.enableRender = enableRender;
			thing.collisionSize = collisionSize;
			return thing;
		}


		bool isPickedup = false;


		void Update(float fElapsedTime) {

			engineReference -> UpdateThings();
			CheckForPickup();

		}


		void CheckForPickup() {
			if (!isPickedup) {
				float distanceToItem = (engineReference->player.position - position).mag2();
				if (distanceToItem < 0.3) {


					if (texture == 0) {
						if (engineReference->player.health != 100) {
							engineReference->player.health += 20;
							engineReference->InfoLog("Picked up Medkit");
							enableRender = false;
							isPickedup = true;
							std::cout << "Item picked up | ID : " << id << std::endl;
							soundEngine->play2D("sounds/pickup.wav", false);

						}
						if (engineReference->player.health > 100) {
							engineReference->player.health = 100;
						}
					}

					if (texture == 1) {
						if (engineReference->weapons[0].currentAmmo != engineReference->weapons[0].maxAmmo) {
							engineReference->weapons[0].currentAmmo += 20;
							engineReference->weapons[1].currentAmmo += 20;
							engineReference->InfoLog("Picked up 9mm ammo");
							enableRender = false;
							isPickedup = true;
							std::cout << "Item picked up | ID : " << id << std::endl;
							soundEngine->play2D("sounds/pickup.wav", false);
						}
						if (engineReference->weapons[0].currentAmmo > engineReference->weapons[0].maxAmmo || engineReference->weapons[1].currentAmmo > engineReference->weapons[1].maxAmmo) {
							engineReference->weapons[0].currentAmmo = engineReference->weapons[0].maxAmmo;
							engineReference->weapons[1].currentAmmo = engineReference->weapons[1].maxAmmo;
						}
					}

					if (texture == 2) {
						if (engineReference->weapons[2].currentAmmo != engineReference->weapons[2].maxAmmo) {
							engineReference->weapons[2].currentAmmo += 8;
							engineReference->InfoLog("Picked up shells");
							enableRender = false;
							isPickedup = true;
							std::cout << "Item picked up | ID : " << id << std::endl;
							soundEngine->play2D("sounds/pickup.wav", false);

						}
						if (engineReference->weapons[2].currentAmmo > engineReference->weapons[2].maxAmmo) {
							engineReference->weapons[2].currentAmmo = engineReference->weapons[2].maxAmmo;
						}
					}

					if (texture == 3) { // Uzi

						bool changeWeaponAfterPickup = false;
						if (engineReference->weapons[1].onPlayer == false) {
							changeWeaponAfterPickup = true;
						}

						engineReference->weapons[1].onPlayer = true;
						engineReference->InfoLog("Picked up Uzi!");
						enableRender = false;
						isPickedup = true;
						std::cout << "Item picked up | ID : " << id << std::endl;
						soundEngine->play2D("sounds/pickup.wav", false);

						if (changeWeaponAfterPickup) {
							engineReference->player.ChangeWeapon(1);
						}

						if (engineReference->weapons[0].currentAmmo != engineReference->weapons[0].maxAmmo) {
							engineReference->weapons[0].currentAmmo += 20;
							engineReference->weapons[1].currentAmmo += 20;
						}
						if (engineReference->weapons[0].currentAmmo > engineReference->weapons[0].maxAmmo || engineReference->weapons[1].currentAmmo > engineReference->weapons[1].maxAmmo) {
							engineReference->weapons[0].currentAmmo = engineReference->weapons[0].maxAmmo;
							engineReference->weapons[1].currentAmmo = engineReference->weapons[1].maxAmmo;
						}
					}

					if (texture == 4) { // Shotgun

						bool changeWeaponAfterPickup = false;
						if (engineReference->weapons[2].onPlayer == false) {
							changeWeaponAfterPickup = true;
						}

						engineReference->weapons[2].onPlayer = true;
						engineReference->InfoLog("Picked up Shotgun!");
						enableRender = false;
						isPickedup = true;
						soundEngine->play2D("sounds/pickup.wav", false);

						if (changeWeaponAfterPickup) {
							engineReference->player.ChangeWeapon(2);
						}

						if (engineReference->weapons[2].currentAmmo != engineReference->weapons[2].maxAmmo) {
							engineReference->weapons[2].currentAmmo += 8;				
							std::cout << "Item picked up | ID : " << id << std::endl;
						}
						if (engineReference->weapons[2].currentAmmo > engineReference->weapons[2].maxAmmo) {
							engineReference->weapons[2].currentAmmo = engineReference->weapons[2].maxAmmo;
						}
					}

				}
			}
		}

		


	};


	class Decoration : public Thing {

	public:
		Decoration(RaycastEngine* engine) : Thing(engine) {
			//std::cout << "Created Decoration | ID : " << id << std::endl;
		};
		Decoration(RaycastEngine* engine, uint32_t globId, int tex, olc::vf2d pos) : Thing(engine, globId, 0, tex, pos) {
			//std::cout << "Created Decoration, param | ID : " << id << std::endl;

			// Disable collision for lamps
			if (tex == 2) { 
				enableCollision = false;
			}

			// Disable collision for exit sign
			if (tex == 3) {
				enableCollision = false;
			}
		}

		Thing ToThing() {
			Thing thing = Thing(engineReference, id, thingType, texture, position);
			thing.spritePartIndex = spritePartIndex;
			thing.enableCollision = enableCollision;
			thing.enableRender = enableRender;
			thing.collisionSize = collisionSize;
			return thing;
		}

	};


	Enemy* GetEnemyByID(uint32_t id) {
		for (int i = 0; i < enemiesArray.size(); i++) {
			if (enemiesArray[i].id == id) {
				return &enemiesArray[i];
			}
		}
		return nullptr;
	}





	class Player
	{

	public:

		RaycastEngine* engineReference;

		// Position
		olc::vf2d position = olc::vf2d{ 0, 0 };
		olc::vf2d direction = olc::vf2d{ -1, 0 };

		olc::vf2d viewPlane = olc::vf2d{ 0, -0.75 }; //the 2d raycaster version of camera plane
		float cameraVertical = 0;


		// Collision
		bool enableCollision = true;
		float collisionSize = 0.5;


		// Movement
		float moveSpeed = 5.0;
		float rotationSpeed = 3.0;

		olc::vf2d playerVelocity = { 0, 0 };

		bool isMoving = false;
		bool isShooting = false;
		olc::vf2d controlMoveVector = { 0, 0 };
		olc::vf2d controlRotationVector = { 0 ,0 };

		// Stats
		int health = 100;
		bool isAlive = true;
		float damageEffectTimer = 0;

		// HUD and weapons
		float bobValue;
		int activeWeapon = 0;
		float weaponDelay = 0;
		int weaponState; // for changing sprites ( 0 - idle, 1 - shoot, 2 - back to idle )

		float timeToChangeWeapon = 0.6;
		float changeWeaponTimer = 0;
		int weaponToChange = 0;


		void Update(float fElapsedTime) {

			if (isAlive) {

				Move(fElapsedTime);
				WeaponChanging(fElapsedTime);

				// Bobbing
				if (changeWeaponTimer == 0) {
					bobValue += 1000.0f * playerVelocity.mag() * fElapsedTime;
				}

				// Weapon states
				if (weaponDelay > 0) {
					weaponDelay -= fElapsedTime;

					float weaponStateTime = engineReference->weapons[activeWeapon].fireRate / 3;

					if (weaponDelay < weaponStateTime * 2) {
						weaponState = 2;
					}

					if (weaponState == 2 && weaponDelay < weaponStateTime) {
						weaponState = 0;
					}

					if (weaponDelay <= 0) {
						weaponDelay = 0;
					}
				}

			}
			else {

				if (cameraVertical > -20) {
					cameraVertical -= 20 * fElapsedTime;
				}


			}

		}


		
		void WeaponChanging(float fElapsedTime) {
			if (changeWeaponTimer > 0) {
				changeWeaponTimer -= fElapsedTime;
			}

			if (changeWeaponTimer < timeToChangeWeapon/2) {
				activeWeapon = weaponToChange;
			}

			if (changeWeaponTimer < 0) {
				changeWeaponTimer = 0;
			}
		}

		void ChangeWeapon(int weapon) {
			if (changeWeaponTimer == 0 && activeWeapon != weapon && engineReference->weapons[weapon].onPlayer == true) {
				changeWeaponTimer = timeToChangeWeapon;
				weaponState = 0;
				weaponDelay = 0;
				weaponToChange = weapon;
			}
		}




		void Move(float fElapsedTime) {

			// Apply motion ----------------------------------

			playerVelocity += moveSpeed * 4 * 0.001 * controlMoveVector * fElapsedTime;
			// Clamp speed
			if (playerVelocity.mag() > 0.006) {
				playerVelocity -= moveSpeed * 4 * 0.001 * controlMoveVector * fElapsedTime;
			}


			if (engineReference->worldMap[int(position.y) * engineReference->worldMapWidth + int(position.x + (playerVelocity.x * fElapsedTime * 600) + (playerVelocity.norm().x * 0.1))] == false)
				position.x += playerVelocity.x * fElapsedTime * 600;

			if (engineReference->worldMap[int(position.y + (playerVelocity.y * fElapsedTime * 600) + (playerVelocity.norm().y * 0.1)) * engineReference->worldMapWidth + int(position.x)] == false)
				position.y += playerVelocity.y * fElapsedTime * 600;


			// Apply friction ----------------------------------

			playerVelocity = playerVelocity * (1.0 - 0.1 * (fElapsedTime * 1000 / 33));


			// Apply rotation ----------------------------------

			//both camera direction and camera plane must be rotated
			float rotation = fElapsedTime * controlRotationVector.x * rotationSpeed;

			float oldDirX = direction.x;
			direction.x = direction.x * cos(rotation) - direction.y * sin(rotation);
			direction.y = oldDirX * sin(rotation) + direction.y * cos(rotation);
			float oldPlaneX = viewPlane.x;
			viewPlane.x = viewPlane.x * cos(rotation) - viewPlane.y * sin(rotation);
			viewPlane.y = oldPlaneX * sin(rotation) + viewPlane.y * cos(rotation);
		}

		void Shoot() {

			if (weaponState == 0 && weaponDelay == 0 && changeWeaponTimer == 0) { // If weapon is idle

				if (engineReference->weapons[activeWeapon].currentAmmo > 0) { // If player have ammo

				
					// Change weapon state
					weaponState = 1; // Shooting
					weaponDelay = engineReference->weapons[activeWeapon].fireRate;

					// Shooting

					engineReference->weapons[activeWeapon].currentAmmo--;

					// Weapon 0 and 1 (pistol and uzi) have same ammo, so i equal them
					if (activeWeapon == 0) engineReference->weapons[1].currentAmmo = engineReference->weapons[activeWeapon].currentAmmo;
					if (activeWeapon == 1) engineReference->weapons[0].currentAmmo = engineReference->weapons[activeWeapon].currentAmmo;


					if (activeWeapon == 0) { // Pistol and Uzi
						soundEngine->play2D("sounds/pistol.wav", false);
						ShootRay(0);
					}
					if (activeWeapon == 1) { // Shotgun
						soundEngine->play2D("sounds/uzi.wav", false);
						ShootRay(0);
					}
					if (activeWeapon == 2) { // Shotgun
						soundEngine->play2D("sounds/shotgun.wav", false);
						for (int s = 0; s < 20; s++)
							ShootRay(-0.75 + (s / 19.0) * 1.5); // from -0.5 to  0.5
					}

				}


			}
		}

		void ShootRay(float angle) {

			//std::cout << "Shoot vector : " << direction + viewPlane * angle << std::endl;

			float rayDistance = engineReference->ShootRaycastRay(position, direction + viewPlane * angle);
			//engineReference->aimDist = std::to_string(rayDistance);

			for (int i = 0; i < engineReference->thingsArray.size(); i++) {

				Thing* spriteToTest = &engineReference->thingsArray[engineReference->spriteOrder[engineReference->thingsArray.size() - 1 - i]];

				bool collide = spriteToTest->CheckRayCollision(position, direction, rayDistance);
				if (collide)
				{
					//std::cout << "Hit : " << spriteToTest->position.x << "  " << spriteToTest->position.y << "   " << spriteToTest->texture << std::endl;
					Enemy* hittedEnemy = engineReference->GetEnemyByID(spriteToTest->id);
					if (hittedEnemy != nullptr) {
						//std::cout << "Hited enemy found | ID:  " << hittedEnemy->id << std::endl;
						hittedEnemy->TakeDamage(engineReference->weapons[activeWeapon].damage);
						hittedEnemy = nullptr;
					}
					break;
				}
			}
		}

		void Interact() {

			for (int i = 0; i < engineReference->interactbleWallsArray.size(); i++) {

				olc::vi2d interactionVector = position + direction * 0.5;
				std::cout << "Interaction : " << interactionVector << std::endl;

				if (interactionVector == engineReference->interactbleWallsArray[i].wallPosition) {
					std::cout << "Activated wall : " << engineReference->interactbleWallsArray[i].wallPosition << std::endl;
					engineReference->interactbleWallsArray[i].isActive = true;
					engineReference->interactbleWallsArray[i].Interact();
				}
				
			}
		}

		void TakeDamage(int amount) {
			damageEffectTimer += 0.1;

			health -= amount;

			if (health <= 0) {
				health = 0;
				isAlive = false;
			}
		}

		void Ressurect() {
			cameraVertical = 0;
			health = 100;
			isAlive = true;

			engineReference->weapons[0].currentAmmo = 30;
			engineReference->weapons[1].currentAmmo = 30;
			engineReference->weapons[2].currentAmmo = 0;

			engineReference->weapons[1].onPlayer = false;
			engineReference->weapons[2].onPlayer = false;

			activeWeapon = 0;
		}

	};




	class InteractibleWall {
	
	public:

		RaycastEngine* engineReference;

		olc::vi2d wallPosition;
		bool isActive;

		enum InteractionType { NONE, ENDLEVEL, OPENWALLS };
		InteractionType interactionType;

		std::vector<int> wallsToOpen;


		InteractibleWall(RaycastEngine* engine) {
			engineReference = engine;
			wallPosition = olc::vi2d(-1, -1);
			isActive = false;
			interactionType = InteractionType::NONE;
		}

		InteractibleWall(RaycastEngine* engine, olc::vi2d wallPos, InteractionType type) {
			engineReference = engine;
			wallPosition = wallPos;
			isActive = false;
			interactionType = type;
		}




		void Interact() {

			if (interactionType == InteractionType::NONE)
			{
				return;
			}
			else if (interactionType == InteractionType::ENDLEVEL)
			{
				std::cout << "LEVEL END" << std::endl;
				if (engineReference->worldMapNextMapFile != "") {
					std::string mapFile = "maps/" + engineReference->worldMapNextMapFile + ".map";
					engineReference->LoadMap(mapFile);
					engineReference->gameState = GameState::STATE_GAMEPLAY;
					engineReference->PlayMapMIDI();
				}
			}
			else if (interactionType == InteractionType::OPENWALLS)
			{
				return;
			}
		}
		

	};



	// World objects ------------------------

	Player player;


	std::vector<Thing> thingsArray;

	std::vector<Enemy> enemiesArray;

	std::vector<Decoration> decorationsArray;

	std::vector<Item> itemsArray;


	std::vector<InteractibleWall> interactbleWallsArray;



	void UpdateThings() {

		for (int i = 0; i < thingsArray.size(); i++) {
			
			for (int d = 0; d < decorationsArray.size(); d++) {
				if (thingsArray[i].id == decorationsArray[d].id) {
					thingsArray[i] = decorationsArray[d].ToThing();
					break;
				}
			}

			for (int d = 0; d < itemsArray.size(); d++) {
				if (thingsArray[i].id == itemsArray[d].id) {
					thingsArray[i] = itemsArray[d].ToThing();
					break;
				}
			}

			for (int d = 0; d < enemiesArray.size(); d++) {
				if (thingsArray[i].id == enemiesArray[d].id) {
					thingsArray[i] = enemiesArray[d].ToThing();
					break;
				}
			}
		}

		// ---- Old update (I dont know if it's will be working now) ----
		/*
		// Add decorations
		for (int i = 0; i < decorationsArray.size(); i++) {
			thingsArray[i] = decorationsArray[i].ToThing();
		}
		
		// Add items
		for (int i = decorationsArray.size(); i < decorationsArray.size() + itemsArray.size(); i++) {
			thingsArray[i] = itemsArray[i - decorationsArray.size()].ToThing();
		}
		
		// Add enemies
		for (int i = decorationsArray.size() + itemsArray.size(); i < decorationsArray.size() + itemsArray.size() + enemiesArray.size(); i++) {
			thingsArray[i] = enemiesArray[i - (decorationsArray.size() + itemsArray.size())].ToThing();
		}
		*/

	}







	//===============================================================================================================================
	// ASSETS
	//===============================================================================================================================

	// Assets ---------------------------------------

	// UI
	olc::Sprite spriteCursor;

	// Weapon in HUD sprites
	olc::Sprite weaponPistol;
	olc::Sprite weaponUzi;
	olc::Sprite weaponShotgun;

	// Walls
	olc::Sprite spriteWallAtlas;

	std::vector<olc::Sprite*> wallSprites;



	// Decorations
	olc::Sprite spriteBarell;
	olc::Sprite spritePillar;
	olc::Sprite spriteLamp;
	olc::Sprite spriteExitSign;

	// Enemies sprites
	olc::Sprite spriteRobot1;
	olc::Sprite spriteRobot2;
	olc::Sprite spriteRobot3;
	olc::Sprite spriteBoss;

	std::vector<olc::Sprite*> enemyIconSprites;



	// Item sprites
	olc::Sprite spritesMedkit;
	olc::Sprite itemAmmo9mm;
	olc::Sprite itemAmmoShells;
	olc::Sprite itemUzi;
	olc::Sprite itemShotgun;

	std::vector<olc::Sprite*> itemIconSprites;





	olc::Sprite* SampleIconFromSprite(olc::Sprite* sprite, int32_t x, int32_t y, int32_t w, int32_t h){
		olc::Sprite* newIcon = new olc::Sprite(w, h);

		for (int smplX = 0; smplX < w; smplX++) {
			for (int smplY = 0; smplY < h; smplY++) {
				olc::Pixel color = sprite->GetPixel(x + smplX, y + smplY);
				//olc::Pixel color = sprite->GetPixel(int(1.0 * sprite->width / w * smplX), int(1.0 * sprite->height / h * smplY));
				newIcon->SetPixel(smplX, smplY, color);
			}
		}

		return(newIcon);
	}

	void SampleWallTexturesFromAtlas(olc::Sprite* atlas) {

		int textureCount = (atlas->width / 64) * (atlas->height / 64);

		for (int textureIdx = 0; textureIdx < textureCount; textureIdx++) {
			olc::Sprite* wallTexture = new olc::Sprite(64, 64);

			for (int texX = 0; texX < 64; texX++) {
				for (int texY = 0; texY < 64; texY++) {
					olc::Pixel color = spriteWallAtlas.GetPixel((textureIdx * 64 + texX) % atlas->width, ((textureIdx / 4) * 64 + texY));
					wallTexture->SetPixel(texX, texY, color);
				}
			}

			wallSprites.push_back(wallTexture);
		}
	}



	void LoadSprite(olc::Sprite* sprite, std::string file) {
		olc::rcode loadStatus = sprite->LoadFromFile(file);
		if (loadStatus == olc::FAIL) { std::cout << "Sprite load failed : " << file << std::endl; return; }
		if (loadStatus == olc::NO_FILE) { std::cout << "Sprite not found : " << file << std::endl; return; }
	}

	void LoadAssets() {

		// UI
		LoadSprite(&spriteCursor, "gfx/cursor.png");

		// Walls
		LoadSprite(&spriteWallAtlas, "gfx/wallAtlas.png");
		SampleWallTexturesFromAtlas(&spriteWallAtlas);

		// Decorations
		LoadSprite(&spriteBarell, "gfx/barrel.png");
		LoadSprite(&spritePillar, "gfx/pillar.png");
		LoadSprite(&spriteLamp, "gfx/greenlight.png");
		LoadSprite(&spriteExitSign, "gfx/exitSign.png");
	
		// Enemy
		LoadSprite(&spriteRobot1, "gfx/robot1.png");
		enemyIconSprites.push_back(SampleIconFromSprite(&spriteRobot1, 0, 0, 64, 64));
		LoadSprite(&spriteRobot2, "gfx/robot2.png");
		enemyIconSprites.push_back(SampleIconFromSprite(&spriteRobot2, 0, 0, 64, 64));
		LoadSprite(&spriteRobot3, "gfx/robot3.png");
		enemyIconSprites.push_back(SampleIconFromSprite(&spriteRobot3, 0, 0, 64, 64));
		LoadSprite(&spriteBoss, "gfx/boss.png");
		enemyIconSprites.push_back(SampleIconFromSprite(&spriteBoss, 0, 0, 64, 64));

		// Decorations
		LoadSprite(&weaponPistol, "gfx/weaponPistol.png");
		LoadSprite(&weaponUzi, "gfx/weaponUzi.png");
		LoadSprite(&weaponShotgun, "gfx/weaponShotgun.png");

		// Items
		LoadSprite(&spritesMedkit, "gfx/medkit.png");
		itemIconSprites.push_back(SampleIconFromSprite(&spritesMedkit, 20, 40, 24, 24));
		LoadSprite(&itemAmmo9mm, "gfx/itemAmmo9mm.png");
		itemIconSprites.push_back(SampleIconFromSprite(&itemAmmo9mm, 20, 40, 24, 24));
		LoadSprite(&itemAmmoShells, "gfx/itemAmmoShells.png");
		itemIconSprites.push_back(SampleIconFromSprite(&itemAmmoShells, 20, 40, 24, 24));
		LoadSprite(&itemUzi, "gfx/itemUzi.png");
		itemIconSprites.push_back(SampleIconFromSprite(&itemUzi, 12, 24, 40, 40));
		LoadSprite(&itemShotgun, "gfx/itemShotgun.png");
		itemIconSprites.push_back(SampleIconFromSprite(&itemShotgun, 12, 24, 40, 40));


		// Editor
		LoadSprite(&spriteEditorPlay, "gfx/editorPlay.png");
		LoadSprite(&spriteEditorSave, "gfx/editorSave.png");
		LoadSprite(&spriteEditorLoad, "gfx/editorLoad.png");
		LoadSprite(&spriteEditorSettings, "gfx/editorSettings.png");

		LoadSprite(&spriteEditorToolWall, "gfx/editorToolWall.png");
		LoadSprite(&spriteEditorToolItem, "gfx/editorToolItem.png");
		LoadSprite(&spriteEditorToolDecor, "gfx/editorToolDecor.png");
		LoadSprite(&spriteEditorToolEnemy, "gfx/editorToolEnemy.png");
		LoadSprite(&spriteEditorToolEracer, "gfx/editorToolEraser.png");
		LoadSprite(&spriteEditorToolZoom, "gfx/editorToolZoom.png");
		LoadSprite(&spriteEditorToolGrid, "gfx/editorToolGrid.png");
		LoadSprite(&spriteEditorToolPlayer, "gfx/editorToolPlayer.png");
		LoadSprite(&spriteEditorToolEndLevel, "gfx/editorToolEndLevel.png");
	}





	olc::Sprite* GetWallTexture(int idx) {
		return wallSprites[idx];
	}

	olc::Sprite* GetDecorationSprite(int idx) {
		switch (idx) {
		case 0: return &spriteBarell;
		case 1: return &spritePillar;
		case 2: return &spriteLamp;
		case 3: return &spriteExitSign;
		default: return &spriteBarell;
		}
	}

	olc::Sprite* GetEnemySprite(int idx) {
		switch (idx) {
		case 0: return &spriteRobot1;
		case 1: return &spriteRobot2;
		case 2: return &spriteRobot3;
		case 3: return &spriteBoss;
		default: return &spriteRobot1;
		}
	}

	olc::Sprite* GetItemSprite(int idx) {
		switch (idx) {
		case 0: return &spritesMedkit;
		case 1: return &itemAmmo9mm;
		case 2: return &itemAmmoShells;
		case 3: return &itemUzi;
		case 4: return &itemShotgun;
		default: return &spritesMedkit;
		}
	}

	olc::Sprite* GetWeaponSprite(int idx) {
		switch (idx) {
		case 0: return &weaponPistol;
		case 1: return &weaponUzi;
		case 2: return &weaponShotgun;
		default: return &weaponPistol;
		}
	}



	olc::Sprite spriteEditorPlay;
	olc::Sprite spriteEditorSave;
	olc::Sprite spriteEditorLoad;
	olc::Sprite spriteEditorSettings;

	olc::Sprite spriteEditorToolWall;
	olc::Sprite spriteEditorToolItem;
	olc::Sprite spriteEditorToolDecor;
	olc::Sprite spriteEditorToolEnemy;
	olc::Sprite spriteEditorToolEracer;
	olc::Sprite spriteEditorToolZoom;
	olc::Sprite spriteEditorToolGrid;
	olc::Sprite spriteEditorToolPlayer;
	olc::Sprite spriteEditorToolEndLevel;


	// Drawing stuff

	olc::Pixel DarkColor(olc::Pixel color, float amount = 0.5) {
		olc::Pixel darkColor = olc::Pixel(color.r * amount, color.g * amount, color.b * amount);
		return darkColor;
	}

	void DrawSpriteColorTransparent(int32_t x, int32_t y, olc::Sprite* sprite, olc::Pixel transparancyColor) {

		if (sprite == nullptr)
			return;

		for (int32_t i = 0; i < sprite->width; i++)
			for (int32_t j = 0; j < sprite->height; j++)
				if (sprite->GetPixel(i, j) != transparancyColor)
					Draw(x + i, y + j, sprite->GetPixel(i, j));
	}

	void DrawPartialSpriteColorTransparent(int32_t x, int32_t y, int32_t regionX, int32_t regionY, int32_t regionW, int32_t regionH, olc::Sprite* sprite, olc::Pixel transparancyColor) {

		if (sprite == nullptr)
			return;

		for (int32_t i = regionX; i < regionX + regionW; i++)
			for (int32_t j = regionY; j < regionY + regionH; j++)
				if (sprite->GetPixel(i, j) != transparancyColor)
					Draw(x + (i % regionW), y + (j % regionH), sprite->GetPixel(i, j));
	}



	//===============================================================================================================================
	// UI
	//===============================================================================================================================


	class Button {

	public:

		RaycastEngine* engineReference;

		olc::vi2d position;

		uint32_t width;
		uint32_t height;

		bool enabled;

		bool isPressed;
		bool isHovered;

		bool showBorder;
		bool showBackground;

		olc::Pixel colorBorder;
		olc::Pixel colorBackground;
		olc::Pixel colorText;
		olc::Pixel colorHovered;
		olc::Pixel colorPressed;


		std::string text;
		std::string hoverText;
		olc::Sprite* sprite;



		Button(RaycastEngine* engine){
			engineReference = engine;
			position = olc::vi2d(0,0);
			width = 20;
			height = 40;
			enabled = true;
			isPressed = false;
			isHovered = false;
			showBorder = true;
			showBackground = true;
			colorBorder = olc::GREY;
			colorBackground = olc::BLACK;
			colorText = olc::WHITE;
			colorHovered = olc::YELLOW;
			colorPressed = olc::DARK_YELLOW;
			text = "BTN";
			hoverText = "";
			sprite = nullptr;
		}

		Button(RaycastEngine* engine, olc::vi2d pos, uint32_t w, uint32_t h, std::string txt){
			engineReference = engine;
			position = pos;
			width = w;
			height = h;
			enabled = true;
			isPressed = false;
			isHovered = false;
			showBorder= true;
			showBackground = true;
			colorBorder = olc::GREY;
			colorBackground = olc::BLACK;
			colorText = olc::WHITE;
			colorHovered = olc::YELLOW;
			colorPressed = olc::DARK_YELLOW;
			text = txt;
			hoverText = "";
			sprite = nullptr;
		}

		Button(RaycastEngine* engine, olc::vi2d pos, uint32_t w, uint32_t h, olc::Sprite* img){
			engineReference = engine;
			position = pos;
			width = w;
			height = h;
			enabled = true;
			isPressed = false;
			isHovered = false;
			showBorder = true;
			showBackground = true;
			colorBorder = olc::GREY;
			colorBackground = olc::BLACK;
			colorText = olc::WHITE;
			colorHovered = olc::YELLOW;
			colorPressed = olc::DARK_YELLOW;
			text = "";
			hoverText = "";
			sprite = img;
		}



		void Update(){
			

			// Check mouse input

			isPressed = false;
			isHovered = false;

			olc::vi2d mousePos = olc::vi2d(engineReference->GetMouseX(), engineReference->GetMouseY());
			if (mousePos.x > position.x && mousePos.x < position.x + width && mousePos.y > position.y && mousePos.y < position.y + height)
			{
				isHovered = true;
			}

			if (isHovered && engineReference->GetMouse(0).bPressed) {
				isPressed = true;
			}


			// Draw button

			if (showBorder){
				engineReference->DrawRect(position.x, position.y, width, height, colorBorder); // Border
				if (showBackground){
					engineReference->FillRect(position.x + 1, position.y + 1, width - 1, height - 1, colorBackground); // Background
				}
			}else{
				if (showBackground){
					engineReference->FillRect(position.x, position.y, width, height, colorBackground); // Background
				}
			}

			// Draw selection
			if (isHovered){
				engineReference->DrawRect(position.x, position.y, width, height, colorHovered);
			}

			// Draw content

			if (text != ""){
				engineReference->DrawString(position.x + 6, position.y + (height / 2 - 3), text, colorText);
			}

			if (sprite != nullptr){
				
				for (int smplX = 0; smplX < width - 2; smplX++) {
					for (int smplY = 0; smplY < height - 2; smplY++) {
						olc::Pixel color = sprite->GetPixel(int( (1.0 * sprite->width / (width - 2)) * smplX), int( (1.0 * sprite->height / (height - 2)) * smplY));
						if (color != olc::CYAN) {
							engineReference->Draw(smplX + position.x + 1, smplY + position.y + 1, color);
						}
					}
				}
			}



		}

	};


	class InputField {

	public:

		RaycastEngine* engineReference;

		olc::vi2d position;

		uint32_t width;
		uint32_t height;

		bool enabled;

		bool isPressed;
		bool isHovered;
		bool isFocused;

		bool showBorder;
		bool showBackground;

		olc::Pixel colorBorder;
		olc::Pixel colorBackground;
		olc::Pixel colorText;
		olc::Pixel colorHovered;
		olc::Pixel colorPressed;


		std::string text;
		std::string hoverText;


		int charMaximum;
		bool allowOnlyNumbers;




		InputField(RaycastEngine* engine) {
			engineReference = engine;
			position = olc::vi2d(0, 0);
			width = 20;
			height = 40;
			enabled = true;
			isPressed = false;
			isHovered = false;
			showBorder = true;
			showBackground = true;
			colorBorder = olc::GREY;
			colorBackground = olc::BLACK;
			colorText = olc::WHITE;
			colorHovered = olc::YELLOW;
			colorPressed = olc::DARK_YELLOW;
			text = "BTN";
			hoverText = "";
			charMaximum = 20;
			allowOnlyNumbers = false;
		}

		InputField(RaycastEngine* engine, olc::vi2d pos, uint32_t w, uint32_t h, std::string txt) {
			engineReference = engine;
			position = pos;
			width = w;
			height = h;
			enabled = true;
			isPressed = false;
			isHovered = false;
			showBorder = true;
			showBackground = true;
			colorBorder = olc::GREY;
			colorBackground = olc::BLACK;
			colorText = olc::WHITE;
			colorHovered = olc::YELLOW;
			colorPressed = olc::DARK_YELLOW;
			text = txt;
			hoverText = "";
			charMaximum = 20;
			allowOnlyNumbers = false;
		}

		InputField(RaycastEngine* engine, olc::vi2d pos, uint32_t w, uint32_t h, std::string txt, int maxChars) {
			engineReference = engine;
			position = pos;
			width = w;
			height = h;
			enabled = true;
			isPressed = false;
			isHovered = false;
			showBorder = true;
			showBackground = true;
			colorBorder = olc::GREY;
			colorBackground = olc::BLACK;
			colorText = olc::WHITE;
			colorHovered = olc::YELLOW;
			colorPressed = olc::DARK_YELLOW;
			text = txt;
			hoverText = "";
			charMaximum = maxChars;
			allowOnlyNumbers = false;
		}





		void Update() {


			// Check mouse input

			isPressed = false;
			isHovered = false;

			olc::vi2d mousePos = olc::vi2d(engineReference->GetMouseX(), engineReference->GetMouseY());
			if (mousePos.x > position.x && mousePos.x < position.x + width && mousePos.y > position.y && mousePos.y < position.y + height)
			{
				isHovered = true;
			}
			else if (isFocused && engineReference->GetMouse(0).bPressed) {
				isFocused = false;
			}

			if (isHovered && engineReference->GetMouse(0).bPressed) {
				isPressed = true;
			}



			// Draw field

			if (showBorder) {
				engineReference->DrawRect(position.x, position.y, width, height, colorBorder); // Border
				if (showBackground) {
					engineReference->FillRect(position.x + 1, position.y + 1, width - 1, height - 1, colorBackground); // Background
				}
			}
			else {
				if (showBackground) {
					engineReference->FillRect(position.x, position.y, width, height, colorBackground); // Background
				}
			}

			// Draw selection
			if (isHovered) {
				engineReference->DrawRect(position.x, position.y, width, height, colorHovered);
			}
			if (isFocused) {
				engineReference->DrawRect(position.x, position.y, width, height, olc::RED);
			}

			// Draw content

			if (text != "") {
				engineReference->DrawString(position.x + 6, position.y + (height / 2 - 3), text, colorText);
			}


			// Handle input from keyboard

			if (isFocused) {

				if (engineReference->GetKey(olc::Key::BACK).bPressed) text = text.substr(0, text.length() - 1);

				if (text.length() < charMaximum) {

					if (!allowOnlyNumbers) {
						if (engineReference->GetKey(olc::Key::Q).bPressed) text += "Q";
						if (engineReference->GetKey(olc::Key::W).bPressed) text += "W";
						if (engineReference->GetKey(olc::Key::E).bPressed) text += "E";
						if (engineReference->GetKey(olc::Key::R).bPressed) text += "R";
						if (engineReference->GetKey(olc::Key::T).bPressed) text += "T";
						if (engineReference->GetKey(olc::Key::Y).bPressed) text += "Y";
						if (engineReference->GetKey(olc::Key::U).bPressed) text += "U";
						if (engineReference->GetKey(olc::Key::I).bPressed) text += "I";
						if (engineReference->GetKey(olc::Key::O).bPressed) text += "O";
						if (engineReference->GetKey(olc::Key::P).bPressed) text += "P";
						if (engineReference->GetKey(olc::Key::A).bPressed) text += "A";
						if (engineReference->GetKey(olc::Key::S).bPressed) text += "S";
						if (engineReference->GetKey(olc::Key::D).bPressed) text += "D";
						if (engineReference->GetKey(olc::Key::F).bPressed) text += "F";
						if (engineReference->GetKey(olc::Key::G).bPressed) text += "G";
						if (engineReference->GetKey(olc::Key::H).bPressed) text += "H";
						if (engineReference->GetKey(olc::Key::J).bPressed) text += "J";
						if (engineReference->GetKey(olc::Key::K).bPressed) text += "K";
						if (engineReference->GetKey(olc::Key::L).bPressed) text += "L";
						if (engineReference->GetKey(olc::Key::Z).bPressed) text += "Z";
						if (engineReference->GetKey(olc::Key::X).bPressed) text += "X";
						if (engineReference->GetKey(olc::Key::C).bPressed) text += "C";
						if (engineReference->GetKey(olc::Key::V).bPressed) text += "V";
						if (engineReference->GetKey(olc::Key::B).bPressed) text += "B";
						if (engineReference->GetKey(olc::Key::N).bPressed) text += "N";
						if (engineReference->GetKey(olc::Key::M).bPressed) text += "M";
					}

					if (engineReference->GetKey(olc::Key::K0).bPressed) text += "0";
					if (engineReference->GetKey(olc::Key::K1).bPressed) text += "1";
					if (engineReference->GetKey(olc::Key::K2).bPressed) text += "2";
					if (engineReference->GetKey(olc::Key::K3).bPressed) text += "3";
					if (engineReference->GetKey(olc::Key::K4).bPressed) text += "4";
					if (engineReference->GetKey(olc::Key::K5).bPressed) text += "5";
					if (engineReference->GetKey(olc::Key::K6).bPressed) text += "6";
					if (engineReference->GetKey(olc::Key::K7).bPressed) text += "7";
					if (engineReference->GetKey(olc::Key::K8).bPressed) text += "8";
					if (engineReference->GetKey(olc::Key::K9).bPressed) text += "9";
				}
			}
		}

	};


	class Slider {
	
	public :
		RaycastEngine* engineReference;

		olc::vi2d position;

		uint32_t width;
		uint32_t height;


		bool isPressed;
		bool isHeld;
		bool isHovered;


		bool showBorder;
		bool showBackground;

		olc::Pixel colorBorder;
		olc::Pixel colorBackground;
		olc::Pixel colorHovered;
		olc::Pixel colorPressed;

		olc::Pixel colorSlider;
		olc::Pixel colorSliderLine;

		std::string hoverText;

		float value;


		Slider(RaycastEngine* engine) {
			engineReference = engine;
			position = olc::vi2d(0, 0);
			width = 20;
			height = 40;
			isPressed = false;
			isHovered = false;
			showBorder = true;
			showBackground = true;
			colorBorder = olc::GREY;
			colorBackground = olc::BLACK;
			colorSlider = olc::WHITE;
			colorSliderLine = olc::WHITE;
			colorHovered = olc::YELLOW;
			colorPressed = olc::DARK_YELLOW;
			hoverText = "";
			value = 0.5;
		}

		Slider(RaycastEngine* engine, olc::vi2d pos, uint32_t w, uint32_t h) {
			engineReference = engine;
			position = pos;
			width = w;
			height = h;
			isPressed = false;
			isHovered = false;
			showBorder = true;
			showBackground = true;
			colorBorder = olc::GREY;
			colorBackground = olc::BLACK;
			colorSlider = olc::WHITE;
			colorSliderLine = olc::WHITE;
			colorHovered = olc::YELLOW;
			colorPressed = olc::DARK_YELLOW;
			hoverText = "";
			value = 0.5;
		}






		void Update() {


			// Check mouse input

			isPressed = false;
			isHovered = false;

			olc::vi2d mousePos = olc::vi2d(engineReference->GetMouseX(), engineReference->GetMouseY());
			if (mousePos.x > position.x && mousePos.x < position.x + width && mousePos.y > position.y && mousePos.y < position.y + height)
			{
				isHovered = true;
			}

			if (isHovered && engineReference->GetMouse(0).bPressed) {
				isPressed = true;
			}

			if (isHovered && engineReference->GetMouse(0).bHeld) {
				isHeld = true;

				int mouseRelPos = engineReference->GetMouseX() - position.x;
				value = mouseRelPos * 1.0 / width;

				if (value < 0.05) {
					value = 0;
				}
			}



			// Draw field

			if (showBorder) {
				engineReference->DrawRect(position.x, position.y, width, height, colorBorder); // Border
				if (showBackground) {
					engineReference->FillRect(position.x + 1, position.y + 1, width - 1, height - 1, colorBackground); // Background
				}
			}
			else {
				if (showBackground) {
					engineReference->FillRect(position.x, position.y, width, height, colorBackground); // Background
				}
			}

			// Draw selection
			if (isHovered) {
				engineReference->DrawRect(position.x, position.y, width, height, colorHovered);
			}


			// Draw content
			engineReference->FillRect(position.x, position.y + height / 2 - 2, width, 4, colorSliderLine); // Slider line

			engineReference->FillRect(position.x + value * width, position.y, 5, height, olc::BLACK); // Slider background
			engineReference->DrawRect(position.x + value * width, position.y, 5, height, colorSlider); // Slider
			


			// Handle input from keyboard

		}
	};




	void InfoLog(std::string text) {

		infoEffectTimer = 0.07;

		if (infoLogTimer0 <= infoLogTimer1 && infoLogTimer0 <= infoLogTimer2) {
			infoLog0 = text; infoLogTimer0 = 3;
			return;
		}
		if (infoLogTimer1 <= infoLogTimer0 && infoLogTimer1 <= infoLogTimer2) {
			infoLog1 = text; infoLogTimer1 = 3;
			return;
		}
		if (infoLogTimer2 <= infoLogTimer1 && infoLogTimer2 <= infoLogTimer0) {
			infoLog2 = text; infoLogTimer2 = 3;
			return;
		}

	}

	void DrawPlayerUI(float fElapsedTime) {

		// Draw weapon -----------------------------

		if (player.isAlive) {


			if (player.changeWeaponTimer > 0) {

				DrawPartialSpriteColorTransparent
				(
					ScreenWidth() / 2 + 0 - sin(player.bobValue + 1.55) * 8,
					ScreenHeight() - 110 - (abs(sin(player.bobValue)) * 8) + 80 - (abs(player.changeWeaponTimer - (player.timeToChangeWeapon / 2)) * (1/ (player.timeToChangeWeapon / 2)) * 80),
					player.weaponState * 128,
					0,
					128,
					128,
					GetWeaponSprite(player.activeWeapon),
					olc::CYAN
				);
			}
			else {
				DrawPartialSpriteColorTransparent
				(
					ScreenWidth() / 2 + 0 - sin(player.bobValue + 1.55) * 8,
					ScreenHeight() - 110 - (abs(sin(player.bobValue)) * 8),
					player.weaponState * 128,
					0,
					128,
					128,
					GetWeaponSprite(player.activeWeapon),
					olc::CYAN
				);
			}



		}



		// Death screen

		if (!player.isAlive) {
			SetPixelMode(olc::Pixel::Mode::ALPHA);
			FillRect(0, 0, ScreenWidth(), ScreenHeight(), olc::Pixel(255, 0, 0, 150));
			SetPixelMode(olc::Pixel::Mode::NORMAL);
		}



		// Draw crosshair -----------------------------

		Draw(ScreenWidth() / 2, ScreenHeight() / 2, olc::WHITE);


		// Draw player status -----------------------------

		FillRect(5, 165, 60, 30, olc::BLACK);
		DrawLine(4, 166, 4, 195, olc::YELLOW);
		DrawLine(4, 195, 64, 195, olc::YELLOW);
		DrawString(9, 161, "Health", olc::WHITE);
		DrawString(11, 176, std::to_string(player.health), olc::VERY_DARK_YELLOW, 2);
		DrawString(10, 175, std::to_string(player.health), olc::YELLOW, 2);

		FillRect(255, 165, 60, 30, olc::BLACK);
		DrawLine(315, 166, 315, 195, olc::YELLOW);
		DrawLine(256, 195, 315, 195, olc::YELLOW);
		DrawString(280, 161, "Ammo", olc::WHITE);
		DrawString(266, 176, std::to_string(weapons[player.activeWeapon].currentAmmo), olc::VERY_DARK_YELLOW, 2);
		DrawString(265, 175, std::to_string(weapons[player.activeWeapon].currentAmmo), olc::YELLOW, 2);

		// Draw info logs -----------------------------


		if (infoLogTimer0 > 0) {
			infoLogTimer0 -= fElapsedTime;
			DrawString(6, 6, infoLog0, olc::BLACK);
			DrawString(5, 5, infoLog0, olc::YELLOW);
		}
		if (infoLogTimer1 > 0) {
			infoLogTimer1 -= fElapsedTime;
			DrawString(6, 16, infoLog1, olc::BLACK);
			DrawString(5, 15, infoLog1, olc::YELLOW);
		}
		if (infoLogTimer2 > 0) {
			infoLogTimer2 -= fElapsedTime;
			DrawString(6, 26, infoLog2, olc::BLACK);
			DrawString(5, 25, infoLog2, olc::YELLOW);
		}


		// Draw info effect -----------------------------

		if (infoEffectTimer > 0) {
			infoEffectTimer -= fElapsedTime;
			DrawRect(0, 0, ScreenWidth() - 1, ScreenHeight() - 1, olc::DARK_YELLOW);
		}

		// Draw damage effect -----------------------------

		if (player.damageEffectTimer > 0) {
			player.damageEffectTimer -= fElapsedTime;
			DrawRect(0, 0, ScreenWidth() - 1, ScreenHeight() - 1, olc::RED);
		}

	}


	void DrawMenu() {


		if (submenu == 0) {
			int menuItemSelected = -1;

			if (GetMouseY() > 67 && GetMouseY() <= 87) {
				menuItemSelected = 0;
			}
			if (GetMouseY() > 87 && GetMouseY() <= 107) {
				menuItemSelected = 1;
			}
			if (GetMouseY() > 107 && GetMouseY() <= 127) {
				menuItemSelected = 2;
			}


			DrawString(72, 22, "Pause", olc::VERY_DARK_YELLOW, 3);
			DrawString(70, 20, "Pause", olc::YELLOW, 3);

			DrawString(62, 67, "Restart", olc::VERY_DARK_YELLOW, 2);
			DrawString(60, 65, "Restart", olc::YELLOW, 2);

			DrawString(62, 87, "Settings", olc::VERY_DARK_YELLOW, 2);
			DrawString(60, 85, "Settings", olc::YELLOW, 2);

			DrawString(62, 107, "Exit to menu", olc::VERY_DARK_YELLOW, 2);
			DrawString(60, 105, "Exit to menu", olc::YELLOW, 2);


			if (menuItemSelected != -1) {
				DrawString(42, 67 + 20 * menuItemSelected, ">", olc::VERY_DARK_YELLOW, 2);
				DrawString(40, 65 + 20 * menuItemSelected, ">", olc::YELLOW, 2);
			}




			if (GetMouse(0).bPressed) {

				if (menuItemSelected == 0) {
					RestartLevel();
					soundEngine->play2D("sounds/pickup.wav", false);
				}
				if (menuItemSelected == 1) {
					submenu = 2;
					soundEngine->play2D("sounds/pickup.wav", false);
				}
				if (menuItemSelected == 2) {
					StopMIDI();
					gameState = GameState::STATE_TITLESCREEN;
					LoadDefaultMap();
					soundEngine->play2D("sounds/pickup.wav", false);
				}

			}


		}
		else if (submenu == 2) {

			int menuItemSelected = -1;


			if (GetMouseY() > 47 && GetMouseY() <= 67) {
				menuItemSelected = 0;
			}

			DrawString(32, 22, "SETTINGS", olc::VERY_DARK_YELLOW, 2);
			DrawString(30, 20, "SETTINGS", olc::YELLOW, 2);

			DrawString(62, 47, "<- Back", olc::VERY_DARK_RED, 2);
			DrawString(60, 45, "<- Back", olc::RED, 2);




			DrawString(62, 82, "MIDI", olc::VERY_DARK_YELLOW, 2);
			DrawString(60, 80, "MIDI", olc::YELLOW, 2);

			DrawString(62, 112, "FX", olc::VERY_DARK_YELLOW, 2);
			DrawString(60, 110, "FX", olc::YELLOW, 2);

			DrawString(62, 142, "M SEN", olc::VERY_DARK_YELLOW, 2);
			DrawString(60, 140, "M SEN", olc::YELLOW, 2);


			midiVolumeSlider.showBorder = false;
			midiVolumeSlider.colorSlider = olc::YELLOW;
			midiVolumeSlider.colorSliderLine = olc::DARK_YELLOW;
			midiVolumeSlider.showBackground = false;
			midiVolumeSlider.Update();

			//if (midiVolumeSlider.isPressed) {
			//	PlayMapMIDI();
			//}
			tsf_set_volume(g_TinySoundFont, midiVolumeSlider.value);


			fxVolumeSlider.showBorder = false;
			fxVolumeSlider.colorSlider = olc::YELLOW;
			fxVolumeSlider.colorSliderLine = olc::DARK_YELLOW;
			fxVolumeSlider.showBackground = false;
			fxVolumeSlider.Update();

			if (fxVolumeSlider.isPressed) {
				soundEngine->play2D("sounds/pistol.wav", false);
			}
			soundEngine->setSoundVolume(fxVolumeSlider.value);



			mouseSensSlider.showBorder = false;
			mouseSensSlider.colorSlider = olc::YELLOW;
			mouseSensSlider.colorSliderLine = olc::DARK_YELLOW;
			mouseSensSlider.showBackground = false;
			mouseSensSlider.Update();

			mouseSensitivity = mouseSensSlider.value * 2;



			if (menuItemSelected != -1) {
				DrawString(42, 47 + 20 * menuItemSelected, ">", olc::VERY_DARK_YELLOW, 2);
				DrawString(40, 45 + 20 * menuItemSelected, ">", olc::YELLOW, 2);
			}


			if (GetMouse(0).bPressed) {
				if (menuItemSelected == 0) {
					submenu = 0;
					soundEngine->play2D("sounds/pickup.wav", false);
				}
			}

		}
	}



	void DrawEndScreen() {

		FillRect(0, 0, ScreenWidth(), ScreenHeight(), olc::BLACK);
		DrawRect(0, 0, ScreenWidth() - 1, ScreenHeight() - 1, olc::YELLOW);

		DrawString(32, 22, "THE END", olc::VERY_DARK_YELLOW, 2);
		DrawString(30, 20, "THE END", olc::YELLOW, 2);

		DrawString(10, 60, "You did it! The terrible machine is", olc::WHITE, 1);
		DrawString(10, 70, "off and everything will be back to", olc::WHITE, 1);
		DrawString(10, 80, "normal soon. You smell burnt plastic", olc::WHITE, 1);
		DrawString(10, 90, "and look at the remains of a once", olc::WHITE, 1);
		DrawString(10, 100, "great invention. It remains only ", olc::WHITE, 1);
		DrawString(10, 110, "to deal with the hordes of already", olc::WHITE, 1);
		DrawString(10, 120, "created machines in other parts", olc::WHITE, 1);
		DrawString(10, 130, "of the complex.", olc::WHITE, 1);
		DrawString(5, 150, "You can test yourself in user made maps!", olc::WHITE, 1);

	}


	void DrawLoreScreen() {

		FillRect(0, 0, ScreenWidth(), ScreenHeight(), olc::BLACK);
		DrawRect(0, 0, ScreenWidth() - 1, ScreenHeight() - 1, olc::YELLOW);

		DrawString(32, 22, "STORY", olc::VERY_DARK_YELLOW, 2);
		DrawString(30, 20, "STORY", olc::YELLOW, 2);

		DrawString(10, 60, "You are in a secret complex for the", olc::WHITE, 1);
		DrawString(10, 70, "development	of new technologies.", olc::WHITE, 1);
		DrawString(10, 80, "You are one of the volunteers", olc::WHITE, 1);
		DrawString(10, 90, "for the human cryofreezing program.", olc::WHITE, 1);
		DrawString(10, 100, "You should have been defrosted after", olc::WHITE, 1);
		DrawString(10, 110, "10 years and after waking up you", olc::WHITE, 1);
		DrawString(10, 120, "feel that something is wrong around", olc::WHITE, 1);
		DrawString(10, 130, "you. There are only hostile machines", olc::WHITE, 1);
		DrawString(10, 140, "around. You need to figure out who", olc::WHITE, 1);
		DrawString(10, 150, "or WHAT is behind this.", olc::WHITE, 1);
		DrawString(5, 170, "PRESS SPACE OR E TO CONTINUE", olc::WHITE, 1);


	}

	//===============================================================================================================================
	// TITLE SCREEN
	//===============================================================================================================================
 
	int submenu = 0;
	
	std::vector<std::string> mapsToSelect;
	int mapSelectPage = 0;
	int mapPageCount = 0;


	Slider midiVolumeSlider = Slider(this, olc::vi2d(150, 80), 130, 20);
	Slider fxVolumeSlider = Slider(this, olc::vi2d(150, 110), 130, 20);
	Slider mouseSensSlider = Slider(this, olc::vi2d(150, 140), 130, 20);


	void DrawTitleScreen(float fElapsedTime) {


		// MAIN MENU ---------------------------------------------

		if (submenu == 0) {

			int menuItemSelected = -1;

			if (GetMouseY() > 67 && GetMouseY() <= 87) {
				menuItemSelected = 0;
			}
			if (GetMouseY() > 87 && GetMouseY() <= 107) {
				menuItemSelected = 1;
			}
			if (GetMouseY() > 107 && GetMouseY() <= 127) {
				menuItemSelected = 2;
			}
			if (GetMouseY() > 127 && GetMouseY() <= 147) {
				menuItemSelected = 3;
			}
			if (GetMouseY() > 147 && GetMouseY() <= 167) {
				menuItemSelected = 4;
			}


			FillRect(0, 0, ScreenWidth(), ScreenHeight(), olc::BLACK);
			DrawRect(0, 0, ScreenWidth() - 1, ScreenHeight() - 1, olc::YELLOW);



			DrawString(32, 22, "OMNIVERSE MACKINA", olc::VERY_DARK_YELLOW, 2);
			DrawString(30, 20, "OMNIVERSE MACKINA", olc::YELLOW, 2);

			DrawString(62, 67, "New game", olc::VERY_DARK_YELLOW, 2);
			DrawString(60, 65, "New game", olc::YELLOW, 2);
					   
			DrawString(62, 87, "Select map", olc::VERY_DARK_YELLOW, 2);
			DrawString(60, 85, "Select map", olc::YELLOW, 2);
					   
			DrawString(62, 107, "Map editor", olc::VERY_DARK_YELLOW, 2);
			DrawString(60, 105, "Map editor", olc::YELLOW, 2);
					   
			DrawString(62, 127, "Settings", olc::VERY_DARK_YELLOW, 2);
			DrawString(60, 125, "Settings", olc::YELLOW, 2);
					   
			DrawString(62, 147, "Exit", olc::VERY_DARK_YELLOW, 2);
			DrawString(60, 145, "Exit", olc::YELLOW, 2);


			if (menuItemSelected != -1) {
				DrawString(42, 67 + 20 * menuItemSelected, ">", olc::VERY_DARK_YELLOW, 2);
				DrawString(40, 65 + 20 * menuItemSelected, ">", olc::YELLOW, 2);
			}




			if (GetMouse(0).bPressed) {

				if (menuItemSelected == 0) { // New game
					isEditorInPlayMode = false;
					// Load map
					LoadMap("maps/LEVEL01.map");
					gameState = GameState::STATE_LORESCREEN;
					//gameState = GameState::STATE_GAMEPLAY;
					// Start MIDI
					PlayMapMIDI();
					soundEngine->play2D("sounds/pickup.wav", false);

					isNewGameStarted = true;
					bossDefeated = 0;
				}
				if (menuItemSelected == 1) { // Select map

					// Reading maps/ directory
					mapsToSelect.clear();
					std::string path = "maps/";
					for (const auto& entry : std::filesystem::directory_iterator(path)) {
						std::cout << "Found map file : " << entry.path() << std::endl;
						mapsToSelect.push_back(entry.path().string());
					}
					mapPageCount = ceil(mapsToSelect.size() / 4.0);

					submenu = 1;
					soundEngine->play2D("sounds/pickup.wav", false);
				}
				if (menuItemSelected == 2) { // Editor
					StopMIDI();
					LoadDefaultMap();
					gameState = GameState::STATE_EDITOR;
					soundEngine->play2D("sounds/pickup.wav", false);
				}
				if (menuItemSelected == 3) { // Settings				
					submenu = 2;
					soundEngine->play2D("sounds/pickup.wav", false);
				}
				if (menuItemSelected == 4) { // Exit
					exit(0);
				}

			}
		}

		// MAP SELECTION ----------------------------------------------

		else if (submenu == 1) {

			int menuItemSelected = -1;

			if (GetMouseY() > 47 && GetMouseY() <= 67) {
				menuItemSelected = 0;
			}
			if (GetMouseY() > 67 && GetMouseY() <= 87) {
				menuItemSelected = 1;
			}
			if (GetMouseY() > 87 && GetMouseY() <= 107) {
				menuItemSelected = 2;
			}
			if (GetMouseY() > 107 && GetMouseY() <= 127) {
				menuItemSelected = 3;
			}
			if (GetMouseY() > 127 && GetMouseY() <= 147) {
				menuItemSelected = 4;
			}
			if (GetMouseY() > 147 && GetMouseY() <= 167) {
				menuItemSelected = 5;
			}
			if (GetMouseY() > 167 && GetMouseY() <= 187) {
				menuItemSelected = 6;
			}


			FillRect(0, 0, ScreenWidth(), ScreenHeight(), olc::BLACK);
			DrawRect(0, 0, ScreenWidth() - 1, ScreenHeight() - 1, olc::YELLOW);



			DrawString(32, 22, "SELECT MAP", olc::VERY_DARK_YELLOW, 2);
			DrawString(30, 20, "SELECT MAP", olc::YELLOW, 2);

			DrawString(62, 47, "<- Back", olc::VERY_DARK_RED, 2);
			DrawString(60, 45, "<- Back", olc::RED, 2);

			DrawString(62, 67, "Prev page", olc::VERY_DARK_RED, 2);
			DrawString(60, 65, "Prev page", olc::RED, 2);

			for (int mapString = 0; mapString < 4; mapString++) {

				if (mapString + mapSelectPage * 4 < mapsToSelect.size()) {
					std::string mapName = mapsToSelect[mapString + mapSelectPage * 4].substr(5, mapsToSelect[mapString + mapSelectPage * 4].length() - 9);

					DrawString(62, 87 + mapString * 20, mapName, olc::VERY_DARK_YELLOW, 2);
					DrawString(60, 85 + mapString * 20, mapName, olc::YELLOW, 2);
				}
				
			}

			DrawString(62, 167, "Next page", olc::VERY_DARK_RED, 2);
			DrawString(60, 165, "Next page", olc::RED, 2);


			if (menuItemSelected != -1) {
				DrawString(42, 47 + 20 * menuItemSelected, ">", olc::VERY_DARK_YELLOW, 2);
				DrawString(40, 45 + 20 * menuItemSelected, ">", olc::YELLOW, 2);
			}



			if (GetMouse(0).bPressed) {

				if (menuItemSelected == 0) {
					submenu = 0;
					soundEngine->play2D("sounds/pickup.wav", false);
				}
				if (menuItemSelected == 1) {
					if (mapSelectPage > 0) {
						mapSelectPage--;
					}		
					soundEngine->play2D("sounds/pickup.wav", false);
				}
				if (menuItemSelected == 2) { 
					isEditorInPlayMode = false;
					// Load map
					LoadMap(mapsToSelect[menuItemSelected - 2 + mapSelectPage * 4]);
					gameState = GameState::STATE_GAMEPLAY;
					// Start MIDI
					PlayMapMIDI();
					soundEngine->play2D("sounds/pickup.wav", false);

					isNewGameStarted = false;
					bossDefeated = 0;
				}
				if (menuItemSelected == 3) { 
					isEditorInPlayMode = false;
					// Load map
					LoadMap(mapsToSelect[menuItemSelected - 2 + mapSelectPage * 4]);
					gameState = GameState::STATE_GAMEPLAY;
					// Start MIDI
					PlayMapMIDI();
					soundEngine->play2D("sounds/pickup.wav", false);

					isNewGameStarted = false;
					bossDefeated = 0;
				}
				if (menuItemSelected == 4) { 
					isEditorInPlayMode = false;
					// Load map
					LoadMap(mapsToSelect[menuItemSelected - 2 + mapSelectPage * 4]);
					gameState = GameState::STATE_GAMEPLAY;
					// Start MIDI
					PlayMapMIDI();
					soundEngine->play2D("sounds/pickup.wav", false);

					isNewGameStarted = false;
					bossDefeated = 0;
				}
				if (menuItemSelected == 5) { 
					isEditorInPlayMode = false;
					// Load map
					LoadMap(mapsToSelect[menuItemSelected - 2 + mapSelectPage * 4]);
					gameState = GameState::STATE_GAMEPLAY;
					// Start MIDI
					PlayMapMIDI();
					soundEngine->play2D("sounds/pickup.wav", false);

					isNewGameStarted = false;
					bossDefeated = 0;
				}
				if (menuItemSelected == 6) {
					if (mapSelectPage < mapPageCount -1) {
						mapSelectPage++;
					}
					soundEngine->play2D("sounds/pickup.wav", false);
				}

			}
		}
		else if (submenu == 2) {

			int menuItemSelected = -1;

			FillRect(0, 0, ScreenWidth(), ScreenHeight(), olc::BLACK);
			DrawRect(0, 0, ScreenWidth() - 1, ScreenHeight() - 1, olc::YELLOW);


			if (GetMouseY() > 47 && GetMouseY() <= 67) {
				menuItemSelected = 0;
			}

			DrawString(32, 22, "SETTINGS", olc::VERY_DARK_YELLOW, 2);
			DrawString(30, 20, "SETTINGS", olc::YELLOW, 2);

			DrawString(62, 47, "<- Back", olc::VERY_DARK_RED, 2);
			DrawString(60, 45, "<- Back", olc::RED, 2);




			DrawString(62, 82, "MIDI", olc::VERY_DARK_YELLOW, 2);
			DrawString(60, 80, "MIDI", olc::YELLOW, 2);

			DrawString(62, 112, "FX", olc::VERY_DARK_YELLOW, 2);
			DrawString(60, 110, "FX", olc::YELLOW, 2);

			DrawString(62, 142, "M SEN", olc::VERY_DARK_YELLOW, 2);
			DrawString(60, 140, "M SEN", olc::YELLOW, 2);


			midiVolumeSlider.showBorder = false;
			midiVolumeSlider.colorSlider = olc::YELLOW;
			midiVolumeSlider.colorSliderLine = olc::DARK_YELLOW;
			midiVolumeSlider.Update();

			if (midiVolumeSlider.isPressed) {
				PlayMapMIDI();
			}
			tsf_set_volume(g_TinySoundFont, midiVolumeSlider.value);


			fxVolumeSlider.showBorder = false;
			fxVolumeSlider.colorSlider = olc::YELLOW;
			fxVolumeSlider.colorSliderLine = olc::DARK_YELLOW;
			fxVolumeSlider.Update();

			if (fxVolumeSlider.isPressed) {
				soundEngine->play2D("sounds/pistol.wav", false);
			}
			soundEngine->setSoundVolume(fxVolumeSlider.value);


			mouseSensSlider.showBorder = false;
			mouseSensSlider.colorSlider = olc::YELLOW;
			mouseSensSlider.colorSliderLine = olc::DARK_YELLOW;
			mouseSensSlider.Update();

			mouseSensitivity = mouseSensSlider.value * 2;



			if (menuItemSelected != -1) {
				DrawString(42, 47 + 20 * menuItemSelected, ">", olc::VERY_DARK_YELLOW, 2);
				DrawString(40, 45 + 20 * menuItemSelected, ">", olc::YELLOW, 2);
			}


			if (GetMouse(0).bPressed) {
				if (menuItemSelected == 0) {
					submenu = 0;
					soundEngine->play2D("sounds/pickup.wav", false);
				}
			}

		}

		

	}


	//===============================================================================================================================
	// WORLD PHYSICS
	//===============================================================================================================================


	// For ray collision calculations

	bool LineIntersection(olc::vf2d ln1pnt1, olc::vf2d ln1pnt2, olc::vf2d ln2pnt1, olc::vf2d ln2pnt2){
		
		olc::vf2d line1 = ln1pnt2 - ln1pnt1;
		olc::vf2d line2 = ln2pnt2 - ln2pnt1;

		float cross1 = line1.cross(ln2pnt1 - ln1pnt1);
		float cross2 = line1.cross(ln2pnt2 - ln1pnt1);

		if ( copysign(1, cross1) == copysign(1, cross2) || cross1 == 0 || cross2 == 0 )
			return false;
	
		cross1 = line2.cross(ln1pnt1 - ln2pnt1);
		cross2 = line2.cross(ln1pnt2 - ln2pnt1);

		if ( copysign(1, cross1) == copysign(1, cross2) || cross1 == 0 || cross2 == 0 )
			return false;

		return true;
	}


	//x-coordinate in camera space,  [0 - 319] pixels -> [-1 - 1]
	float ShootRaycastRay(olc::vf2d from, olc::vf2d rayDir) {

		//which box of the map we're in
		olc::vi2d mapPosition = from;

		//length of ray from current position to next x or y-side
		olc::vf2d sideDist;

		//length of ray from one x or y-side to next x or y-side
		olc::vf2d deltaDist = olc::vf2d(std::abs(1 / rayDir.x), std::abs(1 / rayDir.y));

		float perpWallDist;

		//what direction to step in x or y-direction (either +1 or -1)
		olc::vi2d step;


		int hit = 0; //was there a wall hit?
		int side; //was a NS(0) or a EW(1) wall hit?


		//calculate step and initial sideDist
		if (rayDir.x < 0)
		{
			step.x = -1;
			sideDist.x = (from.x - mapPosition.x) * deltaDist.x;
		}
		else
		{
			step.x = 1;
			sideDist.x = (mapPosition.x + 1.0 - from.x) * deltaDist.x;
		}
		if (rayDir.y < 0)
		{
			step.y = -1;
			sideDist.y = (from.y - mapPosition.y) * deltaDist.y;
		}
		else
		{
			step.y = 1;
			sideDist.y = (mapPosition.y + 1.0 - from.y) * deltaDist.y;
		}

		int stepsDone = 0;

		//perform DDA
		while (hit == 0 && stepsDone < 50)
		{
			//jump to next map square, OR in x-direction, OR in y-direction
			if (sideDist.x < sideDist.y)
			{
				sideDist.x += deltaDist.x;
				mapPosition.x += step.x;
				side = 0;
			}
			else
			{
				sideDist.y += deltaDist.y;
				mapPosition.y += step.y;
				side = 1;
			}
			stepsDone++;
			//Check if ray has hit a wall
			if (worldMap[mapPosition.y * worldMapWidth + mapPosition.x] > 0) hit = 1;
		}


		//Calculate distance projected on camera direction (Euclidean distance will give fisheye effect!)
		if (side == 0) perpWallDist = (mapPosition.x - from.x + (1 - step.x) / 2) / rayDir.x;
		else           perpWallDist = (mapPosition.y - from.y + (1 - step.y) / 2) / rayDir.y;

		return perpWallDist;
	}









	//===============================================================================================================================
	// RENDERING
	//===============================================================================================================================

	//arrays used to sort the sprites
	int* spriteOrder;
	float* spriteDistance;

	//sort algorithm
	//sort the sprites based on distance
	void sortSprites(int* order, float* dist, int amount)
	{
		std::vector<std::pair<float, int>> sprites(amount);
		for (int i = 0; i < amount; i++) {
			sprites[i].first = dist[i];
			sprites[i].second = order[i];
		}
		std::sort(sprites.begin(), sprites.end());
		// restore in reverse order to go from farthest to nearest
		for (int i = 0; i < amount; i++) {
			dist[i] = sprites[amount - i - 1].first;
			order[i] = sprites[amount - i - 1].second;
		}
	}


	void RaycastRender() {


		//FLOOR CASTING
		for (int y = ScreenHeight() / 2; y < ScreenHeight(); y++)
		{
			// rayDir for leftmost ray (x = 0) and rightmost ray (x = w)
			olc::vf2d rayDir0 = player.direction - player.viewPlane;

			olc::vf2d rayDir1 = player.direction + player.viewPlane;


			// Current y position compared to the center of the screen (the horizon)
			int p = y - ScreenHeight() / 2;

			// Vertical position of the camera.
			float posZ = 0.5 * ScreenHeight() + player.cameraVertical * 4;
			float posZ2 = 0.5 * ScreenHeight() - player.cameraVertical * 4;

			// Horizontal distance from the camera to the floor for the current row.
			// 0.5 is the z position exactly in the middle between floor and ceiling.
			float rowDistance = posZ / p;
			float rowDistance2 = posZ2 / p;


			// calculate the real world step vector we have to add for each x (parallel to camera plane)
			// adding step by step avoids multiplications with a weight in the inner loop
			float floorStepX = rowDistance * (rayDir1.x - rayDir0.x) / ScreenWidth();
			float floorStepY = rowDistance * (rayDir1.y - rayDir0.y) / ScreenWidth();

			// real world coordinates of the leftmost column. This will be updated as we step to the right.
			float floorX = player.position.x + rowDistance * rayDir0.x;
			float floorY = player.position.y + rowDistance * rayDir0.y;



			// calculate the real world step vector we have to add for each x (parallel to camera plane)
			// adding step by step avoids multiplications with a weight in the inner loop
			float floorStepX2 = rowDistance2 * (rayDir1.x - rayDir0.x) / ScreenWidth();
			float floorStepY2 = rowDistance2 * (rayDir1.y - rayDir0.y) / ScreenWidth();

			// real world coordinates of the leftmost column. This will be updated as we step to the right.
			float floorX2 = player.position.x + rowDistance2 * rayDir0.x;
			float floorY2 = player.position.y + rowDistance2 * rayDir0.y;



			for (int x = 0; x < ScreenWidth(); ++x)
			{
				// the cell coord is simply got from the integer parts of floorX and floorY
				int cellX = (int)(floorX);
				int cellY = (int)(floorY);

				// get the texture coordinate
				floorX += floorStepX;
				floorY += floorStepY;


				float shading = 1 - (0.8 * (ScreenHeight() - y) / (ScreenHeight() / 2));

				//if (!drawnPixels[y * ScreenWidth() + x]) {
					// floor
					olc::Pixel color = GetWallTexture(worldMapFloorTxtr)->Sample(floorX - cellX + 0.05, floorY - cellY + 0.05);
					color = DarkColor(color, shading);
					Draw(x, y, color);
				//}

			}


			for (int x = 0; x < ScreenWidth(); ++x)
			{
				// the cell coord is simply got from the integer parts of floorX and floorY
				int cellX = (int)(floorX2);
				int cellY = (int)(floorY2);

				// get the texture coordinate
				floorX2 += floorStepX2;
				floorY2 += floorStepY2;


				float shading = 1 - (0.8 * (ScreenHeight() - y) / (ScreenHeight() / 2));


				//if (!drawnPixels[(ScreenHeight() - y - 1) * ScreenWidth() + x]) {
					//ceiling (symmetrical, at screenHeight - y - 1 instead of y)
					olc::Pixel color = GetWallTexture(worldMapCeilTxtr)->Sample(floorX2 - cellX + 0.05, floorY2 - cellY + 0.05);
					color = DarkColor(color, shading);
					Draw(x, ScreenHeight() - y - 1, color);
				//}

			}
		}
		

		// WALL CASTING
		for (int x = 0; x < ScreenWidth(); x++) {

			//calculate ray
			float cameraX = 2 * x / float(ScreenWidth()) - 1; //x-coordinate in camera space,  [0 - 319] pixels -> [-1 - 1]

			//calculate ray position and direction
			olc::vf2d rayDir = player.direction + player.viewPlane * cameraX;

			//which box of the map we're in
			olc::vi2d mapPosition = player.position;

			//length of ray from current position to next x or y-side
			olc::vf2d sideDist;

			//length of ray from one x or y-side to next x or y-side
			olc::vf2d deltaDist = olc::vf2d(std::abs(1 / rayDir.x), std::abs(1 / rayDir.y));

			float perpWallDist;

			//what direction to step in x or y-direction (either +1 or -1)
			olc::vi2d step;


			int hit = 0; //was there a wall hit?
			int side; //was a NS(0) or a EW(1) wall hit?


			//calculate step and initial sideDist
			if (rayDir.x < 0)
			{
				step.x = -1;
				sideDist.x = (player.position.x - mapPosition.x) * deltaDist.x;
			}
			else
			{
				step.x = 1;
				sideDist.x = (mapPosition.x + 1.0 - player.position.x) * deltaDist.x;
			}
			if (rayDir.y < 0)
			{
				step.y = -1;
				sideDist.y = (player.position.y - mapPosition.y) * deltaDist.y;
			}
			else
			{
				step.y = 1;
				sideDist.y = (mapPosition.y + 1.0 - player.position.y) * deltaDist.y;
			}

			int stepsDone = 0;

			//perform DDA
			while (hit == 0 && stepsDone < 50)
			{
				//jump to next map square, OR in x-direction, OR in y-direction
				if (sideDist.x < sideDist.y)
				{
					sideDist.x += deltaDist.x;
					mapPosition.x += step.x;
					side = 0;
				}
				else
				{
					sideDist.y += deltaDist.y;
					mapPosition.y += step.y;
					side = 1;
				}
				stepsDone++;
				//Check if ray has hit a wall
				if (worldMap[mapPosition.y * worldMapWidth + mapPosition.x] > 0) hit = 1;
			}


			//Calculate distance projected on camera direction (Euclidean distance will give fisheye effect!)
			if (side == 0) perpWallDist = (mapPosition.x - player.position.x + (1 - step.x) / 2) / rayDir.x;
			else           perpWallDist = (mapPosition.y - player.position.y + (1 - step.y) / 2) / rayDir.y;


			//Calculate height of line to draw on screen
			int lineHeight = (int)(ScreenHeight() / perpWallDist);

			//calculate lowest and highest pixel to fill in current stripe
			int drawStart = -lineHeight * 1.0 / 2 + ScreenHeight() / 2 + (player.cameraVertical * 4.0 / perpWallDist);
			if (drawStart < 0) drawStart = 0;
			int drawEnd = lineHeight * 1.0 / 2 + ScreenHeight() / 2 + (player.cameraVertical * 4.0 / perpWallDist);
			if (drawEnd >= ScreenHeight()) drawEnd = ScreenHeight();


			// calculate X coordinate of texture
			float wallX;
			if (side == 0) wallX = player.position.y + perpWallDist * rayDir.y;
			else wallX = player.position.x + perpWallDist * rayDir.x;
			wallX -= floor(wallX); // Get floating point part	


			//choose wall texture
			olc::Pixel color;
			int textureNumber = worldMap[mapPosition.y * worldMapWidth + mapPosition.x] - 1;


			float wallYstep = 1.0 / lineHeight;
			float wallY = wallYstep;
			if (lineHeight > ScreenHeight())
				wallY = (lineHeight - ScreenHeight()) / 2 * wallYstep;


			for (int y = drawStart; y < drawEnd; y++) {

				//if (!drawnPixels[y * ScreenWidth() + x]) {

					float shading = perpWallDist;
					if (shading > 15) shading = 15;
					shading = 1 - shading / 15;

					color = GetWallTexture(textureNumber)->Sample(wallX, wallY);
					color = DarkColor(color, shading);

					if (side == 1) color = DarkColor(color);

					Draw(x, y, color);		
					drawnPixels[y * ScreenWidth() + x] = true;

				//}
				wallY += wallYstep;
			}

			//SET THE ZBUFFER FOR THE SPRITE CASTING
			ZBuffer[x] = perpWallDist; //perpendicular distance is used
		}




	


		//SPRITE CASTING
		//sort sprites from far to close
		for (int i = 0; i < thingsArray.size(); i++)
		{
			spriteOrder[i] = i;
			spriteDistance[i] = ((player.position.x - thingsArray[i].position.x) * (player.position.x - thingsArray[i].position.x) + (player.position.y - thingsArray[i].position.y) * (player.position.y - thingsArray[i].position.y)); //sqrt not taken, unneeded
		}
		sortSprites(spriteOrder, spriteDistance, thingsArray.size());

		//after sorting the sprites, do the projection and draw them
		for (int i = 0; i < thingsArray.size(); i++)
		{
			if (thingsArray[spriteOrder[i]].enableRender) {


				// Get sprite data

				int thingTexture = thingsArray[spriteOrder[i]].texture;
				int thingTexturePart = thingsArray[spriteOrder[i]].spritePartIndex;

				olc::Sprite* thingSprite = GetDecorationSprite(thingTexture);

				if (thingsArray[spriteOrder[i]].thingType == 0) { // Decoration
					thingSprite = GetDecorationSprite(thingTexture);
				}
				if (thingsArray[spriteOrder[i]].thingType == 1) { // Enemy
					thingSprite = GetEnemySprite(thingTexture);
				}
				if (thingsArray[spriteOrder[i]].thingType == 2) { // Item
					thingSprite = GetItemSprite(thingTexture);
				}


				//translate sprite position to relative to camera
				olc::vf2d spritePosRel = thingsArray[spriteOrder[i]].position - player.position;

				if (spritePosRel.mag2() < 400) { // Check sprite render distance

					//transform sprite with the inverse camera matrix
					// [ planeX   dirX ] -1                                       [ dirY      -dirX ]
					// [               ]       =  1/(planeX*dirY-dirX*planeY) *   [                 ]
					// [ planeY   dirY ]                                          [ -planeY  planeX ]


					float invDet = 1.0 / (player.viewPlane.x * player.direction.y - player.direction.x * player.viewPlane.y); //required for correct matrix multiplication

					float transformX = invDet * (player.direction.y * spritePosRel.x - player.direction.x * spritePosRel.y);
					float transformY = invDet * (-player.viewPlane.y * spritePosRel.x + player.viewPlane.x * spritePosRel.y); //this is actually the depth inside the screen, that what Z is in 3D

					int spriteScreenX = int((ScreenWidth() / 2) * (1 + transformX / transformY));

					int vMoveScreen = player.cameraVertical * 3 / transformY;

					//calculate height of the sprite on screen
					int spriteHeight = abs(int(ScreenHeight() / (transformY))); //using 'transformY' instead of the real distance prevents fisheye
					//calculate lowest and highest pixel to fill in current stripe
					int drawStartY = -spriteHeight / 2 + ScreenHeight() / 2 + vMoveScreen;
					if (drawStartY < 0) drawStartY = 0;
					int drawEndY = spriteHeight / 2 + ScreenHeight() / 2 + vMoveScreen;
					if (drawEndY >= ScreenHeight()) drawEndY = ScreenHeight() - 1;

					//calculate width of the sprite
					int spriteWidth = abs(int(ScreenHeight() / (transformY)));
					int drawStartX = -spriteWidth / 2 + spriteScreenX;
					if (drawStartX < 0) drawStartX = 0;
					int drawEndX = spriteWidth / 2 + spriteScreenX;
					if (drawEndX >= ScreenWidth()) drawEndX = ScreenWidth() - 1;

					//loop through every vertical stripe of the sprite on screen
					for (int stripe = drawStartX; stripe < drawEndX; stripe++)
					{
						int texX = int(256 * (stripe - (-spriteWidth / 2 + spriteScreenX)) * 64 / spriteWidth) / 256;
						//the conditions in the if are:
						//1) it's in front of camera plane so you don't see things behind you
						//2) it's on the screen (left)
						//3) it's on the screen (right)
						//4) ZBuffer, with perpendicular distance
						if (transformY > 0 && stripe > 0 && stripe < ScreenWidth() && transformY < ZBuffer[stripe])
							for (int y = drawStartY; y < drawEndY; y++) //for every pixel of the current stripe
							{
								int d = (y - vMoveScreen) * 256 - ScreenHeight() * 128 + spriteHeight * 128; //256 and 128 factors to avoid floats
								int texY = ((d * 64) / spriteHeight) / 256;

								olc::Pixel color = olc::RED;
								color = thingSprite->GetPixel(texX + (thingTexturePart * 64), texY);

								float shading = spritePosRel.mag2();
								if (shading > 60) shading = 60;
								shading = 1 - shading / 80;


								if (color != olc::CYAN)
								{
									//Draw(stripe, y, color);
									Draw(stripe, y, DarkColor(color, shading)); //paint pixel if it isn't black, black is the invisible color
									drawnPixels[y * ScreenWidth() + stripe] = true;
								}
							}
					}
				}

			}
		}


		// Clear drawn pixels info array
		for (int i = 0; i < ScreenHeight() * ScreenWidth(); i++) drawnPixels[i] = false;
	}




	//===============================================================================================================================
	// INPUT
	//===============================================================================================================================

	void UserControls(float fElapsedTime) {

		if (!isMenuOpen && !isEditorOpened && player.isAlive) {

			// Clear inputs ---------------------------------------------------------

			player.isMoving = false;
			player.isShooting = false;

			player.controlMoveVector = olc::vf2d(0, 0);
			player.controlRotationVector = olc::vf2d(0, 0);



			// Main controls ---------------------------------------------------------

			// Move forward
			if (GetKey(olc::Key::UP).bHeld || GetKey(olc::Key::W).bHeld)
			{
				player.isMoving = true;
				player.controlMoveVector += player.direction;
			}

			// Move backwards
			if (GetKey(olc::Key::DOWN).bHeld || GetKey(olc::Key::S).bHeld)
			{
				player.isMoving = true;
				player.controlMoveVector -= player.direction;
			}

			// Strafe right
			if (GetKey(olc::Key::D).bHeld)
			{
				player.isMoving = true;
				player.controlMoveVector += player.viewPlane;
			}

			// Strafe left
			if (GetKey(olc::Key::A).bHeld)
			{
				player.isMoving = true;
				player.controlMoveVector -= player.viewPlane;
			}



			// Rotate to the right
			if (GetKey(olc::Key::RIGHT).bHeld)
			{
				player.controlRotationVector = olc::vf2d(1, 0);
			}

			// Rotate to the left
			if (GetKey(olc::Key::LEFT).bHeld)
			{
				player.controlRotationVector = olc::vf2d(-1, 0);
			}


			// Shooting
			if (GetKey(olc::Key::CTRL).bHeld || GetMouse(0).bHeld)
			{
				player.isShooting = true;
				player.Shoot();
			}


			// Weapons

			if (GetKey(olc::Key::K1).bPressed)
			{
				player.ChangeWeapon(0);
			}

			if (GetKey(olc::Key::K2).bPressed)
			{
				player.ChangeWeapon(1);
			}

			if (GetKey(olc::Key::K3).bPressed)
			{
				player.ChangeWeapon(2);
			}


			// Interaction

			if (GetKey(olc::Key::SPACE).bPressed || GetKey(olc::Key::E).bPressed)
			{
				player.Interact();
			}

		}



		if (!player.isAlive) {
			if (GetKey(olc::Key::SPACE).bPressed || GetKey(olc::Key::E).bPressed)
			{
				RestartLevel();
			}
		}


		// Mouse controls ------------------------------------------------------

		if (IsFocused() && !isMenuOpen && !isEditorOpened) {
			if (GetWindowMouse().x != 0 && GetWindowMouse().y != 0) { // To prevent cursor set while dragging window

				float deltaMouseX = 0.05 * mouseSensitivity * (GetWindowMouse().x - GetWindowSize().x / 2) / (fElapsedTime / 0.016);
				player.controlRotationVector += olc::vf2d(deltaMouseX, 0);
				SetCursorPos(GetWindowPosition().x + GetWindowSize().x / 2, GetWindowPosition().y + GetWindowSize().y / 2);
			}
		}
		else if (!IsFocused() && gameState == GameState::STATE_GAMEPLAY){
			gameState = STATE_MENU;
		}



	}



	//===============================================================================================================================
	// MAP EDITOR 
	//===============================================================================================================================

	bool isEditorOpened = false;
	bool isEditorInPlayMode = false;


	olc::vi2d gridShift {0,0};
	olc::vi2d gridOrigin {0,0};

	int editorCellSize = 20;
	bool showGrid = true;


	olc::vf2d selectedCell{ 0,0 };
	
	
	// Controls
	bool mouseOnUI = false;
	olc::vi2d mousePosPrev{ GetMouseX(), GetMouseY() };


	// Tools
	int toolSelected = 0;
	bool eraserSelected = false;
	bool playerPosSelected = false;
	bool levelEndPosSelected = false;


	int selectedToolWall = 0;
	int selectedToolDecor = 0;
	int selectedToolItem = 0;
	int selectedToolEnemy = 0;


	// Windows
	bool showToolSelection = false;
	int toolSelectionPage = 0;
	bool showSettings = false;
	bool showOpenFile = false;
	bool showExitDialog = false;

	// Infolog
	std::string editor_infoLog = "";
	float editor_infoLogTimer = 0;




	void Editor_PlaceDecoration() {
		Decoration newDecoration = Decoration(this, newId, selectedToolDecor, olc::vf2d(selectedCell.x + 0.5, selectedCell.y + 0.5)); newId++;
		decorationsArray.push_back(newDecoration);
		thingsArray.push_back(newDecoration.ToThing());
		spriteOrder = new int[thingsArray.size()];
		spriteDistance = new float[thingsArray.size()];
	}

	void Editor_PlaceItem() {
		Item newItem = Item(this, newId, selectedToolItem, olc::vf2d(selectedCell.x, selectedCell.y)); newId++;
		itemsArray.push_back(newItem);
		thingsArray.push_back(newItem.ToThing());
		spriteOrder = new int[thingsArray.size()];
		spriteDistance = new float[thingsArray.size()];
	}

	void Editor_PlaceEnemy() {
		Enemy newEnemy = Enemy(this, newId, selectedToolEnemy, olc::vf2d(selectedCell.x, selectedCell.y)); newId++;
		enemiesArray.push_back(newEnemy);
		thingsArray.push_back(newEnemy.ToThing());
		spriteOrder = new int[thingsArray.size()];
		spriteDistance = new float[thingsArray.size()];
	}



	void Editor_EraceDecorations() {

		for (int i = 0; i < decorationsArray.size(); i++) {
			if
			(
				decorationsArray[i].position.x >= selectedCell.x &&
				decorationsArray[i].position.x <= selectedCell.x + 1 &&
				decorationsArray[i].position.y >= selectedCell.y &&
				decorationsArray[i].position.y <= selectedCell.y + 1
			)
			{
				int decorId = decorationsArray[i].id;
				std::cout << "Decor to erase : ID : " << decorId << std::endl;

				for (int t = 0; t < thingsArray.size(); t++) {
					if (thingsArray[t].id == decorId) {
						std::swap(thingsArray[t], thingsArray[thingsArray.size() - 1]);
						thingsArray.pop_back();
					}
				}			

				std::swap(decorationsArray[i], decorationsArray[decorationsArray.size() - 1]);
				decorationsArray.pop_back();


				editor_infoLog = "Eraced Decor ID : " + std::to_string(decorId);
				editor_infoLogTimer = 1;
				return;
			}
		}
	}

	void Editor_EraceItems() {

		for (int i = 0; i < itemsArray.size(); i++) {
			if
			(
				itemsArray[i].position.x >= selectedCell.x &&
				itemsArray[i].position.x <= selectedCell.x + 1 &&
				itemsArray[i].position.y >= selectedCell.y &&
				itemsArray[i].position.y <= selectedCell.y + 1
			)
			{
				int itemId = itemsArray[i].id;
				std::cout << "Item to erase : ID : " << itemId << std::endl;

				for (int t = 0; t < thingsArray.size(); t++) {
					if (thingsArray[t].id == itemId) {
						std::swap(thingsArray[t], thingsArray[thingsArray.size() - 1]);
						thingsArray.pop_back();
					}
				}

				std::swap(itemsArray[i], itemsArray[itemsArray.size() - 1]);
				itemsArray.pop_back();

				editor_infoLog = "Eraced Item ID : " + std::to_string(itemId);
				editor_infoLogTimer = 1;
				return;
			}
		}
	}

	void Editor_EraceEnemies() {

		for (int i = 0; i < enemiesArray.size(); i++) {
			if
			(
				enemiesArray[i].position.x >= selectedCell.x &&
				enemiesArray[i].position.x <= selectedCell.x + 1 &&
				enemiesArray[i].position.y >= selectedCell.y &&
				enemiesArray[i].position.y <= selectedCell.y + 1
			)
			{
				int enemyId = enemiesArray[i].id;
				std::cout << "Enemy to erase : ID : " << enemyId << std::endl;

				for (int t = 0; t < thingsArray.size(); t++) {
					if (thingsArray[t].id == enemyId) {
						std::swap(thingsArray[t], thingsArray[thingsArray.size() - 1]);
						thingsArray.pop_back();
					}
				}

				std::swap(enemiesArray[i], enemiesArray[enemiesArray.size() - 1]);
				enemiesArray.pop_back();

				editor_infoLog = "Eraced Enemy ID : " + std::to_string(enemyId);
				editor_infoLogTimer = 1;
				return;
			}
		}
	}



	void Editor_SelectTool(int idx) {
		switch (toolSelected)
		{
		case 0:
			selectedToolWall = idx;
			break;
		case 1:
			selectedToolDecor = idx;

			break;
		case 2:
			selectedToolItem = idx;
			break;
		case 3:
			selectedToolEnemy = idx;
			break;
		default:
			break;
		}
	}



	void Editor_Controls(float fElapsedTime) {

		// Mouse movement
		if (GetMouse(1).bHeld) {
			gridShift.x = (gridShift.x + (GetMouseX() - mousePosPrev.x)) % 20;
			gridOrigin.x = gridOrigin.x + (GetMouseX() - mousePosPrev.x);

			gridShift.y = (gridShift.y + (GetMouseY() - mousePosPrev.y)) % 20;
			gridOrigin.y = gridOrigin.y + (GetMouseY() - mousePosPrev.y);
		}
		mousePosPrev = olc::vi2d(GetMouseX(), GetMouseY());


		// Arrow control
		if (GetKey(olc::Key::LEFT).bHeld || GetKey(olc::Key::A).bHeld) {
			gridShift.x = (gridShift.x + int(10 * (fElapsedTime / 0.016))) % 20;
			gridOrigin.x = gridOrigin.x + int(10 * (fElapsedTime / 0.016));
		}
		if (GetKey(olc::Key::RIGHT).bHeld || GetKey(olc::Key::D).bHeld) {
			gridShift.x = (gridShift.x - int(10 * (fElapsedTime / 0.016))) % 20;
			gridOrigin.x = gridOrigin.x - int(10 * (fElapsedTime / 0.016));
		}
		if (GetKey(olc::Key::UP).bHeld || GetKey(olc::Key::W).bHeld) {
			gridShift.y = (gridShift.y + int(10 * (fElapsedTime / 0.016))) % 20;
			gridOrigin.y = gridOrigin.y + int(10 * (fElapsedTime / 0.016));
		}
		if (GetKey(olc::Key::DOWN).bHeld || GetKey(olc::Key::S).bHeld) {
			gridShift.y = (gridShift.y - int(10 * (fElapsedTime / 0.016))) % 20;
			gridOrigin.y = gridOrigin.y - int(10 * (fElapsedTime / 0.016));
		}



		if (GetKey(olc::Key::NP_ADD).bPressed) {
			editorCellSize = 20;
		}
		if (GetKey(olc::Key::NP_SUB).bPressed) {
			editorCellSize = 10;
		}
		if (GetKey(olc::Key::Z).bPressed) {
			editorCellSize = editorCellSize % 20 + 10;
		}
		if (GetKey(olc::Key::G).bPressed)
		{
			showGrid = !showGrid;
		}
		if (GetKey(olc::Key::E).bPressed)
		{
			playerPosSelected = false;
			levelEndPosSelected = false;
			eraserSelected = !eraserSelected;
		}
		if (GetKey(olc::Key::P).bPressed)
		{
			eraserSelected = false;
			levelEndPosSelected = false;
			playerPosSelected = !playerPosSelected;		
		}



		// Place wall

		if (toolSelected == 0 && !eraserSelected && !playerPosSelected && !levelEndPosSelected && GetMouse(0).bHeld) {
			if (selectedCell.x >= 0 && selectedCell.y >= 0 && selectedCell.x < worldMapWidth && selectedCell.y < worldMapHeight) { // Check map boundaries
				worldMap[selectedCell.y * worldMapWidth + selectedCell.x] = selectedToolWall + 1;
			}
		}

		// Place object

		if (toolSelected != 0 && !eraserSelected && !playerPosSelected && !levelEndPosSelected && GetMouse(0).bPressed) {
			if (selectedCell.x >= 0 && selectedCell.y >= 0 && selectedCell.x < worldMapWidth && selectedCell.y < worldMapHeight) { // Check map boundaries
				if (toolSelected == 1) { // Decor
					Editor_PlaceDecoration();
				}
				else if (toolSelected == 2) { // Item
					Editor_PlaceItem();
				}
				else if (toolSelected == 3) { // Enemy
					Editor_PlaceEnemy();
				}

			}
		}

		// Erace

		if (eraserSelected && !playerPosSelected && !levelEndPosSelected && GetMouse(0).bHeld) {
			if (selectedCell.x >= 0 && selectedCell.y >= 0 && selectedCell.x < worldMapWidth && selectedCell.y < worldMapHeight) { // Check map boundaries

				if (toolSelected == 0) { // Wall
					worldMap[selectedCell.y * worldMapWidth + selectedCell.x] = 0;
				}
				else if (toolSelected == 1) { // Decor
					Editor_EraceDecorations();
				}
				else if (toolSelected == 2) { // Item
					Editor_EraceItems();
				}
				else if (toolSelected == 3) { // Enemy
					Editor_EraceEnemies();
				}
			}
		}


		// Place player

		if (playerPosSelected && !eraserSelected && !levelEndPosSelected && GetMouse(0).bPressed) {
			if (selectedCell.x >= 0 && selectedCell.y >= 0 && selectedCell.x < worldMapWidth && selectedCell.y < worldMapHeight) { // Check map boundaries
				player.position = olc::vf2d(selectedCell.x + 0.5, selectedCell.y + 0.5);
			}
		}


		// Place End level

		if (levelEndPosSelected && !eraserSelected && !playerPosSelected && GetMouse(0).bPressed) {
			if (selectedCell.x >= 0 && selectedCell.y >= 0 && selectedCell.x < worldMapWidth && selectedCell.y < worldMapHeight) { // Check map boundaries
				for (int intWall = 0; intWall < interactbleWallsArray.size(); intWall++) {
					if (interactbleWallsArray[intWall].interactionType == InteractibleWall::InteractionType::ENDLEVEL) {
						interactbleWallsArray[intWall].wallPosition = olc::vf2d(selectedCell.x, selectedCell.y);
					}
				}
			}
		}
	
	}



	void Editor_ShowToolSelection() {

		mouseOnUI = true; // To prevent placing and moving in background

		FillRect(40, 40, 273, 148, olc::BLACK); // Background
		FillRect(195, 30, 75, 10, olc::BLACK); // Connection
		DrawLine(195, 0, 195, 40, olc::YELLOW);
		DrawLine(270, 0, 270, 40, olc::YELLOW);
		DrawLine(40, 40, 195, 40, olc::YELLOW);
		DrawLine(270, 40, 313, 40, olc::YELLOW);


		int wallPageCount = ceil(wallSprites.size() / 21);
		int decorPageCount = ceil(3 / 21);
		int itemPageCount = ceil(itemIconSprites.size() / 21);
		int enemyPageCount = ceil(enemyIconSprites.size() / 21);


		for (int toolY = 0; toolY < 3; toolY++) {
			for (int toolX = 0; toolX < 7; toolX++) {

				if (toolSelected == 0 && (toolX + toolY * 7) > wallSprites.size() - 1) break; // Walls
				if (toolSelected == 1 && (toolX + toolY * 7) > 3) break; // Decor
				if (toolSelected == 2 && (toolX + toolY * 7) > itemIconSprites.size() - 1) break; // Item
				if (toolSelected == 3 && (toolX + toolY * 7) > enemyIconSprites.size() - 1 ) break; // Enemy



				Button selectToolButton = Button(this);
				int textureIdx = 0;
				switch (toolSelected)
				{
				case 0: // Wall
					textureIdx = ((toolX + toolY * 7) + 21*toolSelectionPage);
					selectToolButton = Button(this, olc::vi2d(45 + (toolX * (34 + 4)), 45 + (toolY * (34 + 4))), 34, 34, GetWallTexture(textureIdx));
					break;
				case 1: // Decorations
					textureIdx = (toolX + toolY * 7) + 21 * toolSelectionPage;
					selectToolButton = Button(this, olc::vi2d(45 + (toolX * (34 + 4)), 45 + (toolY * (34 + 4))), 34, 34, GetDecorationSprite(textureIdx));
					break;
				case 2: // Items
					textureIdx = (toolX + toolY * 7) + 21 * toolSelectionPage;
					selectToolButton = Button(this, olc::vi2d(45 + (toolX * (34 + 4)), 45 + (toolY * (34 + 4))), 34, 34, itemIconSprites[textureIdx]);
					break;
				case 3: // Enemies
					textureIdx = (toolX + toolY * 7) + 21 * toolSelectionPage;
					selectToolButton = Button(this, olc::vi2d(45 + (toolX * (34 + 4)), 45 + (toolY * (34 + 4))), 34, 34, enemyIconSprites[textureIdx]);
					break;
				default:
					break;
				}
				selectToolButton.Update();
				if (selectToolButton.isHovered) { mouseOnUI = true; }
				if (selectToolButton.isPressed) { Editor_SelectTool(textureIdx); showToolSelection = false; }
			}
		}


		Button selectToolNextPageButton = Button(this, olc::vi2d(267, 160), 40, 20, "NEXT");
		selectToolNextPageButton.Update();
		if (selectToolNextPageButton.isHovered) { mouseOnUI = true; }
		if (selectToolNextPageButton.isPressed) { 


			if (toolSelected == 0 && toolSelectionPage < wallPageCount-1) toolSelectionPage++;  // Walls
			if (toolSelected == 1 && toolSelectionPage < decorPageCount - 1) toolSelectionPage++; // Decor
			if (toolSelected == 2 && toolSelectionPage < itemPageCount - 1) toolSelectionPage++; // Item
			if (toolSelected == 3 && toolSelectionPage < enemyPageCount - 1) toolSelectionPage++; // Enemy
		
		}

		Button selectToolPrevPageButton = Button(this, olc::vi2d(45, 160), 40, 20, "PREV");
		selectToolPrevPageButton.Update();
		if (selectToolPrevPageButton.isHovered) { mouseOnUI = true; }
		if (selectToolPrevPageButton.isPressed) { 
			if (toolSelectionPage > 0) toolSelectionPage--;
		}


		// Ooooh, this is bad...... But i dont have other solution at this moment :(
		if (!(GetMouseX() > 40 && GetMouseX() < 313 && GetMouseY() > 40 && GetMouseY() < 182) && !(GetMouseX() > 195 && GetMouseX() < 270 && GetMouseY() > 0 && GetMouseY() < 40) && GetMouse(0).bPressed) {
			showToolSelection = false;
		}

	}



	InputField mapTitleInputField = InputField(this, olc::vi2d(35, 60), 170, 15, "DEFAULT");
	InputField mapSaveFileNameInputField = InputField(this, olc::vi2d(35, 95), 170, 15, "DEFAULT");
	InputField mapSizeXInputField = InputField(this, olc::vi2d(225, 60), 50, 15, "20");
	InputField mapSizeYInputField = InputField(this, olc::vi2d(225, 95), 50, 15, "20");
	Button mapApplySizeButton = Button(this, olc::vi2d(220, 150), 60, 20, "APPLY");
	InputField nextMapFileInputField = InputField(this, olc::vi2d(35, 130), 170, 15, "");

	void Editor_ShowSettings() {

		mouseOnUI = true; // To prevent placing and moving in background

		FillRect(20, 40, 273, 142, olc::BLACK); // Background
		FillRect(48, 30, 30, 10, olc::BLACK); // Connection
		DrawLine(48, 0, 48, 40, olc::YELLOW);
		DrawLine(78, 0, 78, 40, olc::YELLOW);
		DrawLine(20, 40, 48, 40, olc::YELLOW);
		DrawLine(78, 40, 292, 40, olc::YELLOW);



		DrawString(olc::vi2d(35, 50), "MAP TITLE");
		mapTitleInputField.Update();
		if (mapTitleInputField.isHovered) { mouseOnUI = true; }
		if (mapTitleInputField.isPressed) { mapTitleInputField.isFocused = true; }

		DrawString(olc::vi2d(35, 85), "SAVE FILE NAME");
		mapSaveFileNameInputField.Update();
		if (mapSaveFileNameInputField.isHovered) { mouseOnUI = true; }
		if (mapSaveFileNameInputField.isPressed) { mapSaveFileNameInputField.isFocused = true; }

		DrawString(olc::vi2d(225, 50), "SIZE X");
		mapSizeXInputField.allowOnlyNumbers = true;
		mapSizeXInputField.charMaximum = 4;
		mapSizeXInputField.Update();
		if (mapSizeXInputField.isHovered) { mouseOnUI = true; }
		if (mapSizeXInputField.isPressed) { mapSizeXInputField.isFocused = true; }

		DrawString(olc::vi2d(225, 85), "SIZE Y");
		mapSizeYInputField.allowOnlyNumbers = true;
		mapSizeYInputField.charMaximum = 4;
		mapSizeYInputField.Update();
		if (mapSizeYInputField.isHovered) { mouseOnUI = true; }
		if (mapSizeYInputField.isPressed) { mapSizeYInputField.isFocused = true; }


		DrawString(olc::vi2d(35, 120), "NEXT MAP FILE NAME");
		nextMapFileInputField.Update();
		if (nextMapFileInputField.isHovered) { mouseOnUI = true; }
		if (nextMapFileInputField.isPressed) { nextMapFileInputField.isFocused = true; }


		mapApplySizeButton.Update();
		mapApplySizeButton.colorBackground = olc::DARK_GREEN;
		if (mapApplySizeButton.isHovered) { mouseOnUI = true; }
		if (mapApplySizeButton.isPressed) 
		{
			// Check minimal map size
			int sizeX = std::stoi(mapSizeXInputField.text);
			int sizeY = std::stoi(mapSizeYInputField.text);
			if (sizeX < 5) {
				sizeX = 5;
				mapSizeXInputField.text = "5";
			}
			if (sizeY < 5) {
				sizeY = 5;
				mapSizeXInputField.text = "5";
			}

			// Change map size
			ChangeMapSize(sizeX, sizeY);
			mapSizeXInputField.text = std::to_string(worldMapWidth);
			mapSizeYInputField.text = std::to_string(worldMapHeight);


			// Map files
			worldMapName = mapTitleInputField.text;
			worldMapFile = mapSaveFileNameInputField.text;
			worldMapNextMapFile = nextMapFileInputField.text;

			// Check if player stay on map
			if (player.position.x >= worldMapWidth || player.position.y >= worldMapHeight) {
				player.position = olc::vf2d(1.5,  1.5);
				editor_infoLog = "Player moved to (1, 1)";
				editor_infoLogTimer = 2;
			}
		}



		// Ooooh, this is bad...... But i dont have other solution at this moment :(
		if (!(GetMouseX() > 20 && GetMouseX() < 292 && GetMouseY() > 40 && GetMouseY() < 182) && !(GetMouseX() > 48 && GetMouseX() < 78 && GetMouseY() > 0 && GetMouseY() < 40) && GetMouse(0).bPressed) {
			showSettings = false;
		}
	}




	InputField mapOpenFileInputField = InputField(this, olc::vi2d(35, 60), 170, 15, "");
	Button mapLoadMapButton = Button(this, olc::vi2d(145, 88), 60, 20, "LOAD");
	Button mapCloseOpenFileButton = Button(this, olc::vi2d(34, 88), 60, 20, "CLOSE");

	void Editor_ShowOpenFile() {

		mouseOnUI = true; // To prevent placing and moving in background

		FillRect(20, 40, 200, 80, olc::BLACK); // Background
		DrawLine(20, 40, 219, 40, olc::YELLOW);

		DrawString(olc::vi2d(35, 50), "FILE NAME TO OPEN");
		mapOpenFileInputField.Update();
		if (mapOpenFileInputField.isHovered) { mouseOnUI = true; }
		if (mapOpenFileInputField.isPressed) { mapOpenFileInputField.isFocused = true; }


		mapLoadMapButton.Update();
		mapLoadMapButton.colorBackground = olc::DARK_GREEN;
		if (mapLoadMapButton.isHovered) { mouseOnUI = true; }
		if (mapLoadMapButton.isPressed)
		{
			worldMapFile = mapOpenFileInputField.text;
			std::string loadFile = "maps/" + worldMapFile + ".map";
			LoadMap(loadFile);
			mapSizeXInputField.text = std::to_string(worldMapWidth);
			mapSizeYInputField.text = std::to_string(worldMapHeight);
			mapTitleInputField.text = worldMapName;
			mapSaveFileNameInputField.text = worldMapFile;
			nextMapFileInputField.text = worldMapNextMapFile;
		}

		mapCloseOpenFileButton.Update();
		mapCloseOpenFileButton.colorBackground = olc::DARK_RED;
		if (mapCloseOpenFileButton.isHovered) { mouseOnUI = true; }
		if (mapCloseOpenFileButton.isPressed)
		{
			showOpenFile = false;
		}

	}


	Button mapExitEditorButton = Button(this, olc::vi2d(145, 88), 60, 20, "YES");
	Button mapCloseExitDialogButton = Button(this, olc::vi2d(34, 88), 60, 20, "NO");

	void Editor_ShowExitDialog() {

		mouseOnUI = true; // To prevent placing and moving in background

		FillRect(20, 40, 200, 80, olc::BLACK); // Background
		DrawLine(20, 40, 219, 40, olc::YELLOW);

		DrawString(olc::vi2d(35, 50), "EXIT EDITOR ?");

		mapExitEditorButton.Update();
		mapExitEditorButton.colorBackground = olc::DARK_GREEN;
		if (mapExitEditorButton.isHovered) { mouseOnUI = true; }
		if (mapExitEditorButton.isPressed)
		{
			showExitDialog = false;
			gameState = GameState::STATE_TITLESCREEN;
		}

		mapCloseExitDialogButton.Update();
		mapCloseExitDialogButton.colorBackground = olc::DARK_RED;
		if (mapCloseExitDialogButton.isHovered) { mouseOnUI = true; }
		if (mapCloseExitDialogButton.isPressed)
		{
			LoadDefaultMap();
			showExitDialog = false;
		}

	}


	void Editor_DrawEditor(float fElapsedTime) {

		
		mouseOnUI = false;




		// Background -----------------------

		FillRect(0, 0, ScreenWidth(), ScreenHeight(), olc::VERY_DARK_GREY);


		// Draw map -----------------------

		int sampleSize = editorCellSize - 1 + !showGrid; // Little optimisation

		for (int i = 0; i < worldMapWidth; i++) {
			for (int j = 0; j < worldMapHeight; j++) {

				if (worldMap[j * worldMapWidth + i] != 0) {
					for (int smplX = 0; smplX < sampleSize; smplX++) {
						for (int smplY = 0; smplY < sampleSize; smplY++) {
							olc::Pixel color = GetWallTexture(worldMap[j * worldMapWidth + i] - 1)->GetPixel(int(64.0 / editorCellSize * smplX), int(64.0 / editorCellSize * smplY));
							Draw(gridOrigin.x + 1 + smplX + i * editorCellSize, gridOrigin.y + 1 + smplY + j * editorCellSize, color);
						}
					}
				}
			}
		}



		// Grid -----------------------

		if (showGrid) {
			// Vertical lines
			for (int i = 0; i < int(ScreenWidth() / editorCellSize) + 2; i++) {
				DrawLine(i * editorCellSize + gridShift.x, 0, i * editorCellSize + gridShift.x, ScreenHeight(), olc::GREY);
			}
			// Horisontal lines
			for (int j = 0; j < int(ScreenHeight() / editorCellSize) + 2; j++) {
				DrawLine(0, j * editorCellSize + gridShift.y, ScreenWidth(), j * editorCellSize + gridShift.y, olc::GREY);
			}


			// Draw grid origin
			DrawLine(gridOrigin.x, 0, gridOrigin.x, ScreenHeight(), olc::YELLOW);
			DrawLine(0, gridOrigin.y, ScreenWidth(), gridOrigin.y, olc::YELLOW);

			// Draw end of map
			DrawLine(gridOrigin.x + worldMapWidth * editorCellSize, 0, gridOrigin.x + worldMapWidth * editorCellSize, ScreenHeight(), olc::RED);
			DrawLine(0, gridOrigin.y + worldMapHeight * editorCellSize, ScreenWidth(), gridOrigin.y + worldMapHeight * editorCellSize, olc::RED);
		}



		// Draw objects -----------------------


		//
		// Note : decorationsArray[d].position.x - 0.5 in fact tells that i need to shift sprite half size
		//

		// Draw Decorations

		for (int d = 0; d < decorationsArray.size(); d++) {

			for (int smplX = 0; smplX < editorCellSize - 1; smplX++) {
				for (int smplY = 0; smplY < editorCellSize - 1; smplY++) {
					olc::Pixel color = GetDecorationSprite(decorationsArray[d].texture)->GetPixel(int(64.0 / editorCellSize * smplX), int(64.0 / editorCellSize * smplY));
					if (color != olc::CYAN) {
						int pixelPosX = gridOrigin.x + 1 + smplX + (decorationsArray[d].position.x - 0.5) * editorCellSize;
						int pixelPosY = gridOrigin.y + 1 + smplY + (decorationsArray[d].position.y - 0.5) * editorCellSize;
						Draw(pixelPosX, pixelPosY, color);
					}
				}
			}
		}

		// Draw Items

		for (int i = 0; i < itemsArray.size(); i++) {

			for (int smplX = 0; smplX < editorCellSize - 1; smplX++) {
				for (int smplY = 0; smplY < editorCellSize - 1; smplY++) {
					olc::Pixel color = itemIconSprites[itemsArray[i].texture]->GetPixel(floor((float)itemIconSprites[itemsArray[i].texture]->width / editorCellSize * smplX), floor((float)itemIconSprites[itemsArray[i].texture]->height / editorCellSize * smplY));
					if (color != olc::CYAN) {
						int pixelPosX = gridOrigin.x + 1 + smplX + (itemsArray[i].position.x - 0.5) * editorCellSize;
						int pixelPosY = gridOrigin.y + 1 + smplY + (itemsArray[i].position.y - 0.5) * editorCellSize;
						Draw(pixelPosX, pixelPosY, color);
					}
				}
			}
		}

		// Draw Enemies

		for (int d = 0; d < enemiesArray.size(); d++) {

			for (int smplX = 0; smplX < editorCellSize - 1; smplX++) {
				for (int smplY = 0; smplY < editorCellSize - 1; smplY++) {
					olc::Pixel color = enemyIconSprites[enemiesArray[d].texture]->GetPixel(int((float)enemyIconSprites[enemiesArray[d].texture]->width / editorCellSize * smplX), int((float)enemyIconSprites[enemiesArray[d].texture]->height / editorCellSize * smplY));
					if (color != olc::CYAN) {
						int pixelPosX = gridOrigin.x + 1 + smplX + (enemiesArray[d].position.x - 0.5) * editorCellSize;
						int pixelPosY = gridOrigin.y + 1 + smplY + (enemiesArray[d].position.y - 0.5) * editorCellSize;
						Draw(pixelPosX, pixelPosY, color);
					}
				}
			}
		}

		// Draw Player

		for (int smplX = 0; smplX < editorCellSize - 1; smplX++) {
			for (int smplY = 0; smplY < editorCellSize - 1; smplY++) {
				olc::Pixel color = spriteEditorToolPlayer.GetPixel(int((float)spriteEditorToolPlayer.width / editorCellSize * smplX), int((float)spriteEditorToolPlayer.height / editorCellSize * smplY));
				if (color != olc::CYAN) {
					int pixelPosX = gridOrigin.x + smplX + (player.position.x - 0.5) * editorCellSize;
					int pixelPosY = gridOrigin.y + smplY + (player.position.y - 0.5) * editorCellSize;
					Draw(pixelPosX, pixelPosY, color);
				}
			}
		}


		// Draw End level

		for (int intWall = 0; intWall < interactbleWallsArray.size(); intWall++) {
			if (interactbleWallsArray[intWall].interactionType == InteractibleWall::InteractionType::ENDLEVEL) {
				for (int smplX = 0; smplX < editorCellSize - 1; smplX++) {
					for (int smplY = 0; smplY < editorCellSize - 1; smplY++) {
						olc::Pixel color = spriteEditorToolEndLevel.GetPixel(int((float)spriteEditorToolEndLevel.width / editorCellSize * smplX), int((float)spriteEditorToolEndLevel.height / editorCellSize * smplY));
						if (color != olc::CYAN) {
							int pixelPosX = gridOrigin.x + smplX + (interactbleWallsArray[intWall].wallPosition.x) * editorCellSize;
							int pixelPosY = gridOrigin.y + smplY + (interactbleWallsArray[intWall].wallPosition.y) * editorCellSize;
							Draw(pixelPosX, pixelPosY, color);
						}
					}
				}
			}
		}



		// Cell selection ----------------------- 

		selectedCell.x = (GetMouseX() - gridOrigin.x) / (editorCellSize * 1.0);
		selectedCell.y = (GetMouseY() - gridOrigin.y) / (editorCellSize * 1.0);


		if (toolSelected == 0 || toolSelected == 1 || eraserSelected) { // Wall or Decoration

			selectedCell.x = floor(selectedCell.x);
			selectedCell.y = floor(selectedCell.y);

			// Rectangle
			DrawRect(gridOrigin.x + selectedCell.x * editorCellSize, gridOrigin.y + selectedCell.y * editorCellSize, editorCellSize, editorCellSize, olc::RED);

			std::string cellXText = std::to_string(selectedCell.x);
			std::string cellYText = std::to_string(selectedCell.y);
			std::string cellText = cellXText.substr(0, cellXText.length() - 5) + " " + cellYText.substr(0, cellYText.length() - 5);
			DrawString(5, ScreenHeight() - 10, cellText);
		}
		else if (toolSelected == 2 || toolSelected == 3) { // Items or Enemy

			selectedCell.x = (floor(selectedCell.x * 2)) / 2; // Round to the nearest 0.5 
			selectedCell.y = (floor(selectedCell.y * 2)) / 2;

			// Dot
			FillRect(gridOrigin.x + selectedCell.x * editorCellSize - 3, gridOrigin.y + selectedCell.y * editorCellSize - 3, 7, 7, olc::RED);

			std::string cellXText = std::to_string(selectedCell.x);
			std::string cellYText = std::to_string(selectedCell.y);
			std::string cellText = cellXText.substr(0, cellXText.length() - 5) + " " + cellYText.substr(0, cellYText.length() - 5);
			DrawString(5, ScreenHeight() - 10, cellText);
		}





		// UI -----------------------

		// Hovering text
		std::string hoverText = "";

		// Debug
		//int debugMX = GetMouseX();
		//int debugMY = GetMouseY();
		//std::string debugMText = std::to_string(debugMX) + " " + std::to_string(debugMY);
		//DrawString(200, 180, debugMText, olc::WHITE);


		// Upper menu background
		FillRect(0, 0, 320, 30, olc::BLACK);





		// Save button
		Button saveButton = Button(this, olc::vi2d(3, 3), 22, 22, &spriteEditorSave);
		saveButton.showBorder = false;
		saveButton.hoverText = "SAVE";
		saveButton.Update();
		if (saveButton.isHovered) { mouseOnUI = true; hoverText = saveButton.hoverText; }
		if (saveButton.isPressed) 
		{
			worldMapName = mapTitleInputField.text;
			worldMapFile = mapSaveFileNameInputField.text;
			std::string saveFile = "maps/" + worldMapFile + ".map";
			SaveMap(saveFile);

			editor_infoLogTimer = 2;
			editor_infoLog = "Map saved as : " + saveFile;
		}

		// Load button
		Button loadButton = Button(this, olc::vi2d(28, 3), 22, 22, &spriteEditorLoad);
		loadButton.showBorder = false;
		loadButton.hoverText = "LOAD";
		loadButton.Update();
		if (loadButton.isHovered) { mouseOnUI = true; hoverText = loadButton.hoverText; }
		if (loadButton.isPressed) {
			showToolSelection = false;
			showSettings = false;
			showOpenFile = true;
		}

		// Settings button
		Button settingsButton = Button(this, olc::vi2d(53, 3), 22, 22, &spriteEditorSettings);
		settingsButton.showBorder = false;
		settingsButton.hoverText = "SETTINGS";
		settingsButton.Update();
		if (settingsButton.isHovered) { mouseOnUI = true; hoverText = settingsButton.hoverText; }
		if (settingsButton.isPressed) {
			showSettings = !showSettings;
			showToolSelection = false;
			showOpenFile = false;

			mapSizeXInputField.text = std::to_string(worldMapWidth);
			mapSizeYInputField.text = std::to_string(worldMapHeight);
			mapTitleInputField.text = worldMapName;
			mapSaveFileNameInputField.text = worldMapFile;
			nextMapFileInputField.text = worldMapNextMapFile;
		}

		// Play button
		Button playButton = Button(this, olc::vi2d(279, 0), 30, 30, &spriteEditorPlay);
		playButton.showBorder = false;
		playButton.hoverText = "PLAY";
		playButton.Update();
		if (playButton.isHovered) { mouseOnUI = true; hoverText = playButton.hoverText; }
		if (playButton.isPressed) 
		{
			// Save
			worldMapName = mapTitleInputField.text;
			worldMapFile = mapSaveFileNameInputField.text;
			std::string saveFile = "maps/" + worldMapFile + ".map";
			SaveMap(saveFile);

			editor_infoLogTimer = 2;
			editor_infoLog = "Map saved as : " + saveFile;

			// Load
			isEditorInPlayMode = true;
			std::string loadFile = "maps/" + worldMapFile + ".map";
			LoadMap(loadFile);

			// Start MIDI
			PlayMapMIDI();

			// Play
			player.Ressurect();
			gameState = STATE_GAMEPLAY;
		}




		// Tool Wall button
		Button toolWallButton = Button(this, olc::vi2d(90, 5), 20, 20, &spriteEditorToolWall);
		toolWallButton.showBorder = true;
		toolWallButton.hoverText = "WALL";
		toolWallButton.Update();
		if (toolWallButton.isHovered) { mouseOnUI = true; hoverText = toolWallButton.hoverText; }
		if (toolWallButton.isPressed) { toolSelected = 0; toolSelectionPage = 0; }

		// Tool Decorations button
		Button toolDecorButton = Button(this, olc::vi2d(115, 5), 20, 20, &spriteEditorToolDecor);
		toolDecorButton.showBorder = true;
		toolDecorButton.hoverText = "DECOR";
		toolDecorButton.Update();
		if (toolDecorButton.isHovered) { mouseOnUI = true; hoverText = toolDecorButton.hoverText; }
		if (toolDecorButton.isPressed) { toolSelected = 1; toolSelectionPage = 0; }

		// Tool Items button
		Button toolItemButton = Button(this, olc::vi2d(140, 5), 20, 20, &spriteEditorToolItem);
		toolItemButton.showBorder = true;
		toolItemButton.hoverText = "ITEM";
		toolItemButton.Update();
		if (toolItemButton.isHovered) { mouseOnUI = true; hoverText = toolItemButton.hoverText; }
		if (toolItemButton.isPressed) { toolSelected = 2; toolSelectionPage = 0; }

		// Tool Enemies button
		Button toolEnemyButton = Button(this, olc::vi2d(165, 5), 20, 20, &spriteEditorToolEnemy);
		toolEnemyButton.showBorder = true;
		toolEnemyButton.hoverText = "ENEMY";
		toolEnemyButton.Update();
		if (toolEnemyButton.isHovered) { mouseOnUI = true; hoverText = toolEnemyButton.hoverText; }
		if (toolEnemyButton.isPressed) { toolSelected = 3; toolSelectionPage = 0; }




		// Tool Eraser button
		Button toolEraserButton = Button(this, olc::vi2d(3, 35), 20, 20, &spriteEditorToolEracer);
		toolEraserButton.showBorder = true;
		toolEraserButton.hoverText = "ERASER";
		toolEraserButton.Update();
		if (toolEraserButton.isHovered) { mouseOnUI = true; hoverText = toolEraserButton.hoverText; }
		if (toolEraserButton.isPressed) { playerPosSelected = false; levelEndPosSelected = false;  eraserSelected = !eraserSelected; }

		// Tool Zoom button
		Button toolZoomButton = Button(this, olc::vi2d(3, 60), 20, 20, &spriteEditorToolZoom);
		toolZoomButton.showBorder = true;
		toolZoomButton.hoverText = "ZOOM";
		toolZoomButton.Update();
		if (toolZoomButton.isHovered) { mouseOnUI = true; hoverText = toolZoomButton.hoverText; }
		if (toolZoomButton.isPressed) editorCellSize = editorCellSize % 20 + 10;

		// Tool Grid button
		Button toolGridButton = Button(this, olc::vi2d(3, 85), 20, 20, &spriteEditorToolGrid);
		toolGridButton.showBorder = true;
		toolGridButton.hoverText = "GRID";
		toolGridButton.Update();
		if (toolGridButton.isHovered) { mouseOnUI = true; hoverText = toolGridButton.hoverText; }
		if (toolGridButton.isPressed) showGrid = !showGrid;

		// Tool Player button
		Button toolPlayerButton = Button(this, olc::vi2d(3, 125), 20, 20, &spriteEditorToolPlayer);
		toolPlayerButton.showBorder = true;
		toolPlayerButton.hoverText = "START POSITION";
		toolPlayerButton.Update();
		if (toolPlayerButton.isHovered) { mouseOnUI = true; hoverText = toolPlayerButton.hoverText; }
		if (toolPlayerButton.isPressed) { eraserSelected = false; levelEndPosSelected = false; playerPosSelected = !playerPosSelected; }

		// Tool EndLevel button
		Button toolEndLevelButton = Button(this, olc::vi2d(3, 150), 20, 20, &spriteEditorToolEndLevel);
		toolEndLevelButton.showBorder = true;
		toolEndLevelButton.hoverText = "END LEVEL";
		toolEndLevelButton.Update();
		if (toolEndLevelButton.isHovered) { mouseOnUI = true; hoverText = toolEndLevelButton.hoverText; }
		if (toolEndLevelButton.isPressed) { eraserSelected = false; playerPosSelected = false; levelEndPosSelected = !levelEndPosSelected; }


		// Tool label
		DrawString(200, 5, "TOOL:", olc::WHITE);

		// Tool button
		Button textureButton = Button(this);
		switch (toolSelected)
		{
		case 0: // Wall
			textureButton = Button(this, olc::vi2d(239, 2), 25, 25, GetWallTexture(selectedToolWall));
			break;
		case 1: // Decorations
			textureButton = Button(this, olc::vi2d(239, 2), 25, 25, GetDecorationSprite(selectedToolDecor));
			break;
		case 2: // Items
			textureButton = Button(this, olc::vi2d(239, 2), 25, 25, itemIconSprites[selectedToolItem]);
			break;
		case 3: // Enemies
			textureButton = Button(this, olc::vi2d(239, 2), 25, 25, enemyIconSprites[selectedToolEnemy]);
			break;
		}
		textureButton.Update();
		if (textureButton.isHovered) { mouseOnUI = true; }
		if (textureButton.isPressed) { 
			showToolSelection = !showToolSelection; 
			showSettings = false; 
			showOpenFile = false;
			toolSelectionPage = 0;
		}



		// Selected tool highlighting -------------------------------

		DrawRect(88 + toolSelected * 25, 3, 24, 24, olc::YELLOW);
		DrawRect(89 + toolSelected * 25, 4, 22, 22, olc::YELLOW);


		if (eraserSelected) {
			DrawRect(2, 34, 22, 22, olc::YELLOW);
			DrawRect(1, 33, 24, 24, olc::YELLOW);
			// Cursor
			DrawSpriteColorTransparent(GetMouseX() + 3, GetMouseY() + 5, &spriteEditorToolEracer, olc::CYAN);
		}

		if (playerPosSelected) {
			DrawRect(2, 124, 22, 22, olc::YELLOW);
			DrawRect(1, 123, 24, 24, olc::YELLOW);
			// Cursor
			DrawSpriteColorTransparent(GetMouseX() + 3, GetMouseY() + 5, &spriteEditorToolPlayer, olc::CYAN);
		}

		if (levelEndPosSelected) {
			DrawRect(2, 149, 22, 22, olc::YELLOW);
			DrawRect(1, 148, 24, 24, olc::YELLOW);
			// Cursor
			DrawSpriteColorTransparent(GetMouseX() + 3, GetMouseY() + 5, &spriteEditorToolEndLevel, olc::CYAN);
		}



		// Tool selection -----------------------

		if (showToolSelection) {
			Editor_ShowToolSelection();
		}


		// Settings window selection -----------------------

		if (showSettings) {
			Editor_ShowSettings();
		}

		// Open file window -----------------------

		if (showOpenFile) {
			Editor_ShowOpenFile();
		}

		// Open exit dialog -----------------------

		if (showExitDialog) {
			Editor_ShowExitDialog();
		}



		// Hovering text -----------------------

		if (hoverText != "") {
			DrawString(GetMouseX() - 1, GetMouseY() + 18, hoverText, olc::BLACK);
			DrawString(GetMouseX() - 2, GetMouseY() + 17, hoverText, olc::WHITE);
		}





		// Info log -----------------------

		if (editor_infoLogTimer > 0) {
			editor_infoLogTimer -= fElapsedTime;
			DrawString(31, 38, editor_infoLog, olc::BLACK);
			DrawString(30, 37, editor_infoLog);
		}



		// Editor controls -----------------------

		if (!showSettings && !showToolSelection && !showOpenFile && !showExitDialog && !mouseOnUI) {
			Editor_Controls(fElapsedTime);
		}



		// Exit editor -----------------------

		if (GetKey(olc::Key::ESCAPE).bPressed) {
			showExitDialog = true;
		}


	}



	//===============================================================================================================================
	// MAIN ENGINE FUNCTIONS
	//===============================================================================================================================


	void RestartLevel() {
		std::string loadFile = "maps/" + worldMapFile + ".map";
		LoadMap(loadFile);
		player.Ressurect();
	}


	bool OnUserCreate() override
	{
		// Initialisation ====================================================================

		// Called once at the start, so create things here
		drawnPixels = new bool[ScreenHeight() * ScreenWidth()];
		for (int i = 0; i < ScreenHeight() * ScreenWidth(); i++) drawnPixels[i] = false;

		// Clear Z buffer
		ZBuffer = new float[ScreenWidth()];


		LoadDefaultMap();

		// Assets ====================================================================

		LoadAssets();



		// Create objects ====================================================================

		player.engineReference = this;
		player.position = olc::vf2d{ 1.5, 1.5 };
		player.Ressurect();


		// ===================================================================


		gameState = STATE_TITLESCREEN;


		midiFileName = "MENU";
		PlayMapMIDI();

		return true;
	}



	bool OnUserUpdate(float fElapsedTime) override
	{

		// Game states ==============================================================================


		switch (gameState) {

		case STATE_GAMEPLAY:

			isEditorOpened = false;
			isMenuOpen = false;
			showCursor = false;

			// Rendering
			RaycastRender();
			DrawPlayerUI(fElapsedTime);

			// Player update
			player.Update(fElapsedTime);

			// Object updates
			for (int i = 0; i < enemiesArray.size(); i++) {
				enemiesArray[i].Update(fElapsedTime);
			}
			for (int i = 0; i < itemsArray.size(); i++) {
				itemsArray[i].Update(fElapsedTime);
			}

			UserControls(fElapsedTime);

			

			if (isEditorInPlayMode) {
				DrawString(80, 180,"EDITOR IN PLAY MODE",olc::WHITE);
				DrawString(80, 190, "PRESS M TO RETURN", olc::WHITE);

				if (GetKey(olc::Key::M).bPressed) {
					isEditorInPlayMode = false;

					// Load map
					std::string loadFile = "maps/" + worldMapFile + ".map";
					LoadMap(loadFile);
					player.Ressurect();


					isNewGameStarted = false;
					bossDefeated = 0;


					// Open editor
					gameState = GameState::STATE_EDITOR;

					// Stop music
					StopMIDI();
				}
			}


			if (GetKey(olc::Key::ESCAPE).bPressed)
			{
				!isMenuOpen ? gameState = STATE_MENU : gameState = STATE_GAMEPLAY;
				submenu = 0;
				SetCursorPos(GetWindowPosition().x + GetWindowSize().x / 2, GetWindowPosition().y + GetWindowSize().y / 2); // Return cursor to center
			}



			if (bossDefeated >= 4) {
				gameState = STATE_ENDGAME;
			}

			//if (GetKey(olc::Key::H).bPressed) {
			//	gameState = STATE_ENDGAME;
			//}
			//

			break;



		case STATE_MENU:
			
			isMenuOpen = true;
			showCursor = true;

			RaycastRender();
			DrawPlayerUI(fElapsedTime);

			DrawMenu();

			UserControls(fElapsedTime);


			if (GetKey(olc::Key::ESCAPE).bPressed)
			{
				!isMenuOpen ? gameState = STATE_MENU : gameState = STATE_GAMEPLAY;
				submenu = 0;
				SetCursorPos(GetWindowPosition().x + GetWindowSize().x / 2, GetWindowPosition().y + GetWindowSize().y / 2); // Return cursor to center
			}

			break;



		case STATE_EDITOR:

			isEditorOpened = true;
			showCursor = true;

			Editor_DrawEditor(fElapsedTime);

			UserControls(fElapsedTime);
			break;


		case STATE_TITLESCREEN:

			showCursor = true;

			DrawTitleScreen(fElapsedTime);
			break;


		case STATE_ENDGAME:


			DrawEndScreen();

			if (GetKey(olc::Key::ESCAPE).bPressed)
			{
				!isMenuOpen ? gameState = STATE_MENU : gameState = STATE_ENDGAME;
				submenu = 0;
				SetCursorPos(GetWindowPosition().x + GetWindowSize().x / 2, GetWindowPosition().y + GetWindowSize().y / 2); // Return cursor to center
			}

			break;

		case STATE_LORESCREEN:


			DrawLoreScreen();

			if (GetKey(olc::Key::SPACE).bPressed || GetKey(olc::Key::E).bPressed)
			{
				gameState = STATE_GAMEPLAY;
				SetCursorPos(GetWindowPosition().x + GetWindowSize().x / 2, GetWindowPosition().y + GetWindowSize().y / 2); // Return cursor to center
			}

			break;


		}

		

		


		// UI ==============================================================================


		// Draw mouse cursor
		if (showCursor) {
			DrawSpriteColorTransparent(GetMouseX(), GetMouseY(), &spriteCursor, olc::CYAN);
		}



		// Debug info ==============================================================================

		
		//std::string aimDist;

		//std::string text = "VEL : " + std::to_string(player.playerVelocity.x) + ", " + std::to_string(player.playerVelocity.y);
		//DrawString(10, 10, text, olc::WHITE);

		//std::string text = "VERCAM : " + std::to_string(player.cameraVertical);
		//DrawString(10, 10, text, olc::WHITE);
		
		//text = "POS : " + std::to_string(player.position.x) + ", " + std::to_string(player.position.y);
		//DrawString(10, 20, text, olc::WHITE);
		//
		//text = "MV : " + std::to_string(player.controlMoveVector.x) + ", " + std::to_string(player.controlMoveVector.y);
		//DrawString(10, 30, text, olc::WHITE);
		//
		//text = "DIR : " + std::to_string(player.direction.x) + ", " + std::to_string(player.direction.y);
		//DrawString(10, 50, text, olc::WHITE);
		
		
		//DrawString(10, 60, aimDist, olc::WHITE);
		
		
		//text = "SPRITES";
		//DrawString(250, 10, text, olc::WHITE);
		//for (int i = 0; i < thingsArray.size(); i++) {
		//	text = std::to_string(thingsArray[spriteOrder[thingsArray.size() - 1 - i]].texture);
		//	DrawString(300, 20+10*i, text, olc::WHITE);
		//}
		




		// MIDI ==========================================================================================

		// MIDI Looping
		if (gameState == GameState::STATE_GAMEPLAY) {
			if (g_MidiMessage == NULL) {
				//fprintf(stderr, "End of MIDI\n");
				PlayMapMIDI();
			}
		}




		//std::cout << "=== END FRAME ===" << std::endl;
		return true;
	}




	void PlayMapMIDI() {
		std::string fullMidiFileName = "sounds/" + midiFileName + ".mid";
		char* midiCSTR = new char[fullMidiFileName.length() + 1];
		strcpy(midiCSTR, fullMidiFileName.c_str());

		//std::cout << midiCSTR << std::endl;

		RestartMIDI(midiCSTR);
	}



};



//===============================================================================================================================
// MAIN() 
//===============================================================================================================================




int main()
{

	// Sound engines
	StartMIDIEngine();
	StartWAVEngine();

	
	ShowCursor(false);

	RaycastEngine engine;

	if (engine.Construct(320, 200, 4, 4, false, true))
		engine.Start();


	// After we are finished, we have to delete the irrKlang Device created earlier
	// with createIrrKlangDevice(). Use ::drop() to do that. In irrKlang, you should
	// delete all objects you created with a method or function that starts with 'create'.
	// (an exception is the play2D()- or play3D()-method, see the documentation or the
	// next example for an explanation)
	// The object is deleted simply by calling ->drop().
	soundEngine->drop(); // delete engine

	return 0;
}
