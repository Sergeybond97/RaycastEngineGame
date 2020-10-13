#include "t_editor.h"
#include "c_engine.h"



Editor::Editor(RaycastEngine* eng) {
	engine = eng;

	CreateUIElements();
}


void Editor::CreateUIElements() {

	mapTitleInputField = InputField(engine, vi2d(35, 60), 170, 15, "DEFAULT");
	mapSaveFileNameInputField = InputField(engine, vi2d(35, 95), 170, 15, "DEFAULT");
	mapSizeXInputField = InputField(engine, vi2d(225, 60), 50, 15, "20");
	mapSizeYInputField = InputField(engine, vi2d(225, 95), 50, 15, "20");
	mapApplySizeButton = Button(engine, vi2d(220, 150), 60, 20, "APPLY");
	nextMapFileInputField = InputField(engine, vi2d(35, 130), 170, 15, "");

	mapOpenFileInputField = InputField(engine, vi2d(35, 60), 170, 15, "");
	mapLoadMapButton = Button(engine, vi2d(145, 88), 60, 20, "LOAD");
	mapCloseOpenFileButton = Button(engine, vi2d(34, 88), 60, 20, "CLOSE");


	mapExitEditorButton = Button(engine, vi2d(145, 88), 60, 20, "YES");
	mapCloseExitDialogButton = Button(engine, vi2d(34, 88), 60, 20, "NO");

}


void Editor::PlaceDecoration() {
	Decoration newDecoration = Decoration(engine, engine->newId, selectedToolDecor, vf2d(selectedCell.x + 0.5, selectedCell.y + 0.5)); engine->newId++;
	engine->decorationsArray.push_back(newDecoration);
	engine->thingsArray.push_back(newDecoration.ToThing());
	engine->spriteOrder = new int[engine->thingsArray.size()];
	engine->spriteDistance = new float[engine->thingsArray.size()];
}
	 
void Editor::PlaceItem() {
	Item newItem = Item(engine, engine->newId, selectedToolItem, vf2d(selectedCell.x, selectedCell.y)); engine->newId++;
	engine->itemsArray.push_back(newItem);
	engine->thingsArray.push_back(newItem.ToThing());
	engine->spriteOrder = new int[engine->thingsArray.size()];
	engine->spriteDistance = new float[engine->thingsArray.size()];
}
	 
void Editor::PlaceEnemy() {
	Enemy newEnemy = Enemy(engine, engine->newId, selectedToolEnemy, vf2d(selectedCell.x, selectedCell.y)); engine->newId++;
	engine->enemiesArray.push_back(newEnemy);
	engine->thingsArray.push_back(newEnemy.ToThing());
	engine->spriteOrder = new int[engine->thingsArray.size()];
	engine->spriteDistance = new float[engine->thingsArray.size()];
}
	 
	 
	 
void Editor::EraceDecorations() {

	for (int i = 0; i < engine->decorationsArray.size(); i++) {
		if
			(
				engine->decorationsArray[i].position.x >= selectedCell.x &&
				engine->decorationsArray[i].position.x <= selectedCell.x + 1 &&
				engine->decorationsArray[i].position.y >= selectedCell.y &&
				engine->decorationsArray[i].position.y <= selectedCell.y + 1
			)
		{
			int decorId = engine->decorationsArray[i].id;
			std::cout << "Decor to erase : ID : " << decorId << std::endl;

			for (int t = 0; t < engine->thingsArray.size(); t++) {
				if (engine->thingsArray[t].id == decorId) {
					std::swap(engine->thingsArray[t], engine->thingsArray[engine->thingsArray.size() - 1]);
					engine->thingsArray.pop_back();
				}
			}

			std::swap(engine->decorationsArray[i], engine->decorationsArray[engine->decorationsArray.size() - 1]);
			engine->decorationsArray.pop_back();


			editor_infoLog = "Eraced Decor ID : " + std::to_string(decorId);
			editor_infoLogTimer = 1;
			return;
		}
	}
}
	 
void Editor::EraceItems() {

	for (int i = 0; i < engine->itemsArray.size(); i++) {
		if
			(
				engine->itemsArray[i].position.x >= selectedCell.x &&
				engine->itemsArray[i].position.x <= selectedCell.x + 1 &&
				engine->itemsArray[i].position.y >= selectedCell.y &&
				engine->itemsArray[i].position.y <= selectedCell.y + 1
			)
		{
			int itemId = engine->itemsArray[i].id;
			std::cout << "Item to erase : ID : " << itemId << std::endl;

			for (int t = 0; t < engine->thingsArray.size(); t++) {
				if (engine->thingsArray[t].id == itemId) {
					std::swap(engine->thingsArray[t], engine->thingsArray[engine->thingsArray.size() - 1]);
					engine->thingsArray.pop_back();
				}
			}

			std::swap(engine->itemsArray[i], engine->itemsArray[engine->itemsArray.size() - 1]);
			engine->itemsArray.pop_back();

			editor_infoLog = "Eraced Item ID : " + std::to_string(itemId);
			editor_infoLogTimer = 1;
			return;
		}
	}
}
	 
