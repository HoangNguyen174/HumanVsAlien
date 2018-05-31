#include "ConstructBlueprint.hpp"
#include "SoulStoneEngine/JobSystem/MemoryPoolManager.hpp"

std::map< std::string, ConstructBlueprint* > ConstructBlueprint::s_registeredConstructBlueprint;

void ConstructBlueprint::LoadBlueprint( const std::string& filePath )
{
	XMLLoaderUtilities constructXML( filePath );

	tinyxml2::XMLElement* rootElement = constructXML.m_root;
	constructXML.ValidateXMLChildElements( rootElement, "ConstructDef", "" );
	constructXML.ValidateXMLAttributes( rootElement, "", "" );

	for(tinyxml2::XMLElement* constructDefElement = rootElement->FirstChildElement();constructDefElement != nullptr; constructDefElement = constructDefElement->NextSiblingElement())
	{
		ConstructBlueprint* tempConstructBp = new ConstructBlueprint();
		constructXML.ValidateXMLChildElements( constructDefElement, "Ability", "" );
		constructXML.ValidateXMLAttributes( constructDefElement, "name,textureSheet,textureTileCoords,size,overlappable", "" );
		tempConstructBp->m_name = XMLLoaderUtilities::GetStringXMLAttribute( constructDefElement , "name", "" ); 
		tempConstructBp->m_textureSheet = XMLLoaderUtilities::GetStringXMLAttribute( constructDefElement , "textureSheet", "Building.png" );
		tempConstructBp->m_textureTileCoords = XMLLoaderUtilities::GetVector2iXMLAttribute( constructDefElement , "textureTileCoords", Vector2i(0,0) );
		tempConstructBp->m_radius = XMLLoaderUtilities::GetFloatXMLAttribute( constructDefElement, "size", 0.f );
		tempConstructBp->m_isOverlappable = XMLLoaderUtilities::GetBoolAttribute( constructDefElement, "overlappable", true );

		for(tinyxml2::XMLElement* abilityElement = constructDefElement->FirstChildElement("Ability"); abilityElement != nullptr; abilityElement = abilityElement->NextSiblingElement("Ability") )
		{
			AbilityInfo* ability = new AbilityInfo;
			constructXML.ValidateXMLChildElements( abilityElement, "", "" );
			constructXML.ValidateXMLAttributes( abilityElement, "name,radius", "");
			ability->name = XMLLoaderUtilities::GetStringXMLAttribute( abilityElement, "name", "" );
			ability->effectiveRadius = XMLLoaderUtilities::GetFloatXMLAttribute( abilityElement, "radius", 0.f );
			tempConstructBp->m_abilityList.push_back(ability);
		}

		if( s_registeredConstructBlueprint.find( tempConstructBp->m_name ) == s_registeredConstructBlueprint.end() )
		{
			s_registeredConstructBlueprint[ tempConstructBp->m_name ] = tempConstructBp;
		}
		else
		{
			DebuggerPrintf( "Construct with name %s already exist! Double check the %s.\n", tempConstructBp->m_name.c_str(), filePath.c_str() );
			exit(0);
		}
	}
}

