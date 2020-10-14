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


// Editor
#include "t_editor.h"






struct Weapon {
	float damage;
	float fireRate;

	bool onPlayer;

	int currentAmmo;
	int maxAmmo;
};




class RaycastEngine {


public:

	//static RaycastEngine* p_instance;

	RaycastEngine(RECore* core) 
	{
		std::cout << "RaycastEngine Constructor" << std::endl;
		engineCore = core;
	};
    RaycastEngine(const RaycastEngine&);
    //RaycastEngine& operator=(RaycastEngine&);
	
	static RaycastEngine* getInstance() {
	//	//static RaycastEngine p_instance;
	//	//if (p_instance == nullptr)
	//	//	p_instance = new RaycastEngine();
		return nullptr;
	}



	//===============================================================================================================================
	// CORE
	//===============================================================================================================================

	RECore* engineCore;


	// - Drawing

	// Draws a single Pixel
	bool Draw(int32_t x, int32_t y, Color c = WHITE) {
		return engineCore->Draw(x,y,c);
	}

	bool Draw(const vi2d& pos, Color c = WHITE) {
		return engineCore->Draw(pos, c);
	}

	// Draws a line from (x1,y1) to (x2,y2)
	void DrawLine(int32_t x1, int32_t y1, int32_t x2, int32_t y2, Color c = WHITE) {
		return engineCore->DrawLine(x1, y1, x2, y2, c);
	}

	void DrawLine(const vi2d& pos1, const vi2d& pos2, Color c = WHITE) {
		return engineCore->DrawLine(pos1, pos2, c);
	}

	// Draws a rectangle at (x,y) to (x+w,y+h)
	void DrawRect(int32_t x, int32_t y, int32_t w, int32_t h, Color c = WHITE) {
		return engineCore->DrawRect(x, y, w, h, c);
	}

	void DrawRect(const vi2d& pos, const vi2d& size, Color c = WHITE) {
		return engineCore->DrawRect(pos, size, c);
	}

	// Fills a rectangle at (x,y) to (x+w,y+h)
	void FillRect(int32_t x, int32_t y, int32_t w, int32_t h, Color c = WHITE) {
		return engineCore->FillRect(x, y, w, h, c);
	}

	void FillRect(const vi2d& pos, const vi2d& size, Color c = WHITE) {
		return engineCore->FillRect(pos, size, c);
	}

	// Draws a single line of text
	void DrawString(int32_t x, int32_t y, const std::string& sText, Color col = WHITE, uint32_t scale = 1) {
		return engineCore->DrawString(x, y, sText, col, scale);
	}

	void DrawString(const vi2d& pos, const std::string& sText, Color col = WHITE, uint32_t scale = 1) {
		return engineCore->DrawString(pos, sText, col, scale);
	}



	// - Controls

	HWButton GetKey(Key k) {
		return engineCore->GetKey(k);
	}

	HWButton GetMouse(uint32_t b) {
		return engineCore->GetMouse(b);
	}

	int32_t GetMouseX()
	{
		return engineCore->GetMouseX();
	}

	int32_t GetMouseY()
	{
		return engineCore->GetMouseY();
	}


	// - Window

	int32_t ScreenWidth() {
		return engineCore->ScreenWidth();
	}

	int32_t ScreenHeight() {
		return engineCore->ScreenHeight();
	}

	bool IsFocused() {
		return engineCore->IsFocused();
	}

	vi2d GetWindowMouse() {
		return engineCore->GetWindowMouse();
	}

	vi2d GetWindowSize() {
		return engineCore->GetWindowSize();
	}

	vi2d GetWindowPosition() {
		return engineCore->GetWindowPosition();
	}


	//===============================================================================================================================
	// AUDIO
	//===============================================================================================================================



	// FX =============================================================

	irrklang::ISoundEngine* soundEngine;

	void StartWAVEngine();




	// MIDI =============================================================


	void StartMIDIEngine();

	void RestartMIDI(char* fileName);

	void StopMIDI();




