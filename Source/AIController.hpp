#ifndef AI_CONTROLLER_H
#define AI_CONTROLLER_H
#include "SoulStoneEngine/Utilities/GameCommon.hpp"
#include "Actor.hpp"
#include "Map.hpp"
#include "AIStrategy.hpp"

const float MAX_TIME_TO_REACH_GOAL_SEC = 900000.f;

class AIController
{
	public:
		std::vector<Actor*>		m_AIList;
		Map*					m_currentMap;

	public:
		AIController( Map* map );
		void Update();
		void InitializeAIStrategy();
		void SetStateForActor( Actor* actor );
};

#endif