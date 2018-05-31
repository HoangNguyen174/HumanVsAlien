#ifndef ITEM_BLUEPRINT_H
#define ITEM_BLUEPRINT_H
#include "SoulStoneEngine/Utilities/XMLLoaderUtilities.hpp"
#include "SoulStoneEngine/Utilities/GameCommon.hpp"

enum EquipSlot { MAIN_HAND = 0, OFF_HAND, BODY, HEAD, HEALTH, ENERGY, UNEQUIPABLE = 6 };
enum ItemType { RANGE_WEAPON = 0, MELEE_WEAPON, ARMOR, HEALTH_PORTION, ENERGY_PORTION = 4 };

class ItemBlueprint
{
	public:
		static std::map<std::string, ItemBlueprint*>  s_registeredItemBlueprint;
		std::string									  m_name;
		std::string									  m_abb;
		ItemType									  m_type;
		EquipSlot									  m_equipSlot;
		std::string									  m_textureSheet;
		TileCoords2D								  m_textureTileCoords;
		float										  m_damage;
		float										  m_attackRate;
		float										  m_attackRange;
		float										  m_toughness;
		float										  m_recoverAmount;
		float										  m_radius;
		bool										  m_isEquipable;

	public:
		static void LoadBlueprint( const std::string& filePath );

	private:
		ItemBlueprint() {};
		static ItemType GetItemType( const std::string& type );
		static EquipSlot GetEquipSlot( const std::string& slot );
};

#endif