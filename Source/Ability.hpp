#ifndef ABILITY_H
#define ABILITY_H
#include "AbilityBlueprint.hpp"
#include "SoulStoneEngine/Utilities/Sprite.hpp"

class Actor;

class Ability
{
	public:
		static int				s_ID;
		int						m_abilityID;
		AbilityBlueprint*		m_abilityBlueprint;
		Texture*				m_iconTexture;
		Texture*				m_effectTexture;
		float					m_radius;
		Flag					m_removeFlag;
		Flag					m_addFlag;
		WorldCoords2D			m_worldPosition;
		bool					m_isVBODirty;
		GLuint					m_vboID;
		int						m_numVertex;
		bool					m_isSelected;
		bool					m_isActive;
		Actor*					m_target;
		float					m_coolDown;
		float					m_energyCost;
		float					m_timeSinceLastCast;
		Actor*					m_actorOwner;
		bool					m_isCasting;
		Sprite*					m_effectSprite;
		float					m_remainingTime;
		std::string				m_soundNameFullPath;

	public:
		Ability ( const std::string& blueprintName, Actor* owner );
		void ActivateAbility( float elapsedTime );
		void Update( float elapsedTime );
		void RenderCursor( const WorldCoords2D& pos );
		void RenderEffect();
		void RenderIcon();

	private:
		void PopulateFromBlueprint( const AbilityBlueprint& blueprint );
		void CreateVBO();
		void RenderVBO();
		bool CanCastYet();
};

#endif