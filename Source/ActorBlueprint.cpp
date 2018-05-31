#include "ActorBlueprint.hpp"
#include "SoulStoneEngine/JobSystem/MemoryPoolManager.hpp"

std::map< std::string, ActorBlueprint* > ActorBlueprint::s_registeredActorBlueprint;

void ActorBlueprint::LoadBlueprint( const std::string& filePath )
{
	XMLLoaderUtilities actorXML( filePath );

	tinyxml2::XMLElement* rootElement = actorXML.m_root;
	actorXML.ValidateXMLChildElements( rootElement, "ActorDef", "" );
	actorXML.ValidateXMLAttributes( rootElement, "", "" );

	for(tinyxml2::XMLElement* actorDefElement = rootElement->FirstChildElement(); actorDefElement != nullptr; actorDefElement = actorDefElement->NextSiblingElement())
	{
		ActorBlueprint* tempActorblueprint = new ActorBlueprint();
		actorXML.ValidateXMLChildElements( actorDefElement, "ActorMainStat,ActorSecondaryStat,ActorProperty,DefaultStrategy", "DropList,AbilityList,Sound" );
		actorXML.ValidateXMLAttributes( actorDefElement, "name,textureSheet,textureTileCoords", "isPlayer" );
		tempActorblueprint->m_parent = nullptr;
		tempActorblueprint->m_name = XMLLoaderUtilities::GetStringXMLAttribute( actorDefElement, "name", "anonymous" ); 
		tempActorblueprint->m_textureSheet = XMLLoaderUtilities::GetStringXMLAttribute( actorDefElement, "textureSheet", "Actor.png" );
		tempActorblueprint->m_textureTileCoords = XMLLoaderUtilities::GetVector2iXMLAttribute( actorDefElement, "textureTileCoords", Vector2i(0,0) );
		tempActorblueprint->m_isPlayer = XMLLoaderUtilities::GetBoolAttribute( actorDefElement, "isPlayer", false );

		for(tinyxml2::XMLElement* actorStatElement = actorDefElement->FirstChildElement("ActorMainStat"); actorStatElement != nullptr; actorStatElement = actorStatElement->NextSiblingElement("ActorMainStat") )
		{
			actorXML.ValidateXMLChildElements( actorStatElement, "", "" );
			actorXML.ValidateXMLAttributes( actorStatElement, "health,str,int,dex", "");
			tempActorblueprint->m_health = XMLLoaderUtilities::GetFloatXMLAttribute( actorStatElement, "health", 1.f );
			tempActorblueprint->m_strength = XMLLoaderUtilities::GetFloatXMLAttribute( actorStatElement, "str", 1.f );
			tempActorblueprint->m_intelligent = XMLLoaderUtilities::GetFloatXMLAttribute( actorStatElement, "int", 1.f );
			tempActorblueprint->m_dexterity = XMLLoaderUtilities::GetFloatXMLAttribute( actorStatElement, "dex", 1.f );

			tinyxml2::XMLElement* actorSecondaryStatElement = actorStatElement->NextSiblingElement("ActorSecondaryStat");

			actorXML.ValidateXMLChildElements( actorSecondaryStatElement, "", "" );
			actorXML.ValidateXMLAttributes( actorSecondaryStatElement, "toughness,attackRate,attackRange,speed,awareRadius", "" );
			tempActorblueprint->m_toughness = XMLLoaderUtilities::GetFloatXMLAttribute( actorSecondaryStatElement, "toughness", 1.f );
			tempActorblueprint->m_attackRate = XMLLoaderUtilities::GetFloatXMLAttribute( actorSecondaryStatElement, "attackRate", 1.f );
			tempActorblueprint->m_attackRange = XMLLoaderUtilities::GetFloatXMLAttribute( actorSecondaryStatElement, "attackRange", 1.f );
			tempActorblueprint->m_speed = XMLLoaderUtilities::GetFloatXMLAttribute( actorSecondaryStatElement, "speed", 1.f );
			tempActorblueprint->m_awareRadius = XMLLoaderUtilities::GetFloatXMLAttribute( actorSecondaryStatElement, "awareRadius", 0.f );

			tinyxml2::XMLElement* actorPropertyElement = actorStatElement->NextSiblingElement("ActorProperty");

			actorXML.ValidateXMLChildElements( actorPropertyElement, "", "" );
			actorXML.ValidateXMLAttributes( actorPropertyElement, "faction,size,visionRange,braveness", "exp" );
			tempActorblueprint->m_faction = XMLLoaderUtilities::GetStringXMLAttribute( actorPropertyElement, "faction", "No faction" );
			tempActorblueprint->m_radius = XMLLoaderUtilities::GetFloatXMLAttribute( actorPropertyElement, "size", 0.1f );
			tempActorblueprint->m_visionRange = XMLLoaderUtilities::GetFloatXMLAttribute( actorPropertyElement, "visionRange", 0.f );
			tempActorblueprint->m_expPoint = XMLLoaderUtilities::GetIntXMLAttribute( actorPropertyElement, "exp", 0 );
			tempActorblueprint->m_braveness = XMLLoaderUtilities::GetVector2XMLAttribute( actorPropertyElement, "braveness", Vector2(1.0,1.0) );

			tinyxml2::XMLElement* initStrategyElement = actorStatElement->NextSiblingElement("DefaultStrategy");
			actorXML.ValidateXMLChildElements( initStrategyElement, "", "" );
			actorXML.ValidateXMLAttributes( initStrategyElement, "strategy", "radius,protectTarget" );

			tempActorblueprint->m_initStrategy = XMLLoaderUtilities::GetStringXMLAttribute( initStrategyElement, "strategy", "Inactive" );
			
			if( tempActorblueprint->m_initStrategy.compare( "Wandering" ) == 0 || tempActorblueprint->m_initStrategy.compare( "Patrol") == 0  )
			{
				tempActorblueprint->m_wanderRadius = XMLLoaderUtilities::GetFloatXMLAttribute( initStrategyElement, "radius", 0.f );
			}
			else if( tempActorblueprint->m_initStrategy.compare( "Protecting" ) == 0 )
			{
				tempActorblueprint->m_protectTarget = XMLLoaderUtilities::GetStringXMLAttribute( initStrategyElement, "protectTarget", "NoTarget" );
				if( tempActorblueprint->m_protectTarget.compare( "NoTarget") == 0 )
				{
					DebuggerPrintf( "Protecting behavior need to have target.\n" );
					exit(0);
				}
			 } 

			tinyxml2::XMLElement* dropListElement = actorStatElement->NextSiblingElement("DropList");

			if( dropListElement != nullptr )
			{
				actorXML.ValidateXMLChildElements( dropListElement, "Item", "" );
				actorXML.ValidateXMLAttributes( dropListElement, "", "");
				for( tinyxml2::XMLElement* itemElement = dropListElement->FirstChildElement("Item"); itemElement != nullptr; itemElement = itemElement->NextSiblingElement("Item") )
				{
					actorXML.ValidateXMLChildElements( itemElement, "", "" );
					actorXML.ValidateXMLAttributes( itemElement, "name,percent", "");
					DropItem* item = new DropItem;
					item->name = actorXML.GetStringXMLAttribute( itemElement, "name", "" );
					item->dropRate = actorXML.GetFloatXMLAttribute( itemElement, "percent", 0.f );
					tempActorblueprint->m_dropList.push_back( item );
				}
			}

			tinyxml2::XMLElement* abilityListElement = actorStatElement->NextSiblingElement("AbilityList");
			if( abilityListElement != nullptr )
			{
				actorXML.ValidateXMLChildElements( abilityListElement, "Ability", "" );
				actorXML.ValidateXMLAttributes( abilityListElement, "", "");

				for(tinyxml2::XMLElement* abilityElement = abilityListElement->FirstChildElement("Ability"); abilityElement != nullptr; abilityElement = abilityElement->NextSiblingElement("Ability"))
				{
					actorXML.ValidateXMLChildElements( abilityElement, "", "" );
					actorXML.ValidateXMLAttributes( abilityElement, "name", "");
					std::string name = actorXML.GetStringXMLAttribute( abilityElement, "name", "None" );
					if( name.compare( "None" ) != 0 )
					{
						tempActorblueprint->m_abilityList.push_back( name );
					}
				}
			}

			tinyxml2::XMLElement* soundElement = actorStatElement->NextSiblingElement( "Sound" );
			actorXML.ValidateXMLChildElements( soundElement, "", "" );
			actorXML.ValidateXMLAttributes( soundElement, "", "getHit,lvUp,death,attack" );
			tempActorblueprint->m_hurtSoundName = actorXML.GetStringXMLAttribute( soundElement, "getHit", "NoSound" );
			tempActorblueprint->m_lvUpSoundName = actorXML.GetStringXMLAttribute( soundElement, "lvUp", "NoSound" );
			tempActorblueprint->m_deathSoundName = actorXML.GetStringXMLAttribute( soundElement, "death", "NoSound" );
			tempActorblueprint->m_attackSoundName = actorXML.GetStringXMLAttribute( soundElement, "attack", "NoSound" );

			if( s_registeredActorBlueprint.find( tempActorblueprint->m_name ) == s_registeredActorBlueprint.end() )
			{
				s_registeredActorBlueprint[ tempActorblueprint->m_name ] = tempActorblueprint;
			}
			else
			{
				DebuggerPrintf( "Actor with name %s already exist! Double check the %s.\n", tempActorblueprint->m_name.c_str(), filePath.c_str() );
				exit(0);
			}
		}
	}
}


