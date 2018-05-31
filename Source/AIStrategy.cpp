#include "AIStrategy.hpp"
#include "SoulStoneEngine/JobSystem/MemoryPoolManager.hpp"

AIStrategy::AIStrategy()
{
}

AIStrategy::AIStrategy( Actor* actor, Map* map )
{
	m_actor = actor;
	m_map = map;
}

void AIStrategy::Update( float  )
{
}

void InactiveStrategy::Update( float elapsedTime )
{
	m_actor->MoveToGoal(elapsedTime);
}

void IdleStrategy::Update( float elapsedTime )
{
	
}

void HuntingStrategy::Update( float elapsedTime )
{
	Actor* target = m_actor->m_target;
	TileCoords2D targetTileCoords = ConvertWorldCoords2DToTileCoords2D( target->m_worldPosition );
	TileIndex targetTileIndex = ConvertTileCoords2DToTileIndex( targetTileCoords, m_map->m_mapWidthX );
	bool isTartgetVisible = m_map->m_tileList[targetTileIndex].m_isVisibleNow;

	bool isTargetInRange = m_actor->IsTargetInAttackRange( m_actor->m_target );

	if( target != nullptr )
	{
		if( isTartgetVisible )
			m_actor->m_targetLastSeenPosition = target->m_worldPosition;

		//if outside of range -> pursue
		if( !isTargetInRange || !m_actor->IsShortTermGoalInLineOfSight( target->m_worldPosition) )
		{
			m_actor->m_goalWorldPosition = m_actor->m_targetLastSeenPosition;//target->m_worldPosition;
			if( m_actor->m_path.size() == 0 )
			{
				if( m_actor->m_actorBlueprint->m_isPlayer )
					m_map->CalculateAStarPath( m_actor->m_worldPosition, target->m_worldPosition, m_actor->m_path, false, nullptr );
				else
					m_map->CalculateAStarPath( m_actor->m_worldPosition, target->m_worldPosition, m_actor->m_path, true, nullptr );
			}
			m_map->AddActorToWaitingForPathList( m_actor );
			m_actor->MoveToGoal(elapsedTime);
		}
		else // attack if in attack range
		{
			//stop moving
			m_actor->m_path.clear();
			m_actor->m_goalWorldPosition = m_actor->m_worldPosition;

			bool canActorAttackTarget = m_actor->CanAttackTarget( m_actor->m_target );

			if( canActorAttackTarget )
				m_actor->Attack( m_actor->m_target );
		}
	}

	if( ( m_actor->m_target != nullptr && m_actor->m_target->m_health <= 0.0f ) || !isTartgetVisible )
	{
		m_actor->ChangeToNonOffensiveStrategy();
	}
}

void FleeStrategy::Update( float elapsedTime )
{
	if( m_actor->m_path.size() > 0 )
	{
		m_actor->MoveToGoal(elapsedTime);
		return;
	}

	TileIndex randomTile = MathUtilities::GetRandomNumber( 0, m_map->m_mapSize );

	WorldCoords2D worldCenter = Vector2( m_map->m_tileList[randomTile].m_tileCoords.x + 0.5f, 
										 m_map->m_tileList[randomTile].m_tileCoords.y + 0.5f );

	while( ( worldCenter - m_actor->m_worldPosition ).CalcLengthSquare() < 20.f &&
			 m_map->m_tileList[randomTile].m_tileBlueprint->m_isSolid )
	{
		randomTile = MathUtilities::GetRandomNumber( 0, m_map->m_mapSize );

		worldCenter = Vector2( m_map->m_tileList[randomTile].m_tileCoords.x + 0.5f, 
						       m_map->m_tileList[randomTile].m_tileCoords.y + 0.5f );
	}

	m_actor->m_goalWorldPosition = worldCenter;

	m_map->CalculateAStarPath( m_actor->m_worldPosition, m_actor->m_goalWorldPosition, m_actor->m_path, true, nullptr );
	m_map->AddActorToWaitingForPathList( m_actor );
}

AttentiveStrategy::AttentiveStrategy( Actor* actor, Map* map )
{
	m_actor = actor;
	m_map = map;
	m_radius = m_actor->m_wanderRadius;
	m_centerPoint = m_actor->m_worldPosition;
}

