#pragma once

#include "olcPixelGameEngine.h"

class Slider {
public:
	//position x and y, width and height
	float x, y, w, h;
	float min, value, max;
	olc::Pixel palette[4];
	int index;
	bool activeSlider = false;
	olc::vi2d prevMouse;

	void draw(olc::PixelGameEngine* pge) {
		if (min > max) {
			std::swap(min, max);
		}
		olc::Pixel head = palette[2];
		olc::vi2d currMouse = pge->GetMousePos();
		olc::vi2d dragMouse = currMouse - prevMouse;
		prevMouse = currMouse;
		//if mouse is over the slider
		if (currMouse.x >= x && currMouse.y >= y && currMouse.x < x + w && currMouse.y < y + h) {
			head = palette[3];
			//if mouse clicks on the slider 
			if (pge->GetMouse(0).bPressed || pge->GetMouse(0).bHeld) {
				activeSlider = true;
			}
		}
		if (pge->GetMouse(0).bReleased) {
			activeSlider = false;
		}
		if (activeSlider) {
			value += abs(max - min) * dragMouse.x / w;
			value = (value <= min) ? min : value;
			value = (value > max) ? max : value;
		}
		pge->FillRect(x - h, y - h, h, 3 * h, palette[0]);//border
		pge->FillRect(x, y, w, h, palette[1]);//line
		pge->FillRect(x + w * (value - min) / (max - min), y - 1 * h, h, 3 * h, head);//head
		pge->DrawString(0, 32 + index * 64 + 16, std::to_string(value), olc::WHITE);//value

	}
};