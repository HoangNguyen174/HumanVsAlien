#ifndef SPACE_SHIP_MAP
#define SPACE_SHIP_MAP
#include "SoulStoneEngine/Utilities/GameCommon.hpp"
#include "Tile.hpp"

class Room
{
	public:
		int					m_width;
		int					m_height;
		int					m_roomArea;
		WorldCoords2D		m_roomCenter;
		TileCoords2D		m_roomTileCoordsMins;
		TileCoords2D		m_roomTileCoordsMaxs;

	public:
		Room( Vector2i minMaxWidth, Vector2i minMaxHeight );
		Room() {};
};


#endif