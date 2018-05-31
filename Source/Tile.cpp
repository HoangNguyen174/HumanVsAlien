#include "Tile.hpp"
#include "SoulStoneEngine/JobSystem/MemoryPoolManager.hpp"

Tile::Tile( const std::string& blueprintName, const TileCoords2D& tileCoords )
{
	m_isVisibleNow = false;
	m_isVisibleLastTime = false;
	std::map<std::string, TileBlueprint*>::iterator blueprintIter;
	blueprintIter = TileBlueprint::s_registeredTileBlueprint.find( blueprintName );
	if( blueprintIter != TileBlueprint::s_registeredTileBlueprint.end() )
	{
		TileBlueprint* blueprint = blueprintIter->second;
		m_tileBlueprint = blueprint;
		m_tileCoords = tileCoords;
	}
}
