#include "Actor.hpp"
#include "SoulStoneEngine/Debugger/DebugLine.hpp"
#include "AIStrategy.hpp"
#include "World.hpp"
#include "Projectile.hpp"
#include "SoulStoneEngine/JobSystem/MemoryPoolManager.hpp"

int Actor::s_ID = 0;

Actor::Actor( const std::string& blueprintName, const WorldCoords2D& worldPosition )
{
	s_ID++;
	m_playerID = s_ID;
	m_worldPosition = worldPosition;
	m_orientationDegree = 0.f;
	m_timeToReachGoal = 0.f;
	m_target = nullptr;
	m_isDead = false;
	m_timeOfLastAttack = 0.f;
	m_strategy = nullptr;
	m_isSelected = false;
	m_isCursorOnActor = false;
	m_inventory = new Inventory();
	m_damageModifierFromItem = 0;
	m_attackRangeModifierFromItem = 0;
	m_numVertex = 0;
	m_vboID = 0;
	m_scaleFactor = 1;
	m_isFaceRight = true;
	m_currentLevel = 1;
	m_currentExp = 0;
	m_previousLevel = m_currentLevel;
	m_requiredExpTolevelUp = CalExpRequiredToLevelUp();
	m_isFleeing = false;
	m_color = RGBColor::White();
	m_flashRedRemaningTimeSec = 0.f;

	for( int i = 0; i < ITEM_SLOT_NUM; i++ )
	{
		m_currentEquipedItemSlot[i] = nullptr;
	}

	std::map<std::string, ActorBlueprint*>::iterator iter;
	iter = ActorBlueprint::s_registeredActorBlueprint.find( blueprintName );

	if( iter != ActorBlueprint::s_registeredActorBlueprint.end() )
	{
		ActorBlueprint* blueprint = iter->second;
		m_actorBlueprint = blueprint;
		PopulateFromBlueprint( *blueprint );
		ConstructSoundPath();
	}
	else
	{
		DebuggerPrintf( "Cannot Find Actor with name %s.\n", blueprintName.c_str() );
		exit(0);
	}
}

Actor::~Actor()
{
	g_glRender->DeleteBuffers( 1, &m_vboID );
}

void Actor::ConstructSoundPath()
{
	std::string root = "./Data/Sound/";

	if( m_deathSoundName.compare( "NoSound" ) != 0 )
	{
		m_deathSoundName = root + m_deathSoundName;
	}
	if( m_hurtSoundName.compare( "NoSound" ) != 0 )
	{
		m_hurtSoundName = root + m_hurtSoundName;
	}
	if( m_lvUpSoundName.compare( "NoSound" ) != 0 )
	{
		m_lvUpSoundName = root + m_lvUpSoundName;
	}
	if( m_attackSoundName.compare( "NoSound" ) != 0 )
	{
		m_attackSoundName = root + m_attackSoundName;
	}
}

