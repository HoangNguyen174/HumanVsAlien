#ifndef HUD_H
#define HUD_H
#include "Map.hpp"
#include "Item.hpp"
#include "SoulStoneEngine/Utilities/GameCommon.hpp" 
#include "PlayerController.hpp"

class HUD
{
	public:
		Actor*				m_veteran;
		Actor*				m_engineer;
		Actor*				m_doctor;
		Map					m_currentMap;
		Texture*			m_texture;
		Texture*			m_veteranTexture;
		Texture*			m_engineerTexture;
		Texture*			m_doctorTexture;
		Actor*				m_selectedActor;
		WorldCoords2D		m_mouseScreenPos;
		PlayerController*	m_playerController;



	public:
		HUD() {};
		HUD( const Map& map );
		void Update();
		void Render();
		bool IsMouseIsInHudArea();

	private:
		void RenderPlayerHealthAndManaBar();
		void RenderPlayerPotrait();
		void RenderAbility();
		void RenderPotraitWithTexCoords( const Vector2& center, float radius, const Vector2i texCoords, bool isSelected, bool isMouseOn );
		void RenderInventory( Actor* actor );
		void RenderItem( Item* item, const WorldCoords2D& position );
		void ProcessMouseClick();
		void ProcessIfPlayerClickOnPlayerPotrait();
		void ProcessIfPlayerClickOnItem();
		void ProcessIfPlayerClickOnPortionOrAbility();
		
};

extern bool	IsInCharacterHud;
extern bool	IsInItemHud;
extern bool	IsInAbilityHud;
extern bool	IsInventoryAndAbilityShowing;
#endif
