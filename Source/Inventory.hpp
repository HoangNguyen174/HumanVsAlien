#ifndef INVENTORY_H
#define INVENTORY_H
#include "Item.hpp"
#include "SoulStoneEngine/Utilities/GameCommon.hpp"

class Inventory
{
	public:
		std::vector<Item*>		m_itemList;

	public:
		int	GetIndexFirstItemInEquipSlot( EquipSlot slot );
		int	GetIndexNextItemInEquipSlot( Item* item );
		int GetIndexPrevItemInEquipSlot( Item* item );
		int GetNumberOfHealthPortion();
		int GetNumberOfEnergyPortion();
		void DeleteItem( Item* item );
};

#endif