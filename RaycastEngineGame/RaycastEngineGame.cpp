

#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"


#include <regex>
#include <iostream>
#include <fstream>

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


void StartMIDI() {

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

	TinyMidiLoader = tml_load_filename("sw_underground.mid");
	if (!TinyMidiLoader)
	{
		fprintf(stderr, "Could not load MIDI file\n");
	}

	//Set up the global MidiMessage pointer to the first MIDI message
	g_MidiMessage = TinyMidiLoader;

	// Load the SoundFont from a file
	g_TinySoundFont = tsf_load_filename("gzdoom.sf2");
	if (!g_TinySoundFont)
	{
		fprintf(stderr, "Could not load SoundFont\n");
	}

	//Initialize preset on special 10th MIDI channel to use percussion sound bank (128) if available
	tsf_channel_set_bank_preset(g_TinySoundFont, 9, 128, 0);

	// Set the SoundFont rendering output mode
	tsf_set_output(g_TinySoundFont, TSF_STEREO_INTERLEAVED, OutputAudioSpec.freq, 0.0f);

	// Request the desired audio output format
	if (SDL_OpenAudio(&OutputAudioSpec, TSF_NULL) < 0)
	{
		fprintf(stderr, "Could not open the audio hardware or the desired audio output format\n");
	}


	tsf_set_volume(g_TinySoundFont, 0.0);


	// Start the actual audio playback here
	// The audio thread will begin to call our AudioCallback function
	SDL_PauseAudio(0);


	//Wait until the entire MIDI file has been played back (until the end of the linked message list is reached)
	//while (g_MidiMessage != NULL) SDL_Delay(100);

	// We could call tsf_close(g_TinySoundFont) and tml_free(TinyMidiLoader)
	// here to free the memory and resources but we just let the OS clean up
	// because the process ends here.

}


