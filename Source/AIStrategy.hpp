#ifndef AI_STRATEGY_H
#define AI_STRATEGY_H
#include "Actor.hpp"
#include "Map.hpp"

class AIStrategy
{
	public:
		Actor* m_actor;
		Map* m_map;

	public:
		AIStrategy();
		AIStrategy( Actor* actor, Map* map );
		virtual void Update ( float elapsedTime ) = 0;
};

class InactiveStrategy : public AIStrategy
{
	public:
		InactiveStrategy( Actor* actor, Map* map ) : AIStrategy( actor, map ) {}
		void Update( float elapsedTime );
};

class IdleStrategy : public AIStrategy
{
	public:
		IdleStrategy( Actor* actor, Map* map ) : AIStrategy( actor, map ) {}
		void Update( float elapsedTime );
};

class HuntingStrategy : public AIStrategy
{
	public:
		HuntingStrategy( Actor* actor, Map* map ) : AIStrategy( actor, map ) {}
		void Update( float elapsedTime );
};

class FleeStrategy : public AIStrategy
{
	public:
		FleeStrategy( Actor* actor, Map* map ) : AIStrategy( actor, map ) {}
		void Update( float elapsedTime );
};

// patrol, guarding, wandering
class AttentiveStrategy : public AIStrategy
{
	public:
		float				m_radius;
		WorldCoords2D		m_centerPoint; //for patrol;

	public:
		AttentiveStrategy( Actor* actor, Map* map );
		void Update( float elapsedTime );

	private:
		void SetRandomGoalWithinRadius();
		void SetRandomGoalAroundCenterWithinRadius();
};

#endif