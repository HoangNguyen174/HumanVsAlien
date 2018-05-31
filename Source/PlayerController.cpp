#include "PlayerController.hpp"
#include "AIStrategy.hpp"
#include "World.hpp"
#include "HUD.hpp"
#include "SoulStoneEngine/JobSystem/MemoryPoolManager.hpp"

PlayerController::PlayerController( Map* map )
{
	m_currentMap = map;
	Actor*	player = nullptr;
	m_currentPlayerID = 0;
	m_isFirstAbilitySelected = false;
	m_isSecondAbilitySelected = false;
	m_activateFirstAbility = false;
	m_activateSecondAbility = false;

	for ( unsigned int i = 0; i < map->m_actorList.size(); i++ )
	{
		if( map->m_actorList[i]->m_actorBlueprint->m_isPlayer == true )
		{
			player = map->m_actorList[i];
			InactiveStrategy* inactiveStrategy = new InactiveStrategy( player, map );
			delete player->m_strategy;
			player->m_strategy = nullptr;
			player->m_strategy = inactiveStrategy;
		}
	}

	player = nullptr;
}

void PlayerController::SetPlayerDestinationToSpecifiedTile( const WorldCoords2D& goalWorldPosition )
{
	if( /*m_player == nullptr &&*/ m_selectedPlayerList.size() == 0 )
		return;
	
	bool isSeen = false;

	TileCoords2D goalTilePosition = ConvertWorldCoords2DToTileCoords2D( goalWorldPosition );
	TileIndex goalIndex = ConvertTileCoords2DToTileIndex( goalTilePosition, m_currentMap->m_mapWidthX );

	if( goalIndex >= m_currentMap->m_mapHeightY * m_currentMap->m_mapWidthX || goalIndex < 0 )
		return;

	if( m_currentMap->m_tileList[goalIndex].m_isVisibleNow || m_currentMap->m_tileList[goalIndex].m_isVisibleLastTime )
	{
		isSeen = true;
	}

	for( unsigned int i = 0; i < m_selectedPlayerList.size(); i++ )
	{
		TileCoords2D playerCurrentPosition = ConvertWorldCoords2DToTileCoords2D( m_selectedPlayerList[i]->m_worldPosition );

		if( !m_currentMap->m_tileList[goalIndex].m_tileBlueprint->m_isSolid || !isSeen )
		{
			m_currentMap->CalculateAStarPath( m_selectedPlayerList[i]->m_worldPosition, goalWorldPosition, m_selectedPlayerList[i]->m_path, false, nullptr );
			m_selectedPlayerList[i]->m_goalWorldPosition = goalWorldPosition;
		}

		if( isSeen && m_currentMap->m_tileList[goalIndex].m_tileBlueprint->m_isSolid  )
		{
			m_selectedPlayerList[i]->m_path.clear();
			m_selectedPlayerList[i]->m_goalWorldPosition = m_selectedPlayerList[i]->m_worldPosition;
		}

		InactiveStrategy* inactiveStrategy = new InactiveStrategy( m_selectedPlayerList[i], m_currentMap );

		delete m_selectedPlayerList[i]->m_strategy;
		m_selectedPlayerList[i]->m_strategy = nullptr;

		m_selectedPlayerList[i]->m_strategy = inactiveStrategy;
	}
}

void PlayerController::GetSelectedPlayerWithMouse()
{
	bool missPlayer = true;
	Actor*	player = nullptr;

	for( unsigned int i = 0; i < m_currentMap->m_actorList.size(); i++ )
	{
		Actor* actor = m_currentMap->m_actorList[i];
		float distanceSquare = ( actor->m_worldPosition - g_mouseWorldPosition ).CalcLengthSquare();
		float radiusSquare = actor->m_radius * actor->m_radius;

		if( distanceSquare <= radiusSquare && actor->m_actorBlueprint->m_isPlayer )
		{
			missPlayer = false;
			if( !g_isHoldingShift )
			{
				if( m_currentPlayerID != actor->m_playerID && player != nullptr )
					player->m_isSelected = false;

				player = actor;
			
				if( m_selectedPlayerList.size() >= 1 )
				{
					ClearPlayerList();
					player->m_isSelected = true;
					m_selectedPlayerList.push_back( player );
				}
				else
				{
					player->m_isSelected = true;
					m_selectedPlayerList.push_back( player );
				}
			}
			else
			{
				if( !IsPlayerAlreadySelected( actor ) )
				{
					actor->m_isSelected = true;
					m_selectedPlayerList.push_back( actor );
				}
			}
		}
	}

	if( missPlayer && !m_isFirstAbilitySelected )
	{
		if( IsInventoryAndAbilityShowing && IsInItemHud )
		{

		}
		else
		{
			ClearPlayerList();
		}
	}

	player = nullptr;
}

void PlayerController::SetTargetForSelectedPlayer()
{
	if( m_selectedPlayerList.size() == 0 )
		return;

	for( unsigned int i = 0; i < m_currentMap->m_actorList.size(); i++ )
	{
		Actor* actor = m_currentMap->m_actorList[i];
		float distanceSquare = ( actor->m_worldPosition - g_mouseWorldPosition ).CalcLengthSquare();
		float radiusSquare = actor->m_radius * actor->m_radius;
		TileCoords2D actorTileCoords = ConvertWorldCoords2DToTileCoords2D( actor->m_worldPosition );
		TileIndex tileIndex = ConvertTileCoords2DToTileIndex( actorTileCoords, g_currentMap->m_mapWidthX );

		if( distanceSquare <= radiusSquare && 
			!actor->m_actorBlueprint->m_isPlayer &&
			g_currentMap->m_tileList[tileIndex].m_isVisibleNow )
		{
			for( unsigned int i = 0; i < m_selectedPlayerList.size(); i++ )
			{
				m_selectedPlayerList[i]->m_target = actor;
				HuntingStrategy* huntStrategy = new HuntingStrategy( m_selectedPlayerList[i], m_currentMap );
				delete m_selectedPlayerList[i]->m_strategy;
				m_selectedPlayerList[i]->m_strategy = nullptr;
				m_selectedPlayerList[i]->m_strategy = huntStrategy;
			}

			return;
		}
	}
}

