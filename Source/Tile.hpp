#ifndef TILE_H
#define TILE_H
#include "SoulStoneEngine/Utilities/GameCommon.hpp"
#include "TileBlueprint.hpp"

class Tile
{
	public:
		TileBlueprint*		m_tileBlueprint;
		TileCoords2D		m_tileCoords;
		bool				m_isVisibleNow;
		bool				m_isVisibleLastTime;

	public:
		Tile() {};
		Tile( const std::string& blueprintName, const TileCoords2D& tileCoords );
};


#endif