void Actor::PopulateFromBlueprint( const ActorBlueprint& blueprint )
{
	m_strength = blueprint.m_strength;
	m_intelligent = blueprint.m_intelligent;
	m_dexterity = blueprint.m_dexterity;
	m_maximumHealth = blueprint.m_health * BASE_HEALTH;
	m_maximumEnergy = blueprint.m_intelligent * BASE_ENERGY;
	m_energy = m_maximumEnergy;
	m_health = m_maximumHealth;
	m_previousHealth = m_health;
	m_radius = blueprint.m_radius;
	m_speed = blueprint.m_speed;
	m_attackRange = blueprint.m_attackRange;
	m_attackRate = blueprint.m_attackRate;
	m_toughness = blueprint.m_toughness;
	m_awareRadius  = blueprint.m_awareRadius;
	m_wanderRadius = blueprint.m_wanderRadius;
	m_initStrategy = blueprint.m_initStrategy;
	m_visionRange = blueprint.m_visionRange;
	m_expGiveWhenDie = blueprint.m_expPoint;
	m_braveness = MathUtilities::GetRandomFloatInRange( blueprint.m_braveness.x, blueprint.m_braveness.y );
	m_healthToFlee = HEALTH_PERCENT_TO_FLEE * m_maximumHealth;

	m_protectedTarget = nullptr;
	m_baseAttackRange = m_attackRange;

	m_deathSoundName = blueprint.m_deathSoundName;
	m_lvUpSoundName = blueprint.m_lvUpSoundName;
	m_hurtSoundName = blueprint.m_hurtSoundName;
	m_attackSoundName = blueprint.m_attackSoundName;
	
	for( unsigned int i = 0; i < blueprint.m_abilityList.size(); i++ )
	{
		Ability* ability = new Ability( blueprint.m_abilityList[i], this );
		m_abilityList.push_back( ability );
	}

	m_texture = Texture::CreateOrGetTexture( "./Data/Texture/" + blueprint.m_textureSheet );

	if( blueprint.m_isPlayer )
	{
		m_goalWorldPosition = m_worldPosition; 
	}
}

void Actor::Update( float elapsedTime )
{
	CheckIfMouseCursorOnActor();

	EquipItemToEquipSlot();

	CalStatModifierFromItem();

	TurnActor();

	ExploreUnknownTileWithinRadius();

	if( m_actorBlueprint->m_isPlayer )
		UpdateLevelAndStat();

	if( m_strategy != nullptr )
		m_strategy->Update( elapsedTime );

	if( m_health <= 0.f )
		m_health = 0.f;

	if( m_health > m_maximumHealth )
		m_health = m_maximumHealth;

	if( m_energy > m_maximumEnergy )
		m_energy = m_maximumEnergy;

	if( m_health < m_previousHealth )
	{
		g_audioSystem->PlaySoundByName( m_hurtSoundName.c_str(),1,false );
		m_previousHealth = m_health;
		m_flashRedRemaningTimeSec = 1.f;
	}

	if( m_flashRedRemaningTimeSec > 0.f )
	{
		m_flashRedRemaningTimeSec -= elapsedTime;
	}

	FleeIfDangeous();
}

void Actor::FleeIfDangeous()
{
	if( m_health <= m_healthToFlee && !m_isFleeing )
	{
		float rollDice = MathUtilities::GetRandomFloatZeroToOne();
		if( rollDice > m_braveness )
		{
			m_isFleeing = true;
			m_target = nullptr;
			m_path.clear();
			FleeStrategy* fleeStrategy = new FleeStrategy( this, g_currentMap );
			
			delete m_strategy;
			m_strategy = nullptr;

			m_strategy = fleeStrategy;
		}
	}
}

void Actor::UpdateLevelAndStat()
{
	m_requiredExpTolevelUp = CalExpRequiredToLevelUp();
	if( m_currentExp >= m_requiredExpTolevelUp )
	{
		g_audioSystem->PlaySoundByName( m_lvUpSoundName.c_str(),1,false );
		m_currentLevel++;
		m_currentExp = 0;
	}

	//update stat if level up
	if( m_currentLevel != m_previousLevel )
	{
		m_maximumHealth += 10;
		m_health = m_maximumHealth;
		m_strength += 0.1f;
		m_intelligent += 0.1f;
		m_dexterity += 0.1f;
		m_attackRate += 0.1f;
		m_toughness += 0.1f;
		m_previousLevel = m_currentLevel;
	}
}

