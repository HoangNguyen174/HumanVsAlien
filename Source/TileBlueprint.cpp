#include "TileBlueprint.hpp"
#include "SoulStoneEngine/JobSystem/MemoryPoolManager.hpp"

std::map< std::string, TileBlueprint* > TileBlueprint::s_registeredTileBlueprint;

void TileBlueprint::LoadBlueprint(const std::string& filePath)
{
	XMLLoaderUtilities tileXML( filePath );

	tinyxml2::XMLElement* rootELement = tileXML.m_root;

	tileXML.ValidateXMLChildElements( rootELement, "TileDef", "" );
	tileXML.ValidateXMLAttributes( rootELement, "", "" );

	for(tinyxml2::XMLElement* tileDefElement = rootELement->FirstChildElement(); tileDefElement != nullptr; tileDefElement = tileDefElement->NextSiblingElement())
	{
		TileBlueprint* temp = new TileBlueprint();
		tileXML.ValidateXMLChildElements( tileDefElement, "", "" );
		tileXML.ValidateXMLAttributes( tileDefElement, "name,abb,isSolid,textureSheet,textureTileCoords,gCost", "isOverlappable");
		temp->m_name = XMLLoaderUtilities::GetStringXMLAttribute( tileDefElement, "name", "grass" );
		temp->m_isSolid = XMLLoaderUtilities::GetBoolAttribute( tileDefElement, "isSolid", true );
		temp->m_textureSheet = XMLLoaderUtilities::GetStringXMLAttribute( tileDefElement, "textureSheet", "Tile.png" );
		temp->m_textureTileCoords = XMLLoaderUtilities::GetVector2iXMLAttribute( tileDefElement, "textureTileCoords", Vector2i(0,0) );
		temp->m_gCost = XMLLoaderUtilities::GetFloatXMLAttribute( tileDefElement, "gCost", GCOST_SOLID_TILE );
		temp->m_abb = XMLLoaderUtilities::GetStringXMLAttribute( tileDefElement, "Abb", "W" );

		if( s_registeredTileBlueprint.find( temp->m_name ) == s_registeredTileBlueprint.end() )
		{
			s_registeredTileBlueprint[ temp->m_name ] = temp;
		}
		else
		{
			DebuggerPrintf( "Tile with name %s already exist! Double check the %s.\n", temp->m_name.c_str(), filePath.c_str() );
			exit(0);
		}
	}
}