void AttentiveStrategy::Update( float elapsedTime )
{
	enum State { WANDERING, PATROL, PROTECT };

	State state = WANDERING;  
	if( m_actor->m_actorBlueprint->m_initStrategy.compare( "Wandering") == 0 )
	{
		state = WANDERING;
	}
	else if ( m_actor->m_actorBlueprint->m_initStrategy.compare( "Patrol") == 0 )
	{
		state = PATROL;
	}
	else if ( m_actor->m_actorBlueprint->m_initStrategy.compare( "PROTECT") == 0 )
	{
		state = PROTECT;
	}

	bool isStateChanged = false;
	if( m_actor->m_target == nullptr )
	{
		isStateChanged = m_actor->ChangeToHuntStrategyIfAnyTargetToAttack();
	}

	if( isStateChanged )
		return;

	switch( state )
	{
		case PATROL :
		{
			if( m_actor->m_goalWorldPosition == Vector2( 0.f,0.f ) )
			{
				SetRandomGoalAroundCenterWithinRadius();
				m_map->CalculateAStarPath( m_actor->m_worldPosition, m_actor->m_goalWorldPosition, m_actor->m_path, true, nullptr );
				m_map->AddActorToWaitingForPathList( m_actor );
			}

			float distCurrentToGoalSquare = ( m_actor->m_worldPosition - m_actor->m_goalWorldPosition ).CalcLengthSquare();

			if( distCurrentToGoalSquare <= ( m_actor->m_radius * m_actor->m_radius ) )
			{
				SetRandomGoalAroundCenterWithinRadius();
				m_map->CalculateAStarPath( m_actor->m_worldPosition, m_actor->m_goalWorldPosition, m_actor->m_path, true, nullptr );
				m_map->AddActorToWaitingForPathList( m_actor );
			}
			m_actor->MoveToGoal(elapsedTime);
			break;
		}
		case WANDERING :
		{	
 			if( m_actor->m_goalWorldPosition == Vector2( 0.f,0.f ) )
			{
 				SetRandomGoalWithinRadius();
				m_map->CalculateAStarPath( m_actor->m_worldPosition, m_actor->m_goalWorldPosition, m_actor->m_path, true, nullptr );
				m_map->AddActorToWaitingForPathList( m_actor );
			}

			float distCurrentToGoalSquare = ( m_actor->m_worldPosition - m_actor->m_goalWorldPosition ).CalcLengthSquare();

			if( distCurrentToGoalSquare <= ( m_actor->m_radius * m_actor->m_radius ) )
			{
				SetRandomGoalWithinRadius();
				m_map->CalculateAStarPath( m_actor->m_worldPosition, m_actor->m_goalWorldPosition, m_actor->m_path, true, nullptr );
				m_map->AddActorToWaitingForPathList( m_actor );
			}
			m_actor->MoveToGoal(elapsedTime);
			break;
		}
	}
}

void AttentiveStrategy::SetRandomGoalWithinRadius()
{
	float randX = MathUtilities::GetRandomFloatInRange( 1.f, m_map->m_mapWidthX - 2.f );
	float randY = MathUtilities::GetRandomFloatInRange( 1.f, m_map->m_mapHeightY - 2.f );
	WorldCoords2D newGoal = WorldCoords2D( randX, randY );
	TileIndex tileIndexOfNewGoal = ConvertTileCoords2DToTileIndex( ConvertWorldCoords2DToTileCoords2D( newGoal ), m_map->m_mapWidthX );
	int maxIndexOfMap = m_map->m_mapHeightY * m_map->m_mapWidthX - 1;

	while( m_map->m_tileList[tileIndexOfNewGoal].m_tileBlueprint->m_isSolid 
		   || tileIndexOfNewGoal > maxIndexOfMap
		   || m_map->m_tileList[tileIndexOfNewGoal].m_tileBlueprint->m_name.compare( "Larva" ) == 0 )
	{
		randX = MathUtilities::GetRandomFloatInRange( 1.f, m_map->m_mapWidthX - 2.f );
		randY = MathUtilities::GetRandomFloatInRange( 1.f, m_map->m_mapHeightY - 2.f );
		newGoal = WorldCoords2D( randX, randY );
		tileIndexOfNewGoal = ConvertTileCoords2DToTileIndex( ConvertWorldCoords2DToTileCoords2D( newGoal ), m_map->m_mapWidthX );
	}
	m_actor->m_goalWorldPosition = newGoal;
}

void AttentiveStrategy::SetRandomGoalAroundCenterWithinRadius()
{
	float randX = MathUtilities::GetRandomFloatInRange( 1.f, m_map->m_mapWidthX - 2.f );
	float randY = MathUtilities::GetRandomFloatInRange( 1.f, m_map->m_mapHeightY - 2.f );
	WorldCoords2D newGoal = WorldCoords2D( randX, randY );
	TileIndex tileIndexOfNewGoal = ConvertTileCoords2DToTileIndex( ConvertWorldCoords2DToTileCoords2D( newGoal ), m_map->m_mapWidthX );
	int maxIndexOfMap = m_map->m_mapHeightY * m_map->m_mapWidthX - 1;

	float distanceSquareFromCenterToGoal = ( m_centerPoint - newGoal ).CalcLengthSquare();
	float radiusSquare = m_radius * m_radius;

	while( m_map->m_tileList[tileIndexOfNewGoal].m_tileBlueprint->m_isSolid 
		|| tileIndexOfNewGoal > maxIndexOfMap
		|| m_map->m_tileList[tileIndexOfNewGoal].m_tileBlueprint->m_name.compare( "Larva" ) == 0
		|| ( distanceSquareFromCenterToGoal > radiusSquare ))
	{
		randX = MathUtilities::GetRandomFloatInRange( 1.f, m_map->m_mapWidthX - 2.f );
		randY = MathUtilities::GetRandomFloatInRange( 1.f, m_map->m_mapHeightY - 2.f );
		newGoal = WorldCoords2D( randX, randY );
		tileIndexOfNewGoal = ConvertTileCoords2DToTileIndex( ConvertWorldCoords2DToTileCoords2D( newGoal ), m_map->m_mapWidthX );
		distanceSquareFromCenterToGoal = ( m_centerPoint - newGoal ).CalcLengthSquare();
	}
	m_actor->m_goalWorldPosition = newGoal;
}



