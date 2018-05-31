#include "AIController.hpp"
#include "SoulStoneEngine/JobSystem/MemoryPoolManager.hpp"

AIController::AIController( Map* map )
{
	m_currentMap = map;
	InitializeAIStrategy();
}

void AIController::Update()
{

}


void AIController::InitializeAIStrategy()
{
	for ( unsigned int i = 0; i < m_currentMap->m_actorList.size(); i++ )
	{
		if( m_currentMap->m_actorList[i]->m_actorBlueprint->m_isPlayer != true )
		{
			m_AIList.push_back( m_currentMap->m_actorList[i] );
			m_AIList.back()->m_goalWorldPosition = m_AIList.back()->m_worldPosition;
			Actor* actor = m_currentMap->m_actorList[i];
			if( actor->m_initStrategy.compare( "Inactive") == 0 )
			{
				InactiveStrategy* inactiveStrategy = new InactiveStrategy( actor, m_currentMap );
				actor->m_strategy = inactiveStrategy;
			}
			else if( actor->m_initStrategy.compare( "Wandering") == 0 || actor->m_initStrategy.compare( "Patrol") == 0 )
			{
				AttentiveStrategy* attentiveStrategy = new AttentiveStrategy( actor, m_currentMap );
				actor->m_strategy = attentiveStrategy;
			}
			actor->m_initStrategy = "None";
		}
	}

}
