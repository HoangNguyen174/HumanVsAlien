#include "Inventory.hpp"
#include "SoulStoneEngine/JobSystem/MemoryPoolManager.hpp"

int Inventory::GetIndexFirstItemInEquipSlot( EquipSlot slot )
{
	for( unsigned int i = 0; i < m_itemList.size(); i++ )
	{
		if( m_itemList[i]->m_itemBlueprint->m_equipSlot == slot )
		{
			return i;
		}
	}
	return -1;
}

int Inventory::GetIndexNextItemInEquipSlot( Item* item )
{
	if( item == nullptr )
		return -1;

	int indexOfItem = 0;

	for( unsigned int i = 0; i < m_itemList.size(); i++ )
	{
		Item* itemInList = m_itemList[i];
		if( itemInList->m_itemID == item->m_itemID )
		{
			indexOfItem = i;
		}
	}

	EquipSlot slot = item->m_itemBlueprint->m_equipSlot;

	for( unsigned int i = 0; i < m_itemList.size(); i++ )
	{
		if( m_itemList[i]->m_itemBlueprint->m_equipSlot == slot && i > indexOfItem )
		{
			return i;
		}
	}
	return GetIndexFirstItemInEquipSlot( item->m_itemBlueprint->m_equipSlot );
}

int Inventory::GetIndexPrevItemInEquipSlot(Item* item)
{
	if( item == nullptr )
		return -1;

	int indexOfItem;

	for( unsigned int i = 0; i < m_itemList.size(); i++ )
	{
		Item* itemInList = m_itemList[i];
		if( itemInList->m_itemID == item->m_itemID )
		{
			indexOfItem = i;
		}
	}

	EquipSlot slot = item->m_itemBlueprint->m_equipSlot;

	for( unsigned int i = 0; i < m_itemList.size(); i++ )
	{
		if( m_itemList[i]->m_itemBlueprint->m_equipSlot == slot && i < indexOfItem )
		{
			return i;
		}
	}
	return GetIndexFirstItemInEquipSlot( item->m_itemBlueprint->m_equipSlot );
}

int Inventory::GetNumberOfHealthPortion()
{
	int count = 0;
	for( unsigned int i = 0; i < m_itemList.size(); i++ )
	{
		if( m_itemList[i]->m_itemBlueprint->m_equipSlot == HEALTH )
			count++;
	}
	return count;
}

int Inventory::GetNumberOfEnergyPortion()
{
	int count = 0;
	for( unsigned int i = 0; i < m_itemList.size(); i++ )
	{
		if( m_itemList[i]->m_itemBlueprint->m_equipSlot == ENERGY )
			count++;
	}
	return count;
}

void Inventory::DeleteItem(Item* item)
{
	for( unsigned int i = 0; i < m_itemList.size(); i++ )
	{
		if( m_itemList[i]->m_itemID == item->m_itemID )
			m_itemList.erase( m_itemList.begin() + i );
	}
}