void Actor::Render()
{
	if( IsActorInFogOfWar() && !g_isFOWoff )
		return;

	RenderOutLineIfSelectedOrCursorIsOn();

	CreateVBO();

	g_glRender->PushMatrix();
	g_glRender->Translatef( m_worldPosition.x, m_worldPosition.y, 0.f );
	g_glRender->Scalef( static_cast<float>( m_scaleFactor ), 1.f, 1.f );
	g_glRender->Translatef( -m_worldPosition.x, -m_worldPosition.y, 0.f );
	RenderVBO();
	g_glRender->PopMatrix();

	RenderHealthBar();
	
	if( m_actorBlueprint->m_isPlayer)
		RenderExpBar();

	//if( !m_actorBlueprint->m_isPlayer )
	//	RenderLineToTarget();

	RenderAwareRadiusAndAttackRange();

	//VisualizeAStarPath();

// 	glColor4f( 0.f,0.5f,1.f, 0.5f );
// 	glBegin(GL_LINES);
// 	glVertex2f( m_worldPosition.x, m_worldPosition.y );
// 	glVertex2f( m_goalWorldPosition.x, m_goalWorldPosition.y );
// 	glEnd();
}

void Actor::MoveToGoal( float elapsedTime )
{
	 float toleranceSquare = 0.1f * 0.1f;

	if( m_path.size() > 0 )
	{
		WorldCoords2D finalGoal = m_path[0];
		TileIndex finalGoalindex = ConvertWorldCoords2DToTileIndex( finalGoal, g_currentMap->m_mapWidthX );

		Tile& finalGoalTile = g_currentMap->m_tileList[finalGoalindex];

		if( ( finalGoalTile.m_isVisibleLastTime || finalGoalTile.m_isVisibleNow ) && finalGoalTile.m_tileBlueprint->m_isSolid )
		{
			m_path.clear();
			return;
		}
			
		for( unsigned int i = 0; i < m_path.size(); i++ )
		{
			bool isSeen = false;
			bool isSolid = false;

			WorldCoords2D path = m_path[i];
			TileIndex pathTileIndex = ConvertWorldCoords2DToTileIndex( path, g_currentMap->m_mapWidthX );

			if( g_currentMap->m_tileList[pathTileIndex].m_isVisibleNow || g_currentMap->m_tileList[pathTileIndex].m_isVisibleLastTime )
				isSeen = true;

			if( g_currentMap->m_tileList[pathTileIndex].m_tileBlueprint->m_isSolid )
				isSolid = true;

			if( isSolid && isSeen )
			{
				m_path.clear();
				if( m_actorBlueprint->m_isPlayer )
					g_currentMap->CalculateAStarPath( m_worldPosition, finalGoal, m_path, false, nullptr );
				else
					g_currentMap->CalculateAStarPath( m_worldPosition, finalGoal, m_path, true, nullptr );
			}
		}
	}



	if( m_path.size() > 0 )
	{
		m_timeToReachGoal += elapsedTime;
		WorldCoords2D finalGoal = m_path[0];

		if( m_actorBlueprint->m_isPlayer )
		{
			for( unsigned int i = m_path.size() - 1; i > 0; i-- )
			{
				TileIndex pathTileIndex = ConvertWorldCoords2DToTileIndex( m_path[i - 1], g_currentMap->m_mapWidthX );
				bool isNextGoalVisible = g_currentMap->m_tileList[pathTileIndex].m_isVisibleNow;

				if( IsShortTermGoalInLineOfSight( m_path[i - 1] ) && isNextGoalVisible )
				{
					m_path.erase( m_path.begin() + i );
				}
			}
		}

		if( ( m_worldPosition - finalGoal ).CalcLengthSquare() > 0.001f * 0.001f )
		{
			WorldCoords2D nextGoal = m_path.back();

			float distanceSquare = ( m_worldPosition - nextGoal ).CalcLengthSquare();
		
			if( distanceSquare > toleranceSquare )
			{
				m_velocity = ( nextGoal - m_worldPosition ).Normalize() * m_speed;
				m_worldPosition += m_velocity * elapsedTime;
			}
			else
			{
				m_worldPosition = nextGoal;
				m_path.pop_back();
			}
		}
	}

	float distanceToGoalSquare = ( m_worldPosition - m_goalWorldPosition ).CalcLengthSquare();

	if( distanceToGoalSquare > toleranceSquare && m_path.size() == 0 )
	{
		m_velocity = ( m_goalWorldPosition - m_worldPosition ).Normalize() * m_speed ;
		m_worldPosition += m_velocity * elapsedTime;
	}
	
	if( distanceToGoalSquare <= toleranceSquare )
	{
		m_velocity = Vector2( 0.f, 0.f );
		m_worldPosition = m_goalWorldPosition;
		m_path.clear();
	}
}

