#include "MapBlueprint.hpp"
#include "SoulStoneEngine/JobSystem/MemoryPoolManager.hpp"

std::map< std::string, MapBlueprint*> MapBlueprint::s_registeredMapBlueprint;

void MapBlueprint::LoadBlueprint(const std::string& filePath)
{
	XMLLoaderUtilities mapXML( filePath );

	tinyxml2::XMLElement* rootElement = mapXML.m_root;
	mapXML.ValidateXMLChildElements( rootElement, "MapDef", "" );
	mapXML.ValidateXMLAttributes( rootElement, "", "" );

	for(tinyxml2::XMLElement* mapDefElement = rootElement->FirstChildElement(); mapDefElement != nullptr; mapDefElement = mapDefElement->NextSiblingElement())
	{
		MapBlueprint* tempMapBp = new MapBlueprint();
		mapXML.ValidateXMLChildElements( mapDefElement, "", "Room,Encounter,BackgroundMusic" );
		mapXML.ValidateXMLAttributes( mapDefElement, "name,type,width,height", "" );
		tempMapBp->m_name = mapXML.GetStringXMLAttribute( mapDefElement, "name", "" );
		std::string mapTypeStr = mapXML.GetStringXMLAttribute( mapDefElement, "type", "INVALID" );
		if( mapTypeStr.compare("worldMap") == 0 )
		{
			tempMapBp->m_type = WORLD_MAP;
			tempMapBp->m_isWorldMap = true;
		}
		else if( mapTypeStr.compare("subMap") == 0 )
		{
			tempMapBp->m_type = SUB_MAP;
			tempMapBp->m_isWorldMap = false;
		}
		else
		{
			DebuggerPrintf( "Invalid Map Type. Terminate Program.\n" );
			exit(0);
		}
		
		tempMapBp->m_mapWidth = mapXML.GetIntXMLAttribute( mapDefElement, "width", 32 );
		tempMapBp->m_mapHeight = mapXML.GetIntXMLAttribute( mapDefElement, "height", 32 );
	
		tinyxml2::XMLElement* roomElement = mapDefElement->FirstChildElement( "Room" );
		
		mapXML.ValidateXMLChildElements( roomElement, "", "" );
		mapXML.ValidateXMLAttributes( roomElement, "minMaxWidth,minMaxHeight,num", "" );
		tempMapBp->m_roomMinMaxWidth = mapXML.GetVector2iXMLAttribute( roomElement, "minMaxWidth", Vector2i( 0,0 ) );
		tempMapBp->m_roomMinMaxHeight = mapXML.GetVector2iXMLAttribute( roomElement, "minMaxHeight", Vector2i( 0,0 ) );
		tempMapBp->m_minMaxNumRoom = mapXML.GetVector2iXMLAttribute( roomElement, "num", Vector2i( 1, 1) );
	
		for(tinyxml2::XMLElement* encounterElement = roomElement->NextSiblingElement("Encounter"); encounterElement != nullptr; encounterElement = encounterElement->NextSiblingElement("Encounter") )
		{
			mapXML.ValidateXMLChildElements( encounterElement, "", "" );
			mapXML.ValidateXMLAttributes( encounterElement, "name,num", "minDistanceToPlayer" );
			EncounterInfo* encounter = new EncounterInfo;
			encounter->m_name = mapXML.GetStringXMLAttribute( encounterElement, "name", "INVALID" );
			encounter->m_minMaxNum = mapXML.GetVector2iXMLAttribute( encounterElement, "num", Vector2i( 0, 0 ) );
			encounter->m_minDistanceToPlayer = mapXML.GetIntXMLAttribute( encounterElement, "minDistanceToPlayer", 2 );
			
			tempMapBp->m_encounterInfoList.push_back( encounter );
		}

		tinyxml2::XMLElement* bgMusicElement = roomElement->NextSiblingElement( "BackgroundMusic" );
		if( bgMusicElement != nullptr )
		{
			mapXML.ValidateXMLChildElements( bgMusicElement , "", "" );
			mapXML.ValidateXMLAttributes( bgMusicElement , "name", "" );
			tempMapBp->m_backgroundMusic = mapXML.GetStringXMLAttribute( bgMusicElement, "name", "NoMusic" );
		}
		else
			tempMapBp->m_backgroundMusic = "NoMusic";

		if( s_registeredMapBlueprint.find( tempMapBp->m_name ) == s_registeredMapBlueprint.end())
		{
			s_registeredMapBlueprint[ tempMapBp->m_name ] = tempMapBp;
		}
		else
		{
			DebuggerPrintf( "Map with name %s already exist! Double check the %s.\n", tempMapBp->m_name.c_str(), filePath.c_str() );
			exit(0);
		}
	}
}
