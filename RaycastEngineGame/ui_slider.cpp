#include "ui_slider.h"
#include "c_engine.h"


Slider::Slider(RaycastEngine* eng) {
	engine = eng;
	position = vi2d(0, 0);
	width = 20;
	height = 40;
	isPressed = false;
	isHovered = false;
	showBorder = true;
	showBackground = true;
	colorBorder = GREY;
	colorBackground = BLACK;
	colorSlider = WHITE;
	colorSliderLine = WHITE;
	colorHovered = YELLOW;
	colorPressed = DARK_YELLOW;
	hoverText = "";
	value = 0.5;
}

Slider::Slider(RaycastEngine* eng, vi2d pos, uint32_t w, uint32_t h) {
	engine = eng;
	position = pos;
	width = w;
	height = h;
	isPressed = false;
	isHovered = false;
	showBorder = true;
	showBackground = true;
	colorBorder = GREY;
	colorBackground = BLACK;
	colorSlider = WHITE;
	colorSliderLine = WHITE;
	colorHovered = YELLOW;
	colorPressed = DARK_YELLOW;
	hoverText = "";
	value = 0.5;
}






void Slider::Update() {


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

	if (isHovered && engine->GetMouse(0).bHeld) {
		isHeld = true;

		int mouseRelPos = engine->GetMouseX() - position.x;
		value = mouseRelPos * 1.0 / width;

		if (value < 0.05) {
			value = 0;
		}
	}



	// Draw field

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
	engine->FillRect(position.x, position.y + height / 2 - 2, width, 4, colorSliderLine); // Slider line

	engine->FillRect(position.x + value * width, position.y, 5, height, BLACK); // Slider background
	engine->DrawRect(position.x + value * width, position.y, 5, height, colorSlider); // Slider



	// Handle input from keyboard

}