bool Actor::IsShortTermGoalInLineOfSight( const WorldCoords2D& goal )
{
	//shortcut
	Vector2 rayDirection = goal - m_worldPosition;

	float rayMaxLength = rayDirection.CalcLength();

	Vector2 perpendicularToDirVector = rayDirection;
	perpendicularToDirVector.Rotate90Deg();
	perpendicularToDirVector = perpendicularToDirVector.Normalize();

	Vector2 point1 = m_worldPosition + perpendicularToDirVector * m_radius;
	Vector2 point2 = m_worldPosition - perpendicularToDirVector * m_radius;

	HitInfo hit1 = Map::GetImpactedInfoFromRaycast( m_worldPosition, rayDirection, rayMaxLength, *g_currentMap );
	HitInfo hit2 = Map::GetImpactedInfoFromRaycast( point1, rayDirection, rayMaxLength, *g_currentMap );
	HitInfo hit3 = Map::GetImpactedInfoFromRaycast( point2, rayDirection, rayMaxLength, *g_currentMap );

	if( hit1.faceOfTileImpacted == NONE && 
		hit2.faceOfTileImpacted == NONE && 
		hit3.faceOfTileImpacted == NONE )
			return true;
	return false;
}

bool Actor::HasLineOfSight( Actor* /*potentialTarget*/ )
{
	if( m_target == nullptr )
		return false;

	return IsShortTermGoalInLineOfSight( m_target->m_worldPosition );
}

bool Actor::CanAttackYet()
{
	float secondsPerAttack = BASE_SECONDS_PER_ATTACK / m_attackRate;
	float secondsSinceLastAttack = (float)GetCurrentTimeSeconds() - m_timeOfLastAttack;
	if( secondsSinceLastAttack >= secondsPerAttack )
		return true;
	else
		return false;
}

bool Actor::IsTargetInAttackRange( Actor* potentialTarget )
{
	float distanceSquare = ( m_worldPosition - potentialTarget->m_worldPosition ).CalcLengthSquare();
	float attackRangeSquare = m_attackRange * m_attackRange;

	if( distanceSquare <= attackRangeSquare )
		return true;
	else
		return false;
}

bool Actor::CanAttackTarget( Actor* potentialTarget )
{
	bool isInRange = IsTargetInAttackRange( potentialTarget );
	bool isReadyToAttack = CanAttackYet();
	bool hasLineOfSight = HasLineOfSight( potentialTarget );

	if( isInRange && isReadyToAttack && hasLineOfSight )
		return true;
	else
		return false;
}

void Actor::Attack( Actor* potentialTarget )
{
	float damage = m_strength * BASE_DAMAGE_PER_HIT + m_damageModifierFromItem;
	damage /= ( m_target->m_toughness );
	potentialTarget->m_health -= damage;
	if( potentialTarget->m_health < 0.f )
		potentialTarget->m_health = 0.f;
	m_timeOfLastAttack = (float)GetCurrentTimeSeconds();

	if( m_attackSoundName.compare( "NoSound" ) != 0 )
		g_audioSystem->PlaySoundByName( m_attackSoundName.c_str(), 1, false );

	if( m_target->m_actorBlueprint->m_isPlayer && m_target->m_path.size() > 0 )
		return;
	
	if( !potentialTarget->m_isFleeing )
		UpdateAggroListOfTarget( potentialTarget );
}