void Editor::EraceEnemies() {

	for (int i = 0; i < engine->enemiesArray.size(); i++) {
		if
			(
				engine->enemiesArray[i].position.x >= selectedCell.x &&
				engine->enemiesArray[i].position.x <= selectedCell.x + 1 &&
				engine->enemiesArray[i].position.y >= selectedCell.y &&
				engine->enemiesArray[i].position.y <= selectedCell.y + 1
			)
		{
			int enemyId = engine->enemiesArray[i].id;
			std::cout << "Enemy to erase : ID : " << enemyId << std::endl;

			for (int t = 0; t < engine->thingsArray.size(); t++) {
				if (engine->thingsArray[t].id == enemyId) {
					std::swap(engine->thingsArray[t], engine->thingsArray[engine->thingsArray.size() - 1]);
					engine->thingsArray.pop_back();
				}
			}

			std::swap(engine->enemiesArray[i], engine->enemiesArray[engine->enemiesArray.size() - 1]);
			engine->enemiesArray.pop_back();

			editor_infoLog = "Eraced Enemy ID : " + std::to_string(enemyId);
			editor_infoLogTimer = 1;
			return;
		}
	}
}
	 
	 
	 
void Editor::SelectTool(int idx) {
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
	 
	 
	 
void Editor::Controls(float fElapsedTime) {

	// Mouse movement
	if (engine->GetMouse(1).bHeld) {
		gridShift.x = (gridShift.x + (engine->GetMouseX() - mousePosPrev.x)) % 20;
		gridOrigin.x = gridOrigin.x + (engine->GetMouseX() - mousePosPrev.x);

		gridShift.y = (gridShift.y + (engine->GetMouseY() - mousePosPrev.y)) % 20;
		gridOrigin.y = gridOrigin.y + (engine->GetMouseY() - mousePosPrev.y);
	}
	mousePosPrev = vi2d(engine->GetMouseX(), engine->GetMouseY());


	// Arrow control
	if (engine->GetKey(Key::LEFT).bHeld || engine->GetKey(Key::A).bHeld) {
		gridShift.x = (gridShift.x + int(10 * (fElapsedTime / 0.016))) % 20;
		gridOrigin.x = gridOrigin.x + int(10 * (fElapsedTime / 0.016));
	}
	if (engine->GetKey(Key::RIGHT).bHeld || engine->GetKey(Key::D).bHeld) {
		gridShift.x = (gridShift.x - int(10 * (fElapsedTime / 0.016))) % 20;
		gridOrigin.x = gridOrigin.x - int(10 * (fElapsedTime / 0.016));
	}
	if (engine->GetKey(Key::UP).bHeld || engine->GetKey(Key::W).bHeld) {
		gridShift.y = (gridShift.y + int(10 * (fElapsedTime / 0.016))) % 20;
		gridOrigin.y = gridOrigin.y + int(10 * (fElapsedTime / 0.016));
	}
	if (engine->GetKey(Key::DOWN).bHeld || engine->GetKey(Key::S).bHeld) {
		gridShift.y = (gridShift.y - int(10 * (fElapsedTime / 0.016))) % 20;
		gridOrigin.y = gridOrigin.y - int(10 * (fElapsedTime / 0.016));
	}



	if (engine->GetKey(Key::NP_ADD).bPressed) {
		editorCellSize = 20;
	}
	if (engine->GetKey(Key::NP_SUB).bPressed) {
		editorCellSize = 10;
	}
	if (engine->GetKey(Key::Z).bPressed) {
		editorCellSize = editorCellSize % 20 + 10;
	}
	if (engine->GetKey(Key::G).bPressed)
	{
		showGrid = !showGrid;
	}
	if (engine->GetKey(Key::E).bPressed)
	{
		playerPosSelected = false;
		levelEndPosSelected = false;
		eraserSelected = !eraserSelected;
	}
	if (engine->GetKey(Key::P).bPressed)
	{
		eraserSelected = false;
		levelEndPosSelected = false;
		playerPosSelected = !playerPosSelected;
	}



	// Place wall

	if (toolSelected == 0 && !eraserSelected && !playerPosSelected && !levelEndPosSelected && engine->GetMouse(0).bHeld) {
		if (selectedCell.x >= 0 && selectedCell.y >= 0 && selectedCell.x < engine->worldMapWidth && selectedCell.y < engine->worldMapHeight) { // Check map boundaries
			engine->worldMap[selectedCell.y * engine->worldMapWidth + selectedCell.x] = selectedToolWall + 1;
		}
	}

	// Place object

	if (toolSelected != 0 && !eraserSelected && !playerPosSelected && !levelEndPosSelected && engine->GetMouse(0).bPressed) {
		if (selectedCell.x >= 0 && selectedCell.y >= 0 && selectedCell.x < engine->worldMapWidth && selectedCell.y < engine->worldMapHeight) { // Check map boundaries
			if (toolSelected == 1) { // Decor
				PlaceDecoration();
			}
			else if (toolSelected == 2) { // Item
				PlaceItem();
			}
			else if (toolSelected == 3) { // Enemy
				PlaceEnemy();
			}

		}
	}

	// Erace

	if (eraserSelected && !playerPosSelected && !levelEndPosSelected && engine->GetMouse(0).bHeld) {
		if (selectedCell.x >= 0 && selectedCell.y >= 0 && selectedCell.x < engine->worldMapWidth && selectedCell.y < engine->worldMapHeight) { // Check map boundaries

			if (toolSelected == 0) { // Wall
				engine->worldMap[selectedCell.y * engine->worldMapWidth + selectedCell.x] = 0;
			}
			else if (toolSelected == 1) { // Decor
				EraceDecorations();
			}
			else if (toolSelected == 2) { // Item
				EraceItems();
			}
			else if (toolSelected == 3) { // Enemy
				EraceEnemies();
			}
		}
	}


	// Place player

	if (playerPosSelected && !eraserSelected && !levelEndPosSelected && engine->GetMouse(0).bPressed) {
		if (selectedCell.x >= 0 && selectedCell.y >= 0 && selectedCell.x < engine->worldMapWidth && selectedCell.y < engine->worldMapHeight) { // Check map boundaries
			engine->player.position = vf2d(selectedCell.x + 0.5, selectedCell.y + 0.5);
		}
	}


	// Place End level

	if (levelEndPosSelected && !eraserSelected && !playerPosSelected && engine->GetMouse(0).bPressed) {
		if (selectedCell.x >= 0 && selectedCell.y >= 0 && selectedCell.x < engine->worldMapWidth && selectedCell.y < engine->worldMapHeight) { // Check map boundaries
			for (int intWall = 0; intWall < engine->interactbleWallsArray.size(); intWall++) {
				if (engine->interactbleWallsArray[intWall].interactionType == InteractibleWall::InteractionType::ENDLEVEL) {
					engine->interactbleWallsArray[intWall].wallPosition = vf2d(selectedCell.x, selectedCell.y);
				}
			}
		}
	}

}
	 
	 
	 
void Editor::ShowToolSelection() {

	mouseOnUI = true; // To prevent placing and moving in background

	engine->FillRect(40, 40, 273, 148, BLACK); // Background
	engine->FillRect(195, 30, 75, 10, BLACK); // Connection
	engine->DrawLine(195, 0, 195, 40, YELLOW);
	engine->DrawLine(270, 0, 270, 40, YELLOW);
	engine->DrawLine(40, 40, 195, 40, YELLOW);
	engine->DrawLine(270, 40, 313, 40, YELLOW);


	int wallPageCount = ceil(engine->wallSprites.size() / 21);
	int decorPageCount = ceil(3 / 21);
	int itemPageCount = ceil(engine->itemIconSprites.size() / 21);
	int enemyPageCount = ceil(engine->enemyIconSprites.size() / 21);


	for (int toolY = 0; toolY < 3; toolY++) {
		for (int toolX = 0; toolX < 7; toolX++) {

			if (toolSelected == 0 && (toolX + toolY * 7) > engine->wallSprites.size() - 1) break; // Walls
			if (toolSelected == 1 && (toolX + toolY * 7) > 3) break; // Decor
			if (toolSelected == 2 && (toolX + toolY * 7) > engine->itemIconSprites.size() - 1) break; // Item
			if (toolSelected == 3 && (toolX + toolY * 7) > engine->enemyIconSprites.size() - 1) break; // Enemy



			Button selectToolButton = Button(engine);
			int textureIdx = 0;
			switch (toolSelected)
			{
			case 0: // Wall
				textureIdx = ((toolX + toolY * 7) + 21 * toolSelectionPage);
				selectToolButton = Button(engine, vi2d(45 + (toolX * (34 + 4)), 45 + (toolY * (34 + 4))), 34, 34, engine->GetWallTexture(textureIdx));
				break;
			case 1: // Decorations
				textureIdx = (toolX + toolY * 7) + 21 * toolSelectionPage;
				selectToolButton = Button(engine, vi2d(45 + (toolX * (34 + 4)), 45 + (toolY * (34 + 4))), 34, 34, engine->GetDecorationSprite(textureIdx));
				break;
			case 2: // Items
				textureIdx = (toolX + toolY * 7) + 21 * toolSelectionPage;
				selectToolButton = Button(engine, vi2d(45 + (toolX * (34 + 4)), 45 + (toolY * (34 + 4))), 34, 34, engine->itemIconSprites[textureIdx]);
				break;
			case 3: // Enemies
				textureIdx = (toolX + toolY * 7) + 21 * toolSelectionPage;
				selectToolButton = Button(engine, vi2d(45 + (toolX * (34 + 4)), 45 + (toolY * (34 + 4))), 34, 34, engine->enemyIconSprites[textureIdx]);
				break;
			default:
				break;
			}
			selectToolButton.Update();
			if (selectToolButton.isHovered) { mouseOnUI = true; }
			if (selectToolButton.isPressed) { SelectTool(textureIdx); showToolSelection = false; }
		}
	}


	Button selectToolNextPageButton = Button(engine, vi2d(267, 160), 40, 20, "NEXT");
	selectToolNextPageButton.Update();
	if (selectToolNextPageButton.isHovered) { mouseOnUI = true; }
	if (selectToolNextPageButton.isPressed) {


		if (toolSelected == 0 && toolSelectionPage < wallPageCount - 1) toolSelectionPage++;  // Walls
		if (toolSelected == 1 && toolSelectionPage < decorPageCount - 1) toolSelectionPage++; // Decor
		if (toolSelected == 2 && toolSelectionPage < itemPageCount - 1) toolSelectionPage++; // Item
		if (toolSelected == 3 && toolSelectionPage < enemyPageCount - 1) toolSelectionPage++; // Enemy

	}

	Button selectToolPrevPageButton = Button(engine, vi2d(45, 160), 40, 20, "PREV");
	selectToolPrevPageButton.Update();
	if (selectToolPrevPageButton.isHovered) { mouseOnUI = true; }
	if (selectToolPrevPageButton.isPressed) {
		if (toolSelectionPage > 0) toolSelectionPage--;
	}

	// Maybe place this somewere else
	int mousePosX = engine->GetMouseX();
	int mousePosY = engine->GetMouseY();


	// Ooooh, this is bad...... But i dont have other solution at this moment :(
	if (!(mousePosX > 40 && mousePosX < 313 && mousePosY > 40 && mousePosY < 182) && !(mousePosX > 195 && mousePosX < 270 && mousePosY > 0 && mousePosY < 40) && engine->GetMouse(0).bPressed) {
		showToolSelection = false;
	}

}


void Editor::ShowSettings() {

	mouseOnUI = true; // To prevent placing and moving in background

	engine->FillRect(20, 40, 273, 142, BLACK); // Background
	engine->FillRect(48, 30, 30, 10, BLACK); // Connection
	engine->DrawLine(48, 0, 48, 40, YELLOW);
	engine->DrawLine(78, 0, 78, 40, YELLOW);
	engine->DrawLine(20, 40, 48, 40, YELLOW);
	engine->DrawLine(78, 40, 292, 40, YELLOW);



	engine->DrawString(vi2d(35, 50), "MAP TITLE");
	mapTitleInputField.Update();
	if (mapTitleInputField.isHovered) { mouseOnUI = true; }
	if (mapTitleInputField.isPressed) { mapTitleInputField.isFocused = true; }

	engine->DrawString(vi2d(35, 85), "SAVE FILE NAME");
	mapSaveFileNameInputField.Update();
	if (mapSaveFileNameInputField.isHovered) { mouseOnUI = true; }
	if (mapSaveFileNameInputField.isPressed) { mapSaveFileNameInputField.isFocused = true; }

	engine->DrawString(vi2d(225, 50), "SIZE X");
	mapSizeXInputField.allowOnlyNumbers = true;
	mapSizeXInputField.charMaximum = 4;
	mapSizeXInputField.Update();
	if (mapSizeXInputField.isHovered) { mouseOnUI = true; }
	if (mapSizeXInputField.isPressed) { mapSizeXInputField.isFocused = true; }

	engine->DrawString(vi2d(225, 85), "SIZE Y");
	mapSizeYInputField.allowOnlyNumbers = true;
	mapSizeYInputField.charMaximum = 4;
	mapSizeYInputField.Update();
	if (mapSizeYInputField.isHovered) { mouseOnUI = true; }
	if (mapSizeYInputField.isPressed) { mapSizeYInputField.isFocused = true; }


	engine->DrawString(vi2d(35, 120), "NEXT MAP FILE NAME");
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
		engine->ChangeMapSize(sizeX, sizeY);
		mapSizeXInputField.text = std::to_string(engine->worldMapWidth);
		mapSizeYInputField.text = std::to_string(engine->worldMapHeight);


		// Map files
		engine->worldMapName = mapTitleInputField.text;
		engine->worldMapFile = mapSaveFileNameInputField.text;
		engine->worldMapNextMapFile = nextMapFileInputField.text;

		// Check if player stay on map
		if (engine->player.position.x >= engine->worldMapWidth || engine->player.position.y >= engine->worldMapHeight) {
			engine->player.position = vf2d(1.5, 1.5);
			editor_infoLog = "Player moved to (1, 1)";
			editor_infoLogTimer = 2;
		}
	}


	// Maybe place this somewere else
	int mousePosX = engine->GetMouseX();
	int mousePosY = engine->GetMouseY();


	// Ooooh, this is bad...... But i dont have other solution at this moment :(
	if (!(mousePosX > 20 && mousePosX < 292 && mousePosY > 40 && mousePosY < 182) && !(mousePosX > 48 && mousePosX < 78 && mousePosY > 0 && mousePosY < 40) && engine->GetMouse(0).bPressed) {
		showSettings = false;
	}
}



