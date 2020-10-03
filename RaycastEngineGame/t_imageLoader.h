#pragma once
#include <iostream>


//#define NOMINMAX
//#define VC_EXTRALEAN
//#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <gdiplus.h>
#include <Shlwapi.h>
#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "Shlwapi.lib")


#include "r_sprite.h"



class ImageLoader
{
public:
	ImageLoader();
	~ImageLoader();
	olc::rcode LoadImageResource(Sprite* spr, const std::string& sImageFile);
};

