#pragma once
#pragma warning(disable : 4244)
#pragma warning(disable : 4018)



// Standart libraries
#include <regex>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <windows.h>


// FX
#include "irrKlang.h"
#pragma comment(lib, "irrKlang.lib") // link with irrKlang.dll

#include "minisdl_audio.h"
#include "tsf.h"
#include "tml.h"



// Core elements
#include "e_core.h"


// UI
#include "ui_button.h"
#include "ui_inputField.h"
#include "ui_slider.h"

// Objects
#include "c_thing.h"
#include "c_enemy.h"
#include "c_item.h"
#include "c_decoration.h"
#include "c_player.h"
#include "c_interactibleWall.h"

// Image loader
#include "t_imageLoader.h"






// Holds the global instance pointer
static tsf* g_TinySoundFont;

// Holds global MIDI playback state
static float g_Msec;               //current playback time
static tml_message* g_MidiMessage;  //next message to be played






class RaycastEngine {


private:


	RaycastEngine() 
	{
		std::cout << "RaycastEngine Constructor";
	};
    RaycastEngine(const RaycastEngine&);
    //RaycastEngine& operator=(RaycastEngine&);
	
public:
	static RaycastEngine* getInstance() {
		static RaycastEngine p_instance;
		//if (p_instance == nullptr)
		//	p_instance = new RaycastEngine();
		return &p_instance;
	}




	// Engine Core =============================================================

	RECore engineCore;


	// Core wrapper =============================================================

	// - Drawing

	// Draws a single Pixel
	bool Draw(int32_t x, int32_t y, Color c = WHITE) {
		return engineCore.Draw(x,y,c);
	}

	bool Draw(const vi2d& pos, Color c = WHITE) {
		return engineCore.Draw(pos, c);
	}

	// Draws a line from (x1,y1) to (x2,y2)
	void DrawLine(int32_t x1, int32_t y1, int32_t x2, int32_t y2, Color c = WHITE) {
		return engineCore.DrawLine(x1, y1, x2, y2, c);
	}

	void DrawLine(const vi2d& pos1, const vi2d& pos2, Color c = WHITE) {
		return engineCore.DrawLine(pos1, pos2, c);
	}

	// Draws a rectangle at (x,y) to (x+w,y+h)
	void DrawRect(int32_t x, int32_t y, int32_t w, int32_t h, Color c = WHITE) {
		return engineCore.DrawRect(x, y, w, h, c);
	}

	void DrawRect(const vi2d& pos, const vi2d& size, Color c = WHITE) {
		return engineCore.DrawRect(pos, size, c);
	}

	// Fills a rectangle at (x,y) to (x+w,y+h)
	void FillRect(int32_t x, int32_t y, int32_t w, int32_t h, Color c = WHITE) {
		return engineCore.FillRect(x, y, w, h, c);
	}

	void FillRect(const vi2d& pos, const vi2d& size, Color c = WHITE) {
		return engineCore.FillRect(pos, size, c);
	}

	// Draws a single line of text
	void DrawString(int32_t x, int32_t y, const std::string& sText, Color col = WHITE, uint32_t scale = 1) {
		return engineCore.DrawString(x, y, sText, col, scale);
	}

	void DrawString(const vi2d& pos, const std::string& sText, Color col = WHITE, uint32_t scale = 1) {
		return engineCore.DrawString(pos, sText, col, scale);
	}



	// - Controls

	HWButton GetKey(Key k) {
		return engineCore.GetKey(k);
	}

	HWButton GetMouse(uint32_t b) {
		return engineCore.GetMouse(b);
	}

	int32_t GetMouseX()
	{
		return engineCore.GetMouseX();
	}

	int32_t GetMouseY()
	{
		return engineCore.GetMouseY();
	}


	// - Window

	int32_t ScreenWidth() {
		return engineCore.ScreenWidth();
	}

	int32_t ScreenHeight() {
		return engineCore.ScreenHeight();
	}

	bool IsFocused() {
		return engineCore.IsFocused();
	}

	vi2d GetWindowMouse() {
		return engineCore.GetWindowMouse();
	}

	vi2d GetWindowSize() {
		return engineCore.GetWindowSize();
	}

	vi2d GetWindowPosition() {
		return engineCore.GetWindowPosition();
	}









	// FX =============================================================

	irrklang::ISoundEngine* soundEngine;

	void StartWAVEngine() {

		// start the sound engine with default parameters
		soundEngine = irrklang::createIrrKlangDevice();

		if (!soundEngine)
		{
			printf("SOUND : Could not startup FX sound engine\n");
		}

		soundEngine->setSoundVolume(0.2f);

	}




	// MIDI =============================================================


	void StartMIDIEngine();

	void RestartMIDI(char* fileName);

	void StopMIDI();













	// Drawing stuff

