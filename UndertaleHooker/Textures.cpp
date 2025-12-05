#include "pch.h"
#include "Textures.h"

UTexture Textures::GetTexture(int SpriteID)
{
	if (SpriteID == 1)
	{
		//make a new texture
		UTexture Texture(SpriteID, 128, 128, false);
		return Texture;
	}
	else if (SpriteID == 2)
	{
		//make a new texture
		UTexture Texture(SpriteID, 100, 100, false);
		return Texture;
	}
	//add more here
}