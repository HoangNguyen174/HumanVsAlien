#include "Construct.hpp"
#include "World.hpp"
#include "SoulStoneEngine/JobSystem/MemoryPoolManager.hpp"

int Construct::s_ID = 0;

Construct::Construct(const std::string& blueprintName, const WorldCoords2D& worldPosition)
{
	s_ID++;
	m_constructID = s_ID;
	m_worldPosition = worldPosition;
	std::map<std::string, ConstructBlueprint*>::iterator constructBpIter;
	
	constructBpIter = ConstructBlueprint::s_registeredConstructBlueprint.find( blueprintName );
	m_isVBODirty = true;
	m_numVertex = 0;
	m_vboID = 0;

	if( constructBpIter != ConstructBlueprint::s_registeredConstructBlueprint.end() )
	{
		ConstructBlueprint* blueprint = constructBpIter->second;
		m_constructBlueprint = blueprint;
		PopulateFromBlueprint( *blueprint );
	}
	else
	{
		DebuggerPrintf( "Cannot Find Construct with name %s.\n", blueprintName.c_str() );
		exit(0);
	}
}

void Construct::PopulateFromBlueprint(const ConstructBlueprint& blueprint)
{
	m_radius = blueprint.m_radius;
	m_isOverlappable = blueprint.m_isOverlappable;
	m_texture = Texture::CreateOrGetTexture( "./Data/Texture/" + blueprint.m_textureSheet );
	for( unsigned int i = 0; i < blueprint.m_abilityList.size(); i++ )
	{
		Ability* ability = new Ability( blueprint.m_abilityList[i]->name, nullptr );
		ability->m_worldPosition = m_worldPosition;
		ability->m_radius += m_radius;
		m_abilityList.push_back(ability);
	}
}

void Construct::CreateVBO()
{
	float texFrameWidthX = 1.f;
	float texFrameHeightY = 1.f;
	Vertex2D vertex;
	Vector2 texCoords;

	Vector2 itemTopLeftTexCoords;
	itemTopLeftTexCoords.x = (float)m_constructBlueprint->m_textureTileCoords.x * texFrameWidthX;
	itemTopLeftTexCoords.y = (float)m_constructBlueprint->m_textureTileCoords.y * texFrameHeightY;

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
	g_glRender->BindBuffer( GL_ARRAY_BUFFER, 0 );

	m_numVertex = m_vertexList.size();
	m_vertexList.clear();
	vector<Vertex2D>().swap(m_vertexList);
	m_isVBODirty = false;
}

void Construct::RenderVBO()
{
	glBindTexture(GL_TEXTURE_2D, m_texture->m_openglTextureID );
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
}

void Construct::Render()
{
	if( m_isVBODirty )
		CreateVBO();
	RenderVBO();

	for( unsigned int i = 0; i < m_abilityList.size(); i++ )
		m_abilityList[i]->RenderEffect();
}

void Construct::Update( float elapsedTime )
{
	for( unsigned int i = 0; i < m_abilityList.size(); i++ )
		m_abilityList[i]->ActivateAbility( elapsedTime );
}
