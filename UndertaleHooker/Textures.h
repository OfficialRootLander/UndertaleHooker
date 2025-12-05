#pragma once
#ifndef TEXTURES_H
#define TEXTURES_H
#include <iostream>

struct UTexture
{
	int SpriteID;
	int Width;
	int Height;
	bool Invisible;

	UTexture(int ESpriteID, int EWidth, int EHeight, bool EInvisible)
		: SpriteID(ESpriteID), Width(EWidth), Height(EHeight), Invisible(EInvisible)
	{
	}

	int GetSpriteID() const
	{
		return SpriteID;
	}

	int GetWidth() const
	{
		return Width;
	}

	int GetHeight() const
	{
		return Height;
	}

	bool AInvisible() const
	{
		return Invisible;
	}
};

class Textures
{
public:
	static UTexture GetTexture(int SpriteID);
};

#endif