void Actor::UpdateAggroListOfTarget( Actor* actor )
{
	bool isAttackerInTheList = false;
	int attackerIndex= -1;

	for( unsigned int i = 0; i < actor->m_aggroList.size(); i++ )
	{
		if( actor->m_aggroList[i]->targetID == m_playerID )
		{
			isAttackerInTheList = true;
			attackerIndex = i;
			break;
		}
	}

	if( isAttackerInTheList )
	{
		actor->m_aggroList[attackerIndex]->aggroLevel += 1.f;
	}
	else
	{
		AggroInfo* newAttacker = new AggroInfo;
		newAttacker->targetID = m_playerID;
		newAttacker->aggroLevel = 1.0f;
		newAttacker->actor = this;
		actor->m_aggroList.push_back( newAttacker );
	}

	int actorWithHighestAggroIndex = 0;

	for( unsigned int i = 0; i < actor->m_aggroList.size(); i++ )
	{
		if( actor->m_aggroList[actorWithHighestAggroIndex]->aggroLevel < actor->m_aggroList[i]->aggroLevel )
		{
			actorWithHighestAggroIndex = i;
		}
	}

	if( actor->m_aggroList.size() == 0 )
		return;

	actor->m_target = actor->m_aggroList[actorWithHighestAggroIndex]->actor;
	HuntingStrategy* huntStrategy = new HuntingStrategy( actor, g_currentMap );
	delete actor->m_strategy;
	actor->m_strategy = nullptr;
	actor->m_strategy = huntStrategy;
}

bool Actor::IsTargetInAwareRange(Actor* potentialTarget)
{
	float distanceSquare = ( m_worldPosition - potentialTarget->m_worldPosition ).CalcLengthSquare();
	float awareRadiusSquare = m_awareRadius * m_awareRadius;

	if( distanceSquare <= awareRadiusSquare )
		return true;
	else
		return false;
}

bool Actor::IsDead()
{
	if( m_health <= 0.f )
		return true;
	return false;
}

void Actor::RenderHealthBar()
{
	float offSetFromCenter = 0.05f;

	Vector2 bottemLeft = Vector2( m_worldPosition.x - 0.5f, m_worldPosition.y + m_radius + offSetFromCenter );

	g_glRender->Draw2DHollowRectangle( bottemLeft, 1.f, 0.1f, RGBColor( 1.f,1.f,0.f,1.f) );
	g_glRender->Draw2DFilledRectangle( bottemLeft, ( m_health / m_maximumHealth ), 0.1f, RGBColor( 0.f,1.f,0.f,1.f) );
}

void Actor::RenderExpBar()
{
	float offSetFromCenter = 0.2f;

	Vector2 bottemLeft = Vector2( m_worldPosition.x - 0.5f, m_worldPosition.y - m_radius - offSetFromCenter );

	g_glRender->Draw2DHollowRectangle( bottemLeft, 1.f, 0.1f, RGBColor( 0.f,0.f,0.f,1.f) );
	g_glRender->Draw2DFilledRectangle( bottemLeft, ( (float)m_currentExp / (float)m_requiredExpTolevelUp ), 0.1f, RGBColor( 1.f,1.f,0.f,1.f) );
	std::string level = std::to_string( static_cast<long double>(m_currentLevel) );
	g_glRender->RenderText( bottemLeft + Vector2(0.f, -0.2f), RGBColor::Black() ,nullptr, nullptr, 0.2f, "Lvl. " + level );
}

void Actor::RenderOutLineIfSelectedOrCursorIsOn()
{
	if( m_isSelected || m_isCursorOnActor )
		g_glRender->Draw2DHollowCircle( m_worldPosition, m_radius + 0.05f, RGBColor( 1.f,0.5f,0.f,1.) );
}

int Actor::CalExpRequiredToLevelUp()
{
	m_requiredExpTolevelUp = m_currentLevel * BASE_EXP;
	return m_requiredExpTolevelUp;
}

