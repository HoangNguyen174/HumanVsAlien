#include "AbilityBlueprint.hpp"
#include "SoulStoneEngine/JobSystem/MemoryPoolManager.hpp"

std::map< std::string, AbilityBlueprint* > AbilityBlueprint::s_registeredAbilityBlueprint;

void AbilityBlueprint::LoadBlueprint( const std::string& filePath )
{
	XMLLoaderUtilities abilityXML( filePath );

	tinyxml2::XMLElement* rootElement = abilityXML.m_root;
	abilityXML.ValidateXMLChildElements( rootElement, "AbilityDef", "" );
	abilityXML.ValidateXMLAttributes( rootElement, "", "" );

	for(tinyxml2::XMLElement* abilityDefElement = rootElement->FirstChildElement();abilityDefElement != nullptr; abilityDefElement = abilityDefElement->NextSiblingElement())
	{
		AbilityBlueprint* tempAbilityBp = new AbilityBlueprint();
		abilityXML.ValidateXMLChildElements( abilityDefElement, "AbilityProperty,AbilityEffect,SoundEffect", "" );
		abilityXML.ValidateXMLAttributes( abilityDefElement, "name,textureSheet,textureTileCoords", "" );
		tempAbilityBp->m_name = XMLLoaderUtilities::GetStringXMLAttribute( abilityDefElement , "name", "" );
		tempAbilityBp->m_textureSheet = XMLLoaderUtilities::GetStringXMLAttribute( abilityDefElement, "textureSheet", "ActorSheet.png" );
		tempAbilityBp->m_textureTileCoords = XMLLoaderUtilities::GetVector2iXMLAttribute( abilityDefElement, "textureTileCoords", Vector2i( 0, 0 ) );

		tinyxml2::XMLElement* abilityPropertyElement = abilityDefElement->FirstChildElement("AbilityProperty");
		abilityXML.ValidateXMLChildElements( abilityPropertyElement, "", "" );
		abilityXML.ValidateXMLAttributes( abilityPropertyElement, "", "damageTarget,radius,ignoreDifferentFaction,ignoreSameFaction,duration,addFlag,removeFlag,isTargetSpell,cd,energyCost" );
		tempAbilityBp->m_damageTarget = XMLLoaderUtilities::GetFloatXMLAttribute( abilityPropertyElement, "damageTarget", 0.f );
		tempAbilityBp->m_radius =  XMLLoaderUtilities::GetFloatXMLAttribute( abilityPropertyElement, "radius", 0.f );
		tempAbilityBp->m_duration = XMLLoaderUtilities::GetFloatXMLAttribute( abilityPropertyElement, "duration", 0.f );
		std::string flag = XMLLoaderUtilities::GetStringXMLAttribute( abilityPropertyElement, "addFlag", "" );
		tempAbilityBp->m_addFlag = GetFlag( flag );
		flag = XMLLoaderUtilities::GetStringXMLAttribute( abilityPropertyElement, "removeFlag", "" );
		tempAbilityBp->m_removeFlag = GetFlag( flag );
		tempAbilityBp->m_ignoreDifferentFaction = XMLLoaderUtilities::GetBoolAttribute( abilityPropertyElement, "ignoreDifferentFaction", "true" );
		tempAbilityBp->m_ignoreSameFaction = XMLLoaderUtilities::GetBoolAttribute( abilityPropertyElement, "ignoreSameFaction", "true" );
		tempAbilityBp->m_isTargetSpell = XMLLoaderUtilities::GetBoolAttribute( abilityPropertyElement, "isTargetSpell", "true" );
		tempAbilityBp->m_coolDown = XMLLoaderUtilities::GetFloatXMLAttribute( abilityPropertyElement, "cd", 0.f );
		tempAbilityBp->m_eneryCost = XMLLoaderUtilities::GetFloatXMLAttribute( abilityPropertyElement, "energyCost", 0.f );

		tinyxml2::XMLElement* abilityEffectElement = abilityDefElement->FirstChildElement("AbilityEffect");
		abilityXML.ValidateXMLChildElements( abilityEffectElement, "", "" );
		abilityXML.ValidateXMLAttributes( abilityEffectElement, "textureSheet,frameWidth,frameHeight,texWidth,texHeight,radius", "" );
		tempAbilityBp->m_effectTexture = XMLLoaderUtilities::GetStringXMLAttribute( abilityEffectElement, "textureSheet", "Invalid" );
		tempAbilityBp->m_effectTexHeight = XMLLoaderUtilities::GetIntXMLAttribute( abilityEffectElement, "texHeight", 0 );
		tempAbilityBp->m_effectTexWidth = XMLLoaderUtilities::GetIntXMLAttribute( abilityEffectElement, "texWidth", 0 );
		tempAbilityBp->m_effectFrameWidth = XMLLoaderUtilities::GetIntXMLAttribute( abilityEffectElement, "frameWidth", 0 );
		tempAbilityBp->m_effectFrameHeight = XMLLoaderUtilities::GetIntXMLAttribute( abilityEffectElement, "frameHeight", 0 );
		tempAbilityBp->m_drawEffectRadius = XMLLoaderUtilities::GetFloatXMLAttribute( abilityEffectElement, "radius", 0.f );

		tinyxml2::XMLElement* soundEffectElement = abilityDefElement->FirstChildElement("SoundEffect");
		abilityXML.ValidateXMLChildElements( soundEffectElement, "", "" );
		abilityXML.ValidateXMLAttributes( soundEffectElement, "name", "" );
		tempAbilityBp->m_soundName = XMLLoaderUtilities::GetStringXMLAttribute( soundEffectElement, "name", "NoSound" );

		if( s_registeredAbilityBlueprint.find( tempAbilityBp->m_name ) == s_registeredAbilityBlueprint.end() )
		{
			s_registeredAbilityBlueprint[ tempAbilityBp->m_name ] = tempAbilityBp;
		}
		else
		{
			DebuggerPrintf( "Ability with name %s already exist! Double check the %s.\n", tempAbilityBp->m_name.c_str(), filePath.c_str() );
			exit(0);
		}
	}
}

Flag AbilityBlueprint::GetFlag( const std::string& flag )
{
	if( flag.compare( "slow" ) )
		return SLOW;
	else if( flag.compare( "confusion" ) )
		return CONFUSION;
	else if( flag.compare( "burn" ) )
		return BURN;
	else if( flag.compare( "poison" ) )
		return POISON;
	else
		return NORMAL;
}