	Color ShadeColor(Color color, float amount = 0.5) {
		Color shadeColor = Color(color.r * amount, color.g * amount, color.b * amount);
		return shadeColor;
	}

	void DrawSpriteColorTransparent(int32_t x, int32_t y, Sprite* sprite, Color transparancyColor) {

		if (sprite == nullptr)
			return;

		for (int32_t i = 0; i < sprite->width; i++)
			for (int32_t j = 0; j < sprite->height; j++)
				if (sprite->GetPixel(i, j) != transparancyColor)
					Draw(x + i, y + j, sprite->GetPixel(i, j));
	}

	void DrawPartialSpriteColorTransparent(int32_t x, int32_t y, int32_t regionX, int32_t regionY, int32_t regionW, int32_t regionH, Sprite* sprite, Color transparancyColor) {

		if (sprite == nullptr)
			return;

		for (int32_t i = regionX; i < regionX + regionW; i++)
			for (int32_t j = regionY; j < regionY + regionH; j++)
				if (sprite->GetPixel(i, j) != transparancyColor)
					Draw(x + (i % regionW), y + (j % regionH), sprite->GetPixel(i, j));
	}



	// Some internal stuff ---------------------------

	enum GameState { STATE_GAMEPLAY, STATE_MENU, STATE_EDITOR, STATE_TITLESCREEN, STATE_ENDGAME, STATE_LORESCREEN };
	GameState gameState;


	// Stuff for rendering
	bool* drawnPixels;
	float* ZBuffer;


	// UI

	bool showCursor = false;
	bool isMenuOpen = false;


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

	uint32_t newId = 1;

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

