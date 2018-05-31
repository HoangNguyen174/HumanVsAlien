#ifndef PLAYER_CONTROLLER_H
#define PLAYER_CONTROLLER_H
#include "SoulStoneEngine/Utilities/GameCommon.hpp"
#include "Actor.hpp"
#include "Map.hpp"

enum PLAYER_NAME { VETERAN = 0, ENGINEER, DOCTOR };

class PlayerController
{
	public:
		std::vector<Actor*>					m_selectedPlayerList;			
		Map*								m_currentMap;
		int									m_currentPlayerID;
		bool								m_isFirstAbilitySelected;
		bool								m_isSecondAbilitySelected;
		bool								m_activateFirstAbility;
		bool								m_activateSecondAbility;
			
	public:
		PlayerController( Map* map ) ;
		void Update( float elapsedTime );
		void RenderAbilityCursorForPlayer();
		void SetPlayerDestinationToSpecifiedTile( const WorldCoords2D& goalWorldPosition );
		void SetPlayerDestinationToCurrentPos();
		void GetSelectedPlayerWithMouse();
		void GetSelectedPlayerWithKeyboard( int keyboardIndex );
		void GetAllPlayerSelected();
		void SetTargetForSelectedPlayer();
		bool IsPlayerAlreadySelected( Actor* actor );
		void ClearPlayerList();
};

#endif