void RestartMIDI() {

	tml_message* TinyMidiLoader = NULL;
	g_Msec = 0;


	TinyMidiLoader = tml_load_filename("sw_underground.mid");
	if (!TinyMidiLoader)
	{
		fprintf(stderr, "Could not load MIDI file\n");
	}

	//Set up the global MidiMessage pointer to the first MIDI message
	g_MidiMessage = TinyMidiLoader;

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


	enum GameState { STATE_GAMEPLAY, STATE_MENU, STATE_EDITOR };
	GameState gameState;


	

	// Some internal stuff ---------------------------

	bool isMenuOpen = false;


	bool* drawnPixels;
	//1D Zbuffer
	float* ZBuffer;



	bool showCursor = false;




	std::string infoLog0 = "";
	std::string infoLog1 = "";
	std::string infoLog2 = "";
	float infoLogTimer0 = 0;
	float infoLogTimer1 = 0;
	float infoLogTimer2 = 0;

	float infoEffectTimer = 0;








	//===============================================================================================================================
	// MAP
	//===============================================================================================================================



	uint32_t newId;




	std::string worldMapName;

	int worldMapWidth;
	int worldMapHeight;

	std::vector<int> worldMap;






	void LoadMap(std::string mapName) {

		// Clear arrays
		enemiesArray.clear();
		decorationsArray.clear();
		itemsArray.clear();
		thingsArray.clear();



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
					std::cout << "-> Param found : " << param << '\n';

					if (std::regex_match(param, std::regex("(name=\")(.*)(\")"))) {
						param = param.substr(6, param.length() - 7);
						std::cout << "-> Name found : " << param << '\n';
						paramMapName = param;
						readMapName = true;
					}

					if (std::regex_match(param, std::regex("(width=\")(.*)(\")"))) {
						param = param.substr(7, param.length() - 8);
						std::cout << "-> Width found : " << param << '\n';
						paramMapWidth = std::stoi(param);
						readMapWidth = true;
					}

					if (std::regex_match(param, std::regex("(height=\")(.*)(\")"))) {
						param = param.substr(8, param.length() - 9);
						std::cout << "-> Height found : " << param << '\n';
						paramMapHeight = std::stoi(param);
						readMapHeight = true;
					}
				}

				// Check for labels ---------------------------------

				if (readMapName && readMapWidth && readMapHeight) {
					if (std::regex_match(line, std::regex("(label:)(.*)"))) {
						std::string label;
						label = line.substr(6);
						std::cout << "-> Label found : " << label << '\n';

						if (std::regex_match(label, std::regex("(MAPSTART)"))) {
							readingMapLayout = true;
							continue;
							std::cout << "-> MAPSTART found : " << '\n';
						}

						if (std::regex_match(label, std::regex("(MAPEND)"))) {
							readingMapLayout = false;
							readMapLayout = true;
							continue;
							std::cout << "-> MAPSTART found" << '\n';
						}
					}
				}


				// Reading map ---------------------------------

				if (readingMapLayout) {
					std::cout << "Map row : " << line << '\n';
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

					// Enemies ---------------------------------

					if (std::regex_match(line, std::regex("(enemy:)(.*)"))) {
						std::string enemy;
						enemy = line.substr(6);
						std::cout << "-> Enemy found : " << enemy << '\n';


						int readType = -1;
						olc::vf2d readPosition{ -1, -1 };

						std::smatch match;



						// Type
						std::regex_search(enemy.cbegin(), enemy.cend(), match, std::regex("(type=\")([0-9]*)(\":)"));
						std::string foundType = match.str();
						if (foundType != "") {
							foundType = foundType.substr(0, foundType.length() - 1); // remove ":" in the end
							//std::cout << "-> Type found : " << foundType << '\n';
							readType = std::stoi(foundType.substr(6, foundType.length() - 7));
							std::cout << "-> Type read : " << readType << '\n';
						} else {
							std::cout << "-> ERROR : Enemy type reading fail" << std::endl;
						}

						// Position
						std::regex_search(enemy.cbegin(), enemy.cend(), match, std::regex("(position=\")(.*)(\":)"));
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

							std::cout << "-> Position read V : " << readPosition << '\n';
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
						std::string decoration;
						decoration = line.substr(11);
						std::cout << "-> Decoration found : " << decoration << '\n';


						int readType = -1;
						olc::vf2d readPosition{ -1, -1 };

						std::smatch match;



						// Type
						std::regex_search(decoration.cbegin(), decoration.cend(), match, std::regex("(type=\")([0-9]*)(\":)"));
						std::string foundType = match.str();
						if (foundType != "") {
							foundType = foundType.substr(0, foundType.length() - 1); // remove ":" in the end
							//std::cout << "-> Type found : " << foundType << '\n';
							readType = std::stoi(foundType.substr(6, foundType.length() - 7));
							std::cout << "-> Type read : " << readType << '\n';
						} else {
							std::cout << "-> ERROR : Decoration type reading fail" << std::endl;
						}

						// Position
						std::regex_search(decoration.cbegin(), decoration.cend(), match, std::regex("(position=\")(.*)(\":)"));
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

							std::cout << "-> Position read V : " << readPosition << '\n';
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
						std::string item;
						item = line.substr(5);
						std::cout << "-> Item found : " << item << '\n';


						int readType = -1;
						olc::vf2d readPosition{ -1, -1 };

						std::smatch match;



						// Type
						std::regex_search(item.cbegin(), item.cend(), match, std::regex("(type=\")([0-9]*)(\":)"));
						std::string foundType = match.str();
						if (foundType != "") {
							foundType = foundType.substr(0, foundType.length() - 1); // remove ":" in the end
							//std::cout << "-> Type found : " << foundType << '\n';
							readType = std::stoi(foundType.substr(6, foundType.length() - 7));
							std::cout << "-> Type read : " << readType << '\n';
						}
						else {
							std::cout << "-> ERROR : Item type reading fail" << std::endl;
						}

						// Position
						std::regex_search(item.cbegin(), item.cend(), match, std::regex("(position=\")(.*)(\":)"));
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

							std::cout << "-> Position read V : " << readPosition << '\n';
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


				}


			}
			myfile.close();


			// Apply parameters
			worldMap = mapLayout;
			worldMapWidth = paramMapWidth;
			worldMapHeight = paramMapHeight;
			worldMapName = paramMapName;

			std::cout << "============================" << std::endl;
			std::cout << "Map : " << worldMapName << std::endl;
			std::cout << "Width : " << worldMapWidth << "   Height : " << worldMapHeight << std::endl;
			std::cout << "============================" << std::endl;


		}
		else std::cout << "Unable to open file";
	}


	void SaveMap(std::string mapName) {

		std::ofstream myfile(mapName);
		if (myfile.is_open())
		{
			myfile << "param:name=\"" << worldMapName << "\"" << std::endl;
			myfile << "param:width=\"" << std::to_string(worldMapWidth) << "\"" << std::endl;
			myfile << "param:height=\"" << std::to_string(worldMapHeight) << "\"" << std::endl;

			myfile << "label:MAPSTART" << std::endl;

			for (int i = 0; i < worldMapHeight; i++) {
				for (int j = 0; j < worldMapWidth; j++) {
					myfile << worldMap[i * worldMapWidth + j] << ",";
				}
				myfile << std::endl;
			}

			myfile << "label:MAPEND" << std::endl;

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





	//===============================================================================================================================
	// GAME OBJECTS
	//===============================================================================================================================


	struct Weapon {
		float damage;
		float fireRate;

		bool onPlayer;
		int weaponState; // for changing sprites ( 0 - idle, 1 - shoot, 2 - back to idle )

		int currentAmmo;
		int maxAmmo;
	};

	Weapon weapons[3] = {
		// Pistol
		{5.0, 0.2, true, 0, 40, 100},
		// Rifle
		{5.0, 0.1, true, 0, 100, 100},
		// Shotgun
		{5.0, 0.7, true, 0, 50, 50},
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
			std::cout << "Created Enemy | ID : " << id << std::endl;
		};
		Enemy(RaycastEngine* engine, uint32_t globId, int tex, olc::vf2d pos) : Thing(engine, globId, 1 , tex, pos) {
			std::cout << "Created Enemy, param | ID : " << id << std::endl;
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

		float timer;

		int health = 100;
		bool isAlive = true;

		olc::vf2d velocity{ 0,0 };
		olc::vf2d moveVector{ 1, 1 };
		float moveSpeed = 0.2;


		void Update(float fElapsedTime) {
			if (isAlive) {
				timer -= fElapsedTime;

				// Change idle states

				if (timer <= 0 && enemyState == 0) {
					enemyState = 1;
					spritePartIndex = enemyState; // Change sprite part to render
					timer = 0.5;



					if (rand() % 1000 > 500) {
						moveVector = moveVector * olc::vf2d(1 - rand() % 2000 / 1000.0, 1 - rand() % 2000 / 1000.0);
					}
				}

				if (timer <= 0 && enemyState == 1) {
					enemyState = 2;
					spritePartIndex = enemyState; // Change sprite part to render
					timer = 0.5;
				}

				if (timer <= 0 && enemyState == 2) {
					enemyState = 0;
					spritePartIndex = enemyState; // Change sprite part to render
					timer = 0.5;
				}


				// Change dying states

				if (timer <= 0 && enemyState == 3) {
					enemyState = 4;
					spritePartIndex = enemyState; // Change sprite part to render
					timer = 0.3;
				}

				if (timer <= 0 && enemyState == 4) {
					enemyState = 5;
					spritePartIndex = enemyState; // Change sprite part to render
					isAlive = false;
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


				engineReference->UpdateThings();
			}
		}


		void TakeDamage() {
			health -= 20;
			std::cout << "Damage, health remains : " << health << std::endl;
			if (health <= 0) {
				health = 0;
				enableCollision = false;

				enemyState = 3;
				timer = 0.3;
				spritePartIndex = enemyState;

				engineReference->UpdateThings();
			}
		}





	};


	class Item : public Thing {

	public:

		Item(RaycastEngine* engine) : Thing(engine) {
			std::cout << "Created Item | ID : " << id << std::endl;
		};
		Item(RaycastEngine* engine, uint32_t globId, int tex, olc::vf2d pos) : Thing(engine, globId, 2 ,tex, pos) {
			std::cout << "Created Item, param  | ID : " << id << std::endl;

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

			CheckForPickup();

		}


		void CheckForPickup() {
			if (!isPickedup) {
				float distanceToItem = (engineReference->player.position - position).mag2();
				if (distanceToItem < 0.3) {
					enableRender = false;
					isPickedup = true;
					std::cout << "Item picked up | ID : " << id << std::endl;

					switch (texture) // Switch item type
					{
					case 0: // Medkit
						engineReference->player.health += 20;
						engineReference->InfoLog("Picked up Medkit");
						break;
					case 1: // Ammo 9mm
						engineReference->weapons[0].currentAmmo += 10;
						engineReference->weapons[1].currentAmmo += 10;
						engineReference->InfoLog("Picked up 9mm ammo");
						break;
					case 2: // Ammo Shells
						engineReference->weapons[2].currentAmmo += 10;
						engineReference->InfoLog("Picked up shells");
						break;

					default:
						break;
					}

				}
			}
		}

		


	};


	class Decoration : public Thing {

	public:
		Decoration(RaycastEngine* engine) : Thing(engine) {
			std::cout << "Created Decoration | ID : " << id << std::endl;
		};
		Decoration(RaycastEngine* engine, uint32_t globId, int tex, olc::vf2d pos) : Thing(engine, globId, 0, tex, pos) {
			std::cout << "Created Decoration, param | ID : " << id << std::endl;

			// Disable collision for lamps
			if (tex == 2) { 
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
		//std::cout << "Getting enemy | ID:  " << id << std::endl;
		for (int i = 0; i < enemiesArray.size(); i++) {
			if (enemiesArray[i].id == id) {
				//std::cout << "Found enemy | ID:  " << id << std::endl;
				return &enemiesArray[i];
			}
		}
		//std::cout << "Enemy with ID: " << id << " not found" << std::endl;
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

		// Collision
		bool enableCollision = true;
		float collisionSize = 0.5;


		// Movement
		float moveSpeed = 5.0;
		float rotationSpeed = 3.0;

		olc::vf2d playerVelocity = { 0, 0 };

		float bobValue;


		bool isMoving = false;
		bool isShooting = false;
		olc::vf2d controlMoveVector = { 0, 0 };
		olc::vf2d controlRotationVector = { 0 ,0 };


		int health = 100;


		int activeWeapon = 0;
		float weaponDelay = 0;


		void Update(float fElapsedTime) {

			Move(fElapsedTime);

			// Bobbing

			bobValue += 1000.0f * playerVelocity.mag() * fElapsedTime;

			// Weapon states

			if (engineReference->weapons[activeWeapon].weaponState != 0) {
				weaponDelay -= fElapsedTime;

				if (weaponDelay < engineReference->weapons[activeWeapon].fireRate / 2) {
					engineReference->weapons[activeWeapon].weaponState = 2;
				}

				if (weaponDelay <= 0) {
					weaponDelay = 0;
					engineReference->weapons[activeWeapon].weaponState = 0;
				}
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

			if (engineReference->weapons[activeWeapon].weaponState == 0) { // If weapon is idle

				if (engineReference->weapons[activeWeapon].currentAmmo > 0) { // If player have ammo

					// Change weapon state
					engineReference->weapons[activeWeapon].weaponState = 1; // Shooting
					weaponDelay = engineReference->weapons[activeWeapon].fireRate;

					// Shooting

					engineReference->weapons[activeWeapon].currentAmmo--;

					// Weapon 0 and 1 (pistol and uzi) have same ammo, so i equal them
					if (activeWeapon == 0) engineReference->weapons[1].currentAmmo = engineReference->weapons[activeWeapon].currentAmmo;
					if (activeWeapon == 1) engineReference->weapons[0].currentAmmo = engineReference->weapons[activeWeapon].currentAmmo;


					if (activeWeapon == 0 || activeWeapon == 1) { // Pistol and Uzi
						ShootRay(0);
					}
					if (activeWeapon == 2) { // Shotgun
						for (int s = 0; s < 20; s++)
							ShootRay(-0.5 + s / 20.0); // from -0.5 to  0.5
					}

				}


			}
		}



		void ShootRay(float angle) {

			float rayDistance = engineReference->ShootCameraRay(angle);
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
						hittedEnemy->TakeDamage();
						hittedEnemy = nullptr;
					}
					break;
				}
			}
		}




	};






	// World objects ------------------------

	Player player;


	std::vector<Thing> thingsArray;

	std::vector<Enemy> enemiesArray;

	std::vector<Decoration> decorationsArray;

	std::vector<Item> itemsArray;



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

	// Walls
	olc::Sprite spritesWall1;
	olc::Sprite spritesWall2;
	olc::Sprite spritesWall3;
	olc::Sprite spritesWall4;
	olc::Sprite spritesWall5;
	olc::Sprite spritesWall6;
	olc::Sprite spritesWall7;
	olc::Sprite spritesWall8;

	olc::Sprite spriteBarell;
	olc::Sprite spritePillar;
	olc::Sprite spriteLamp;



	olc::Sprite spritesRobot1;


	// Weapon sprites
	olc::Sprite weaponPistol;
	olc::Sprite weaponUzi;
	olc::Sprite weaponShotgun;


	// Item sprites
	olc::Sprite spritesMedkit;
	olc::Sprite itemAmmo9mm;
	olc::Sprite itemAmmoShells;



	std::vector<olc::Sprite> itemIconSprites;



	olc::Sprite SampleIconFromSprite(olc::Sprite* sprite, int32_t x, int32_t y, int32_t w, int32_t h){
		std::cout << "SampleIconFromSprite" << std::endl;
		olc::Sprite newIcon = olc::Sprite(w, h);

		std::cout << "Sampling..." << std::endl;
		for (int smplX = 0; smplX < w; smplX++) {
			for (int smplY = 0; smplY < h; smplY++) {
				olc::Pixel color = sprite->GetPixel(x + smplX, y + smplY);
				//olc::Pixel color = sprite->GetPixel(int(1.0 * sprite->width / w * smplX), int(1.0 * sprite->height / h * smplY));
				newIcon.SetPixel(smplX, smplY, color);
			}
		}

		DrawSprite(olc::vi2d(100,100), &newIcon);

		std::cout << "Return" << std::endl;
		return(newIcon);
	}


	void LoadSprite(olc::Sprite* sprite, std::string file) {
		olc::rcode loadStatus = sprite->LoadFromFile(file);
		if (loadStatus == olc::FAIL) { std::cout << "Sprite load failed : " << file << std::endl; return; }
		if (loadStatus == olc::NO_FILE) { std::cout << "Sprite not found : " << file << std::endl; return; }
	}

	void LoadAssets() {

		//olc::rcode loadStatus;

		LoadSprite(&spriteCursor, "gfx/cursor.png");

		// Load the sprite
		LoadSprite(&spritesWall1, "gfx/wall_stone1.png");
		LoadSprite(&spritesWall2, "gfx/wall_brick1.png");
		LoadSprite(&spritesWall3, "gfx/wall_metal1.png");
		LoadSprite(&spritesWall4, "gfx/wall_stoneBlue1.png");
		LoadSprite(&spritesWall5, "gfx/wall_wood1.png");
		LoadSprite(&spritesWall6, "gfx/wall_concrete1.png");
		LoadSprite(&spritesWall7, "gfx/wall_concreteBrick1.png");
		LoadSprite(&spritesWall8, "gfx/wall_concreteBrick2.png");

		LoadSprite(&spriteBarell, "gfx/barrel.png");
		LoadSprite(&spritePillar, "gfx/pillar.png");
		LoadSprite(&spriteLamp, "gfx/greenlight.png");

	

		LoadSprite(&spritesRobot1, "gfx/robot1.png");

		LoadSprite(&weaponPistol, "gfx/weaponPistol.png");
		LoadSprite(&weaponUzi, "gfx/weaponUzi.png");
		LoadSprite(&weaponShotgun, "gfx/weaponShotgun.png");


		LoadSprite(&spritesMedkit, "gfx/medkit.png");
		itemIconSprites.push_back(SampleIconFromSprite(&spritesMedkit, 20, 39, 23, 23));
		LoadSprite(&itemAmmo9mm, "gfx/itemAmmo9mm.png");
		itemIconSprites.push_back(SampleIconFromSprite(&itemAmmo9mm, 20, 39, 23, 23));
		LoadSprite(&itemAmmoShells, "gfx/itemAmmoShells.png");
		itemIconSprites.push_back(SampleIconFromSprite(&itemAmmoShells, 20, 39, 23, 23));


		// Editor
		LoadSprite(&spriteEditorPlay, "gfx/editorPlay.png");
		LoadSprite(&spriteEditorSave, "gfx/editorSave.png");
		LoadSprite(&spriteEditorLoad, "gfx/editorLoad.png");
		LoadSprite(&spriteEditorSettings, "gfx/editorSettings.png");

		LoadSprite(&spriteEditorToolWall, "gfx/editorToolWall.png");
		LoadSprite(&spriteEditorToolItem, "gfx/editorToolItem.png");
		LoadSprite(&spriteEditorToolDecor, "gfx/editorToolDecor.png");
		LoadSprite(&spriteEditorToolEnemy, "gfx/editorToolEnemy.png");
	}



	olc::Sprite* GetWallTexture(int idx) {
		switch(idx){
			case 0: return &spritesWall1;
			case 1: return &spritesWall2;
			case 2: return &spritesWall3;
			case 3: return &spritesWall4;
			case 4: return &spritesWall5;
			case 5: return &spritesWall6;
			case 6: return &spritesWall7;
			case 7: return &spritesWall8;
			default: return &spritesWall5;
		}
	}

	olc::Sprite* GetDecorationSprite(int idx) {
		switch (idx) {
		case 0: return &spriteBarell;
		case 1: return &spritePillar;
		case 2: return &spriteLamp;
		default: return &spriteBarell;
		}
	}

	olc::Sprite* GetEnemySprite(int idx) {
		switch (idx) {
		case 0: return &spritesRobot1;
		default: return &spritesRobot1;
		}
	}

	olc::Sprite* GetItemSprite(int idx) {
		switch (idx) {
		case 0: return &spritesMedkit;
		case 1: return &itemAmmo9mm;
		case 2: return &itemAmmoShells;
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

		DrawPartialSpriteColorTransparent
		(
			ScreenWidth() / 2 + 0 - sin(player.bobValue + 1.55) * 8,
			ScreenHeight() - 110 - (abs(sin(player.bobValue)) * 8),
			weapons[player.activeWeapon].weaponState * 128,
			0,
			128,
			128,
			GetWeaponSprite(player.activeWeapon),
			olc::CYAN
		);


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

	}


	void DrawMenu() {

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

		DrawString(72, 27, "Pause", olc::VERY_DARK_YELLOW, 3);
		DrawString(70, 25, "Pause", olc::YELLOW, 3);

		DrawString(92, 67, "New game", olc::VERY_DARK_YELLOW, 2);
		DrawString(90, 65, "New game", olc::YELLOW, 2);

		DrawString(92, 87, "Load game", olc::VERY_DARK_YELLOW, 2);
		DrawString(90, 85, "Load game", olc::YELLOW, 2);

		DrawString(92, 107, "Save game", olc::VERY_DARK_YELLOW, 2);
		DrawString(90, 105, "Save game", olc::YELLOW, 2);

		DrawString(92, 127, "Exit", olc::VERY_DARK_YELLOW, 2);
		DrawString(90, 125, "Exit", olc::YELLOW, 2);


		if (menuItemSelected != -1) {
			DrawString(72, 67 + 20 * menuItemSelected, ">", olc::VERY_DARK_YELLOW, 2);
			DrawString(70, 65 + 20 * menuItemSelected, ">", olc::YELLOW, 2);
		}




		if (GetMouse(0).bPressed) {
			switch (menuItemSelected)
			{
			case 0:
				OnUserCreate();
				break;
			case 1:
				break;
			case 2:
				break;
			case 3:
				exit(0);
				break;
			default:
				break;
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
	float ShootCameraRay(float cameraX) {

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
			int drawStart = -lineHeight / 2 + ScreenHeight() / 2;
			if (drawStart < 0)drawStart = 0;
			int drawEnd = lineHeight / 2 + ScreenHeight() / 2;
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
				float shading = perpWallDist;
				if (shading > 15) shading = 15;
				shading = 1 - shading / 15;

				color = GetWallTexture(textureNumber)->Sample(wallX, wallY);
				color = DarkColor(color, shading);

				if (side == 1) color = DarkColor(color);

				Draw(x, y, color);
				wallY += wallYstep;

				drawnPixels[y * ScreenWidth() + x] = true;
			}

			//SET THE ZBUFFER FOR THE SPRITE CASTING
			ZBuffer[x] = perpWallDist; //perpendicular distance is used
		}


		//FLOOR CASTING
		for (int y = ScreenHeight() / 2; y < ScreenHeight(); y++)
		{
			// rayDir for leftmost ray (x = 0) and rightmost ray (x = w)
			olc::vf2d rayDir0 = player.direction - player.viewPlane;

			olc::vf2d rayDir1 = player.direction + player.viewPlane;


			// Current y position compared to the center of the screen (the horizon)
			int p = y - ScreenHeight() / 2;

			// Vertical position of the camera.
			float posZ = 0.5 * ScreenHeight();

			// Horizontal distance from the camera to the floor for the current row.
			// 0.5 is the z position exactly in the middle between floor and ceiling.
			float rowDistance = posZ / p;

			// calculate the real world step vector we have to add for each x (parallel to camera plane)
			// adding step by step avoids multiplications with a weight in the inner loop
			float floorStepX = rowDistance * (rayDir1.x - rayDir0.x) / ScreenWidth();
			float floorStepY = rowDistance * (rayDir1.y - rayDir0.y) / ScreenWidth();

			// real world coordinates of the leftmost column. This will be updated as we step to the right.
			float floorX = player.position.x + rowDistance * rayDir0.x;
			float floorY = player.position.y + rowDistance * rayDir0.y;

			for (int x = 0; x < ScreenWidth(); ++x)
			{
				// the cell coord is simply got from the integer parts of floorX and floorY
				int cellX = (int)(floorX);
				int cellY = (int)(floorY);

				// get the texture coordinate
				floorX += floorStepX;
				floorY += floorStepY;


				float shading = 1 - (0.8 * (ScreenHeight() - y) / (ScreenHeight() / 2));

				if (!drawnPixels[y * ScreenWidth() + x]) {
					// floor
					olc::Pixel color = GetWallTexture(7)->Sample(floorX - cellX, floorY - cellY);
					color = DarkColor(color, shading);
					Draw(x, y, color);

					//ceiling (symmetrical, at screenHeight - y - 1 instead of y)
					color = GetWallTexture(5)->Sample(floorX - cellX, floorY - cellY);
					color = DarkColor(color, shading);
					Draw(x, ScreenHeight() - y - 1, color);
				}
			}
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

					//calculate height of the sprite on screen
					int spriteHeight = abs(int(ScreenHeight() / (transformY))); //using 'transformY' instead of the real distance prevents fisheye
					//calculate lowest and highest pixel to fill in current stripe
					int drawStartY = -spriteHeight / 2 + ScreenHeight() / 2;
					if (drawStartY < 0) drawStartY = 0;
					int drawEndY = spriteHeight / 2 + ScreenHeight() / 2;
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
								int d = (y) * 256 - ScreenHeight() * 128 + spriteHeight * 128; //256 and 128 factors to avoid floats
								int texY = ((d * 64) / spriteHeight) / 256;


								olc::Pixel color;
								if (thingsArray[spriteOrder[i]].thingType == 0) { // Decoration
									color = GetDecorationSprite(thingsArray[spriteOrder[i]].texture)->GetPixel(texX + (thingsArray[spriteOrder[i]].spritePartIndex * 64), texY);
								}
								if (thingsArray[spriteOrder[i]].thingType == 1) { // Enemy
									color = GetEnemySprite(thingsArray[spriteOrder[i]].texture)->GetPixel(texX + (thingsArray[spriteOrder[i]].spritePartIndex * 64), texY);
								}
								if (thingsArray[spriteOrder[i]].thingType == 2) { // Item
									color = GetItemSprite(thingsArray[spriteOrder[i]].texture)->GetPixel(texX + (thingsArray[spriteOrder[i]].spritePartIndex * 64), texY);
								}


								float shading = spritePosRel.mag2();
								if (shading > 60) shading = 60;
								shading = 1 - shading / 80;


								if (color != olc::BLACK && color != olc::CYAN)
								{
									Draw(stripe, y, DarkColor(color, shading)); //paint pixel if it isn't black, black is the invisible color
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

		if (!isMenuOpen && !isEditorOpened) {

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
				player.activeWeapon = 0;
			}

			if (GetKey(olc::Key::K2).bPressed)
			{
				player.activeWeapon = 1;
			}

			if (GetKey(olc::Key::K3).bPressed)
			{
				player.activeWeapon = 2;
			}

		}
		

		// Mouse controls ------------------------------------------------------

		if (IsFocused() && !isMenuOpen && !isEditorOpened) {
			if (GetWindowMouse().x != 0 && GetWindowMouse().y != 0) { // To prevent cursor set while dragging window

				float deltaMouseX = 0.05 * (GetWindowMouse().x - GetWindowSize().x / 2) / (fElapsedTime / 0.016);
				player.controlRotationVector += olc::vf2d(deltaMouseX, 0);
				SetCursorPos(GetWindowPosition().x + GetWindowSize().x / 2, GetWindowPosition().y + GetWindowSize().y / 2);
			}
		}
		else if (!IsFocused()){
			isMenuOpen = true;
			showCursor = true;
			gameState = STATE_MENU;
		}


		// Other ------------------------------------------------------

		if (GetKey(olc::Key::ESCAPE).bPressed)
		{
			isMenuOpen = !isMenuOpen;
			showCursor = isMenuOpen;
			isMenuOpen ? gameState = STATE_MENU : gameState = STATE_GAMEPLAY;

			isEditorOpened = false;

			SetCursorPos(GetWindowPosition().x + GetWindowSize().x / 2, GetWindowPosition().y + GetWindowSize().y / 2); // Return cursor to center
		}



		if (GetKey(olc::Key::M).bPressed)
		{
			isEditorOpened = !isEditorOpened;
			showCursor = isEditorOpened;
			isEditorOpened ? gameState = STATE_EDITOR : gameState = STATE_GAMEPLAY;

			isMenuOpen = false;
			
			SetCursorPos(GetWindowPosition().x + GetWindowSize().x / 2, GetWindowPosition().y + GetWindowSize().y / 2); // Return cursor to center
		}




	}



	//===============================================================================================================================
	// MAP EDITOR 
	//===============================================================================================================================

	bool isEditorOpened = false;


	olc::vi2d gridShift {0,0};
	olc::vi2d gridOrigin {0,0};

	int editorCellSize = 20;
	bool showGrid = true;


	olc::vf2d selectedCell{ 0,0 };
	
	

	olc::vi2d mousePosPrev{ GetMouseX(), GetMouseY() };



	int toolSelected = 0;

	int selectedToolWall = 0;
	int selectedToolDecor = 0;
	int selectedToolItem = 0;
	int selectedToolEnemy = 0;


	bool showToolSelection = false;
	int toolSelectionPage = 0;




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


	void Editor_DrawEditor(float fElapsedTime) {
		
		bool mouseOnUI = false;


		

		// Background -----------------------

		FillRect(0, 0, ScreenWidth(), ScreenHeight(), olc::VERY_DARK_GREY);


		// Draw map -----------------------

		int sampleSize = editorCellSize - 1 + !showGrid; // Little optimisation

		for (int i = 0; i < worldMapWidth; i++) {
			for (int j = 0; j < worldMapHeight; j++) {
				
				if (worldMap[j * worldMapWidth + i] != 0){
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
		}



		// Draw objects -----------------------


		// Draw Decorations

		for (int d = 0; d < decorationsArray.size(); d++) {

			for (int smplX = 0; smplX < editorCellSize - 1; smplX++) {
				for (int smplY = 0; smplY < editorCellSize - 1; smplY++) {
					olc::Pixel color = GetDecorationSprite(decorationsArray[d].texture)->GetPixel(int(64.0 / editorCellSize * smplX), int(64.0 / editorCellSize * smplY));
					if (color != olc::BLACK && color != olc::CYAN) {
						// decorationsArray[d].position.x - 0.5 in fact tells that i need to shift sprite half size
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
					//olc::Pixel color = itemIconSprites[itemsArray[i].texture].GetPixel(floor(24.0 / editorCellSize * smplX), floor(24.0 / editorCellSize * smplY));
					olc::Pixel color = itemIconSprites[itemsArray[i].texture].GetPixel(smplX, smplY);
					if (color != olc::BLACK && color != olc::CYAN) {
						int pixelPosX = gridOrigin.x + 1 + smplX + (itemsArray[i].position.x - 0.5) * editorCellSize;
						int pixelPosY = gridOrigin.y + 1 + smplY + (itemsArray[i].position.y - 0.5) * editorCellSize;
						Draw(pixelPosX, pixelPosY, color);
					}
				}
			}
		}

		// Draw Enemies

		//for (int d = 0; d < decorationsArray.size(); d++) {
		//
		//	for (int smplX = 0; smplX < editorCellSize - 1; smplX++) {
		//		for (int smplY = 0; smplY < editorCellSize - 1; smplY++) {
		//			olc::Pixel color = GetDecorationSprite(decorationsArray[d].texture)->GetPixel(int(64.0 / editorCellSize * smplX), int(64.0 / editorCellSize * smplY));
		//			if (color != olc::BLACK && color != olc::CYAN) {
		//				int pixelPosX = gridOrigin.x + 1 + smplX + (decorationsArray[d].position.x - 0.5) * editorCellSize;
		//				int pixelPosY = gridOrigin.y + 1 + smplY + (decorationsArray[d].position.y - 0.5) * editorCellSize;
		//				Draw(pixelPosX, pixelPosY, color);
		//			}
		//		}
		//	}
		//}


		// Cell selection ----------------------- 


		selectedCell.x = (GetMouseX() - gridOrigin.x) / (editorCellSize * 1.0);
		selectedCell.y = (GetMouseY() - gridOrigin.y) / (editorCellSize * 1.0);


		if (toolSelected == 0 || toolSelected == 1) { // Wall or Decoration

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
			std::string cellText = cellXText.substr(0, cellXText.length()-5) + " " + cellYText.substr(0, cellYText.length() - 5);
			DrawString(5, ScreenHeight() - 10, cellText);
		}




		// UI -----------------------

		std::string hoverText = "";


		int debugMX = GetMouseX();
		int debugMY = GetMouseY();
		std::string debugMText = std::to_string(debugMX) + " " + std::to_string(debugMY);
		DrawString(200, 180, debugMText, olc::WHITE);



		FillRect(0,0,320,30,olc::BLACK);

		// Save button
		Button saveButton = Button(this, olc::vi2d(3, 3), 22, 22, &spriteEditorSave);
		saveButton.showBorder = false;
		saveButton.hoverText = "SAVE";
		saveButton.Update();
		if (saveButton.isHovered) { mouseOnUI = true; hoverText = saveButton.hoverText; }
		if (saveButton.isPressed) SaveMap("maps/map02.txt");

		// Load button
		Button loadButton = Button(this, olc::vi2d(28, 3), 22, 22, &spriteEditorLoad);
		loadButton.showBorder = false;
		loadButton.hoverText = "LOAD";
		loadButton.Update();
		if (loadButton.isHovered) { mouseOnUI = true; hoverText = loadButton.hoverText; }
		if (loadButton.isPressed) LoadMap("maps/map02.txt");

		// Settings button
		Button settingsButton = Button(this, olc::vi2d(53, 3), 22, 22, &spriteEditorSettings);
		settingsButton.showBorder = false;
		settingsButton.hoverText = "SETTINGS";
		settingsButton.Update();
		if (settingsButton.isHovered) { mouseOnUI = true; hoverText = settingsButton.hoverText; }
		if (settingsButton.isPressed) LoadMap("maps/map02.txt");

		// Play button
		Button playButton = Button(this, olc::vi2d(279, 0), 30, 30, &spriteEditorPlay);
		playButton.showBorder = false;
		playButton.hoverText = "PLAY";
		playButton.Update();
		if (playButton.isHovered) { mouseOnUI = true; hoverText = playButton.hoverText; }
		if (playButton.isPressed) LoadMap("maps/map02.txt");



		// Tool Wall button
		Button toolWallButton = Button(this, olc::vi2d(90, 5), 20, 20, &spriteEditorToolWall);
		toolWallButton.showBorder = true;
		toolWallButton.hoverText = "WALL";
		toolWallButton.Update();
		if (toolWallButton.isHovered) { mouseOnUI = true; hoverText = toolWallButton.hoverText; }
		if (toolWallButton.isPressed) toolSelected = 0;

		// Tool Decorations button
		Button toolDecorButton = Button(this, olc::vi2d(115, 5), 20, 20, &spriteEditorToolDecor);
		toolDecorButton.showBorder = true;
		toolDecorButton.hoverText = "DECOR";
		toolDecorButton.Update();
		if (toolDecorButton.isHovered) { mouseOnUI = true; hoverText = toolDecorButton.hoverText; }
		if (toolDecorButton.isPressed) toolSelected = 1;

		// Tool Items button
		Button toolItemButton = Button(this, olc::vi2d(140, 5), 20, 20, &spriteEditorToolItem);
		toolItemButton.showBorder = true;
		toolItemButton.hoverText = "ITEM";
		toolItemButton.Update();
		if (toolItemButton.isHovered) { mouseOnUI = true; hoverText = toolItemButton.hoverText; }
		if (toolItemButton.isPressed) toolSelected = 2;

		// Tool Enemies button
		Button toolEnemyButton = Button(this, olc::vi2d(165, 5), 20, 20, &spriteEditorToolEnemy);
		toolEnemyButton.showBorder = true;
		toolEnemyButton.hoverText = "ENEMY";
		toolEnemyButton.Update();
		if (toolEnemyButton.isHovered) { mouseOnUI = true; hoverText = toolEnemyButton.hoverText; }
		if (toolEnemyButton.isPressed) toolSelected = 3;

	
		// Selected tool
		DrawRect(88 + toolSelected * 25, 3, 24, 24, olc::YELLOW);
		DrawRect(89 + toolSelected * 25, 4, 22, 22, olc::YELLOW);



		// Tool
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
			textureButton = Button(this, olc::vi2d(239, 2), 25, 25, GetItemSprite(selectedToolItem));
			break;
		case 3: // Enemies
			textureButton = Button(this, olc::vi2d(239, 2), 25, 25, GetEnemySprite(selectedToolEnemy));
			break;
		}
		textureButton.Update();
		if (textureButton.isHovered) { mouseOnUI = true; }
		if (textureButton.isPressed) showToolSelection = true;




		// Tool selection -----------------------

		if (showToolSelection) {

			mouseOnUI = true; // To prevent placing and moving in background
			
			FillRect(40, 40, 273, 142, olc::BLACK); // Background
			FillRect(195, 30, 75, 10, olc::BLACK); // Connection
			DrawLine(195, 0, 195, 40, olc::YELLOW);
			DrawLine(270, 0, 270, 40, olc::YELLOW);
			DrawLine(40, 40, 195, 40, olc::YELLOW);
			DrawLine(270, 40, 313, 40, olc::YELLOW);


			for (int toolY = 0; toolY < 3; toolY++) {
				for (int toolX = 0; toolX < 7; toolX++) {

					if (toolSelected == 0 && (toolX + toolY * 7) > 8) break;
					if (toolSelected == 1 && (toolX + toolY * 7) > 2) break;
					if (toolSelected == 2 && (toolX + toolY * 7) > 2) break;
					if (toolSelected == 3 && (toolX + toolY * 7) > 0) break;



					Button selectToolButton = Button(this);
					int textureIdx = 0;
					switch (toolSelected)
					{
					case 0: // Wall
						textureIdx = (toolX + toolY * 7) % 9;
						selectToolButton = Button(this, olc::vi2d(45 + (toolX * (34 + 4)), 45 + (toolY * (34 + 4))), 34, 34, GetWallTexture(textureIdx));
						break;
					case 1: // Decorations
						textureIdx = (toolX + toolY * 7) % 3;
						selectToolButton = Button(this, olc::vi2d(45 + (toolX * (34 + 4)), 45 + (toolY * (34 + 4))), 34, 34, GetDecorationSprite(textureIdx));
						break;
					case 2: // Items
						textureIdx = (toolX + toolY * 7) % 3;
						selectToolButton = Button(this, olc::vi2d(45 + (toolX * (34 + 4)), 45 + (toolY * (34 + 4))), 34, 34, &itemIconSprites[textureIdx]);
						selectToolButton.colorBackground = olc::WHITE;
						break;
					case 3: // Enemies
						textureIdx = (toolX + toolY * 7) % 1;
						selectToolButton = Button(this, olc::vi2d(45 + (toolX * (34 + 4)), 45 + (toolY * (34 + 4))), 34, 34, GetEnemySprite(textureIdx));
						break;
					default:
						break;
					}
					selectToolButton.Update();
					if (selectToolButton.isHovered) { mouseOnUI = true; }
					if (selectToolButton.isPressed) { Editor_SelectTool(textureIdx); showToolSelection = false; }
				}
			}

			// Ooooh, this is bad...... But i dont have other solution at this moment :(
			if (!(GetMouseX() > 40 && GetMouseX() < 313 && GetMouseY() > 40 && GetMouseY() < 182) && !(GetMouseX() > 195 && GetMouseX() < 270 && GetMouseY() > 0 && GetMouseY() < 40) && GetMouse(0).bPressed) {
				showToolSelection = false;
			}

		}




		// Hovering text -----------------------

		if (hoverText != "") {
			DrawString(GetMouseX() - 1, GetMouseY() + 18, hoverText, olc::BLACK);
			DrawString(GetMouseX() - 2, GetMouseY() + 17, hoverText, olc::WHITE);
		}





		// Editor controls -----------------------


		// Mouse movement
		if (GetMouse(1).bHeld && !mouseOnUI) {
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

		if (GetKey(olc::Key::G).bPressed)
		{
			showGrid = !showGrid;
		}




		// Place object

		if (GetMouse(0).bPressed && !mouseOnUI) {

			if (selectedCell.x >= 0 && selectedCell.y >= 0) { // Check map boundaries


				if (toolSelected == 0) { // Wall
					worldMap[selectedCell.y * worldMapWidth + selectedCell.x] = selectedToolWall + 1;
				}
				else if (toolSelected == 1) { // Decor
					Decoration newDecoration = Decoration(this, newId, selectedToolDecor, olc::vf2d(selectedCell.x + 0.5, selectedCell.y + 0.5)); newId++;
					decorationsArray.push_back(newDecoration);
					thingsArray.push_back(newDecoration.ToThing());
					spriteOrder = new int[thingsArray.size()];
					spriteDistance = new float[thingsArray.size()];
				}
				else if (toolSelected == 2) { // Item
					Item newItem = Item(this, newId, selectedToolItem, olc::vf2d(selectedCell.x, selectedCell.y)); newId++;
					itemsArray.push_back(newItem);
					thingsArray.push_back(newItem.ToThing());
					spriteOrder = new int[thingsArray.size()];
					spriteDistance = new float[thingsArray.size()];
				}
				else if (toolSelected == 3) { // Enemy
					Enemy newEnemy = Enemy(this, newId, selectedToolEnemy, olc::vf2d(selectedCell.x, selectedCell.y)); newId++;
					enemiesArray.push_back(newEnemy);
					thingsArray.push_back(newEnemy.ToThing());
					spriteOrder = new int[thingsArray.size()];
					spriteDistance = new float[thingsArray.size()];
				}


			}

		}

		// Erace

		if (GetKey(olc::Key::E).bPressed && !mouseOnUI) {
			if (selectedCell.x >= 0 && selectedCell.y >= 0) { // Check map boundaries


				if (toolSelected == 0) { // Wall
					worldMap[selectedCell.y * worldMapWidth + selectedCell.x] = 0;
				}
				else if (toolSelected == 1) { // Decor

				}
				else if (toolSelected == 2) { // Item

				}
				else if (toolSelected == 3) { // Enemy

				}
			}
		}

	}




	//===============================================================================================================================
	// MAIN ENGINE FUNCTIONS
	//===============================================================================================================================


	bool OnUserCreate() override
	{

		LoadMap("maps/map02.txt");


		// Initialisation ====================================================================

		// Called once at the start, so create things here
		drawnPixels = new bool[ScreenHeight() * ScreenWidth()];
		for (int i = 0; i < ScreenHeight() * ScreenWidth(); i++) drawnPixels[i] = false;

		// Clear Z buffer
		ZBuffer = new float[ScreenWidth()];



		// Assets ====================================================================

		LoadAssets();



		// Create objects ====================================================================




		// Create player -----------------------------

		player.engineReference = this;
		player.position = olc::vf2d{ 22.0, 12.0 };
		player.direction = olc::vf2d{ -1, 0 };
		player.viewPlane = olc::vf2d{ 0, -0.75 };




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






		// --- delete from vector fuction ---
		// change places with last element
		// pop_back()


		gameState = STATE_GAMEPLAY;


		return true;
	}



	bool OnUserUpdate(float fElapsedTime) override
	{

		// Game states ==============================================================================


		switch (gameState) {

		case STATE_GAMEPLAY:

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

			break;



		case STATE_MENU:
			
			RaycastRender();
			DrawPlayerUI(fElapsedTime);

			DrawMenu();
			break;



		case STATE_EDITOR:
			Editor_DrawEditor(fElapsedTime);
			break;

		}
		


		// Game logic ==============================================================================

		UserControls(fElapsedTime);



		// UI ==============================================================================


		// Draw mouse cursor
		if (showCursor) {
			DrawSpriteColorTransparent(GetMouseX(), GetMouseY(), &spriteCursor, olc::CYAN);
		}



		// Debug info ==============================================================================

		/*
		std::string aimDist;

		std::string text = "VEL : " + std::to_string(player.playerVelocity.x) + ", " + std::to_string(player.playerVelocity.y);
		DrawString(10, 10, text, olc::WHITE);
		
		text = "POS : " + std::to_string(player.position.x) + ", " + std::to_string(player.position.y);
		DrawString(10, 20, text, olc::WHITE);
		
		text = "MV : " + std::to_string(player.controlMoveVector.x) + ", " + std::to_string(player.controlMoveVector.y);
		DrawString(10, 30, text, olc::WHITE);
		
		text = "DIR : " + std::to_string(player.direction.x) + ", " + std::to_string(player.direction.y);
		DrawString(10, 50, text, olc::WHITE);
		
		
		DrawString(10, 60, aimDist, olc::WHITE);
		
		
		text = "SPRITES";
		DrawString(250, 10, text, olc::WHITE);
		for (int i = 0; i < thingsArrayLength; i++) {
			text = std::to_string(thingsArray[spriteOrder[thingsArrayLength - 1 - i]].texture);
			DrawString(300, 20+10*i, text, olc::WHITE);
		}
		
		text = "WS : " + std::to_string(weapons[player.activeWeapon].weaponState);
		DrawString(10, 70, text, olc::WHITE);
		*/




		// MIDI ==========================================================================================

		// MIDI Looping
		if (g_MidiMessage == NULL) {
			//fprintf(stderr, "End of MIDI\n");
			RestartMIDI();
		}



		//std::cout << "=== END FRAME ===" << std::endl;
		return true;
	}



};



//===============================================================================================================================
// MAIN() 
//===============================================================================================================================



int main()
{

	StartMIDI();

	ShowCursor(false);

	RaycastEngine engine;
	if (engine.Construct(320, 200, 3, 3, false, false))
		engine.Start();


	return 0;
}