void Actor::CheckIfMouseCursorOnActor()
{
	float distanceSquare = ( m_worldPosition - g_mouseWorldPosition ).CalcLengthSquare();
	float radiusSquare = m_radius * m_radius;
	if( distanceSquare <= radiusSquare )
	{
		m_isCursorOnActor = true;
	}
	else
		m_isCursorOnActor = false;
}

void Actor::RenderLineToTarget()
{
	glColor4f( 1.f,0.f,0.f, 0.5f );
	if( m_target != nullptr )
	{
		glBegin(GL_LINES);
		glVertex2f( m_worldPosition.x, m_worldPosition.y );
		glVertex2f( m_target->m_worldPosition.x, m_target->m_worldPosition.y );
		glEnd();
	}
}

void Actor::RenderAwareRadiusAndAttackRange()
{
	g_glRender->Draw2DHollowCircle( m_worldPosition, m_awareRadius, RGBColor( 1.f,1.f,0.0f,0.2f) );
	g_glRender->Draw2DHollowCircle( m_worldPosition, m_attackRange, RGBColor( 1.f,0.0f,0.0f,0.2f) );
}

void Actor::CalStatModifierFromItem()
{
	m_attackRangeModifierFromItem = 0;
	m_damageModifierFromItem = 0;

	for( int i = 0; i < ITEM_SLOT_NUM; i++ )
	{
		if( m_currentEquipedItemSlot[i] != nullptr)
		{
			m_damageModifierFromItem += m_currentEquipedItemSlot[i]->m_damage;
			m_attackRangeModifierFromItem += m_currentEquipedItemSlot[i]->m_attackRange;
		}
	}
	m_attackRange = m_baseAttackRange + m_attackRangeModifierFromItem;
}

void Actor::EquipItemToEquipSlot()
{
	InitializeEquipSlot( MAIN_HAND );
	InitializeEquipSlot( OFF_HAND );
	InitializeEquipSlot( BODY );
	InitializeEquipSlot( HEAD );
	InitializeEquipSlot( HEALTH );
	InitializeEquipSlot( ENERGY );
}

void Actor::InitializeEquipSlot( EquipSlot slot )
{
	if( slot == UNEQUIPABLE )
		return;

	if( m_currentEquipedItemSlot[slot] == nullptr )
	{
		int index = m_inventory->GetIndexFirstItemInEquipSlot( slot );
		if( index != -1 )
			m_currentEquipedItemSlot[ slot ] = m_inventory->m_itemList[index];
	}
}

void Actor::CreateVBO()
{
	float texFrameWidthX = 1.f / 5.f;
	float texFrameHeightY = 1.f / 5.f;
	Vertex2D vertex;
	Vector2 texCoords;

	Vector2 itemTopLeftTexCoords;
	itemTopLeftTexCoords.x = (float)m_actorBlueprint->m_textureTileCoords.x * texFrameWidthX;
	itemTopLeftTexCoords.y = (float)m_actorBlueprint->m_textureTileCoords.y * texFrameHeightY;

	vertex.m_color = m_color;
	vertex.m_color.m_blue -= m_flashRedRemaningTimeSec;
	vertex.m_color.m_green -= m_flashRedRemaningTimeSec;
	Vector2 center = m_worldPosition;

	//bottom left
	vertex.m_position = Vector2( center.x - m_radius, center.y - m_radius );
	vertex.m_texCoords = Vector2( itemTopLeftTexCoords.x, itemTopLeftTexCoords.y + texFrameHeightY );
	m_vertexList.push_back( vertex );

	//bottom right 
	vertex.m_position = Vector2( center.x + m_radius, center.y - m_radius );
	vertex.m_texCoords = Vector2( itemTopLeftTexCoords.x + texFrameWidthX , itemTopLeftTexCoords.y + texFrameHeightY );
	m_vertexList.push_back( vertex );

	//top right
	vertex.m_position = Vector2( center.x + m_radius, center.y + m_radius );
	vertex.m_texCoords = Vector2( itemTopLeftTexCoords.x + texFrameWidthX , itemTopLeftTexCoords.y );
	m_vertexList.push_back( vertex );

	//top left
	vertex.m_position = Vector2( center.x - m_radius, center.y + m_radius );
	vertex.m_texCoords = itemTopLeftTexCoords;
	m_vertexList.push_back( vertex );

	if( m_vboID == 0 )
	{
		g_glRender->GenerateBuffer( 1, &m_vboID );
	}

	g_glRender->BindBuffer( GL_ARRAY_BUFFER, m_vboID );
	g_glRender->BufferData( GL_ARRAY_BUFFER, sizeof( Vertex2D ) * m_vertexList.size(), m_vertexList.data(), GL_STATIC_DRAW );

	m_numVertex = m_vertexList.size();
	m_vertexList.clear();
	vector<Vertex2D>().swap(m_vertexList);
}

