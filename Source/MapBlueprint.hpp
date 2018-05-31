#ifndef MAP_BLUEPRINT_H
#define MAP_BLUEPRINT_H
#include "SoulStoneEngine/Utilities/GameCommon.hpp"
#include "SoulStoneEngine/Utilities/XMLLoaderUtilities.hpp"

enum MapType { INVALID_MAP = -1, WORLD_MAP, SUB_MAP };

struct EncounterInfo
{
	std::string		m_name;
	Vector2i		m_minMaxNum;
	int				m_minDistanceToPlayer;
};

class MapBlueprint
{
	public:
		static std::map< std::string, MapBlueprint* >		s_registeredMapBlueprint;
		std::string											m_name;
		MapType												m_type;
		Vector2i											m_minMaxNumRoom;
		Vector2i											m_roomMinMaxWidth;
		Vector2i											m_roomMinMaxHeight;
		std::vector<EncounterInfo*>							m_encounterInfoList;
		int													m_mapWidth;
		int													m_mapHeight;
		bool												m_isWorldMap;
		std::string											m_backgroundMusic;
		
	public:
		static void LoadBlueprint( const std::string& filePath );

	private:
		MapBlueprint() {};
};

#endif