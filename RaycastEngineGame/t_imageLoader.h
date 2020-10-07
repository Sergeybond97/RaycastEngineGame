#pragma once
#include <iostream>


#include "e_core.h"



class ImageLoader
{
public:
	ImageLoader();
	~ImageLoader();
	rcode LoadImageResource(Sprite* spr, const std::string& sImageFile);
};