void Actor::RenderVBO()
{
	glBindTexture(GL_TEXTURE_2D, m_texture->m_openglTextureID );

	g_glRender->Enable( GL_TEXTURE_2D );
	g_glRender->EnableClientState( GL_VERTEX_ARRAY );
	g_glRender->EnableClientState( GL_COLOR_ARRAY );
	g_glRender->EnableClientState( GL_TEXTURE_COORD_ARRAY );
	g_glRender->BindBuffer( GL_ARRAY_BUFFER, m_vboID );

	g_glRender->VertexPointer(2, GL_FLOAT, sizeof(Vertex2D), (const GLvoid*) offsetof( Vertex2D, m_position) );
	g_glRender->ColorPointer(4, GL_FLOAT, sizeof(Vertex2D), (const GLvoid*) offsetof( Vertex2D, m_color) );
	g_glRender->TexCoordPointer(2, GL_FLOAT, sizeof(Vertex2D), (const GLvoid*) offsetof( Vertex2D, m_texCoords) );

	g_glRender->DrawArray( GL_QUADS,0, m_numVertex );

	g_glRender->DisableClientState( GL_VERTEX_ARRAY );
	g_glRender->DisableClientState( GL_COLOR_ARRAY );
	g_glRender->DisableClientState( GL_TEXTURE_COORD_ARRAY );
	g_glRender->Disable( GL_TEXTURE_2D );
	g_glRender->BindBuffer( GL_ARRAY_BUFFER, 0 );
}

void Actor::TurnActor()
{
	if( m_velocity.x < 0 )
	{
		if( m_isFaceRight )
		{
			m_scaleFactor = -1;
			m_isFaceRight = false;
		}
	}
	else
	{
		if( !m_isFaceRight )
		{
			m_scaleFactor = 1;
			m_isFaceRight = true;
		}
	}
}

