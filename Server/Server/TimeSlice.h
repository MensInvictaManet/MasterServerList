#pragma once

#include <algorithm>

float gameSeconds = 0.0f;
float tickSeconds = 0.0f;

inline void DetermineTimeSlice()
{
	// Get the time slice
	static int elapsedTime[2] = { 0, 0 };

	elapsedTime[0] = SDL_GetTicks();
	gameSeconds = float(elapsedTime[0]) / 1000.0f;
	tickSeconds = std::max(std::min(float(elapsedTime[0] - elapsedTime[1]) / 1000.0f, 1.0f), 0.0f);
	elapsedTime[1] = elapsedTime[0];
}