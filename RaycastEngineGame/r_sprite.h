#pragma once
#include <iostream>

#include "olcPixelGameEngine.h"
#include "r_color.h"


class Sprite
{
public:
	Sprite();
	//Sprite(const std::string& sImageFile);
	Sprite(int32_t w, int32_t h);
	Sprite(const Sprite&) = delete;
	~Sprite();

public:
	int32_t width = 0;
	int32_t height = 0;
	enum Flip { NONE = 0, HORIZ = 1, VERT = 2 };

	//olc::rcode LoadFromFile(const std::string& sImageFile);
	Color GetPixel(int32_t x, int32_t y);
	bool SetPixel(int32_t x, int32_t y, Color p);
	Color GetPixel(const olc::vi2d& a);
	bool  SetPixel(const olc::vi2d& a, Color p);
	Color Sample(float x, float y);
	Color SampleBL(float u, float v);

	Color* pColData = nullptr;
};





