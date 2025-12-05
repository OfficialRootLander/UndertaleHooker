#include "pch.h"
#include "RootSystem.h"

void RootSystem::CoutAllPlayersFromVector(std::vector<RootVector> RootVectors)
{
	for (int i = 0; i < RootVectors.size(); i++)
	{
		std::cout << "Found RootVector For Player" << (i + 1) << " x: " << RootVectors.at(i).GetX() << " y: " << RootVectors.at(i).GetY() << " \n";
	}
}