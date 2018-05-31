#ifndef ACTOR_BLUEPRINT_H
#define ACTOR_BLUEPRINT_H
#include "SoulStoneEngine/Utilities/XMLLoaderUtilities.hpp"
#include "SoulStoneEngine/Utilities/GameCommon.hpp"

struct DropItem
{
	std::string name;
	float		dropRate;
};

class ActorBlueprint
{
	public:
		static std::map<std::string, ActorBlueprint*>   s_registeredActorBlueprint;

	public:
		ActorBlueprint*									m_parent;
		std::string										m_name;
		std::string										m_textureSheet;
		bool											m_isPlayer;
		float											m_health;
		float											m_strength;
		float											m_dexterity;
		float											m_intelligent;
		std::string										m_faction;
		float											m_radius;
		float											m_speed;
		float											m_toughness;
		float											m_attackRate;
		float											m_attackRange;
		TileCoords2D									m_textureTileCoords;
		std::string										m_initStrategy;
		std::string										m_protectTarget;
		float											m_awareRadius;
		float											m_wanderRadius;
		float											m_visionRange;
		std::vector<DropItem*>							m_dropList;
		int												m_expPoint;
		Vector2											m_braveness;
		std::vector<std::string>						m_abilityList;
		std::string										m_deathSoundName;
		std::string										m_lvUpSoundName;
		std::string										m_attackSoundName;
		std::string										m_hurtSoundName;

	public:
		static void LoadBlueprint( const std::string& filePath );

	private:
		ActorBlueprint() {};
	
		
};

#endif