#include "c_engine.h"


// MIDI
#include "minisdl_audio.h"
#define TSF_IMPLEMENTATION
#include "tsf.h"
#define TML_IMPLEMENTATION
#include "tml.h"


//RaycastEngine* RaycastEngine::p_instance = nullptr;



//===============================================================================================================================
// AUDIO
//===============================================================================================================================

// Holds the global instance pointer
static tsf* g_TinySoundFont;

// Holds global MIDI playback state
static float g_Msec;               //current playback time
static tml_message* g_MidiMessage;  //next message to be played




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



void RaycastEngine::StartWAVEngine() {

	// start the sound engine with default parameters
	soundEngine = irrklang::createIrrKlangDevice();

	if (!soundEngine)
	{
		printf("SOUND : Could not startup FX sound engine\n");
	}

	soundEngine->setSoundVolume(0.2f);

}



//===============================================================================================================================
// DRAWING
//===============================================================================================================================

Color RaycastEngine::ShadeColor(Color color, float amount) {
	Color shadeColor = Color(color.r * amount, color.g * amount, color.b * amount);
	return shadeColor;
}

void RaycastEngine::DrawSpriteColorTransparent(int32_t x, int32_t y, Sprite* sprite, Color transparancyColor) {

	if (sprite == nullptr)
		return;

	for (int32_t i = 0; i < sprite->width; i++)
		for (int32_t j = 0; j < sprite->height; j++)
			if (sprite->GetPixel(i, j) != transparancyColor)
				Draw(x + i, y + j, sprite->GetPixel(i, j));
}

void RaycastEngine::DrawPartialSpriteColorTransparent(int32_t x, int32_t y, int32_t regionX, int32_t regionY, int32_t regionW, int32_t regionH, Sprite* sprite, Color transparancyColor) {

	if (sprite == nullptr)
		return;

	for (int32_t i = regionX; i < regionX + regionW; i++)
		for (int32_t j = regionY; j < regionY + regionH; j++)
			if (sprite->GetPixel(i, j) != transparancyColor)
				Draw(x + (i % regionW), y + (j % regionH), sprite->GetPixel(i, j));
}



//===============================================================================================================================
// MAP
//===============================================================================================================================

void RaycastEngine::LoadDefaultMap() {

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
	worldMap.clear();

	// Starting with id = 1
	newId = 1; // 0 is reserved !!!


	// First row (11111111111111111111)
	for (int i = 0; i < worldMapWidth; i++) {
		worldMap.push_back(1);
	}
	// Center (10000000000000001)
	for (int i = 0; i < 18; i++) {
		for (int j = 0; j < worldMapWidth; j++) {
			if (j % 19 == 0) {
				worldMap.push_back(1);
			}
			else {
				worldMap.push_back(0);
			}
		}
	}
	// Last row (11111111111111111111)
	for (int i = 0; i < worldMapWidth; i++) {
		worldMap.push_back(1);
	}


	midiFileName = "NAILS";

	player.position = vf2d(1.5, 1.5);

	InteractibleWall endLevel = InteractibleWall(this, vf2d(3, 3), InteractibleWall::InteractionType::ENDLEVEL);
	interactbleWallsArray.push_back(endLevel);

}

