#ifndef TILE_BLUEPRINT_H
#define TILE_BLUEPRINT_H
#include "SoulStoneEngine/Utilities/XMLLoaderUtilities.hpp"
#include "SoulStoneEngine/Utilities/GameCommon.hpp"

const float GCOST_SOLID_TILE = 9999.f;

class TileBlueprint
{
	public:
		static std::map<std::string, TileBlueprint*> s_registeredTileBlueprint;
		std::string									  m_name;
		std::string									  m_textureSheet;
		std::string									  m_abb;
		TileCoords2D								  m_textureTileCoords;
		float										  m_gCost;
		bool										  m_isSolid;

	public:
		static void LoadBlueprint( const std::string& filePath );

	private:
		TileBlueprint() {};
		
};

#endif