#include "ItemBlueprint.hpp"
#include "SoulStoneEngine/JobSystem/MemoryPoolManager.hpp"

std::map< std::string, ItemBlueprint* > ItemBlueprint::s_registeredItemBlueprint;

void ItemBlueprint::LoadBlueprint(const std::string& filePath )
{
	XMLLoaderUtilities itemXML( filePath );

	tinyxml2::XMLElement* rootElement = itemXML.m_root;

	itemXML.ValidateXMLChildElements( rootElement, "ItemDef", "" );
	itemXML.ValidateXMLChildElements( rootElement, "", "" );

	for( tinyxml2::XMLElement* itemDefElement = rootElement->FirstChildElement(); itemDefElement != nullptr; itemDefElement = itemDefElement->NextSiblingElement())
	{
		ItemBlueprint* tempItem = new ItemBlueprint();
		itemXML.ValidateXMLChildElements( itemDefElement, "ItemProperty", "" );
		itemXML.ValidateXMLAttributes( itemDefElement, "name,abb,type,textureSheet,textureTileCoords,radius","equipSlot" );
		tempItem->m_name = XMLLoaderUtilities::GetStringXMLAttribute( itemDefElement, "name", "" );
		tempItem->m_abb = XMLLoaderUtilities::GetStringXMLAttribute( itemDefElement, "abb", "" );
		std::string tempStr = XMLLoaderUtilities::GetStringXMLAttribute( itemDefElement, "type", "" );
		tempItem->m_type = GetItemType( tempStr );
		tempStr = XMLLoaderUtilities::GetStringXMLAttribute( itemDefElement, "equipSlot", "None" );
		tempItem->m_equipSlot = GetEquipSlot( tempStr );
		tempItem->m_textureSheet = XMLLoaderUtilities::GetStringXMLAttribute( itemDefElement, "textureSheet", "Actor.png" );
		tempItem->m_textureTileCoords = XMLLoaderUtilities::GetVector2iXMLAttribute( itemDefElement, "textureTileCoords", Vector2i(0,0) );
		tempItem->m_radius = XMLLoaderUtilities::GetFloatXMLAttribute( itemDefElement, "radius", 0.f );

		tinyxml2::XMLElement* itemPropertyElement = itemDefElement->FirstChildElement("ItemProperty");
		
		itemXML.ValidateXMLChildElements( itemPropertyElement, "", "" );
		itemXML.ValidateXMLAttributes( itemPropertyElement, "equipable", "damage,attackRate,attackRange,toughness,recoverAmount" );
		tempItem->m_damage = XMLLoaderUtilities::GetFloatXMLAttribute( itemPropertyElement, "damage", 0.f );
		tempItem->m_attackRate = XMLLoaderUtilities::GetFloatXMLAttribute( itemPropertyElement, "attackRate", 0.f );
		tempItem->m_attackRange = XMLLoaderUtilities::GetFloatXMLAttribute( itemPropertyElement, "attackRange", 0.f );
		tempItem->m_toughness = XMLLoaderUtilities::GetFloatXMLAttribute( itemPropertyElement, "toughness", 0.f );
		tempItem->m_recoverAmount = XMLLoaderUtilities::GetFloatXMLAttribute( itemPropertyElement, "recoverAmount", 0.f );
		tempItem->m_isEquipable = XMLLoaderUtilities::GetBoolAttribute( itemPropertyElement, "equipable", false );

		if( s_registeredItemBlueprint.find( tempItem->m_name ) == s_registeredItemBlueprint.end())
		{
			s_registeredItemBlueprint[ tempItem->m_name ] = tempItem;
		}
		else
		{
			DebuggerPrintf( "Actor with name %s already exist! Double check the %s.\n", tempItem->m_name.c_str(), filePath.c_str() );
			exit(0);
		}
	}
}

ItemType ItemBlueprint::GetItemType(const std::string& type)
{
	if( type.compare("RangeWeapon") == 0 )
		return RANGE_WEAPON;
	if( type.compare("MeleeWeapon") == 0 )
		return MELEE_WEAPON;
	if( type.compare("Armor") == 0 )
		return ARMOR;
	if( type.compare("HealthPortion") == 0 )
		return HEALTH_PORTION;
	if( type.compare("EnergyPortion") == 0 )
		return ENERGY_PORTION;
}

EquipSlot ItemBlueprint::GetEquipSlot(const std::string& slot)
{
	if( slot.compare("MainHand") == 0 )
		return MAIN_HAND;
	if( slot.compare("OffHand") == 0 )
		return OFF_HAND;
	if( slot.compare("Body") == 0 )
		return BODY;
	if( slot.compare("Head") == 0 )
		return HEAD;
	if( slot.compare("Health") == 0 )
		return HEALTH;
	if( slot.compare("Energy") == 0 )
		return ENERGY;
	if( slot.compare("None") == 0 )
		return UNEQUIPABLE;
	return UNEQUIPABLE;
}