void RaycastEngine::LoadMap(std::string mapName) {

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


					vf2d readPosition{ -1, -1 };

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

						readPosition = vf2d(std::stof(posX), std::stof(posY));

						//std::cout << "-> Position read V : " << readPosition << '\n';
					}
					else {
						std::cout << "-> ERROR : Player position reading fail" << std::endl;
					}


					if (readPosition != vf2d(-1, -1)) {
						player.position = readPosition;
					}
					else {
						std::cout << "-> ERROR : Player reading fail" << std::endl;
						player.position = vf2d(1.5, 1.5);
					}

				}


				// End of level ---------------------------------

				if (std::regex_match(line, std::regex("(endlevel:)(.*)"))) {
					std::string endLevelLine;
					endLevelLine = line.substr(9);
					//std::cout << "-> Level end found : " << endLevelLine << '\n';


					vf2d readPosition{ -1, -1 };

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

						readPosition = vf2d(std::stof(posX), std::stof(posY));

						//std::cout << "-> Position read V : " << readPosition << '\n';
					}
					else {
						std::cout << "-> ERROR : Level end position reading fail" << std::endl;
					}

					if (readPosition != vf2d(-1, -1)) {
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
					vf2d readPosition{ -1, -1 };

					std::smatch match;



					// Type
					std::regex_search(enemyLine.cbegin(), enemyLine.cend(), match, std::regex("(type=\")([0-9]*)(\":)"));
					std::string foundType = match.str();
					if (foundType != "") {
						foundType = foundType.substr(0, foundType.length() - 1); // remove ":" in the end
						//std::cout << "-> Type found : " << foundType << '\n';
						readType = std::stoi(foundType.substr(6, foundType.length() - 7));
						//std::cout << "-> Type read : " << readType << '\n';
					}
					else {
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

						readPosition = vf2d(std::stof(posX), std::stof(posY));

						//std::cout << "-> Position read V : " << readPosition << '\n';
					}
					else {
						std::cout << "-> ERROR : Enemy position reading fail" << std::endl;
					}

					if (readType != -1 && readPosition != vf2d(-1, -1)) {
						Enemy newEnemy = Enemy(this, newId, readType, readPosition);
						enemiesArray.push_back(newEnemy); newId++;
					}
					else {
						std::cout << "Enemy reading fail" << std::endl;
					}

				}




				// Decorations ---------------------------------

				if (std::regex_match(line, std::regex("(decoration:)(.*)"))) {
					std::string decorationLine;
					decorationLine = line.substr(11);
					//std::cout << "-> Decoration found : " << decorationLine << '\n';


					int readType = -1;
					vf2d readPosition{ -1, -1 };

					std::smatch match;



					// Type
					std::regex_search(decorationLine.cbegin(), decorationLine.cend(), match, std::regex("(type=\")([0-9]*)(\":)"));
					std::string foundType = match.str();
					if (foundType != "") {
						foundType = foundType.substr(0, foundType.length() - 1); // remove ":" in the end
						//std::cout << "-> Type found : " << foundType << '\n';
						readType = std::stoi(foundType.substr(6, foundType.length() - 7));
						//std::cout << "-> Type read : " << readType << '\n';
					}
					else {
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

						readPosition = vf2d(std::stof(posX), std::stof(posY));

						//std::cout << "-> Position read V : " << readPosition << '\n';
					}
					else {
						std::cout << "-> ERROR : Decoration position reading fail" << std::endl;
					}


					if (readType != -1 && readPosition != vf2d(-1, -1)) {
						Decoration newDecoration = Decoration(this, newId, readType, readPosition);
						decorationsArray.push_back(newDecoration); newId++;
					}
					else {
						std::cout << "-> ERROR : Decoration reading fail" << std::endl;
					}
				}





				// Items ---------------------------------

				if (std::regex_match(line, std::regex("(item:)(.*)"))) {
					std::string itemLine;
					itemLine = line.substr(5);
					//std::cout << "-> Item found : " << itemLine << '\n';


					int readType = -1;
					vf2d readPosition{ -1, -1 };

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

						readPosition = vf2d(std::stof(posX), std::stof(posY));

						//std::cout << "-> Position read V : " << readPosition << '\n';
					}
					else {
						std::cout << "-> ERROR : Item position reading fail" << std::endl;
					}


					if (readType != -1 && readPosition != vf2d(-1, -1)) {
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

void RaycastEngine::SaveMap(std::string mapName) {

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

void RaycastEngine::ChangeMapSize(int w, int h) {

	std::vector<int> newWorldMap;

	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {


			if (x < worldMapWidth && y < worldMapHeight) {
				//std::cout << worldMap[y * worldMapWidth + x];
				newWorldMap.push_back(worldMap[y * worldMapWidth + x]);
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



void RaycastEngine::RestartLevel() {
	std::string loadFile = "maps/" + worldMapFile + ".map";
	LoadMap(loadFile);
	player.Ressurect();
}

void RaycastEngine::PlayMapMIDI() {
	std::string fullMidiFileName = "sounds/" + midiFileName + ".mid";
	char* midiCSTR = new char[fullMidiFileName.length() + 1];
	strcpy_s(midiCSTR, fullMidiFileName.length() + 1, fullMidiFileName.c_str());

	//std::cout << midiCSTR << std::endl;

	RestartMIDI(midiCSTR);
}



//===============================================================================================================================
// GAME OBJECTS
//===============================================================================================================================

void RaycastEngine::UpdateThings() {

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

}

Enemy* RaycastEngine::GetEnemyByID(uint32_t id) {
	for (int i = 0; i < enemiesArray.size(); i++) {
		if (enemiesArray[i].id == id) {
			return &enemiesArray[i];
		}
	}
	return nullptr;
}



//===============================================================================================================================
// ASSETS
//===============================================================================================================================

Sprite* RaycastEngine::SampleIconFromSprite(Sprite* sprite, int32_t x, int32_t y, int32_t w, int32_t h) {
	Sprite* newIcon = new Sprite(w, h);

	for (int smplX = 0; smplX < w; smplX++) {
		for (int smplY = 0; smplY < h; smplY++) {
			Color color = sprite->GetPixel(x + smplX, y + smplY);
			//Color color = sprite->GetPixel(int(1.0 * sprite->width / w * smplX), int(1.0 * sprite->height / h * smplY));
			newIcon->SetPixel(smplX, smplY, color);
		}
	}

	return(newIcon);
}

void RaycastEngine::SampleWallTexturesFromAtlas(Sprite* atlas) {

	int textureCount = (atlas->width / 64) * (atlas->height / 64);

	for (int textureIdx = 0; textureIdx < textureCount; textureIdx++) {
		Sprite* wallTexture = new Sprite(64, 64);

		for (int texX = 0; texX < 64; texX++) {
			for (int texY = 0; texY < 64; texY++) {
				Color color = spriteWallAtlas.GetPixel((textureIdx * 64 + texX) % atlas->width, ((textureIdx / 4) * 64 + texY));
				wallTexture->SetPixel(texX, texY, color);
			}
		}

		wallSprites.push_back(wallTexture);
	}
}


void RaycastEngine::LoadSprite(Sprite* sprite, std::string file) {
	rcode loadStatus = imageLoader.LoadImageResource(sprite, file);

	if (loadStatus == FAIL) { std::cout << "Sprite load failed : " << file << std::endl; return; }
	if (loadStatus == NO_FILE) { std::cout << "Sprite not found : " << file << std::endl; return; }
}

void RaycastEngine::LoadAssets() {

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


Sprite* RaycastEngine::GetWallTexture(int idx) {
	return wallSprites[idx];
}

Sprite* RaycastEngine::GetDecorationSprite(int idx) {
	switch (idx) {
	case 0: return &spriteBarell;
	case 1: return &spritePillar;
	case 2: return &spriteLamp;
	case 3: return &spriteExitSign;
	default: return &spriteBarell;
	}
}

Sprite* RaycastEngine::GetEnemySprite(int idx) {
	switch (idx) {
	case 0: return &spriteRobot1;
	case 1: return &spriteRobot2;
	case 2: return &spriteRobot3;
	case 3: return &spriteBoss;
	default: return &spriteRobot1;
	}
}

Sprite* RaycastEngine::GetItemSprite(int idx) {
	switch (idx) {
	case 0: return &spritesMedkit;
	case 1: return &itemAmmo9mm;
	case 2: return &itemAmmoShells;
	case 3: return &itemUzi;
	case 4: return &itemShotgun;
	default: return &spritesMedkit;
	}
}

Sprite* RaycastEngine::GetWeaponSprite(int idx) {
	switch (idx) {
	case 0: return &weaponPistol;
	case 1: return &weaponUzi;
	case 2: return &weaponShotgun;
	default: return &weaponPistol;
	}
}




//===============================================================================================================================
// WORLD PHYSICS
//===============================================================================================================================

//x-coordinate in camera space,  [0 - 319] pixels -> [-1 - 1]
float RaycastEngine::ShootRaycastRay(vf2d from, vf2d rayDir) {

	//which box of the map we're in
	vi2d mapPosition = from;

	//length of ray from current position to next x or y-side
	vf2d sideDist;

	//length of ray from one x or y-side to next x or y-side
	vf2d deltaDist = vf2d(std::abs(1 / rayDir.x), std::abs(1 / rayDir.y));

	float perpWallDist;

	//what direction to step in x or y-direction (either +1 or -1)
	vi2d step;


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

bool RaycastEngine::LineIntersection(vf2d ln1pnt1, vf2d ln1pnt2, vf2d ln2pnt1, vf2d ln2pnt2) {

	vf2d line1 = ln1pnt2 - ln1pnt1;
	vf2d line2 = ln2pnt2 - ln2pnt1;

	float cross1 = line1.cross(ln2pnt1 - ln1pnt1);
	float cross2 = line1.cross(ln2pnt2 - ln1pnt1);

	if (copysign(1, cross1) == copysign(1, cross2) || cross1 == 0 || cross2 == 0)
		return false;

	cross1 = line2.cross(ln1pnt1 - ln2pnt1);
	cross2 = line2.cross(ln1pnt2 - ln2pnt1);

	if (copysign(1, cross1) == copysign(1, cross2) || cross1 == 0 || cross2 == 0)
		return false;

	return true;
}



//===============================================================================================================================
// RENDERING
//===============================================================================================================================

void RaycastEngine::sortSprites(int* order, float* dist, int amount)
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

void RaycastEngine::RaycastRender() {


	//FLOOR CASTING
	for (int y = ScreenHeight() / 2; y < ScreenHeight(); y++)
	{
		// rayDir for leftmost ray (x = 0) and rightmost ray (x = w)
		vf2d rayDir0 = player.direction - player.viewPlane;

		vf2d rayDir1 = player.direction + player.viewPlane;


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
			Color color = GetWallTexture(worldMapFloorTxtr)->Sample(floorX - cellX + 0.05, floorY - cellY + 0.05);
			color = ShadeColor(color, shading);
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
			Color color = GetWallTexture(worldMapCeilTxtr)->Sample(floorX2 - cellX + 0.05, floorY2 - cellY + 0.05);
			color = ShadeColor(color, shading);
			Draw(x, ScreenHeight() - y - 1, color);
			//}

		}
	}


	// WALL CASTING
	for (int x = 0; x < ScreenWidth(); x++) {

		//calculate ray
		float cameraX = 2 * x / float(ScreenWidth()) - 1; //x-coordinate in camera space,  [0 - 319] pixels -> [-1 - 1]

		//calculate ray position and direction
		vf2d rayDir = player.direction + player.viewPlane * cameraX;

		//which box of the map we're in
		vi2d mapPosition = player.position;

		//length of ray from current position to next x or y-side
		vf2d sideDist;

		//length of ray from one x or y-side to next x or y-side
		vf2d deltaDist = vf2d(std::abs(1 / rayDir.x), std::abs(1 / rayDir.y));

		float perpWallDist;

		//what direction to step in x or y-direction (either +1 or -1)
		vi2d step;


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
		Color color;
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
			color = ShadeColor(color, shading);

			if (side == 1) color = ShadeColor(color);

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

			Sprite* thingSprite = GetDecorationSprite(thingTexture);

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
			vf2d spritePosRel = thingsArray[spriteOrder[i]].position - player.position;

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

							Color color = RED;
							color = thingSprite->GetPixel(texX + (thingTexturePart * 64), texY);

							float shading = spritePosRel.mag2();
							if (shading > 60) shading = 60;
							shading = 1 - shading / 80;


							if (color != CYAN)
							{
								//Draw(stripe, y, color);
								Draw(stripe, y, ShadeColor(color, shading)); //paint pixel if it isn't black, black is the invisible color
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
// UI
//===============================================================================================================================

void RaycastEngine::InfoLog(std::string text) {

	infoEffectTimer = 0.07f;

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

void RaycastEngine::DrawPlayerUI(float fElapsedTime) {

	// Draw weapon -----------------------------

	if (player.isAlive) {


		if (player.changeWeaponTimer > 0) {

			DrawPartialSpriteColorTransparent
			(
				ScreenWidth() / 2 + 0 - sin(player.bobValue + 1.55) * 8,
				ScreenHeight() - 110 - (abs(sin(player.bobValue)) * 8) + 80 - (abs(player.changeWeaponTimer - (player.timeToChangeWeapon / 2)) * (1 / (player.timeToChangeWeapon / 2)) * 80),
				player.weaponState * 128,
				0,
				128,
				128,
				GetWeaponSprite(player.activeWeapon),
				CYAN
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
				CYAN
			);
		}



	}



	// Death screen

	if (!player.isAlive) {
		FillRect(0, 0, ScreenWidth(), ScreenHeight(), Color(255, 0, 0, 150));
	}



	// Draw crosshair -----------------------------

	Draw(ScreenWidth() / 2, ScreenHeight() / 2, WHITE);


	// Draw player status -----------------------------

	FillRect(5, 165, 60, 30, BLACK);
	DrawLine(4, 166, 4, 195, YELLOW);
	DrawLine(4, 195, 64, 195, YELLOW);
	DrawString(9, 161, "Health", WHITE);
	DrawString(11, 176, std::to_string(player.health), VERY_DARK_YELLOW, 2);
	DrawString(10, 175, std::to_string(player.health), YELLOW, 2);

	FillRect(255, 165, 60, 30, BLACK);
	DrawLine(315, 166, 315, 195, YELLOW);
	DrawLine(256, 195, 315, 195, YELLOW);
	DrawString(280, 161, "Ammo", WHITE);
	DrawString(266, 176, std::to_string(weapons[player.activeWeapon].currentAmmo), VERY_DARK_YELLOW, 2);
	DrawString(265, 175, std::to_string(weapons[player.activeWeapon].currentAmmo), YELLOW, 2);

	// Draw info logs -----------------------------


	if (infoLogTimer0 > 0) {
		infoLogTimer0 -= fElapsedTime;
		DrawString(6, 6, infoLog0, BLACK);
		DrawString(5, 5, infoLog0, YELLOW);
	}
	if (infoLogTimer1 > 0) {
		infoLogTimer1 -= fElapsedTime;
		DrawString(6, 16, infoLog1, BLACK);
		DrawString(5, 15, infoLog1, YELLOW);
	}
	if (infoLogTimer2 > 0) {
		infoLogTimer2 -= fElapsedTime;
		DrawString(6, 26, infoLog2, BLACK);
		DrawString(5, 25, infoLog2, YELLOW);
	}


	// Draw info effect -----------------------------

	if (infoEffectTimer > 0) {
		infoEffectTimer -= fElapsedTime;
		DrawRect(0, 0, ScreenWidth() - 1, ScreenHeight() - 1, DARK_YELLOW);
	}

	// Draw damage effect -----------------------------

	if (player.damageEffectTimer > 0) {
		player.damageEffectTimer -= fElapsedTime;
		DrawRect(0, 0, ScreenWidth() - 1, ScreenHeight() - 1, RED);
	}

}

void RaycastEngine::DrawMenu() {


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


		DrawString(72, 22, "Pause", VERY_DARK_YELLOW, 3);
		DrawString(70, 20, "Pause", YELLOW, 3);

		DrawString(62, 67, "Restart", VERY_DARK_YELLOW, 2);
		DrawString(60, 65, "Restart", YELLOW, 2);

		DrawString(62, 87, "Settings", VERY_DARK_YELLOW, 2);
		DrawString(60, 85, "Settings", YELLOW, 2);

		DrawString(62, 107, "Exit to menu", VERY_DARK_YELLOW, 2);
		DrawString(60, 105, "Exit to menu", YELLOW, 2);


		if (menuItemSelected != -1) {
			DrawString(42, 67 + 20 * menuItemSelected, ">", VERY_DARK_YELLOW, 2);
			DrawString(40, 65 + 20 * menuItemSelected, ">", YELLOW, 2);
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

		DrawString(32, 22, "SETTINGS", VERY_DARK_YELLOW, 2);
		DrawString(30, 20, "SETTINGS", YELLOW, 2);

		DrawString(62, 47, "<- Back", VERY_DARK_RED, 2);
		DrawString(60, 45, "<- Back", RED, 2);




		DrawString(62, 82, "MIDI", VERY_DARK_YELLOW, 2);
		DrawString(60, 80, "MIDI", YELLOW, 2);

		DrawString(62, 112, "FX", VERY_DARK_YELLOW, 2);
		DrawString(60, 110, "FX", YELLOW, 2);

		DrawString(62, 142, "M SEN", VERY_DARK_YELLOW, 2);
		DrawString(60, 140, "M SEN", YELLOW, 2);


		midiVolumeSlider.showBorder = false;
		midiVolumeSlider.colorSlider = YELLOW;
		midiVolumeSlider.colorSliderLine = DARK_YELLOW;
		midiVolumeSlider.showBackground = false;
		midiVolumeSlider.Update();

		if (midiVolumeSlider.isPressed) {
			PlayMapMIDI();
		}
		tsf_set_volume(g_TinySoundFont, midiVolumeSlider.value);


		fxVolumeSlider.showBorder = false;
		fxVolumeSlider.colorSlider = YELLOW;
		fxVolumeSlider.colorSliderLine = DARK_YELLOW;
		fxVolumeSlider.showBackground = false;
		fxVolumeSlider.Update();

		if (fxVolumeSlider.isPressed) {
			soundEngine->play2D("sounds/pistol.wav", false);
		}
		soundEngine->setSoundVolume(fxVolumeSlider.value);



		mouseSensSlider.showBorder = false;
		mouseSensSlider.colorSlider = YELLOW;
		mouseSensSlider.colorSliderLine = DARK_YELLOW;
		mouseSensSlider.showBackground = false;
		mouseSensSlider.Update();

		mouseSensitivity = mouseSensSlider.value * 2;



		if (menuItemSelected != -1) {
			DrawString(42, 47 + 20 * menuItemSelected, ">", VERY_DARK_YELLOW, 2);
			DrawString(40, 45 + 20 * menuItemSelected, ">", YELLOW, 2);
		}


		if (GetMouse(0).bPressed) {
			if (menuItemSelected == 0) {
				submenu = 0;
				soundEngine->play2D("sounds/pickup.wav", false);
			}
		}

	}
}

void RaycastEngine::DrawEndScreen() {

	FillRect(0, 0, ScreenWidth(), ScreenHeight(), BLACK);
	DrawRect(0, 0, ScreenWidth() - 1, ScreenHeight() - 1, YELLOW);

	DrawString(32, 22, "THE END", VERY_DARK_YELLOW, 2);
	DrawString(30, 20, "THE END", YELLOW, 2);

	DrawString(10, 60, "You did it! The terrible machine is", WHITE, 1);
	DrawString(10, 70, "off and everything will be back to", WHITE, 1);
	DrawString(10, 80, "normal soon. You smell burnt plastic", WHITE, 1);
	DrawString(10, 90, "and look at the remains of a once", WHITE, 1);
	DrawString(10, 100, "great invention. It remains only ", WHITE, 1);
	DrawString(10, 110, "to deal with the hordes of already", WHITE, 1);
	DrawString(10, 120, "created machines in other parts", WHITE, 1);
	DrawString(10, 130, "of the complex.", WHITE, 1);
	DrawString(5, 150, "You can test yourself in user made maps!", WHITE, 1);

}

void RaycastEngine::DrawLoreScreen() {

	FillRect(0, 0, ScreenWidth(), ScreenHeight(), BLACK);
	DrawRect(0, 0, ScreenWidth() - 1, ScreenHeight() - 1, YELLOW);

	DrawString(32, 22, "STORY", VERY_DARK_YELLOW, 2);
	DrawString(30, 20, "STORY", YELLOW, 2);

	DrawString(10, 60, "You are in a secret complex for the", WHITE, 1);
	DrawString(10, 70, "development	of new technologies.", WHITE, 1);
	DrawString(10, 80, "You are one of the volunteers", WHITE, 1);
	DrawString(10, 90, "for the human cryofreezing program.", WHITE, 1);
	DrawString(10, 100, "You should have been defrosted after", WHITE, 1);
	DrawString(10, 110, "10 years and after waking up you", WHITE, 1);
	DrawString(10, 120, "feel that something is wrong around", WHITE, 1);
	DrawString(10, 130, "you. There are only hostile machines", WHITE, 1);
	DrawString(10, 140, "around. You need to figure out who", WHITE, 1);
	DrawString(10, 150, "or WHAT is behind this.", WHITE, 1);
	DrawString(5, 170, "PRESS SPACE OR E TO CONTINUE", WHITE, 1);


}



//===============================================================================================================================
// TITLE SCREEN
//===============================================================================================================================

void RaycastEngine::DrawTitleScreen(float fElapsedTime) {


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


		FillRect(0, 0, ScreenWidth(), ScreenHeight(), BLACK);
		DrawRect(0, 0, ScreenWidth() - 1, ScreenHeight() - 1, YELLOW);



		DrawString(32, 22, "OMNIVERSE MACKINA", VERY_DARK_YELLOW, 2);
		DrawString(30, 20, "OMNIVERSE MACKINA", YELLOW, 2);

		DrawString(62, 67, "New game", VERY_DARK_YELLOW, 2);
		DrawString(60, 65, "New game", YELLOW, 2);

		DrawString(62, 87, "Select map", VERY_DARK_YELLOW, 2);
		DrawString(60, 85, "Select map", YELLOW, 2);

		DrawString(62, 107, "Map editor", VERY_DARK_YELLOW, 2);
		DrawString(60, 105, "Map editor", YELLOW, 2);

		DrawString(62, 127, "Settings", VERY_DARK_YELLOW, 2);
		DrawString(60, 125, "Settings", YELLOW, 2);

		DrawString(62, 147, "Exit", VERY_DARK_YELLOW, 2);
		DrawString(60, 145, "Exit", YELLOW, 2);


		if (menuItemSelected != -1) {
			DrawString(42, 67 + 20 * menuItemSelected, ">", VERY_DARK_YELLOW, 2);
			DrawString(40, 65 + 20 * menuItemSelected, ">", YELLOW, 2);
		}




		if (GetMouse(0).bPressed) {

			if (menuItemSelected == 0) { // New game
				mapEditor.isEditorInPlayMode = false;
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


		FillRect(0, 0, ScreenWidth(), ScreenHeight(), BLACK);
		DrawRect(0, 0, ScreenWidth() - 1, ScreenHeight() - 1, YELLOW);



		DrawString(32, 22, "SELECT MAP", VERY_DARK_YELLOW, 2);
		DrawString(30, 20, "SELECT MAP", YELLOW, 2);

		DrawString(62, 47, "<- Back", VERY_DARK_RED, 2);
		DrawString(60, 45, "<- Back", RED, 2);

		DrawString(62, 67, "Prev page", VERY_DARK_RED, 2);
		DrawString(60, 65, "Prev page", RED, 2);

		for (int mapString = 0; mapString < 4; mapString++) {

			if (mapString + mapSelectPage * 4 < mapsToSelect.size()) {
				std::string mapName = mapsToSelect[mapString + mapSelectPage * 4].substr(5, mapsToSelect[mapString + mapSelectPage * 4].length() - 9);

				DrawString(62, 87 + mapString * 20, mapName, VERY_DARK_YELLOW, 2);
				DrawString(60, 85 + mapString * 20, mapName, YELLOW, 2);
			}

		}

		DrawString(62, 167, "Next page", VERY_DARK_RED, 2);
		DrawString(60, 165, "Next page", RED, 2);


		if (menuItemSelected != -1) {
			DrawString(42, 47 + 20 * menuItemSelected, ">", VERY_DARK_YELLOW, 2);
			DrawString(40, 45 + 20 * menuItemSelected, ">", YELLOW, 2);
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
				mapEditor.isEditorInPlayMode = false;
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
				mapEditor.isEditorInPlayMode = false;
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
				mapEditor.isEditorInPlayMode = false;
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
				mapEditor.isEditorInPlayMode = false;
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
				if (mapSelectPage < mapPageCount - 1) {
					mapSelectPage++;
				}
				soundEngine->play2D("sounds/pickup.wav", false);
			}

		}
	}
	else if (submenu == 2) {

		int menuItemSelected = -1;

		FillRect(0, 0, ScreenWidth(), ScreenHeight(), BLACK);
		DrawRect(0, 0, ScreenWidth() - 1, ScreenHeight() - 1, YELLOW);


		if (GetMouseY() > 47 && GetMouseY() <= 67) {
			menuItemSelected = 0;
		}

		DrawString(32, 22, "SETTINGS", VERY_DARK_YELLOW, 2);
		DrawString(30, 20, "SETTINGS", YELLOW, 2);

		DrawString(62, 47, "<- Back", VERY_DARK_RED, 2);
		DrawString(60, 45, "<- Back", RED, 2);




		DrawString(62, 82, "MIDI", VERY_DARK_YELLOW, 2);
		DrawString(60, 80, "MIDI", YELLOW, 2);

		DrawString(62, 112, "FX", VERY_DARK_YELLOW, 2);
		DrawString(60, 110, "FX", YELLOW, 2);

		DrawString(62, 142, "M SEN", VERY_DARK_YELLOW, 2);
		DrawString(60, 140, "M SEN", YELLOW, 2);


		midiVolumeSlider.showBorder = false;
		midiVolumeSlider.colorSlider = YELLOW;
		midiVolumeSlider.colorSliderLine = DARK_YELLOW;
		midiVolumeSlider.Update();

		if (midiVolumeSlider.isPressed) {
			PlayMapMIDI();
		}
		tsf_set_volume(g_TinySoundFont, midiVolumeSlider.value);


		fxVolumeSlider.showBorder = false;
		fxVolumeSlider.colorSlider = YELLOW;
		fxVolumeSlider.colorSliderLine = DARK_YELLOW;
		fxVolumeSlider.Update();

		if (fxVolumeSlider.isPressed) {
			soundEngine->play2D("sounds/pistol.wav", false);
		}
		soundEngine->setSoundVolume(fxVolumeSlider.value);


		mouseSensSlider.showBorder = false;
		mouseSensSlider.colorSlider = YELLOW;
		mouseSensSlider.colorSliderLine = DARK_YELLOW;
		mouseSensSlider.Update();

		mouseSensitivity = mouseSensSlider.value * 2;



		if (menuItemSelected != -1) {
			DrawString(42, 47 + 20 * menuItemSelected, ">", VERY_DARK_YELLOW, 2);
			DrawString(40, 45 + 20 * menuItemSelected, ">", YELLOW, 2);
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
// INPUT
//===============================================================================================================================

void RaycastEngine::UserControls(float fElapsedTime) {

	if (!isMenuOpen && !mapEditor.isEditorOpened && player.isAlive) {

		// Clear inputs ---------------------------------------------------------

		player.isMoving = false;
		player.isShooting = false;

		player.controlMoveVector = vf2d(0, 0);
		player.controlRotationVector = vf2d(0, 0);



		// Main controls ---------------------------------------------------------

		// Move forward
		if (GetKey(Key::UP).bHeld || GetKey(Key::W).bHeld)
		{
			player.isMoving = true;
			player.controlMoveVector += player.direction;
		}

		// Move backwards
		if (GetKey(Key::DOWN).bHeld || GetKey(Key::S).bHeld)
		{
			player.isMoving = true;
			player.controlMoveVector -= player.direction;
		}

		// Strafe right
		if (GetKey(Key::D).bHeld)
		{
			player.isMoving = true;
			player.controlMoveVector += player.viewPlane;
		}

		// Strafe left
		if (GetKey(Key::A).bHeld)
		{
			player.isMoving = true;
			player.controlMoveVector -= player.viewPlane;
		}



		// Rotate to the right
		if (GetKey(Key::RIGHT).bHeld)
		{
			player.controlRotationVector = vf2d(1, 0);
		}

		// Rotate to the left
		if (GetKey(Key::LEFT).bHeld)
		{
			player.controlRotationVector = vf2d(-1, 0);
		}


		// Shooting
		if (GetKey(Key::CTRL).bHeld || GetMouse(0).bHeld)
		{
			player.isShooting = true;
			player.Shoot();
		}


		// Weapons

		if (GetKey(Key::K1).bPressed)
		{
			player.ChangeWeapon(0);
		}

		if (GetKey(Key::K2).bPressed)
		{
			player.ChangeWeapon(1);
		}

		if (GetKey(Key::K3).bPressed)
		{
			player.ChangeWeapon(2);
		}


		// Interaction

		if (GetKey(Key::SPACE).bPressed || GetKey(Key::E).bPressed)
		{
			player.Interact();
		}

	}



	if (!player.isAlive) {
		if (GetKey(Key::SPACE).bPressed || GetKey(Key::E).bPressed)
		{
			RestartLevel();
		}
	}


	// Mouse controls ------------------------------------------------------

	if (IsFocused() && !isMenuOpen && !mapEditor.isEditorOpened) {
		if (GetWindowMouse().x != 0 && GetWindowMouse().y != 0) { // To prevent cursor set while dragging window

			float deltaMouseX = 0.05 * mouseSensitivity * (GetWindowMouse().x - GetWindowSize().x / 2) / (fElapsedTime / 0.016);
			player.controlRotationVector += vf2d(deltaMouseX, 0);
			SetCursorPos(GetWindowPosition().x + GetWindowSize().x / 2, GetWindowPosition().y + GetWindowSize().y / 2);
		}
	}
	else if (!IsFocused() && gameState == GameState::STATE_GAMEPLAY) {
		gameState = STATE_MENU;
	}



}


//===============================================================================================================================
// MAIN FUNCTIONS 
//===============================================================================================================================

bool RaycastEngine::GameStart() {

	// Editor ====================================================================

	mapEditor = Editor(this);

	// Image Loader ====================================================================

	imageLoader = ImageLoader();


	// Sound engines ====================================================================

	StartMIDIEngine();
	StartWAVEngine();


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

	player.position = vf2d{ 1.5, 1.5 };
	player.Ressurect();


	// ===================================================================


	gameState = STATE_TITLESCREEN;


	midiFileName = "MENU";
	PlayMapMIDI();

	return true;
}

bool RaycastEngine::GameUpdate(float fElapsedTime) {

	// Game states ==============================================================================


	switch (gameState) {

	case STATE_GAMEPLAY:

		mapEditor.isEditorOpened = false;
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



		if (mapEditor.isEditorInPlayMode) {
			DrawString(80, 180, "EDITOR IN PLAY MODE", WHITE);
			DrawString(80, 190, "PRESS M TO RETURN", WHITE);

			if (GetKey(Key::M).bPressed) {
				mapEditor.isEditorInPlayMode = false;

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


		if (GetKey(Key::ESCAPE).bPressed)
		{
			!isMenuOpen ? gameState = STATE_MENU : gameState = STATE_GAMEPLAY;
			submenu = 0;
			SetCursorPos(GetWindowPosition().x + GetWindowSize().x / 2, GetWindowPosition().y + GetWindowSize().y / 2); // Return cursor to center
		}



		if (bossDefeated >= 4) {
			gameState = STATE_ENDGAME;
		}

		// Just for testing
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


		if (GetKey(Key::ESCAPE).bPressed)
		{
			!isMenuOpen ? gameState = STATE_MENU : gameState = STATE_GAMEPLAY;
			submenu = 0;
			SetCursorPos(GetWindowPosition().x + GetWindowSize().x / 2, GetWindowPosition().y + GetWindowSize().y / 2); // Return cursor to center
		}

		break;



	case STATE_EDITOR:

		mapEditor.isEditorOpened = true;
		showCursor = true;

		mapEditor.DrawEditor(fElapsedTime);

		UserControls(fElapsedTime);
		break;


	case STATE_TITLESCREEN:

		showCursor = true;

		DrawTitleScreen(fElapsedTime);
		break;


	case STATE_ENDGAME:


		DrawEndScreen();

		if (GetKey(Key::ESCAPE).bPressed)
		{
			!isMenuOpen ? gameState = STATE_MENU : gameState = STATE_ENDGAME;
			submenu = 0;
			SetCursorPos(GetWindowPosition().x + GetWindowSize().x / 2, GetWindowPosition().y + GetWindowSize().y / 2); // Return cursor to center
		}

		break;

	case STATE_LORESCREEN:


		DrawLoreScreen();

		if (GetKey(Key::SPACE).bPressed || GetKey(Key::E).bPressed)
		{
			gameState = STATE_GAMEPLAY;
			SetCursorPos(GetWindowPosition().x + GetWindowSize().x / 2, GetWindowPosition().y + GetWindowSize().y / 2); // Return cursor to center
		}

		break;


	}






	// UI ==============================================================================


	// Draw mouse cursor
	if (showCursor) {
		DrawSpriteColorTransparent(GetMouseX(), GetMouseY(), &spriteCursor, CYAN);
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





