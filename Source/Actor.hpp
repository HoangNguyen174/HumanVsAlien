#ifndef ACTOR_H
#define ACTOR_H
#include "SoulStoneEngine/Utilities/GameCommon.hpp"
#include "ActorBlueprint.hpp"
#include "SoulStoneEngine/Utilities/Sprite.hpp"
#include "Inventory.hpp"
#include "Ability.hpp"

class AIStrategy;

const float BASE_HEALTH = 100.f;
const float BASE_ENERGY = 100.f;
const float BASE_HIT_PER_KILL = 4.f;
const float BASE_DAMAGE_PER_HIT = BASE_HEALTH / BASE_HIT_PER_KILL;
const float BASE_TOUGHNESS = 1.f;
const float BASE_SECONDS_PER_ATTACK = 2.f;
const float BASE_MOVE_TILES_PER_SECOND = 1.f; 
const int	ITEM_SLOT_NUM = 6;
const int	BASE_EXP = 1000;
const float	HEALTH_PERCENT_TO_FLEE = 0.2f; 

class Actor;

struct AggroInfo
{
	int	    targetID;
	float   aggroLevel;
	Actor*  actor;
};

class Actor
{
	public:
		static int					s_ID;
		int							m_playerID;
		AIStrategy*					m_strategy;
		ActorBlueprint*				m_actorBlueprint;
		WorldCoords2D				m_worldPosition;
		WorldCoords2D				m_goalWorldPosition;
		Vector2						m_velocity;
		std::vector<WorldCoords2D>	m_path;
		std::string					m_initStrategy;
		float						m_orientationDegree;
		float						m_radius;
		float						m_timeToReachGoal;
		float						m_timeOfLastAttack;
		float						m_wanderRadius;
		Actor*						m_target;
		Actor*						m_protectedTarget;
		bool						m_isDead;
		bool						m_isSelected;
		bool						m_isCursorOnActor;

		Texture*					m_texture;
		std::vector<Vertex2D>		m_vertexList;
		GLuint						m_vboID;
		int							m_numVertex;
		int							m_scaleFactor;
		bool						m_isFaceRight;
		float						m_visionRange;
		WorldCoords2D				m_targetLastSeenPosition;

		RGBColor					m_color;
		float						m_flashRedRemaningTimeSec;
		bool						m_isFleeing;

		std::vector<AggroInfo*>		m_aggroList;
		std::vector<Ability*>		m_abilityList;

		//actor stat
		float						m_energy;
		float						m_maximumEnergy;
		float						m_health;
		float						m_previousHealth;
		float						m_maximumHealth;
		float						m_strength;
		float						m_intelligent;
		float						m_dexterity;
		float						m_speed;
		float						m_toughness;
		float						m_attackRate;
		float						m_baseAttackRange;
		float						m_attackRange;
		float						m_awareRadius;
		float						m_braveness;
		float						m_healthToFlee;
		std::set<Flag>				m_currentFlagList;

		// item modifier
		float						m_damageModifierFromItem;
		float						m_attackRangeModifierFromItem;
		float						m_strModifierFromItem;
		float						m_defRangeModifierFromItem;
		float						m_attackRateModifierFromItem;
		float						m_toughnessModifierFromItem;
		Item*						m_currentEquipedItemSlot[ITEM_SLOT_NUM];
		Inventory*					m_inventory;

		// exp
		int							m_expGiveWhenDie;
		int							m_currentExp;
		int							m_currentLevel;
		int							m_previousLevel;
		int							m_requiredExpTolevelUp;

		//sound
		std::string					m_deathSoundName;
		std::string					m_lvUpSoundName;
		std::string					m_attackSoundName;
		std::string					m_hurtSoundName;

	public:
		Actor( const std::string& blueprintName, const WorldCoords2D& worldPosition );
		~Actor();
		void Render();
		void Update(float elapsedTime);
		void MoveToGoal( float elapsedTime );
		void Attack( Actor* potentialTarget );
		bool CanAttackTarget( Actor* potentialTarget );
		bool IsTargetInAttackRange( Actor* potentialTarget );
		bool IsTargetInAwareRange( Actor* potentialTarget );
		bool CanAttackYet();
		bool HasLineOfSight( Actor* potentialTarget );
		bool IsDead();
		void CheckIfMouseCursorOnActor();
		void UpdateLevelAndStat();
		int CalExpRequiredToLevelUp();
		bool IsShortTermGoalInLineOfSight( const WorldCoords2D& goal );
		bool ChangeToHuntStrategyIfAnyTargetToAttack();
		bool ChangeToNonOffensiveStrategy();

	private:
		void PopulateFromBlueprint( const ActorBlueprint& blueprint );
		void CalStatModifierFromItem();
		void RenderHealthBar();
		void RenderOutLineIfSelectedOrCursorIsOn();
		void RenderLineToTarget();
		void RenderAwareRadiusAndAttackRange();
		void RenderAttackRange();
		void RenderExpBar();
		void EquipItemToEquipSlot();
		void ExploreUnknownTileWithinRadius();
		void VisualizeAStarPath();
		void CreateVBO();
		void RenderVBO();
		void TurnActor();
		bool IsActorInFogOfWar();
		void FleeIfDangeous();
		void UpdateAggroListOfTarget( Actor* actor );
		void InitializeEquipSlot( EquipSlot slot );
		void UpdateFlag();
		void ConstructSoundPath();
};

#endif