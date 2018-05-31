#include "Room.hpp"
#include "SoulStoneEngine/JobSystem/MemoryPoolManager.hpp"

Room::Room(Vector2i minMaxWidth, Vector2i minMaxHeight)
{
	m_width = MathUtilities::GetRandomNumber( minMaxWidth.x, minMaxWidth.y );
	m_height = MathUtilities::GetRandomNumber( minMaxHeight.x, minMaxHeight.y );
}