void Actor::ExploreUnknownTileWithinRadius()
{
	if( !m_actorBlueprint->m_isPlayer )
		return;

	TileCoords2D maxTileCoords = ConvertWorldCoords2DToTileCoords2D( m_worldPosition + Vector2( m_visionRange, m_visionRange ) );
	TileCoords2D minTileCoords = ConvertWorldCoords2DToTileCoords2D( m_worldPosition - Vector2( m_visionRange, m_visionRange ) );

	TileIndex maxTileIndex = ConvertTileCoords2DToTileIndex( maxTileCoords, g_currentMap->m_mapWidthX );
	TileIndex minTileIndex = ConvertTileCoords2DToTileIndex( minTileCoords, g_currentMap->m_mapWidthX );

	int checkRange = maxTileCoords.x - minTileCoords.x;

 	if( minTileIndex < 0  || m_worldPosition.y <= m_visionRange )
	{
 		minTileCoords = ConvertWorldCoords2DToTileCoords2D( m_worldPosition - Vector2( m_visionRange, m_worldPosition.y ) );
		if( minTileCoords.x < 0 )
			minTileCoords.x = 0;
		minTileIndex = ConvertTileCoords2DToTileIndex( minTileCoords, g_currentMap->m_mapWidthX );
	}

	//minTileIndex = ConvertTileCoords2DToTileIndex( minTileCoords, g_currentMap->m_mapWidthX );

	if( maxTileIndex >= g_currentMap->m_mapSize )
		maxTileIndex = g_currentMap->m_mapSize - 1;

	float visionSquare = m_visionRange * m_visionRange;

	for( int y = 0; y <= checkRange; y++ )
	{
		for( int x = minTileIndex; x <= minTileIndex + checkRange ; x++ )
		{
			int tileIndex = x + g_currentMap->m_mapWidthX * y; 

			if( tileIndex >= g_currentMap->m_mapSize )
				tileIndex = g_currentMap->m_mapSize - 1;

			Tile& tile = g_currentMap->m_tileList[tileIndex];
			
			WorldCoords2D tileCenter = Vector2( tile.m_tileCoords.x + 0.5f, tile.m_tileCoords.y + 0.5f );
			float distanceSquareFromPlayerToCenterTile = ( m_worldPosition - tileCenter ).CalcLengthSquare();
			if( distanceSquareFromPlayerToCenterTile <= visionSquare )
			{
				tile.m_isVisibleNow = true;
				g_currentMap->m_isVBOtoDrawTileDirty = true;
			}
		}
	}
}

bool Actor::IsActorInFogOfWar()
{
	TileCoords2D tileCoords = ConvertWorldCoords2DToTileCoords2D( m_worldPosition );
	TileIndex index = ConvertTileCoords2DToTileIndex( tileCoords, g_currentMap->m_mapWidthX );

	if( !g_currentMap->m_tileList[index].m_isVisibleNow )
		return true;
	return false;
}

void Actor::VisualizeAStarPath()
{
	if( m_isCursorOnActor || m_isSelected )
	{
		g_glRender->PointSize( 10.f );
		g_glRender->BeginDraw( GL_POINTS );
		g_glRender->Color4f( 1.f,0.f,0.f,1.f );

		for( unsigned int i = 0; i < m_path.size(); i++ )
		{
			g_glRender->Vertex3f( m_path[i].x, m_path[i].y, 0.f );
		}
		g_glRender->EndDraw();
		g_glRender->PointSize( 1.f );
	}
}

void Actor::UpdateFlag()
{
	for( unsigned int i = 0; i < m_currentFlagList.size(); i++ )
	{

	}
}

bool Actor::ChangeToHuntStrategyIfAnyTargetToAttack()
{
	for( unsigned int actorIdnex = 0; actorIdnex < g_currentMap->m_actorList.size(); actorIdnex++ )
	{
		Actor* checkActor = g_currentMap->m_actorList[actorIdnex];

		if( m_actorBlueprint->m_faction != checkActor->m_actorBlueprint->m_faction )
		{
			if( IsTargetInAwareRange( checkActor ) )
			{
				m_target = checkActor;
				HuntingStrategy* huntStrategy = new HuntingStrategy( this, g_currentMap );
				delete m_strategy;
				m_strategy = nullptr;
				m_strategy = huntStrategy;
				return true;
			}
		}
	}
	return false;
}

bool Actor::ChangeToNonOffensiveStrategy()
{
	m_target = nullptr;
	m_strategy = nullptr;
	if( !m_actorBlueprint->m_isPlayer )
	{
		AttentiveStrategy* attentiveStrategy = new AttentiveStrategy( this, g_currentMap ); 
		delete m_strategy;
		m_strategy = nullptr;
		m_strategy = attentiveStrategy;
		return true;
	}
	else
	{
		InactiveStrategy* inactiveStrategy = new InactiveStrategy( this, g_currentMap ); 
		delete m_strategy;
		delete m_strategy;
		m_strategy = nullptr;
		m_strategy = inactiveStrategy;
		return true;
	}
}