void Editor::ShowOpenFile() {

	mouseOnUI = true; // To prevent placing and moving in background

	engine->FillRect(20, 40, 200, 80, BLACK); // Background
	engine->DrawLine(20, 40, 219, 40, YELLOW);

	engine->DrawString(vi2d(35, 50), "FILE NAME TO OPEN");
	mapOpenFileInputField.Update();
	if (mapOpenFileInputField.isHovered) { mouseOnUI = true; }
	if (mapOpenFileInputField.isPressed) { mapOpenFileInputField.isFocused = true; }


	mapLoadMapButton.Update();
	mapLoadMapButton.colorBackground = DARK_GREEN;
	if (mapLoadMapButton.isHovered) { mouseOnUI = true; }
	if (mapLoadMapButton.isPressed)
	{
		engine->worldMapFile = mapOpenFileInputField.text;
		std::string loadFile = "maps/" + engine->worldMapFile + ".map";
		engine->LoadMap(loadFile);
		mapSizeXInputField.text = std::to_string(engine->worldMapWidth);
		mapSizeYInputField.text = std::to_string(engine->worldMapHeight);
		mapTitleInputField.text = engine->worldMapName;
		mapSaveFileNameInputField.text = engine->worldMapFile;
		nextMapFileInputField.text = engine->worldMapNextMapFile;
	}

	mapCloseOpenFileButton.Update();
	mapCloseOpenFileButton.colorBackground = DARK_RED;
	if (mapCloseOpenFileButton.isHovered) { mouseOnUI = true; }
	if (mapCloseOpenFileButton.isPressed)
	{
		showOpenFile = false;
	}

}

