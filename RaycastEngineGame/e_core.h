#pragma once

#include "olcPixelGameEngine.h"

#include "e_vector.h"
#include "r_color.h"
#include "r_sprite.h"
#include "i_controls.h"




enum rcode { FAIL = 0, OK = 1, NO_FILE = -1 };


class RECore {

public:
	olc::PixelGameEngine* pixelGameEngine;




	//RECore() {
	//	std::cout << "RECore Default constructor" << std::endl;
	//	pixelGameEngine = nullptr;
	//}

	RECore(olc::PixelGameEngine* pge) {
		std::cout << "RECore: Constructor" << std::endl;
		pixelGameEngine = pge;
		std::cout << "RECore: AppName = " << pixelGameEngine->sAppName << std::endl;
	}

	olc::Pixel ColorToOLCPixel(Color c) {
		return olc::Pixel(c.r, c.g, c.b, c.a);
	}




	// Draws a single Pixel
	bool Draw(int32_t x, int32_t y, Color c = WHITE) {
		return pixelGameEngine->Draw(x, y, ColorToOLCPixel(c));
	}

	bool Draw(const vi2d& pos, Color c = WHITE) {
		return pixelGameEngine->Draw(olc::vi2d(pos.x, pos.y), ColorToOLCPixel(c));
	}

	// Draws a line from (x1,y1) to (x2,y2)
	void DrawLine(int32_t x1, int32_t y1, int32_t x2, int32_t y2, Color c = WHITE) {
		pixelGameEngine->DrawLine(x1, y1, x2, y2, ColorToOLCPixel(c));
	}

	void DrawLine(const vi2d& pos1, const vi2d& pos2, Color c = WHITE) {
		pixelGameEngine->DrawLine(olc::vi2d(pos1.x, pos1.y), olc::vi2d(pos2.x, pos2.y), ColorToOLCPixel(c));
	}

	// Draws a rectangle at (x,y) to (x+w,y+h)
	void DrawRect(int32_t x, int32_t y, int32_t w, int32_t h, Color c = WHITE) {
		pixelGameEngine->DrawRect(x, y, w, h, ColorToOLCPixel(c));
	}

	void DrawRect(const vi2d& pos, const vi2d& size, Color c = WHITE) {
		pixelGameEngine->DrawRect(olc::vi2d(pos.x, pos.y), olc::vi2d(size.x, size.y), ColorToOLCPixel(c));
	}

	// Fills a rectangle at (x,y) to (x+w,y+h)
	void FillRect(int32_t x, int32_t y, int32_t w, int32_t h, Color c = WHITE) {
		pixelGameEngine->FillRect(x, y, w, h, ColorToOLCPixel(c));
	}

	void FillRect(const vi2d& pos, const vi2d& size, Color c = WHITE) {
		pixelGameEngine->FillRect(olc::vi2d(pos.x, pos.y), olc::vi2d(size.x, size.y), ColorToOLCPixel(c));
	}

	// Draws a single line of text
	void DrawString(int32_t x, int32_t y, const std::string& sText, Color col = WHITE, uint32_t scale = 1) {
		pixelGameEngine->DrawString(x, y, sText, ColorToOLCPixel(col), scale);
	}

	void DrawString(const vi2d& pos, const std::string& sText, Color col = WHITE, uint32_t scale = 1) {
		pixelGameEngine->DrawString(olc::vi2d(pos.x, pos.y), sText, ColorToOLCPixel(col), scale);
	}



	// Controls


	HWButton GetKey(Key k) {
		olc::Key APIKey = static_cast<olc::Key>(k);
		olc::HWButton APIButtonStates = pixelGameEngine->GetKey(APIKey);
		HWButton buttonStates;
		buttonStates.bHeld = APIButtonStates.bHeld;
		buttonStates.bPressed = APIButtonStates.bPressed;
		buttonStates.bReleased = APIButtonStates.bReleased;
		return buttonStates;
	}

	HWButton GetMouse(uint32_t b) {
		olc::HWButton APIButtonStates = pixelGameEngine->GetMouse(b);
		HWButton buttonStates;
		buttonStates.bHeld = APIButtonStates.bHeld;
		buttonStates.bPressed = APIButtonStates.bPressed;
		buttonStates.bReleased = APIButtonStates.bReleased;
		return buttonStates;
	}

	int32_t GetMouseX()
	{
		return pixelGameEngine->GetMouseX();
	}

	int32_t GetMouseY()
	{
		return pixelGameEngine->GetMouseY();
	}



	int32_t ScreenWidth() {
		return pixelGameEngine->ScreenWidth();
	}

	int32_t ScreenHeight() {
		return pixelGameEngine->ScreenHeight();
	}


	bool IsFocused() {
		return pixelGameEngine->IsFocused();
	}

	vi2d GetWindowMouse() {
		olc::vi2d vec = pixelGameEngine->GetWindowMouse();
		return vi2d(vec.x, vec.y);
	}

	vi2d GetWindowSize() {
		olc::vi2d vec = pixelGameEngine->GetWindowSize();
		return vi2d(vec.x, vec.y);
	}

	vi2d GetWindowPosition() {
		olc::vi2d vec = pixelGameEngine->GetWindowPosition();
		return vi2d(vec.x, vec.y);
	}






};