		InteractibleWall endLevel = InteractibleWall(vf2d(3, 3), InteractibleWall::InteractionType::ENDLEVEL);
		interactbleWallsArray.push_back(endLevel);

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
							InteractibleWall newWall = InteractibleWall(readPosition, InteractibleWall::InteractionType::ENDLEVEL);
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
							Enemy newEnemy = Enemy(newId, readType, readPosition);
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
							Decoration newDecoration = Decoration(newId, readType, readPosition);
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
							Item newItem = Item(newId, readType, readPosition);
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
		{10.0f, 0.4f, true, 40, 400},
		// Rifle
		{5.0f, 0.1f, true, 40, 400},
		// Shotgun
		{5.0f, 1.2f, true, 50, 50},
	};






	Enemy* GetEnemyByID(uint32_t id) {
		for (int i = 0; i < enemiesArray.size(); i++) {
			if (enemiesArray[i].id == id) {
				return &enemiesArray[i];
			}
		}
		return nullptr;
	}






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

	}


	//===============================================================================================================================
	// ASSETS
	//===============================================================================================================================




	ImageLoader imageLoader;

	// Assets ---------------------------------------

	// UI
	Sprite spriteCursor;

	// Weapon in HUD sprites
	Sprite weaponPistol;
	Sprite weaponUzi;
	Sprite weaponShotgun;

	// Walls
	Sprite spriteWallAtlas;

	std::vector<Sprite*> wallSprites;



	// Decorations
	Sprite spriteBarell;
	Sprite spritePillar;
	Sprite spriteLamp;
	Sprite spriteExitSign;

	// Enemies sprites
	Sprite spriteRobot1;
	Sprite spriteRobot2;
	Sprite spriteRobot3;
	Sprite spriteBoss;

	std::vector<Sprite*> enemyIconSprites;



	// Item sprites
	Sprite spritesMedkit;
	Sprite itemAmmo9mm;
	Sprite itemAmmoShells;
	Sprite itemUzi;
	Sprite itemShotgun;

	std::vector<Sprite*> itemIconSprites;





	Sprite* SampleIconFromSprite(Sprite* sprite, int32_t x, int32_t y, int32_t w, int32_t h) {
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

	void SampleWallTexturesFromAtlas(Sprite* atlas) {

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



	void LoadSprite(Sprite* sprite, std::string file) {
		rcode loadStatus = imageLoader.LoadImageResource(sprite, file);

		if (loadStatus == FAIL) { std::cout << "Sprite load failed : " << file << std::endl; return; }
		if (loadStatus == NO_FILE) { std::cout << "Sprite not found : " << file << std::endl; return; }
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






	Sprite* GetWallTexture(int idx) {
		return wallSprites[idx];
	}

	Sprite* GetDecorationSprite(int idx) {
		switch (idx) {
		case 0: return &spriteBarell;
		case 1: return &spritePillar;
		case 2: return &spriteLamp;
		case 3: return &spriteExitSign;
		default: return &spriteBarell;
		}
	}

	Sprite* GetEnemySprite(int idx) {
		switch (idx) {
		case 0: return &spriteRobot1;
		case 1: return &spriteRobot2;
		case 2: return &spriteRobot3;
		case 3: return &spriteBoss;
		default: return &spriteRobot1;
		}
	}

	Sprite* GetItemSprite(int idx) {
		switch (idx) {
		case 0: return &spritesMedkit;
		case 1: return &itemAmmo9mm;
		case 2: return &itemAmmoShells;
		case 3: return &itemUzi;
		case 4: return &itemShotgun;
		default: return &spritesMedkit;
		}
	}

	Sprite* GetWeaponSprite(int idx) {
		switch (idx) {
		case 0: return &weaponPistol;
		case 1: return &weaponUzi;
		case 2: return &weaponShotgun;
		default: return &weaponPistol;
		}
	}



	Sprite spriteEditorPlay;
	Sprite spriteEditorSave;
	Sprite spriteEditorLoad;
	Sprite spriteEditorSettings;

	Sprite spriteEditorToolWall;
	Sprite spriteEditorToolItem;
	Sprite spriteEditorToolDecor;
	Sprite spriteEditorToolEnemy;
	Sprite spriteEditorToolEracer;
	Sprite spriteEditorToolZoom;
	Sprite spriteEditorToolGrid;
	Sprite spriteEditorToolPlayer;
	Sprite spriteEditorToolEndLevel;


	//===============================================================================================================================
	// UI
	//===============================================================================================================================



	void InfoLog(std::string text) {

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

	void DrawPlayerUI(float fElapsedTime) {

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

			//if (midiVolumeSlider.isPressed) {
			//	PlayMapMIDI();
			//}
			//tsf_set_volume(g_TinySoundFont, midiVolumeSlider.value);


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



	void DrawEndScreen() {

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

	void DrawLoreScreen() {

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

	int submenu = 0;

	std::vector<std::string> mapsToSelect;
	int mapSelectPage = 0;
	int mapPageCount = 0;


	Slider midiVolumeSlider = Slider(vi2d(150, 80), 130, 20);
	Slider fxVolumeSlider = Slider(vi2d(150, 110), 130, 20);
	Slider mouseSensSlider = Slider(vi2d(150, 140), 130, 20);


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
			//tsf_set_volume(g_TinySoundFont, midiVolumeSlider.value);


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
	// WORLD PHYSICS
	//===============================================================================================================================

	//x-coordinate in camera space,  [0 - 319] pixels -> [-1 - 1]
	float ShootRaycastRay(vf2d from, vf2d rayDir) {

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


	bool LineIntersection(vf2d ln1pnt1, vf2d ln1pnt2, vf2d ln2pnt1, vf2d ln2pnt2) {

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
	// INPUT
	//===============================================================================================================================

	void UserControls(float fElapsedTime) {

		if (!isMenuOpen && !isEditorOpened && player.isAlive) {

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

		if (IsFocused() && !isMenuOpen && !isEditorOpened) {
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
	// MAP EDITOR 
	//===============================================================================================================================

	bool isEditorOpened = false;
	bool isEditorInPlayMode = false;


	vi2d gridShift{ 0,0 };
	vi2d gridOrigin{ 0,0 };

	int editorCellSize = 20;
	bool showGrid = true;


	vf2d selectedCell{ 0,0 };


	// Controls
	bool mouseOnUI = false;
	vi2d mousePosPrev{ GetMouseX(), GetMouseY() };


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
		Decoration newDecoration = Decoration(newId, selectedToolDecor, vf2d(selectedCell.x + 0.5, selectedCell.y + 0.5)); newId++;
		decorationsArray.push_back(newDecoration);
		thingsArray.push_back(newDecoration.ToThing());
		spriteOrder = new int[thingsArray.size()];
		spriteDistance = new float[thingsArray.size()];
	}

	void Editor_PlaceItem() {
		Item newItem = Item(newId, selectedToolItem, vf2d(selectedCell.x, selectedCell.y)); newId++;
		itemsArray.push_back(newItem);
		thingsArray.push_back(newItem.ToThing());
		spriteOrder = new int[thingsArray.size()];
		spriteDistance = new float[thingsArray.size()];
	}

	void Editor_PlaceEnemy() {
		Enemy newEnemy = Enemy(newId, selectedToolEnemy, vf2d(selectedCell.x, selectedCell.y)); newId++;
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
		mousePosPrev = vi2d(GetMouseX(), GetMouseY());


		// Arrow control
		if (GetKey(Key::LEFT).bHeld || GetKey(Key::A).bHeld) {
			gridShift.x = (gridShift.x + int(10 * (fElapsedTime / 0.016))) % 20;
			gridOrigin.x = gridOrigin.x + int(10 * (fElapsedTime / 0.016));
		}
		if (GetKey(Key::RIGHT).bHeld || GetKey(Key::D).bHeld) {
			gridShift.x = (gridShift.x - int(10 * (fElapsedTime / 0.016))) % 20;
			gridOrigin.x = gridOrigin.x - int(10 * (fElapsedTime / 0.016));
		}
		if (GetKey(Key::UP).bHeld || GetKey(Key::W).bHeld) {
			gridShift.y = (gridShift.y + int(10 * (fElapsedTime / 0.016))) % 20;
			gridOrigin.y = gridOrigin.y + int(10 * (fElapsedTime / 0.016));
		}
		if (GetKey(Key::DOWN).bHeld || GetKey(Key::S).bHeld) {
			gridShift.y = (gridShift.y - int(10 * (fElapsedTime / 0.016))) % 20;
			gridOrigin.y = gridOrigin.y - int(10 * (fElapsedTime / 0.016));
		}



		if (GetKey(Key::NP_ADD).bPressed) {
			editorCellSize = 20;
		}
		if (GetKey(Key::NP_SUB).bPressed) {
			editorCellSize = 10;
		}
		if (GetKey(Key::Z).bPressed) {
			editorCellSize = editorCellSize % 20 + 10;
		}
		if (GetKey(Key::G).bPressed)
		{
			showGrid = !showGrid;
		}
		if (GetKey(Key::E).bPressed)
		{
			playerPosSelected = false;
			levelEndPosSelected = false;
			eraserSelected = !eraserSelected;
		}
		if (GetKey(Key::P).bPressed)
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
				player.position = vf2d(selectedCell.x + 0.5, selectedCell.y + 0.5);
			}
		}


		// Place End level

		if (levelEndPosSelected && !eraserSelected && !playerPosSelected && GetMouse(0).bPressed) {
			if (selectedCell.x >= 0 && selectedCell.y >= 0 && selectedCell.x < worldMapWidth && selectedCell.y < worldMapHeight) { // Check map boundaries
				for (int intWall = 0; intWall < interactbleWallsArray.size(); intWall++) {
					if (interactbleWallsArray[intWall].interactionType == InteractibleWall::InteractionType::ENDLEVEL) {
						interactbleWallsArray[intWall].wallPosition = vf2d(selectedCell.x, selectedCell.y);
					}
				}
			}
		}

	}



	void Editor_ShowToolSelection() {

		mouseOnUI = true; // To prevent placing and moving in background

		FillRect(40, 40, 273, 148, BLACK); // Background
		FillRect(195, 30, 75, 10, BLACK); // Connection
		DrawLine(195, 0, 195, 40, YELLOW);
		DrawLine(270, 0, 270, 40, YELLOW);
		DrawLine(40, 40, 195, 40, YELLOW);
		DrawLine(270, 40, 313, 40, YELLOW);


		int wallPageCount = ceil(wallSprites.size() / 21);
		int decorPageCount = ceil(3 / 21);
		int itemPageCount = ceil(itemIconSprites.size() / 21);
		int enemyPageCount = ceil(enemyIconSprites.size() / 21);


		for (int toolY = 0; toolY < 3; toolY++) {
			for (int toolX = 0; toolX < 7; toolX++) {

				if (toolSelected == 0 && (toolX + toolY * 7) > wallSprites.size() - 1) break; // Walls
				if (toolSelected == 1 && (toolX + toolY * 7) > 3) break; // Decor
				if (toolSelected == 2 && (toolX + toolY * 7) > itemIconSprites.size() - 1) break; // Item
				if (toolSelected == 3 && (toolX + toolY * 7) > enemyIconSprites.size() - 1) break; // Enemy



				Button selectToolButton = Button();
				int textureIdx = 0;
				switch (toolSelected)
				{
				case 0: // Wall
					textureIdx = ((toolX + toolY * 7) + 21 * toolSelectionPage);
					selectToolButton = Button(vi2d(45 + (toolX * (34 + 4)), 45 + (toolY * (34 + 4))), 34, 34, GetWallTexture(textureIdx));
					break;
				case 1: // Decorations
					textureIdx = (toolX + toolY * 7) + 21 * toolSelectionPage;
					selectToolButton = Button(vi2d(45 + (toolX * (34 + 4)), 45 + (toolY * (34 + 4))), 34, 34, GetDecorationSprite(textureIdx));
					break;
				case 2: // Items
					textureIdx = (toolX + toolY * 7) + 21 * toolSelectionPage;
					selectToolButton = Button(vi2d(45 + (toolX * (34 + 4)), 45 + (toolY * (34 + 4))), 34, 34, itemIconSprites[textureIdx]);
					break;
				case 3: // Enemies
					textureIdx = (toolX + toolY * 7) + 21 * toolSelectionPage;
					selectToolButton = Button(vi2d(45 + (toolX * (34 + 4)), 45 + (toolY * (34 + 4))), 34, 34, enemyIconSprites[textureIdx]);
					break;
				default:
					break;
				}
				selectToolButton.Update();
				if (selectToolButton.isHovered) { mouseOnUI = true; }
				if (selectToolButton.isPressed) { Editor_SelectTool(textureIdx); showToolSelection = false; }
			}
		}


		Button selectToolNextPageButton = Button(vi2d(267, 160), 40, 20, "NEXT");
		selectToolNextPageButton.Update();
		if (selectToolNextPageButton.isHovered) { mouseOnUI = true; }
		if (selectToolNextPageButton.isPressed) {


			if (toolSelected == 0 && toolSelectionPage < wallPageCount - 1) toolSelectionPage++;  // Walls
			if (toolSelected == 1 && toolSelectionPage < decorPageCount - 1) toolSelectionPage++; // Decor
			if (toolSelected == 2 && toolSelectionPage < itemPageCount - 1) toolSelectionPage++; // Item
			if (toolSelected == 3 && toolSelectionPage < enemyPageCount - 1) toolSelectionPage++; // Enemy

		}

		Button selectToolPrevPageButton = Button(vi2d(45, 160), 40, 20, "PREV");
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



	InputField mapTitleInputField = InputField(vi2d(35, 60), 170, 15, "DEFAULT");
	InputField mapSaveFileNameInputField = InputField(vi2d(35, 95), 170, 15, "DEFAULT");
	InputField mapSizeXInputField = InputField(vi2d(225, 60), 50, 15, "20");
	InputField mapSizeYInputField = InputField(vi2d(225, 95), 50, 15, "20");
	Button mapApplySizeButton = Button(vi2d(220, 150), 60, 20, "APPLY");
	InputField nextMapFileInputField = InputField(vi2d(35, 130), 170, 15, "");

	void Editor_ShowSettings() {

		mouseOnUI = true; // To prevent placing and moving in background

		FillRect(20, 40, 273, 142, BLACK); // Background
		FillRect(48, 30, 30, 10, BLACK); // Connection
		DrawLine(48, 0, 48, 40, YELLOW);
		DrawLine(78, 0, 78, 40, YELLOW);
		DrawLine(20, 40, 48, 40, YELLOW);
		DrawLine(78, 40, 292, 40, YELLOW);



		DrawString(vi2d(35, 50), "MAP TITLE");
		mapTitleInputField.Update();
		if (mapTitleInputField.isHovered) { mouseOnUI = true; }
		if (mapTitleInputField.isPressed) { mapTitleInputField.isFocused = true; }

		DrawString(vi2d(35, 85), "SAVE FILE NAME");
		mapSaveFileNameInputField.Update();
		if (mapSaveFileNameInputField.isHovered) { mouseOnUI = true; }
		if (mapSaveFileNameInputField.isPressed) { mapSaveFileNameInputField.isFocused = true; }

		DrawString(vi2d(225, 50), "SIZE X");
		mapSizeXInputField.allowOnlyNumbers = true;
		mapSizeXInputField.charMaximum = 4;
		mapSizeXInputField.Update();
		if (mapSizeXInputField.isHovered) { mouseOnUI = true; }
		if (mapSizeXInputField.isPressed) { mapSizeXInputField.isFocused = true; }

		DrawString(vi2d(225, 85), "SIZE Y");
		mapSizeYInputField.allowOnlyNumbers = true;
		mapSizeYInputField.charMaximum = 4;
		mapSizeYInputField.Update();
		if (mapSizeYInputField.isHovered) { mouseOnUI = true; }
		if (mapSizeYInputField.isPressed) { mapSizeYInputField.isFocused = true; }


		DrawString(vi2d(35, 120), "NEXT MAP FILE NAME");
		nextMapFileInputField.Update();
		if (nextMapFileInputField.isHovered) { mouseOnUI = true; }
		if (nextMapFileInputField.isPressed) { nextMapFileInputField.isFocused = true; }


		mapApplySizeButton.Update();
		mapApplySizeButton.colorBackground = DARK_GREEN;
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
				player.position = vf2d(1.5, 1.5);
				editor_infoLog = "Player moved to (1, 1)";
				editor_infoLogTimer = 2;
			}
		}



		// Ooooh, this is bad...... But i dont have other solution at this moment :(
		if (!(GetMouseX() > 20 && GetMouseX() < 292 && GetMouseY() > 40 && GetMouseY() < 182) && !(GetMouseX() > 48 && GetMouseX() < 78 && GetMouseY() > 0 && GetMouseY() < 40) && GetMouse(0).bPressed) {
			showSettings = false;
		}
	}




	InputField mapOpenFileInputField = InputField(vi2d(35, 60), 170, 15, "");
	Button mapLoadMapButton = Button(vi2d(145, 88), 60, 20, "LOAD");
	Button mapCloseOpenFileButton = Button(vi2d(34, 88), 60, 20, "CLOSE");

	void Editor_ShowOpenFile() {

		mouseOnUI = true; // To prevent placing and moving in background

		FillRect(20, 40, 200, 80, BLACK); // Background
		DrawLine(20, 40, 219, 40, YELLOW);

		DrawString(vi2d(35, 50), "FILE NAME TO OPEN");
		mapOpenFileInputField.Update();
		if (mapOpenFileInputField.isHovered) { mouseOnUI = true; }
		if (mapOpenFileInputField.isPressed) { mapOpenFileInputField.isFocused = true; }


		mapLoadMapButton.Update();
		mapLoadMapButton.colorBackground = DARK_GREEN;
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
		mapCloseOpenFileButton.colorBackground = DARK_RED;
		if (mapCloseOpenFileButton.isHovered) { mouseOnUI = true; }
		if (mapCloseOpenFileButton.isPressed)
		{
			showOpenFile = false;
		}

	}


	Button mapExitEditorButton = Button(vi2d(145, 88), 60, 20, "YES");
	Button mapCloseExitDialogButton = Button(vi2d(34, 88), 60, 20, "NO");

	void Editor_ShowExitDialog() {

		mouseOnUI = true; // To prevent placing and moving in background

		FillRect(20, 40, 200, 80, BLACK); // Background
		DrawLine(20, 40, 219, 40, YELLOW);

		DrawString(vi2d(35, 50), "EXIT EDITOR ?");

		mapExitEditorButton.Update();
		mapExitEditorButton.colorBackground = DARK_GREEN;
		if (mapExitEditorButton.isHovered) { mouseOnUI = true; }
		if (mapExitEditorButton.isPressed)
		{
			showExitDialog = false;
			gameState = GameState::STATE_TITLESCREEN;
		}

		mapCloseExitDialogButton.Update();
		mapCloseExitDialogButton.colorBackground = DARK_RED;
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

		FillRect(0, 0, ScreenWidth(), ScreenHeight(), VERY_DARK_GREY);


		// Draw map -----------------------

		int sampleSize = editorCellSize - 1 + !showGrid; // Little optimisation

		for (int i = 0; i < worldMapWidth; i++) {
			for (int j = 0; j < worldMapHeight; j++) {

				if (worldMap[j * worldMapWidth + i] != 0) {
					for (int smplX = 0; smplX < sampleSize; smplX++) {
						for (int smplY = 0; smplY < sampleSize; smplY++) {
							Color color = GetWallTexture(worldMap[j * worldMapWidth + i] - 1)->GetPixel(int(64.0 / editorCellSize * smplX), int(64.0 / editorCellSize * smplY));
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
				DrawLine(i * editorCellSize + gridShift.x, 0, i * editorCellSize + gridShift.x, ScreenHeight(), GREY);
			}
			// Horisontal lines
			for (int j = 0; j < int(ScreenHeight() / editorCellSize) + 2; j++) {
				DrawLine(0, j * editorCellSize + gridShift.y, ScreenWidth(), j * editorCellSize + gridShift.y, GREY);
			}


			// Draw grid origin
			DrawLine(gridOrigin.x, 0, gridOrigin.x, ScreenHeight(), YELLOW);
			DrawLine(0, gridOrigin.y, ScreenWidth(), gridOrigin.y, YELLOW);

			// Draw end of map
			DrawLine(gridOrigin.x + worldMapWidth * editorCellSize, 0, gridOrigin.x + worldMapWidth * editorCellSize, ScreenHeight(), RED);
			DrawLine(0, gridOrigin.y + worldMapHeight * editorCellSize, ScreenWidth(), gridOrigin.y + worldMapHeight * editorCellSize, RED);
		}



		// Draw objects -----------------------


		//
		// Note : decorationsArray[d].position.x - 0.5 in fact tells that i need to shift sprite half size
		//

		// Draw Decorations

		for (int d = 0; d < decorationsArray.size(); d++) {

			for (int smplX = 0; smplX < editorCellSize - 1; smplX++) {
				for (int smplY = 0; smplY < editorCellSize - 1; smplY++) {
					Color color = GetDecorationSprite(decorationsArray[d].texture)->GetPixel(int(64.0 / editorCellSize * smplX), int(64.0 / editorCellSize * smplY));
					if (color != CYAN) {
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
					Color color = itemIconSprites[itemsArray[i].texture]->GetPixel(floor((float)itemIconSprites[itemsArray[i].texture]->width / editorCellSize * smplX), floor((float)itemIconSprites[itemsArray[i].texture]->height / editorCellSize * smplY));
					if (color != CYAN) {
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
					Color color = enemyIconSprites[enemiesArray[d].texture]->GetPixel(int((float)enemyIconSprites[enemiesArray[d].texture]->width / editorCellSize * smplX), int((float)enemyIconSprites[enemiesArray[d].texture]->height / editorCellSize * smplY));
					if (color != CYAN) {
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
				Color color = spriteEditorToolPlayer.GetPixel(int((float)spriteEditorToolPlayer.width / editorCellSize * smplX), int((float)spriteEditorToolPlayer.height / editorCellSize * smplY));
				if (color != CYAN) {
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
						Color color = spriteEditorToolEndLevel.GetPixel(int((float)spriteEditorToolEndLevel.width / editorCellSize * smplX), int((float)spriteEditorToolEndLevel.height / editorCellSize * smplY));
						if (color != CYAN) {
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
			DrawRect(gridOrigin.x + selectedCell.x * editorCellSize, gridOrigin.y + selectedCell.y * editorCellSize, editorCellSize, editorCellSize, RED);

			std::string cellXText = std::to_string(selectedCell.x);
			std::string cellYText = std::to_string(selectedCell.y);
			std::string cellText = cellXText.substr(0, cellXText.length() - 5) + " " + cellYText.substr(0, cellYText.length() - 5);
			DrawString(5, ScreenHeight() - 10, cellText);
		}
		else if (toolSelected == 2 || toolSelected == 3) { // Items or Enemy

			selectedCell.x = (floor(selectedCell.x * 2)) / 2; // Round to the nearest 0.5 
			selectedCell.y = (floor(selectedCell.y * 2)) / 2;

			// Dot
			FillRect(gridOrigin.x + selectedCell.x * editorCellSize - 3, gridOrigin.y + selectedCell.y * editorCellSize - 3, 7, 7, RED);

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
		FillRect(0, 0, 320, 30, BLACK);





		// Save button
		Button saveButton = Button(vi2d(3, 3), 22, 22, &spriteEditorSave);
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
		Button loadButton = Button(vi2d(28, 3), 22, 22, &spriteEditorLoad);
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
		Button settingsButton = Button(vi2d(53, 3), 22, 22, &spriteEditorSettings);
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
		Button playButton = Button(vi2d(279, 0), 30, 30, &spriteEditorPlay);
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
		Button toolWallButton = Button(vi2d(90, 5), 20, 20, &spriteEditorToolWall);
		toolWallButton.showBorder = true;
		toolWallButton.hoverText = "WALL";
		toolWallButton.Update();
		if (toolWallButton.isHovered) { mouseOnUI = true; hoverText = toolWallButton.hoverText; }
		if (toolWallButton.isPressed) { toolSelected = 0; toolSelectionPage = 0; }

		// Tool Decorations button
		Button toolDecorButton = Button(vi2d(115, 5), 20, 20, &spriteEditorToolDecor);
		toolDecorButton.showBorder = true;
		toolDecorButton.hoverText = "DECOR";
		toolDecorButton.Update();
		if (toolDecorButton.isHovered) { mouseOnUI = true; hoverText = toolDecorButton.hoverText; }
		if (toolDecorButton.isPressed) { toolSelected = 1; toolSelectionPage = 0; }

		// Tool Items button
		Button toolItemButton = Button(vi2d(140, 5), 20, 20, &spriteEditorToolItem);
		toolItemButton.showBorder = true;
		toolItemButton.hoverText = "ITEM";
		toolItemButton.Update();
		if (toolItemButton.isHovered) { mouseOnUI = true; hoverText = toolItemButton.hoverText; }
		if (toolItemButton.isPressed) { toolSelected = 2; toolSelectionPage = 0; }

		// Tool Enemies button
		Button toolEnemyButton = Button(vi2d(165, 5), 20, 20, &spriteEditorToolEnemy);
		toolEnemyButton.showBorder = true;
		toolEnemyButton.hoverText = "ENEMY";
		toolEnemyButton.Update();
		if (toolEnemyButton.isHovered) { mouseOnUI = true; hoverText = toolEnemyButton.hoverText; }
		if (toolEnemyButton.isPressed) { toolSelected = 3; toolSelectionPage = 0; }




		// Tool Eraser button
		Button toolEraserButton = Button(vi2d(3, 35), 20, 20, &spriteEditorToolEracer);
		toolEraserButton.showBorder = true;
		toolEraserButton.hoverText = "ERASER";
		toolEraserButton.Update();
		if (toolEraserButton.isHovered) { mouseOnUI = true; hoverText = toolEraserButton.hoverText; }
		if (toolEraserButton.isPressed) { playerPosSelected = false; levelEndPosSelected = false;  eraserSelected = !eraserSelected; }

		// Tool Zoom button
		Button toolZoomButton = Button(vi2d(3, 60), 20, 20, &spriteEditorToolZoom);
		toolZoomButton.showBorder = true;
		toolZoomButton.hoverText = "ZOOM";
		toolZoomButton.Update();
		if (toolZoomButton.isHovered) { mouseOnUI = true; hoverText = toolZoomButton.hoverText; }
		if (toolZoomButton.isPressed) editorCellSize = editorCellSize % 20 + 10;

		// Tool Grid button
		Button toolGridButton = Button(vi2d(3, 85), 20, 20, &spriteEditorToolGrid);
		toolGridButton.showBorder = true;
		toolGridButton.hoverText = "GRID";
		toolGridButton.Update();
		if (toolGridButton.isHovered) { mouseOnUI = true; hoverText = toolGridButton.hoverText; }
		if (toolGridButton.isPressed) showGrid = !showGrid;

		// Tool Player button
		Button toolPlayerButton = Button(vi2d(3, 125), 20, 20, &spriteEditorToolPlayer);
		toolPlayerButton.showBorder = true;
		toolPlayerButton.hoverText = "START POSITION";
		toolPlayerButton.Update();
		if (toolPlayerButton.isHovered) { mouseOnUI = true; hoverText = toolPlayerButton.hoverText; }
		if (toolPlayerButton.isPressed) { eraserSelected = false; levelEndPosSelected = false; playerPosSelected = !playerPosSelected; }

		// Tool EndLevel button
		Button toolEndLevelButton = Button(vi2d(3, 150), 20, 20, &spriteEditorToolEndLevel);
		toolEndLevelButton.showBorder = true;
		toolEndLevelButton.hoverText = "END LEVEL";
		toolEndLevelButton.Update();
		if (toolEndLevelButton.isHovered) { mouseOnUI = true; hoverText = toolEndLevelButton.hoverText; }
		if (toolEndLevelButton.isPressed) { eraserSelected = false; playerPosSelected = false; levelEndPosSelected = !levelEndPosSelected; }


		// Tool label
		DrawString(200, 5, "TOOL:", WHITE);

		// Tool button
		Button textureButton = Button();
		switch (toolSelected)
		{
		case 0: // Wall
			textureButton = Button(vi2d(239, 2), 25, 25, GetWallTexture(selectedToolWall));
			break;
		case 1: // Decorations
			textureButton = Button(vi2d(239, 2), 25, 25, GetDecorationSprite(selectedToolDecor));
			break;
		case 2: // Items
			textureButton = Button(vi2d(239, 2), 25, 25, itemIconSprites[selectedToolItem]);
			break;
		case 3: // Enemies
			textureButton = Button(vi2d(239, 2), 25, 25, enemyIconSprites[selectedToolEnemy]);
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

		DrawRect(88 + toolSelected * 25, 3, 24, 24, YELLOW);
		DrawRect(89 + toolSelected * 25, 4, 22, 22, YELLOW);


		if (eraserSelected) {
			DrawRect(2, 34, 22, 22, YELLOW);
			DrawRect(1, 33, 24, 24, YELLOW);
			// Cursor
			DrawSpriteColorTransparent(GetMouseX() + 3, GetMouseY() + 5, &spriteEditorToolEracer, CYAN);
		}

		if (playerPosSelected) {
			DrawRect(2, 124, 22, 22, YELLOW);
			DrawRect(1, 123, 24, 24, YELLOW);
			// Cursor
			DrawSpriteColorTransparent(GetMouseX() + 3, GetMouseY() + 5, &spriteEditorToolPlayer, CYAN);
		}

		if (levelEndPosSelected) {
			DrawRect(2, 149, 22, 22, YELLOW);
			DrawRect(1, 148, 24, 24, YELLOW);
			// Cursor
			DrawSpriteColorTransparent(GetMouseX() + 3, GetMouseY() + 5, &spriteEditorToolEndLevel, CYAN);
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
			DrawString(GetMouseX() - 1, GetMouseY() + 18, hoverText, BLACK);
			DrawString(GetMouseX() - 2, GetMouseY() + 17, hoverText, WHITE);
		}





		// Info log -----------------------

		if (editor_infoLogTimer > 0) {
			editor_infoLogTimer -= fElapsedTime;
			DrawString(31, 38, editor_infoLog, BLACK);
			DrawString(30, 37, editor_infoLog);
		}



		// Editor controls -----------------------

		if (!showSettings && !showToolSelection && !showOpenFile && !showExitDialog && !mouseOnUI) {
			Editor_Controls(fElapsedTime);
		}



		// Exit editor -----------------------

		if (GetKey(Key::ESCAPE).bPressed) {
			showExitDialog = true;
		}


	}





	void RestartLevel() {
		std::string loadFile = "maps/" + worldMapFile + ".map";
		LoadMap(loadFile);
		player.Ressurect();
	}




	void PlayMapMIDI() {
		std::string fullMidiFileName = "sounds/" + midiFileName + ".mid";
		char* midiCSTR = new char[fullMidiFileName.length() + 1];
		strcpy_s(midiCSTR, fullMidiFileName.length(), fullMidiFileName.c_str());

		//std::cout << midiCSTR << std::endl;

		RestartMIDI(midiCSTR);
	}









	bool GameStart() {

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

	bool GameUpdate(float fElapsedTime) {

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
				DrawString(80, 180, "EDITOR IN PLAY MODE", WHITE);
				DrawString(80, 190, "PRESS M TO RETURN", WHITE);

				if (GetKey(Key::M).bPressed) {
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


			if (GetKey(Key::ESCAPE).bPressed)
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


			if (GetKey(Key::ESCAPE).bPressed)
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




};