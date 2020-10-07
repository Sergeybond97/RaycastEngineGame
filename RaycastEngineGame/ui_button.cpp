#include "ui_button.h"
#include "c_engine.h"


Button::Button() {
	engine = RaycastEngine::getInstance();
	position = vi2d(0, 0);
	width = 20;
	height = 40;
	enabled = true;
	isPressed = false;
	isHovered = false;
	showBorder = true;
	showBackground = true;
	colorBorder = GREY;
	colorBackground = BLACK;
	colorText = WHITE;
	colorHovered = YELLOW;
	colorPressed = DARK_YELLOW;
	text = "BTN";
	hoverText = "";
	sprite = nullptr;
}

Button::Button(vi2d pos, uint32_t w, uint32_t h, std::string txt) {
	engine = RaycastEngine::getInstance();
	position = pos;
	width = w;
	height = h;
	enabled = true;
	isPressed = false;
	isHovered = false;
	showBorder = true;
	showBackground = true;
	colorBorder = GREY;
	colorBackground = BLACK;
	colorText = WHITE;
	colorHovered = YELLOW;
	colorPressed = DARK_YELLOW;
	text = txt;
	hoverText = "";
	sprite = nullptr;
}

Button::Button(vi2d pos, uint32_t w, uint32_t h, Sprite* img) {
	engine = RaycastEngine::getInstance();
	position = pos;
	width = w;
	height = h;
	enabled = true;
	isPressed = false;
	isHovered = false;
	showBorder = true;
	showBackground = true;
	colorBorder = GREY;
	colorBackground = BLACK;
	colorText = WHITE;
	colorHovered = YELLOW;
	colorPressed = DARK_YELLOW;
	text = "";
	hoverText = "";
	sprite = img;
}



void Button::Update() {


	// Check mouse input

	isPressed = false;
	isHovered = false;

	vi2d mousePos = vi2d(engine->GetMouseX(), engine->GetMouseY());
	if (mousePos.x > position.x && mousePos.x < position.x + width && mousePos.y > position.y && mousePos.y < position.y + height)
	{
		isHovered = true;
	}

	if (isHovered && engine->GetMouse(0).bPressed) {
		isPressed = true;
	}


	// Draw button

	if (showBorder) {
		engine->DrawRect(position.x, position.y, width, height, colorBorder); // Border
		if (showBackground) {
			engine->FillRect(position.x + 1, position.y + 1, width - 1, height - 1, colorBackground); // Background
		}
	}
	else {
		if (showBackground) {
			engine->FillRect(position.x, position.y, width, height, colorBackground); // Background
		}
	}

	// Draw selection
	if (isHovered) {
		engine->DrawRect(position.x, position.y, width, height, colorHovered);
	}

	// Draw content

	if (text != "") {
		engine->DrawString(position.x + 6, position.y + (height / 2 - 3), text, colorText);
	}

	if (sprite != nullptr) {

		for (int smplX = 0; smplX < width - 2; smplX++) {
			for (int smplY = 0; smplY < height - 2; smplY++) {
				Color color = sprite->GetPixel(int((1.0 * sprite->width / (width - 2)) * smplX), int((1.0 * sprite->height / (height - 2)) * smplY));
				if (color != CYAN) {
					engine->Draw(smplX + position.x + 1, smplY + position.y + 1, color);
				}
			}
		}
	}



}