bool PlayerController::IsPlayerAlreadySelected( Actor* actor )
{
	bool alreadySelected = false;
	for( unsigned int i = 0; i < m_selectedPlayerList.size(); i++ )
	{
		Actor* actorInlist = m_selectedPlayerList[i];
		if( actor->m_playerID == actorInlist->m_playerID )
		{
			alreadySelected = true;
		}
	}
	return alreadySelected;
}

void PlayerController::ClearPlayerList()
{
	for( unsigned int i = 0; i < m_selectedPlayerList.size(); i++ )
	{
		m_selectedPlayerList[i]->m_isSelected = false;
		m_selectedPlayerList[i]->m_isCursorOnActor = false;
	}
	m_selectedPlayerList.clear();
}

void PlayerController::SetPlayerDestinationToCurrentPos()
{
	for( unsigned int i = 0; i < m_selectedPlayerList.size(); i++ )
	{
		m_selectedPlayerList[i]->m_goalWorldPosition = m_selectedPlayerList[i]->m_worldPosition;
	}
}

void PlayerController::RenderAbilityCursorForPlayer()
{
	if( m_selectedPlayerList.size() == 0 || m_selectedPlayerList.size() > 1 )
		return;

	Actor* actor = m_selectedPlayerList[0];
	if( m_isFirstAbilitySelected )
	{
		if( actor->m_abilityList.size() > 0 )
		{
			actor->m_abilityList[0]->RenderCursor( g_mouseWorldPosition );
		}
	}
	else if( m_isSecondAbilitySelected )
	{
		if( actor->m_abilityList.size() > 1 )
		{
			actor->m_abilityList[1]->RenderCursor( g_mouseWorldPosition );
		}
	}
}

void PlayerController::Update(float elapsedTime)
{
	if( m_selectedPlayerList.size() == 0 || m_selectedPlayerList.size() > 1 )
	{
		m_isFirstAbilitySelected = false;
		m_isSecondAbilitySelected = false;
	}


	if( m_isFirstAbilitySelected && g_isLeftMouseDown )
	{
		m_isFirstAbilitySelected = false;
		m_activateFirstAbility = true;
	}

	if( m_isSecondAbilitySelected && g_isLeftMouseDown )
	{
		m_isSecondAbilitySelected = false;
		m_activateSecondAbility = true;
	}

	if( m_selectedPlayerList.size() == 1 && m_activateFirstAbility )
	{
		m_selectedPlayerList[0]->m_abilityList[0]->m_worldPosition = g_mouseWorldPosition;
		m_selectedPlayerList[0]->m_abilityList[0]->m_isActive = true;
		m_selectedPlayerList[0]->m_abilityList[0]->ActivateAbility(elapsedTime);
		m_activateFirstAbility = false;
	}

	if( m_selectedPlayerList.size() == 1 && m_activateSecondAbility )
	{
		m_selectedPlayerList[0]->m_abilityList[1]->m_worldPosition = g_mouseWorldPosition;
		m_selectedPlayerList[0]->m_abilityList[1]->m_isActive = true;
		m_selectedPlayerList[0]->m_abilityList[1]->ActivateAbility(elapsedTime);
		m_activateSecondAbility = false;
	}
}

void PlayerController::GetSelectedPlayerWithKeyboard(int keyboardIndex)
{
	std::string playerName;

	switch( keyboardIndex )
	{
		case 1: playerName = "Veteran";
				break;
		case 2: playerName = "Engineer";
				break;
		case 3: playerName = "Doctor";
				break;
	}

	for ( unsigned int i = 0; i < g_currentMap->m_actorList.size(); i++ )
	{
		if( g_currentMap->m_actorList[i]->m_actorBlueprint->m_isPlayer == true )
		{
			Actor* actor = g_currentMap->m_actorList[i];
			if( actor->m_actorBlueprint->m_isPlayer && actor->m_actorBlueprint->m_name.compare( playerName ) == 0 )
			{
				if( !g_isHoldingShift )
				{
					if( m_selectedPlayerList.size() >= 1)
					{
						ClearPlayerList();
						actor->m_isSelected = true;
						m_selectedPlayerList.push_back( actor );
					}
					else
					{
						actor->m_isSelected = true;
						m_selectedPlayerList.push_back( actor );
					}
				}
				else
				{
					if( !IsPlayerAlreadySelected(actor) )
					{
						actor->m_isSelected = true;
						m_selectedPlayerList.push_back( actor );
					}
				}
			}
		}
	}
}

void PlayerController::GetAllPlayerSelected()
{
	ClearPlayerList();
	for ( unsigned int i = 0; i < g_currentMap->m_actorList.size(); i++ )
	{
		if( g_currentMap->m_actorList[i]->m_actorBlueprint->m_isPlayer == true )
		{
			Actor* actor = g_currentMap->m_actorList[i];
			if( actor->m_actorBlueprint->m_isPlayer )
			{
				actor->m_isSelected = true;
				m_selectedPlayerList.push_back( actor );
			}
		}
	}
}