	//===============================================================================================================================
	// DRAWING
	//===============================================================================================================================


	Color ShadeColor(Color color, float amount = 0.5);

	void DrawSpriteColorTransparent(int32_t x, int32_t y, Sprite* sprite, Color transparancyColor);

	void DrawPartialSpriteColorTransparent(int32_t x, int32_t y, int32_t regionX, int32_t regionY, int32_t regionW, int32_t regionH, Sprite* sprite, Color transparancyColor);


	//===============================================================================================================================
	// Some internal stuff 
	//===============================================================================================================================

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


	void LoadDefaultMap();

	void LoadMap(std::string mapName);

	void SaveMap(std::string mapName);

	void ChangeMapSize(int w, int h);




	//===============================================================================================================================
	// GAME OBJECTS
	//===============================================================================================================================

	Weapon weapons[3] = {
		// Pistol
		{10.0f, 0.4f, true, 40, 400},
		// Rifle
		{5.0f, 0.1f, true, 40, 400},
		// Shotgun
		{5.0f, 1.2f, true, 50, 50},
	};


    // World objects ------------------------

    Player player = Player(this);

    std::vector<Thing> thingsArray;

    std::vector<Enemy> enemiesArray;
	Enemy* GetEnemyByID(uint32_t id);

    std::vector<Decoration> decorationsArray;

    std::vector<Item> itemsArray;

    std::vector<InteractibleWall> interactbleWallsArray;


	void UpdateThings();


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



	Sprite* SampleIconFromSprite(Sprite* sprite, int32_t x, int32_t y, int32_t w, int32_t h);

	void SampleWallTexturesFromAtlas(Sprite* atlas);


	void LoadSprite(Sprite* sprite, std::string file);

	void LoadAssets();


	Sprite* GetWallTexture(int idx);

	Sprite* GetDecorationSprite(int idx);

	Sprite* GetEnemySprite(int idx);

	Sprite* GetItemSprite(int idx);

	Sprite* GetWeaponSprite(int idx);



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

	void InfoLog(std::string text);

	void DrawPlayerUI(float fElapsedTime);


	void DrawMenu();



	void DrawEndScreen();

	void DrawLoreScreen();


	//===============================================================================================================================
	// TITLE SCREEN
	//===============================================================================================================================

	int submenu = 0;

	std::vector<std::string> mapsToSelect;
	int mapSelectPage = 0;
	int mapPageCount = 0;


	Slider midiVolumeSlider = Slider(this, vi2d(150, 80), 130, 20);
	Slider fxVolumeSlider = Slider(this, vi2d(150, 110), 130, 20);
	Slider mouseSensSlider = Slider(this, vi2d(150, 140), 130, 20);


	void DrawTitleScreen(float fElapsedTime);




	//===============================================================================================================================
	// WORLD PHYSICS
	//===============================================================================================================================

	//x-coordinate in camera space,  [0 - 319] pixels -> [-1 - 1]
	float ShootRaycastRay(vf2d from, vf2d rayDir);

	bool LineIntersection(vf2d ln1pnt1, vf2d ln1pnt2, vf2d ln2pnt1, vf2d ln2pnt2);



	//===============================================================================================================================
	// RENDERING
	//===============================================================================================================================

	//arrays used to sort the sprites
	int* spriteOrder;
	float* spriteDistance;

	//sort algorithm
	//sort the sprites based on distance
	void sortSprites(int* order, float* dist, int amount);

	void RaycastRender();




	//===============================================================================================================================
	// INPUT
	//===============================================================================================================================

	void UserControls(float fElapsedTime);


	//===============================================================================================================================
	// MAP EDITOR 
	//===============================================================================================================================
	
	Editor mapEditor = Editor(this);



	//===============================================================================================================================
	// LEVELS
	//===============================================================================================================================

	void RestartLevel();

	void PlayMapMIDI();



	//===============================================================================================================================
	// MAIN FUNCTIONS 
	//===============================================================================================================================

	bool GameStart();

	bool GameUpdate(float fElapsedTime);

};