void Editor::ShowExitDialog() {


	mouseOnUI = true; // To prevent placing and moving in background

	engine->FillRect(20, 40, 200, 80, BLACK); // Background
	engine->DrawLine(20, 40, 219, 40, YELLOW);

	engine->DrawString(vi2d(35, 50), "EXIT EDITOR ?");

	mapExitEditorButton.Update();
	mapExitEditorButton.colorBackground = DARK_GREEN;
	if (mapExitEditorButton.isHovered) { mouseOnUI = true; }
	if (mapExitEditorButton.isPressed)
	{
		showExitDialog = false;
		engine->gameState = engine->GameState::STATE_TITLESCREEN;
	}

	mapCloseExitDialogButton.Update();
	mapCloseExitDialogButton.colorBackground = DARK_RED;
	if (mapCloseExitDialogButton.isHovered) { mouseOnUI = true; }
	if (mapCloseExitDialogButton.isPressed)
	{
		engine->LoadDefaultMap();
		showExitDialog = false;
	}

}


void Editor::DrawEditor(float fElapsedTime) {


	mouseOnUI = false;




	// Background -----------------------

	engine->FillRect(0, 0, engine->ScreenWidth(), engine->ScreenHeight(), VERY_DARK_GREY);


	// Draw map -----------------------

	int sampleSize = editorCellSize - 1 + !showGrid; // Little optimisation

	for (int i = 0; i < engine->worldMapWidth; i++) {
		for (int j = 0; j < engine->worldMapHeight; j++) {

			if (engine->worldMap[j * engine->worldMapWidth + i] != 0) {
				for (int smplX = 0; smplX < sampleSize; smplX++) {
					for (int smplY = 0; smplY < sampleSize; smplY++) {
						Color color = engine->GetWallTexture(engine->worldMap[j * engine->worldMapWidth + i] - 1)->GetPixel(int(64.0 / editorCellSize * smplX), int(64.0 / editorCellSize * smplY));
						engine->Draw(gridOrigin.x + 1 + smplX + i * editorCellSize, gridOrigin.y + 1 + smplY + j * editorCellSize, color);
					}
				}
			}
		}
	}



	// Grid -----------------------

	if (showGrid) {
		// Vertical lines
		for (int i = 0; i < int(engine->ScreenWidth() / editorCellSize) + 2; i++) {
			engine->DrawLine(i * editorCellSize + gridShift.x, 0, i * editorCellSize + gridShift.x, engine->ScreenHeight(), GREY);
		}
		// Horisontal lines
		for (int j = 0; j < int(engine->ScreenHeight() / editorCellSize) + 2; j++) {
			engine->DrawLine(0, j * editorCellSize + gridShift.y, engine->ScreenWidth(), j * editorCellSize + gridShift.y, GREY);
		}


		// Draw grid origin
		engine->DrawLine(gridOrigin.x, 0, gridOrigin.x, engine->ScreenHeight(), YELLOW);
		engine->DrawLine(0, gridOrigin.y, engine->ScreenWidth(), gridOrigin.y, YELLOW);

		// Draw end of map
		engine->DrawLine(gridOrigin.x + engine->worldMapWidth * editorCellSize, 0, gridOrigin.x + engine->worldMapWidth * editorCellSize, engine->ScreenHeight(), RED);
		engine->DrawLine(0, gridOrigin.y + engine->worldMapHeight * editorCellSize, engine->ScreenWidth(), gridOrigin.y + engine->worldMapHeight * editorCellSize, RED);
	}



	// Draw objects -----------------------


	//
	// Note : decorationsArray[d].position.x - 0.5 in fact tells that i need to shift sprite half size
	//

	// Draw Decorations

	for (int d = 0; d < engine->decorationsArray.size(); d++) {

		for (int smplX = 0; smplX < editorCellSize - 1; smplX++) {
			for (int smplY = 0; smplY < editorCellSize - 1; smplY++) {
				Color color = engine->GetDecorationSprite(engine->decorationsArray[d].texture)->GetPixel(int(64.0 / editorCellSize * smplX), int(64.0 / editorCellSize * smplY));
				if (color != CYAN) {
					int pixelPosX = gridOrigin.x + 1 + smplX + (engine->decorationsArray[d].position.x - 0.5) * editorCellSize;
					int pixelPosY = gridOrigin.y + 1 + smplY + (engine->decorationsArray[d].position.y - 0.5) * editorCellSize;
					engine->Draw(pixelPosX, pixelPosY, color);
				}
			}
		}
	}

	// Draw Items

	for (int i = 0; i < engine->itemsArray.size(); i++) {

		for (int smplX = 0; smplX < editorCellSize - 1; smplX++) {
			for (int smplY = 0; smplY < editorCellSize - 1; smplY++) {
				Color color = engine->itemIconSprites[engine->itemsArray[i].texture]->GetPixel(floor((float)engine->itemIconSprites[engine->itemsArray[i].texture]->width / editorCellSize * smplX), floor((float)engine->itemIconSprites[engine->itemsArray[i].texture]->height / editorCellSize * smplY));
				if (color != CYAN) {
					int pixelPosX = gridOrigin.x + 1 + smplX + (engine->itemsArray[i].position.x - 0.5) * editorCellSize;
					int pixelPosY = gridOrigin.y + 1 + smplY + (engine->itemsArray[i].position.y - 0.5) * editorCellSize;
					engine->Draw(pixelPosX, pixelPosY, color);
				}
			}
		}
	}

	// Draw Enemies

	for (int d = 0; d < engine->enemiesArray.size(); d++) {

		for (int smplX = 0; smplX < editorCellSize - 1; smplX++) {
			for (int smplY = 0; smplY < editorCellSize - 1; smplY++) {
				Color color = engine->enemyIconSprites[engine->enemiesArray[d].texture]->GetPixel(int((float)engine->enemyIconSprites[engine->enemiesArray[d].texture]->width / editorCellSize * smplX), int((float)engine->enemyIconSprites[engine->enemiesArray[d].texture]->height / editorCellSize * smplY));
				if (color != CYAN) {
					int pixelPosX = gridOrigin.x + 1 + smplX + (engine->enemiesArray[d].position.x - 0.5) * editorCellSize;
					int pixelPosY = gridOrigin.y + 1 + smplY + (engine->enemiesArray[d].position.y - 0.5) * editorCellSize;
					engine->Draw(pixelPosX, pixelPosY, color);
				}
			}
		}
	}

	// Draw Player

	for (int smplX = 0; smplX < editorCellSize - 1; smplX++) {
		for (int smplY = 0; smplY < editorCellSize - 1; smplY++) {
			Color color = engine->spriteEditorToolPlayer.GetPixel(int((float)engine->spriteEditorToolPlayer.width / editorCellSize * smplX), int((float)engine->spriteEditorToolPlayer.height / editorCellSize * smplY));
			if (color != CYAN) {
				int pixelPosX = gridOrigin.x + smplX + (engine->player.position.x - 0.5) * editorCellSize;
				int pixelPosY = gridOrigin.y + smplY + (engine->player.position.y - 0.5) * editorCellSize;
				engine->Draw(pixelPosX, pixelPosY, color);
			}
		}
	}


	// Draw End level

	for (int intWall = 0; intWall < engine->interactbleWallsArray.size(); intWall++) {
		if (engine->interactbleWallsArray[intWall].interactionType == InteractibleWall::InteractionType::ENDLEVEL) {
			for (int smplX = 0; smplX < editorCellSize - 1; smplX++) {
				for (int smplY = 0; smplY < editorCellSize - 1; smplY++) {
					Color color = engine->spriteEditorToolEndLevel.GetPixel(int((float)engine->spriteEditorToolEndLevel.width / editorCellSize * smplX), int((float)engine->spriteEditorToolEndLevel.height / editorCellSize * smplY));
					if (color != CYAN) {
						int pixelPosX = gridOrigin.x + smplX + (engine->interactbleWallsArray[intWall].wallPosition.x) * editorCellSize;
						int pixelPosY = gridOrigin.y + smplY + (engine->interactbleWallsArray[intWall].wallPosition.y) * editorCellSize;
						engine->Draw(pixelPosX, pixelPosY, color);
					}
				}
			}
		}
	}



	// Cell selection ----------------------- 

	selectedCell.x = (engine->GetMouseX() - gridOrigin.x) / (editorCellSize * 1.0);
	selectedCell.y = (engine->GetMouseY() - gridOrigin.y) / (editorCellSize * 1.0);


	if (toolSelected == 0 || toolSelected == 1 || eraserSelected) { // Wall or Decoration

		selectedCell.x = floor(selectedCell.x);
		selectedCell.y = floor(selectedCell.y);

		// Rectangle
		engine->DrawRect(gridOrigin.x + selectedCell.x * editorCellSize, gridOrigin.y + selectedCell.y * editorCellSize, editorCellSize, editorCellSize, RED);

		std::string cellXText = std::to_string(selectedCell.x);
		std::string cellYText = std::to_string(selectedCell.y);
		std::string cellText = cellXText.substr(0, cellXText.length() - 5) + " " + cellYText.substr(0, cellYText.length() - 5);
		engine->DrawString(5, engine->ScreenHeight() - 10, cellText);
	}
	else if (toolSelected == 2 || toolSelected == 3) { // Items or Enemy

		selectedCell.x = (floor(selectedCell.x * 2)) / 2; // Round to the nearest 0.5 
		selectedCell.y = (floor(selectedCell.y * 2)) / 2;

		// Dot
		engine->FillRect(gridOrigin.x + selectedCell.x * editorCellSize - 3, gridOrigin.y + selectedCell.y * editorCellSize - 3, 7, 7, RED);

		std::string cellXText = std::to_string(selectedCell.x);
		std::string cellYText = std::to_string(selectedCell.y);
		std::string cellText = cellXText.substr(0, cellXText.length() - 5) + " " + cellYText.substr(0, cellYText.length() - 5);
		engine->DrawString(5, engine->ScreenHeight() - 10, cellText);
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
	engine->FillRect(0, 0, 320, 30, BLACK);





	// Save button
	Button saveButton = Button(engine, vi2d(3, 3), 22, 22, &engine->spriteEditorSave);
	saveButton.showBorder = false;
	saveButton.hoverText = "SAVE";
	saveButton.Update();
	if (saveButton.isHovered) { mouseOnUI = true; hoverText = saveButton.hoverText; }
	if (saveButton.isPressed)
	{
		engine->worldMapName = mapTitleInputField.text;
		engine->worldMapFile = mapSaveFileNameInputField.text;
		std::string saveFile = "maps/" + engine->worldMapFile + ".map";
		engine->SaveMap(saveFile);

		editor_infoLogTimer = 2;
		editor_infoLog = "Map saved as : " + saveFile;
	}

	// Load button
	Button loadButton = Button(engine, vi2d(28, 3), 22, 22, &engine->spriteEditorLoad);
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
	Button settingsButton = Button(engine, vi2d(53, 3), 22, 22, &engine->spriteEditorSettings);
	settingsButton.showBorder = false;
	settingsButton.hoverText = "SETTINGS";
	settingsButton.Update();
	if (settingsButton.isHovered) { mouseOnUI = true; hoverText = settingsButton.hoverText; }
	if (settingsButton.isPressed) {
		showSettings = !showSettings;
		showToolSelection = false;
		showOpenFile = false;

		mapSizeXInputField.text = std::to_string(engine->worldMapWidth);
		mapSizeYInputField.text = std::to_string(engine->worldMapHeight);
		mapTitleInputField.text = engine->worldMapName;
		mapSaveFileNameInputField.text = engine->worldMapFile;
		nextMapFileInputField.text = engine->worldMapNextMapFile;
	}

	// Play button
	Button playButton = Button(engine, vi2d(279, 0), 30, 30, &engine->spriteEditorPlay);
	playButton.showBorder = false;
	playButton.hoverText = "PLAY";
	playButton.Update();
	if (playButton.isHovered) { mouseOnUI = true; hoverText = playButton.hoverText; }
	if (playButton.isPressed)
	{
		// Save
		engine->worldMapName = mapTitleInputField.text;
		engine->worldMapFile = mapSaveFileNameInputField.text;
		std::string saveFile = "maps/" + engine->worldMapFile + ".map";
		engine->SaveMap(saveFile);

		editor_infoLogTimer = 2;
		editor_infoLog = "Map saved as : " + saveFile;

		// Load
		isEditorInPlayMode = true;
		std::string loadFile = "maps/" + engine->worldMapFile + ".map";
		engine->LoadMap(loadFile);

		// Start MIDI
		engine->PlayMapMIDI();

		// Play
		engine->player.Ressurect();
		engine->gameState = engine->STATE_GAMEPLAY;
	}




	// Tool Wall button
	Button toolWallButton = Button(engine, vi2d(90, 5), 20, 20, &engine->spriteEditorToolWall);
	toolWallButton.showBorder = true;
	toolWallButton.hoverText = "WALL";
	toolWallButton.Update();
	if (toolWallButton.isHovered) { mouseOnUI = true; hoverText = toolWallButton.hoverText; }
	if (toolWallButton.isPressed) { toolSelected = 0; toolSelectionPage = 0; }

	// Tool Decorations button
	Button toolDecorButton = Button(engine, vi2d(115, 5), 20, 20, &engine->spriteEditorToolDecor);
	toolDecorButton.showBorder = true;
	toolDecorButton.hoverText = "DECOR";
	toolDecorButton.Update();
	if (toolDecorButton.isHovered) { mouseOnUI = true; hoverText = toolDecorButton.hoverText; }
	if (toolDecorButton.isPressed) { toolSelected = 1; toolSelectionPage = 0; }

	// Tool Items button
	Button toolItemButton = Button(engine, vi2d(140, 5), 20, 20, &engine->spriteEditorToolItem);
	toolItemButton.showBorder = true;
	toolItemButton.hoverText = "ITEM";
	toolItemButton.Update();
	if (toolItemButton.isHovered) { mouseOnUI = true; hoverText = toolItemButton.hoverText; }
	if (toolItemButton.isPressed) { toolSelected = 2; toolSelectionPage = 0; }

	// Tool Enemies button
	Button toolEnemyButton = Button(engine, vi2d(165, 5), 20, 20, &engine->spriteEditorToolEnemy);
	toolEnemyButton.showBorder = true;
	toolEnemyButton.hoverText = "ENEMY";
	toolEnemyButton.Update();
	if (toolEnemyButton.isHovered) { mouseOnUI = true; hoverText = toolEnemyButton.hoverText; }
	if (toolEnemyButton.isPressed) { toolSelected = 3; toolSelectionPage = 0; }




	// Tool Eraser button
	Button toolEraserButton = Button(engine, vi2d(3, 35), 20, 20, &engine->spriteEditorToolEracer);
	toolEraserButton.showBorder = true;
	toolEraserButton.hoverText = "ERASER";
	toolEraserButton.Update();
	if (toolEraserButton.isHovered) { mouseOnUI = true; hoverText = toolEraserButton.hoverText; }
	if (toolEraserButton.isPressed) { playerPosSelected = false; levelEndPosSelected = false;  eraserSelected = !eraserSelected; }

	// Tool Zoom button
	Button toolZoomButton = Button(engine, vi2d(3, 60), 20, 20, &engine->spriteEditorToolZoom);
	toolZoomButton.showBorder = true;
	toolZoomButton.hoverText = "ZOOM";
	toolZoomButton.Update();
	if (toolZoomButton.isHovered) { mouseOnUI = true; hoverText = toolZoomButton.hoverText; }
	if (toolZoomButton.isPressed) editorCellSize = editorCellSize % 20 + 10;

	// Tool Grid button
	Button toolGridButton = Button(engine, vi2d(3, 85), 20, 20, &engine->spriteEditorToolGrid);
	toolGridButton.showBorder = true;
	toolGridButton.hoverText = "GRID";
	toolGridButton.Update();
	if (toolGridButton.isHovered) { mouseOnUI = true; hoverText = toolGridButton.hoverText; }
	if (toolGridButton.isPressed) showGrid = !showGrid;

	// Tool Player button
	Button toolPlayerButton = Button(engine, vi2d(3, 125), 20, 20, &engine->spriteEditorToolPlayer);
	toolPlayerButton.showBorder = true;
	toolPlayerButton.hoverText = "START POSITION";
	toolPlayerButton.Update();
	if (toolPlayerButton.isHovered) { mouseOnUI = true; hoverText = toolPlayerButton.hoverText; }
	if (toolPlayerButton.isPressed) { eraserSelected = false; levelEndPosSelected = false; playerPosSelected = !playerPosSelected; }

	// Tool EndLevel button
	Button toolEndLevelButton = Button(engine, vi2d(3, 150), 20, 20, &engine->spriteEditorToolEndLevel);
	toolEndLevelButton.showBorder = true;
	toolEndLevelButton.hoverText = "END LEVEL";
	toolEndLevelButton.Update();
	if (toolEndLevelButton.isHovered) { mouseOnUI = true; hoverText = toolEndLevelButton.hoverText; }
	if (toolEndLevelButton.isPressed) { eraserSelected = false; playerPosSelected = false; levelEndPosSelected = !levelEndPosSelected; }


	// Tool label
	engine->DrawString(200, 5, "TOOL:", WHITE);

	// Tool button
	Button textureButton = Button(engine);
	switch (toolSelected)
	{
	case 0: // Wall
		textureButton = Button(engine, vi2d(239, 2), 25, 25, engine->GetWallTexture(selectedToolWall));
		break;
	case 1: // Decorations
		textureButton = Button(engine, vi2d(239, 2), 25, 25, engine->GetDecorationSprite(selectedToolDecor));
		break;
	case 2: // Items
		textureButton = Button(engine, vi2d(239, 2), 25, 25, engine->itemIconSprites[selectedToolItem]);
		break;
	case 3: // Enemies
		textureButton = Button(engine, vi2d(239, 2), 25, 25, engine->enemyIconSprites[selectedToolEnemy]);
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

	engine->DrawRect(88 + toolSelected * 25, 3, 24, 24, YELLOW);
	engine->DrawRect(89 + toolSelected * 25, 4, 22, 22, YELLOW);


	if (eraserSelected) {
		engine->DrawRect(2, 34, 22, 22, YELLOW);
		engine->DrawRect(1, 33, 24, 24, YELLOW);
		// Cursor
		engine->DrawSpriteColorTransparent(engine->GetMouseX() + 3, engine->GetMouseY() + 5, &engine->spriteEditorToolEracer, CYAN);
	}

	if (playerPosSelected) {
		engine->DrawRect(2, 124, 22, 22, YELLOW);
		engine->DrawRect(1, 123, 24, 24, YELLOW);
		// Cursor
		engine->DrawSpriteColorTransparent(engine->GetMouseX() + 3, engine->GetMouseY() + 5, &engine->spriteEditorToolPlayer, CYAN);
	}

	if (levelEndPosSelected) {
		engine->DrawRect(2, 149, 22, 22, YELLOW);
		engine->DrawRect(1, 148, 24, 24, YELLOW);
		// Cursor
		engine->DrawSpriteColorTransparent(engine->GetMouseX() + 3, engine->GetMouseY() + 5, &engine->spriteEditorToolEndLevel, CYAN);
	}



	// Tool selection -----------------------

	if (showToolSelection) {
		ShowToolSelection();
	}


	// Settings window selection -----------------------

	if (showSettings) {
		ShowSettings();
	}

	// Open file window -----------------------

	if (showOpenFile) {
		ShowOpenFile();
	}

	// Open exit dialog -----------------------

	if (showExitDialog) {
		ShowExitDialog();
	}



	// Hovering text -----------------------

	if (hoverText != "") {
		engine->DrawString(engine->GetMouseX() - 1, engine->GetMouseY() + 18, hoverText, BLACK);
		engine->DrawString(engine->GetMouseX() - 2, engine->GetMouseY() + 17, hoverText, WHITE);
	}





	// Info log -----------------------

	if (editor_infoLogTimer > 0) {
		editor_infoLogTimer -= fElapsedTime;
		engine->DrawString(31, 38, editor_infoLog, BLACK);
		engine->DrawString(30, 37, editor_infoLog);
	}



	// Editor controls -----------------------

	if (!showSettings && !showToolSelection && !showOpenFile && !showExitDialog && !mouseOnUI) {
		Controls(fElapsedTime);
	}



	// Exit editor -----------------------

	if (engine->GetKey(Key::ESCAPE).bPressed) {
		showExitDialog = true;
	}


}


