#include "Ability.hpp"
#include "World.hpp"
#include "SoulStoneEngine/Utilities/Time.hpp"
#include "Actor.hpp"
#include "SoulStoneEngine/JobSystem/MemoryPoolManager.hpp"

int Ability::s_ID = 0;

Ability::Ability( const std::string& blueprintName, Actor* owner )
{
	s_ID++;
	m_abilityID = s_ID;
	m_isSelected = false;
	m_isSelected = true;
	m_target = nullptr;
	m_actorOwner = owner;
	m_isCasting = false;
	m_isActive = false;

	std::map<std::string, AbilityBlueprint*>::iterator bpIter;
	bpIter = AbilityBlueprint::s_registeredAbilityBlueprint.find( blueprintName );

	if( bpIter != AbilityBlueprint::s_registeredAbilityBlueprint.end() )
	{
		AbilityBlueprint* blueprint = bpIter->second;
		m_abilityBlueprint = blueprint;
		PopulateFromBlueprint( *blueprint );
	}
	else
	{
		DebuggerPrintf( "Cannot Find Ability with name %s.\n", blueprintName.c_str() );
		exit(0);
	}
}

void Ability::PopulateFromBlueprint( const AbilityBlueprint& blueprint )
{
	m_iconTexture = Texture::CreateOrGetTexture( "./Data/Texture/" + blueprint.m_textureSheet );
	m_radius = blueprint.m_radius;
	m_removeFlag = blueprint.m_removeFlag;
	m_addFlag = blueprint.m_addFlag;
	m_coolDown = blueprint.m_coolDown;
	m_energyCost = blueprint.m_eneryCost;
	m_remainingTime = m_coolDown;
	m_soundNameFullPath = "./Data/Sound/" + blueprint.m_soundName;

	m_effectSprite = new Sprite( "./Data/Texture/" + blueprint.m_effectTexture );
	int width =  blueprint.m_effectFrameWidth;
	int height = blueprint.m_effectFrameHeight;
	int size = width * height;
	m_effectSprite->SetAttributes( Vector2(0.f,0.f),0.f, width , height, size, blueprint.m_effectTexWidth, blueprint.m_effectTexHeight );
	m_effectSprite->m_color = RGBColor::White();
	m_effectSprite->m_scaleX = blueprint.m_drawEffectRadius;
	m_effectSprite->m_scaleY = blueprint.m_drawEffectRadius;

}

void Ability::ActivateAbility( float /*elapsedTime*/ )
{
	if( !m_isActive || !CanCastYet() )
		return;

	if( m_actorOwner != nullptr )
		if( m_actorOwner->m_energy < m_energyCost )
			return;

	g_audioSystem->PlaySoundByName( m_soundNameFullPath.c_str(),1,false );

	m_timeSinceLastCast = static_cast<float>( GetCurrentTimeSeconds() );

	if( m_actorOwner != nullptr )
	{
		m_actorOwner->m_energy -= m_energyCost;
	}

	m_isCasting = true;

	m_effectSprite->m_position = m_worldPosition;
	m_effectSprite->m_isExpired = false;
	m_effectSprite->m_isAnimate = true;

	//activate area spell
	if( !m_abilityBlueprint->m_isTargetSpell )
	{
		for( unsigned int i = 0; i < g_currentMap->m_actorList.size(); i++ )
		{
			Actor* actor = g_currentMap->m_actorList[i];
			WorldCoords2D actorPosition = actor->m_worldPosition;
			//TileIndex tileIndex = ConvertWorldCoords2DToTileIndex( actorPosition, g_currentMap->m_mapWidthX );

			float distanceSquare = ( actorPosition - m_worldPosition ).CalcLengthSquare();

			if( distanceSquare < m_radius * m_radius )
			{
				if( m_abilityBlueprint->m_damageTarget < 0)
				{
					if( actor->m_health < actor->m_maximumHealth )
						actor->m_health -= m_abilityBlueprint->m_damageTarget;
				}
				else
				{
					if( !actor->m_actorBlueprint->m_isPlayer)
						actor->m_health -= m_abilityBlueprint->m_damageTarget;
				}
				
				//add flag
				if( m_addFlag != NONE )
					actor->m_currentFlagList.insert( m_addFlag );

				//remove flag
				//......
			}
		}
	}

// 	m_effectSprite->Update(elapsedTime);
// 
// 	if( m_effectSprite->m_isExpired )
// 	{
// 		m_isActive = false;
// 		m_isCasting = false;
// 	}
}

void Ability::RenderCursor( const WorldCoords2D& pos )
{
	g_glRender->Draw2DHollowCircle( pos, m_radius, RGBColor::Red() );
}

void Ability::RenderEffect()
{
	if( !m_effectSprite->m_isExpired && m_actorOwner != nullptr /*m_isCasting && m_actorOwner != nullptr*/ )
	{
		//g_glRender->Draw2DFilledCircle( m_worldPosition, m_radius, RGBColor( 0.f,1.f,0.f,0.2f) );
		//m_activateSuccessfully = false;
		m_effectSprite->Render();
	}
}

void Ability::RenderIcon()
{
	if( m_isVBODirty )
		CreateVBO();
	RenderVBO();
}

void Ability::CreateVBO()
{
	std::vector<Vertex2D> m_vertexList;
	float texFrameWidthX = 1.f / 12.f;
	float texFrameHeightY = 1.f / 12.f;
	Vertex2D vertex;
	Vector2 texCoords;

	Vector2 itemTopLeftTexCoords;
	itemTopLeftTexCoords.x = (float)m_abilityBlueprint->m_textureTileCoords.x * texFrameWidthX;
	itemTopLeftTexCoords.y = (float)m_abilityBlueprint->m_textureTileCoords.y * texFrameHeightY;

	vertex.m_color = RGBColor( 1.f, 1.f,1.f, 1.f );
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
	m_isVBODirty = false;
}

void Ability::RenderVBO()
{
	glBindTexture(GL_TEXTURE_2D, m_iconTexture->m_openglTextureID );
	g_glRender->BindBuffer( GL_ARRAY_BUFFER, m_vboID );

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

bool Ability::CanCastYet()
{
	float secondsSinceLastCast = static_cast<float>( GetCurrentTimeSeconds() ) - m_timeSinceLastCast;
	if( secondsSinceLastCast >= m_coolDown )
		return true;
	else
		return false;
}

void Ability::Update(float elapsedTime)
{
	if( !m_effectSprite->m_isExpired )
	{
		m_effectSprite->Update(elapsedTime);
	}
	else
	{
		m_isActive = false;
		m_isCasting = false;
	}

	if( static_cast<float>( GetCurrentTimeSeconds() ) - m_timeSinceLastCast <= m_coolDown )
	{
		m_remainingTime = static_cast<float>( GetCurrentTimeSeconds() ) - m_timeSinceLastCast;
	}
}
