#pragma once
#include "e_core.h"
#include "ui_button.h"
#include "ui_inputField.h"
#include "ui_slider.h"

class RaycastEngine;


class Editor
{

public:

	RaycastEngine* engine;

	bool isEditorOpened = false;
	bool isEditorInPlayMode = false;


	vi2d gridShift{ 0,0 };
	vi2d gridOrigin{ 0,0 };

	int editorCellSize = 20;
	bool showGrid = true;


	vf2d selectedCell{ 0,0 };


	// Controls
	bool mouseOnUI = false;
	vi2d mousePosPrev{ 0, 0 };


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



	Editor(RaycastEngine* eng);

	void CreateUIElements();



	void PlaceDecoration();
	void PlaceItem();
	void PlaceEnemy();

	void EraceDecorations();
	void EraceItems();
	void EraceEnemies();



	void SelectTool(int idx);

	void Controls(float fElapsedTime);

	void ShowToolSelection();



	InputField mapTitleInputField = InputField(engine, vi2d(35, 60), 170, 15, "DEFAULT");
	InputField mapSaveFileNameInputField = InputField(engine, vi2d(35, 95), 170, 15, "DEFAULT");
	InputField mapSizeXInputField = InputField(engine, vi2d(225, 60), 50, 15, "20");
	InputField mapSizeYInputField = InputField(engine, vi2d(225, 95), 50, 15, "20");
	Button mapApplySizeButton = Button(engine, vi2d(220, 150), 60, 20, "APPLY");
	InputField nextMapFileInputField = InputField(engine, vi2d(35, 130), 170, 15, "");

	void ShowSettings();




	InputField mapOpenFileInputField = InputField(engine, vi2d(35, 60), 170, 15, "");
	Button mapLoadMapButton = Button(engine, vi2d(145, 88), 60, 20, "LOAD");
	Button mapCloseOpenFileButton = Button(engine, vi2d(34, 88), 60, 20, "CLOSE");

	void ShowOpenFile();


	Button mapExitEditorButton = Button(engine);
	Button mapCloseExitDialogButton = Button(engine);

	void ShowExitDialog();



	void DrawEditor(float fElapsedTime);

};

