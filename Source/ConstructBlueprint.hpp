#ifndef CONSTRUCT_BLUEPRINT_H
#define CONSTRUCT_BLUEPRINT_H
#include "SoulStoneEngine/Utilities/XMLLoaderUtilities.hpp"
#include "SoulStoneEngine/Utilities/GameCommon.hpp"

struct AbilityInfo
{
	std::string name;
	float		effectiveRadius;
};

class ConstructBlueprint
{
	public:
		static std::map<std::string, ConstructBlueprint* >	s_registeredConstructBlueprint;

	public:
		std::string					m_name;
		std::string					m_textureSheet;
		TileCoords2D				m_textureTileCoords;
		float						m_radius;
		bool						m_isOverlappable;
		std::vector<AbilityInfo*>	m_abilityList;
	
	public:
		static void LoadBlueprint( const std::string& filePath );

	private:
		ConstructBlueprint() {};
};

#endif