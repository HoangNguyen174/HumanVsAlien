#ifndef ABILITY_BLUEPRINT_H
#define ABILITY_BLUEPRINT_H
#include "SoulStoneEngine/Utilities/XMLLoaderUtilities.hpp"
#include "SoulStoneEngine/Utilities/GameCommon.hpp"

enum Flag { SLOW, CONFUSION, BURN, POISON, NORMAL };

class AbilityBlueprint
{
	public:
		static std::map<std::string, AbilityBlueprint* >	s_registeredAbilityBlueprint;

	public:
		std::string				m_name;
		std::string				m_textureSheet;
		std::string				m_effectTexture;
		int						m_effectFrameWidth;
		int						m_effectFrameHeight;
		int						m_effectTexWidth;
		int						m_effectTexHeight;
		TileCoords2D			m_textureTileCoords;
		float					m_radius;
		float					m_damageTarget;
		bool					m_ignoreSameFaction;
		bool					m_ignoreDifferentFaction;
		bool					m_isTargetSpell;
		float					m_duration;
		Flag					m_addFlag;
		Flag					m_removeFlag;
		float					m_coolDown;
		float					m_eneryCost;
		std::string				m_soundName;
		float					m_drawEffectRadius;

	public:
		static void LoadBlueprint( const std::string& filePath );

	private:
		AbilityBlueprint() {};
		static Flag GetFlag( const std::string& flag );
};

#endif