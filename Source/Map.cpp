#include "Map.hpp"
#include "World.hpp"
#include "SoulStoneEngine/Utilities/ProfileSection.hpp"

std::vector<Actor*> Map::s_actorWaitingForPathList;

Map::Map( const std::string& blueprintName )
{
	m_visualizeMapInfo = false;

	std::map<std::string, MapBlueprint*>::iterator mapbpIter;

	mapbpIter = MapBlueprint::s_registeredMapBlueprint.find( blueprintName );

	if( mapbpIter != MapBlueprint::s_registeredMapBlueprint.end() )
	{
		MapBlueprint* blueprint = mapbpIter->second;
		m_mapBlueprint = blueprint;
		PopulateFromBluePrint( *blueprint );
	}
	else
	{
		DebuggerPrintf( "Cannot Find Map with name %s.\n", blueprintName.c_str() );
		exit(0);
	}

	m_vboIDOfTileArray = 0;
	m_isVBOtoDrawTileDirty = true;
	SetRandomStartPositionForPlayer();
	AllocateDijkstraMap();
}

Map::Map( const std::string& mapLayoutFilePath, const std::string& mapDetailFilePath )
{	
	std::string soundRoot = "./Data/Sound/";
	m_visualizeMapInfo = false;
	LoadMapLayOutFromFile( mapLayoutFilePath );
	LoadMapDetailFromFile( mapDetailFilePath );

	if( m_backgroundMusic.compare( "NoMusic" ) != 0 )
		m_backgroundMusic = soundRoot + m_backgroundMusic;

	m_vboIDOfTileArray = 0;
	m_isVBOtoDrawTileDirty = true;
	for( unsigned int i = 0; i < m_actorList.size(); i++ )
	{
		if( m_actorList[i]->m_actorBlueprint->m_isPlayer )
			m_startPositionList.push_back( m_actorList[i]->m_worldPosition );
	}
	AllocateDijkstraMap();
}


void Map::AllocateDijkstraMap()
{
	m_dijkstraMapFromPlayer.resize( m_mapSize );
	m_safetyMapFromPlayer.resize( m_mapSize );
	m_dijkstraMapFromAlienShip.resize( m_mapSize );
}

void Map::PopulateFromBluePrint(const MapBlueprint& blueprint)
{
	std::string soundRoot = "./Data/Sound/";
	m_mapWidthX = blueprint.m_mapWidth;
	m_mapHeightY = blueprint.m_mapHeight;
	m_mapSize = m_mapWidthX * m_mapHeightY;
	m_backgroundMusic = blueprint.m_backgroundMusic;

	if( m_backgroundMusic.compare( "NoMusic") != 0 )
	{
		m_backgroundMusic = soundRoot + m_backgroundMusic;
	}

	if( blueprint.m_name.compare( "Cave" ) != 0 )
		GenerateRoomLikeSubMap();
	else
		GenerateCaveLikeSubMap();
	PlaceRandomEnemyOnTheMap( blueprint );
}

Map::~Map()
{
	g_glRender->DeleteBuffers( 1, &m_vboIDOfTileArray );
}

void Map::LoadMapDetailFromFile( const std::string& mapFilePath )
{
	XMLLoaderUtilities mapXML( mapFilePath );

	tinyxml2::XMLElement* rootElement = mapXML.m_root;

	mapXML.ValidateXMLChildElements( rootElement, "", "Actor,Item,Construct,BackgroundMusic" );
	mapXML.ValidateXMLAttributes( rootElement, "", "" );

	tinyxml2::XMLElement* bgMusicElement = rootElement->FirstChildElement("BackgroundMusic");
	if( bgMusicElement != nullptr )
	{
		m_backgroundMusic = mapXML.GetStringXMLAttribute( bgMusicElement, "name", "NoMusic" );
	}
	else
		m_backgroundMusic = "NoMusic";

	for( tinyxml2::XMLElement* actorElement = rootElement->FirstChildElement("Actor"); actorElement != nullptr; actorElement = actorElement->NextSiblingElement("Actor") )
	{
		mapXML.ValidateXMLChildElements( actorElement, "", "" );
		mapXML.ValidateXMLAttributes( actorElement, "name,worldCoords", "" );
		WorldCoords2D worldCoords = XMLLoaderUtilities::GetVector2XMLAttribute( actorElement, "worldCoords", Vector2( 0, 0 ) );
		std::string actorName = actorElement->Attribute("name");
		Actor* temp = new Actor( actorName, worldCoords );
		m_actorList.push_back( temp );
	}

	for( tinyxml2::XMLElement* itemElement = rootElement->FirstChildElement("Item"); itemElement != nullptr; itemElement = itemElement->NextSiblingElement("Item") )
	{
		mapXML.ValidateXMLChildElements( itemElement, "", "" );
		mapXML.ValidateXMLAttributes( itemElement, "name,worldCoords", "" );
		WorldCoords2D worldCoords = XMLLoaderUtilities::GetVector2XMLAttribute( itemElement, "worldCoords", Vector2( 0, 0 ) );
		std::string itemName = itemElement->Attribute("name");
		Item* temp = new Item( itemName, worldCoords );
		m_itemList.push_back( temp );
	}

	for( tinyxml2::XMLElement* constructElement = rootElement->FirstChildElement("Construct");  constructElement != nullptr;  constructElement =  constructElement->NextSiblingElement("Construct") )
	{
		mapXML.ValidateXMLChildElements(  constructElement, "", "" );
		mapXML.ValidateXMLAttributes(  constructElement, "name,worldCoords", "" );
		WorldCoords2D worldCoords = XMLLoaderUtilities::GetVector2XMLAttribute(  constructElement, "worldCoords", Vector2( 0, 0 ) );
		std::string constructName = constructElement->Attribute("name");
		Construct* temp = new Construct( constructName, worldCoords );
		m_constructList.push_back( temp );
	}
}

void Map::LoadMapLayOutFromFile(const std::string& mapFilePath)
{
	XMLLoaderUtilities mapXML( mapFilePath );

	tinyxml2::XMLElement* rootElement = mapXML.m_root;

	mapXML.ValidateXMLChildElements( rootElement, "Row", "" );
	mapXML.ValidateXMLAttributes( rootElement, "name", "" );

	m_name = XMLLoaderUtilities::GetStringXMLAttribute( rootElement, "name", "Error" );

	int rowCount = 0;
	tinyxml2::XMLElement* firstRowEle = rootElement->FirstChildElement("Row");
	std::string firstRow = XMLLoaderUtilities::GetStringXMLAttribute( firstRowEle, "info", "" );
	for( tinyxml2::XMLElement* rowElement = rootElement->FirstChildElement("Row"); rowElement != nullptr; rowElement = rowElement->NextSiblingElement("Row") )
	{
		rowCount++;
	}

	m_mapWidthX = firstRow.size();
	m_mapHeightY = rowCount;
	m_mapSize = m_mapWidthX * m_mapHeightY;
	m_tileList.resize( m_mapWidthX * m_mapHeightY );
	int heightYIndex = 0;
	for( tinyxml2::XMLElement* rowElement = rootElement->FirstChildElement("Row"); rowElement != nullptr; rowElement = rowElement->NextSiblingElement("Row") )
	{
		std::string widthX = XMLLoaderUtilities::GetStringXMLAttribute( rowElement ,"info", "" );
		for( unsigned int widthXIndex = 0; widthXIndex < widthX.size(); widthXIndex++ )
		{
			char tileAbb = widthX[widthXIndex];

			std::string tileName;
			std::map< std::string, TileBlueprint* >::iterator iter;
			TileCoords2D tileCoords = TileCoords2D( widthXIndex, m_mapHeightY - 1 - heightYIndex );
			for( iter = TileBlueprint::s_registeredTileBlueprint.begin(); iter != TileBlueprint::s_registeredTileBlueprint.end(); ++iter )
			{
				TileBlueprint* tileBp = iter->second;
				if( tileBp->m_abb[0] == tileAbb )
				{
					tileName = tileBp->m_name;
					Tile tile( tileName, tileCoords );
					int index = ConvertTileCoords2DToTileIndex( tileCoords, m_mapWidthX );
					m_tileList[index] = tile;
				}
			}
		}
		heightYIndex++;
	}
	
}

void Map::LoadTexture( const std::string& texType, const std::string& filePath, int, int )
{
	if( texType.compare( "Tile" ) == 0 )
	{
		m_tileTexture = Texture::CreateOrGetTexture( filePath );
	}
	else if( texType.compare( "Actor" ) == 0 )
	{
		m_actorTexture = Texture::CreateOrGetTexture( filePath );
	}
}

void Map::CreateVertexArrayForTiles()
{
	float texFrameWidthX = 1.f / TILETEXTURE_WIDTH_IN_FRAME;
	float texFrameHeightY = 1.f / TILETEXTURE_HEIGHT_IN_FRAME;

	float tileWidthX = 1.f;
	float tileHeightY = 1.f;
	Vertex2D vertex;
	Vector2 texCoords;

	for( unsigned int i = 0; i < m_tileList.size(); i++ )
	{
		Tile& tile = m_tileList[i];
		TileCoords2D tileTileCoords = tile.m_tileCoords;
		WorldCoords2D tileBottomLeftWorldCoords = ConvertTileCoords2DToWorldCoords2D( tileTileCoords );
		tileBottomLeftWorldCoords.x *= tileWidthX;
		tileBottomLeftWorldCoords.y *= tileHeightY;
		Vector2 tileTopLeftTexCoords;
		tileTopLeftTexCoords.x = (float)tile.m_tileBlueprint->m_textureTileCoords.x * texFrameWidthX;
		tileTopLeftTexCoords.y = (float)tile.m_tileBlueprint->m_textureTileCoords.y * texFrameHeightY;

		//add vertex to vertex list
		if( tile.m_isVisibleNow )
			vertex.m_color = RGBColor::White();
		else if( tile.m_isVisibleLastTime )
			vertex.m_color = RGBColor( 1.f, 1.f, 1.f, 0.5f );
		else 
			vertex.m_color = RGBColor::Black();

		if( g_isFOWoff )
			vertex.m_color = RGBColor::White();

		//bottom left
		vertex.m_position = tileBottomLeftWorldCoords;
		vertex.m_texCoords = Vector2( tileTopLeftTexCoords.x, tileTopLeftTexCoords.y + texFrameHeightY );
		m_vertexListToDrawTile.push_back( vertex );

		//bottom right
		vertex.m_position = Vector2( tileBottomLeftWorldCoords.x + tileWidthX, tileBottomLeftWorldCoords.y );
		vertex.m_texCoords = Vector2( tileTopLeftTexCoords.x + texFrameWidthX, tileTopLeftTexCoords.y + texFrameHeightY );
		m_vertexListToDrawTile.push_back( vertex );

		//top right
		vertex.m_position = Vector2( tileBottomLeftWorldCoords.x + tileWidthX, tileBottomLeftWorldCoords.y + tileHeightY );
		vertex.m_texCoords = Vector2( tileTopLeftTexCoords.x + texFrameWidthX, tileTopLeftTexCoords.y );
		m_vertexListToDrawTile.push_back( vertex );

		//top left
		vertex.m_position = Vector2( tileBottomLeftWorldCoords.x, tileBottomLeftWorldCoords.y + tileHeightY);
		vertex.m_texCoords = tileTopLeftTexCoords;
		m_vertexListToDrawTile.push_back( vertex );
	}

	if( m_vboIDOfTileArray == 0 )
	{
		g_glRender->GenerateBuffer( 1, &m_vboIDOfTileArray );
	}

	g_glRender->BindBuffer( GL_ARRAY_BUFFER, m_vboIDOfTileArray );
	g_glRender->BufferData( GL_ARRAY_BUFFER, sizeof( Vertex2D ) * m_vertexListToDrawTile.size(), m_vertexListToDrawTile.data(), GL_STATIC_DRAW );

	m_isVBOtoDrawTileDirty = false;
	m_numVertexToDrawTiles = m_vertexListToDrawTile.size();
	m_vertexListToDrawTile.clear();
	vector<Vertex2D>().swap(m_vertexListToDrawTile);
}

void Map::RenderTiles()
{
	glBindTexture(GL_TEXTURE_2D, m_tileTexture->m_openglTextureID );

	g_glRender->Enable( GL_TEXTURE_2D );
	g_glRender->EnableClientState( GL_VERTEX_ARRAY );
	g_glRender->EnableClientState( GL_COLOR_ARRAY );
	g_glRender->EnableClientState( GL_TEXTURE_COORD_ARRAY );
	g_glRender->BindBuffer( GL_ARRAY_BUFFER, m_vboIDOfTileArray );

	g_glRender->VertexPointer(2, GL_FLOAT, sizeof(Vertex2D), (const GLvoid*) offsetof( Vertex2D, m_position) );
	g_glRender->ColorPointer(4, GL_FLOAT, sizeof(Vertex2D), (const GLvoid*) offsetof( Vertex2D, m_color) );
	g_glRender->TexCoordPointer(2, GL_FLOAT, sizeof(Vertex2D), (const GLvoid*) offsetof( Vertex2D, m_texCoords) );

	g_glRender->DrawArray( GL_QUADS,0, m_numVertexToDrawTiles );

	g_glRender->DisableClientState( GL_VERTEX_ARRAY );
	g_glRender->DisableClientState( GL_COLOR_ARRAY );
	g_glRender->DisableClientState( GL_TEXTURE_COORD_ARRAY );
	g_glRender->Disable( GL_TEXTURE_2D );
	g_glRender->BindBuffer( GL_ARRAY_BUFFER, 0 );
}

void Map::RenderActors()
{
	for( unsigned int i = 0; i < m_actorList.size(); i++ )
	{
		m_actorList[i]->Render();
	}
}

void Map::RenderItems()
{
	for( unsigned int i = 0; i < m_itemList.size(); i++ )
	{
		TileIndex index = ConvertWorldCoords2DToTileIndex( m_itemList[i]->m_worldPosition, m_mapWidthX );
		if( m_tileList[index].m_isVisibleNow || g_isFOWoff )
			m_itemList[i]->Render();
	}
}

void Map::RenderConstructs()
{
	for( unsigned int i = 0; i < m_constructList.size(); i++ )
	{
		TileIndex index = ConvertWorldCoords2DToTileIndex( m_constructList[i]->m_worldPosition, m_mapWidthX );
		if( m_tileList[index].m_isVisibleNow || g_isFOWoff )
			m_constructList[i]->Render();
	}
}

void Map::ChangeTypeOfSingleTile( const std::string& newTileType, const TileCoords2D& tileCoords )
{
	TileIndex tileIndex = ConvertTileCoords2DToTileIndex( tileCoords, m_mapWidthX );
	std::map< std::string, TileBlueprint* >::iterator it = TileBlueprint::s_registeredTileBlueprint.find( newTileType );
	if( it != TileBlueprint::s_registeredTileBlueprint.end() )
	{
		TileBlueprint* bpt = it->second;
		m_tileList[tileIndex].m_tileBlueprint = bpt;
	}
	else
	{
		DebuggerPrintf( "Tile type %s does not exist.\n", newTileType.c_str() );
		exit(0);
	}
}

void Map::SpawnNewActor( const std::string& name, const WorldCoords2D& position )
{
	std::map< std::string, ActorBlueprint* >::iterator it;
	it = ActorBlueprint::s_registeredActorBlueprint.find( name );

	if( it != ActorBlueprint::s_registeredActorBlueprint.end() )
	{
		Actor* temp = new Actor( it->first, position );
		m_actorList.push_back( temp );
	}
}



void Map::CalculateAStarPath( const WorldCoords2D& startPosition, const WorldCoords2D& goalPosition, std::vector<WorldCoords2D>& path, bool knowAboutMap, int* /*dijkstraMapPointer*/ )
{
	static ProfileSection AStarProfile( "A Star", TEST );
	AStarProfile.StartProfileSection();

	path.clear();
	m_openNodeList.clear();
	m_closedNodeList.clear();

	unsigned int i = 0;
	std::set<PathNode*>::iterator nodeIter;

	TileCoords2D goalTilePosition = ConvertWorldCoords2DToTileCoords2D( goalPosition );

	PathNode* currentNode = new PathNode;
	currentNode->parent = nullptr;
	currentNode->tileCoords = ConvertWorldCoords2DToTileCoords2D( startPosition );
	currentNode->gCost = 0.f;
	currentNode->hCost = 0;
	currentNode->fCost = 0.f;

	m_openNodeList.insert( currentNode );

	bool stopSearch = false;
	PathNode* returnNode = nullptr;

	while( m_openNodeList.size() > 0 && !stopSearch )
	{
		PathNode* nodeWithSmallestF = GetPathNodeWithSmallestfCostAndPopOutOfOpenList( m_openNodeList );

		if ( nodeWithSmallestF->tileCoords == goalTilePosition )
		{
			stopSearch = true;
			returnNode = nodeWithSmallestF;
			break;
		}

		nodeIter = m_openNodeList.begin();

		while( nodeIter != m_openNodeList.end() )
		{
			PathNode* openNode = *nodeIter;
			if( openNode->tileCoords == nodeWithSmallestF->tileCoords )
			{
				nodeIter = m_openNodeList.erase( nodeIter );
			}
			else
			{
				++nodeIter;
			}
		}

		PathNode* northNode = GetNeighborNodeByCardinalDirection( nodeWithSmallestF, NORTH_CARDINAL_DIRECTION, goalTilePosition, knowAboutMap );
		PathNode* southNode = GetNeighborNodeByCardinalDirection( nodeWithSmallestF, SOUTH_CARDINAL_DIRECTION, goalTilePosition, knowAboutMap );
		PathNode* eastNode = GetNeighborNodeByCardinalDirection( nodeWithSmallestF, EAST_CARDINAL_DIRECTION, goalTilePosition, knowAboutMap );
		PathNode* westNode = GetNeighborNodeByCardinalDirection( nodeWithSmallestF, WEST_CARDINAL_DIRECTION, goalTilePosition, knowAboutMap );
// 		PathNode* northEastNode = GetNeighborNodeByCardinalDirection( nodeWithSmallestF, NORTH_EAST_SEMICARDINAL_DIRECTION, goalTilePosition, knowAboutMap );
// 		PathNode* northWestNode = GetNeighborNodeByCardinalDirection( nodeWithSmallestF, NORTH_WEST_SEMICARDINAL_DIRECTION, goalTilePosition, knowAboutMap );
// 		PathNode* southEastNode = GetNeighborNodeByCardinalDirection( nodeWithSmallestF, SOUTH_EAST_SEMICARDINAL_DIRECTION, goalTilePosition, knowAboutMap );
// 		PathNode* southWestNode = GetNeighborNodeByCardinalDirection( nodeWithSmallestF, SOUTH_WEST_SEMICARDINAL_DIRECTION, goalTilePosition, knowAboutMap );
		PathNode* childNodeList[4];
		childNodeList[0] = northNode;
		childNodeList[1] = southNode;
		childNodeList[2] = eastNode;
		childNodeList[3] = westNode;
// 		childNodeList[4] = northEastNode;
// 		childNodeList[5] = northWestNode;
// 		childNodeList[6] = southEastNode;
// 		childNodeList[7] = southWestNode;

		for( i = 0; i < 4 ; i++ )
		{
			bool isNeighbourInOpenList = false;
			bool isNeighbourInClosedList = false;
			
			// child list is solid and is seen by player, skip
 			if( childNodeList[i]->gCost < 0.f )
			{
				delete childNodeList[i];
				continue;
			}

			for( nodeIter = m_openNodeList.begin(); nodeIter != m_openNodeList.end(); ++nodeIter )
			{
				PathNode* openNode = *nodeIter;
				if( childNodeList[i]->tileCoords == openNode->tileCoords && 
					childNodeList[i]->fCost > openNode->fCost )
				{
					isNeighbourInOpenList = true;
				}
			}

			for( nodeIter = m_closedNodeList.begin(); nodeIter != m_closedNodeList.end(); ++nodeIter )
			{
				PathNode* closedNode = *nodeIter;
				if( childNodeList[i]->tileCoords == closedNode->tileCoords && 
					childNodeList[i]->fCost > closedNode->fCost )
				{
					isNeighbourInClosedList = true;
				}
			}

			if( !isNeighbourInOpenList && !isNeighbourInClosedList )
			{
				m_openNodeList.insert( childNodeList[i] );
			}
		}
		
		m_closedNodeList.insert( nodeWithSmallestF );
	}

	while( returnNode != nullptr )
	{
		WorldCoords2D centerOfTile = ConvertTileCoords2DToWorldCoords2D( returnNode->tileCoords ) + Vector2( 0.5f, 0.5f );
		path.push_back( centerOfTile );
		returnNode = returnNode->parent;
	}
	
	path.insert( path.begin(), goalPosition );

	if( path.size() > 0 )
		path.erase( path.begin() + path.size() - 1 );

	for( nodeIter = m_openNodeList.begin(); nodeIter != m_openNodeList.end(); ++nodeIter )
	{
		PathNode* openNode = *nodeIter;
		delete openNode;
	}

	for( nodeIter = m_closedNodeList.begin(); nodeIter != m_closedNodeList.end(); ++nodeIter )
	{
		PathNode* closedNode = *nodeIter;
		delete closedNode;
	}

	m_openNodeList.clear();
	m_closedNodeList.clear();

	AStarProfile.EndProfileSection();
}

bool Map::IsTileSolid( const TileCoords2D& tile )
{
	for( unsigned int i = 0; i < m_tileList.size(); i++ )
	{
		if( tile == m_tileList[i].m_tileCoords )
		{
			if( m_tileList[i].m_tileBlueprint->m_isSolid == true )
				return true;
		}
	}
	return false;
}

PathNode* Map::GetPathNodeWithSmallestfCostAndPopOutOfOpenList( std::set<PathNode*>& nodeList )
{
	float smallest = 99999.f;
	PathNode* nodeWithSmallestF = nullptr;

	std::set<PathNode*>::iterator nodeIter;

	for( nodeIter = nodeList.begin(); nodeIter != nodeList.end(); ++nodeIter )
	{
		PathNode* node = *nodeIter;
		if( node->fCost < smallest )
		{
			smallest = node->fCost;
			nodeWithSmallestF = node;
		}
	}

	return nodeWithSmallestF;
}

PathNode* Map::GetNeighborNodeByCardinalDirection( PathNode* parentNode, CARDINAL_DIRECTION direction, const TileCoords2D& goalTile, bool knowAboutMap )
{
	PathNode* returnNode = new PathNode;
	PathNode* node;

	TileCoords2D parentNodeTileCoords = parentNode->tileCoords;
	float moveCost = 0.f;
	//float gCost = 0.f;//get gCost from tile
	bool isSolid = false;
	bool isSeen = false;
	const float STRAIGHT_COST = 1.f;
	const float DIAGONAL_COST = 999.4f;
	
	if( direction == NORTH_CARDINAL_DIRECTION )
	{
		returnNode->tileCoords = TileCoords2D( parentNodeTileCoords.x, parentNodeTileCoords.y + 1 );
		moveCost = STRAIGHT_COST;
	}
	else if( direction == SOUTH_CARDINAL_DIRECTION )
	{
		returnNode->tileCoords = TileCoords2D( parentNodeTileCoords.x, parentNodeTileCoords.y - 1 );
		moveCost = STRAIGHT_COST;
	}
	else if( direction == EAST_CARDINAL_DIRECTION )
	{
		returnNode->tileCoords = TileCoords2D( parentNodeTileCoords.x + 1, parentNodeTileCoords.y );
		moveCost = STRAIGHT_COST;
	}
	else if( direction == WEST_CARDINAL_DIRECTION )
	{
		returnNode->tileCoords = TileCoords2D( parentNodeTileCoords.x - 1, parentNodeTileCoords.y );
		moveCost = STRAIGHT_COST;
	}
	else if( direction == NORTH_EAST_SEMICARDINAL_DIRECTION )
	{
		returnNode->tileCoords = TileCoords2D( parentNodeTileCoords.x + 1, parentNodeTileCoords.y + 1 );
		moveCost = DIAGONAL_COST;
	}
	else if( direction == NORTH_WEST_SEMICARDINAL_DIRECTION )
	{
		returnNode->tileCoords = TileCoords2D( parentNodeTileCoords.x - 1, parentNodeTileCoords.y + 1 );
		moveCost = DIAGONAL_COST;
	}
	else if( direction == SOUTH_EAST_SEMICARDINAL_DIRECTION )
	{
		returnNode->tileCoords = TileCoords2D( parentNodeTileCoords.x + 1, parentNodeTileCoords.y - 1);
		moveCost = DIAGONAL_COST;
	}
	else if( direction == SOUTH_WEST_SEMICARDINAL_DIRECTION )
	{
		returnNode->tileCoords = TileCoords2D( parentNodeTileCoords.x - 1, parentNodeTileCoords.y - 1 );
		moveCost = DIAGONAL_COST;
	}

	for( unsigned int i = 0; i < m_tileList.size(); i++ )
	{
		if( returnNode->tileCoords == m_tileList[i].m_tileCoords )
		{
			isSolid = m_tileList[i].m_tileBlueprint->m_isSolid;
			if( m_tileList[i].m_isVisibleLastTime || m_tileList[i].m_isVisibleNow )
			{
				isSeen = true;
			}
		}
	}

	returnNode->parent = parentNode;
	returnNode->hCost = float( abs( goalTile.x - returnNode->tileCoords.x ) + abs( goalTile.y - returnNode->tileCoords.y ) );

	if( !knowAboutMap )
	{
		if( isSolid && isSeen )
		{
			returnNode->gCost = -1.f;
		}
		else 
			returnNode->gCost = parentNode->gCost + moveCost;
	}
	else
	{
		if( isSolid )
		{
			returnNode->gCost = -1.f;
		}
		else 
			returnNode->gCost = parentNode->gCost + moveCost;
	}

	returnNode->fCost = (float)returnNode->hCost + returnNode->gCost;

	node = returnNode;

	return node;
}

void Map::Render()
{
	if( m_isVBOtoDrawTileDirty )
		CreateVertexArrayForTiles();

	RenderTiles();
	RenderConstructs();
	RenderActors();
	RenderItems();

	if( m_visualizeMapInfo )
		VisualizeTileStatus();
}

void Map::Update(float elapsedTime)
{
	CalDijkstraMapFromPlayer();

	for( unsigned int i = 0; i < m_actorList.size(); i++ )
	{
		Actor* actor = m_actorList[i];
		if( !actor->IsDead() )
			actor->Update(elapsedTime);
		else
		{
			g_audioSystem->PlaySoundByName( actor->m_deathSoundName.c_str(),1,false );
			if( !actor->m_actorBlueprint->m_isPlayer )
			{
				if( actor->m_aggroList.size() > 0 )
				{
					float exp = actor->m_expGiveWhenDie / static_cast<float>( actor->m_aggroList.size() );

					for( unsigned int targetIndex = 0;  targetIndex < actor->m_aggroList.size();  targetIndex++ )
					{
						Actor* target = actor->m_aggroList[targetIndex]->actor;
						target->m_currentExp += static_cast<int>(exp);
					}
				}

				SpawnItemWhenActorDie( actor );
			}

			m_actorList.erase( m_actorList.begin() + i );
			for( unsigned int j = 0; j < s_actorWaitingForPathList.size(); j++ )
			{
				Actor* waitingActor = s_actorWaitingForPathList[j];
				if( waitingActor->m_playerID == actor->m_playerID  )
					s_actorWaitingForPathList.erase( s_actorWaitingForPathList.begin() + j );
			}
		}
	}

	for( unsigned int i = 0; i < m_constructList.size(); i++ )
	{
		m_constructList[i]->Update( elapsedTime );
	}

	CheckAndSetIfTileIsAlreadySeenButNotVisibleNow();

	ProcessWaitingForPathList();

	ResolveActorTileCollision();

	ResolveActorActorCollision();

	ResolveActorItemCollision();
}

HitInfo Map::GetImpactedInfoFromRaycast( const WorldCoords2D& rayStartPos, const Vector2& rayDirection, float maxRayLength, const Map& map )
{
	HitInfo returnHitInfo;
	returnHitInfo.distanceToImpactPoint = .1f;

	Vector2 normalRayDir = rayDirection;

	if( normalRayDir.CalcLengthSquare() == 0.f )
	{
		returnHitInfo.faceOfTileImpacted = NONE;
		return returnHitInfo;
	}

	normalRayDir = normalRayDir.Normalize();

	WorldCoords2D currentPos = rayStartPos;
	TileCoords2D currentTile =  ConvertWorldCoords2DToTileCoords2D( currentPos );
	float rayTraveledDistance = 0.f;

	while( rayTraveledDistance <= maxRayLength )
	{
		int xIntercept = INT_MAX;
		int yIntercept = INT_MAX;

		float distanceFromStartToXIntercept = FLT_MAX;
		float distanceFromStartToYIntercept = FLT_MAX;
		float smallestDistanceToAxisIntercept =  FLT_MAX;

		bool crossX = false;
		bool crossY = false;

		WorldCoords2D nextPos = currentPos + normalRayDir;
		TileCoords2D nextTile = ConvertWorldCoords2DToTileCoords2D( nextPos );
		Vector2i tileCoordsDelta = nextTile - currentTile;

		if( tileCoordsDelta.x > 0 )
		{
			xIntercept = currentTile.x + 1;
		}
		else if( tileCoordsDelta.x < 0 )
		{
			xIntercept = currentTile.x;
		}

		if( tileCoordsDelta.y > 0 )
		{
			yIntercept = currentTile.y + 1;
		}
		else if( tileCoordsDelta.y < 0 )
		{
			yIntercept = currentTile.y;
		}

		if( xIntercept != INT_MAX )
		{
			float distanceToXIntercept = ( static_cast<float>(xIntercept) - currentPos.x ) / normalRayDir.x;
			distanceFromStartToXIntercept = distanceToXIntercept; 
		}

		if( yIntercept != INT_MAX )
		{
			float distanceToYIntercept = ( static_cast<float>(yIntercept) - currentPos.y ) / normalRayDir.y;
			distanceFromStartToYIntercept = distanceToYIntercept;
		}

		smallestDistanceToAxisIntercept = MathUtilities::Min2( distanceFromStartToXIntercept, distanceFromStartToYIntercept );

		if( smallestDistanceToAxisIntercept != FLT_MAX )
			currentPos += ( normalRayDir * smallestDistanceToAxisIntercept );
		else
			currentPos += normalRayDir;

		if( distanceFromStartToXIntercept != FLT_MAX )
		{
			crossX = true;
			currentTile.x += tileCoordsDelta.x;
		}
		else if( distanceFromStartToYIntercept != FLT_MAX )
		{
			crossY = true;
			currentTile.y += tileCoordsDelta.y;
		}

		TileIndex currentTileIndex = ConvertTileCoords2DToTileIndex( currentTile, map.m_mapWidthX );
		Tile tile = map.m_tileList[currentTileIndex];

		if( tile.m_tileBlueprint->m_isSolid )
		{
			returnHitInfo.tileCoordsImpacted = currentTile;
			if( crossX )
			{
				if( currentPos.x < nextPos.x )
					returnHitInfo.faceOfTileImpacted = WEST_CARDINAL_DIRECTION;

				if( currentPos.x > nextPos.x )
					returnHitInfo.faceOfTileImpacted = EAST_CARDINAL_DIRECTION;
			}
			else if( crossY  )
			{
				if( currentPos.y < nextPos.y )
					returnHitInfo.faceOfTileImpacted = SOUTH_CARDINAL_DIRECTION ;

				if( currentPos.y > nextPos.y )
					returnHitInfo.faceOfTileImpacted = NORTH_CARDINAL_DIRECTION ;
			}
			returnHitInfo.distanceToImpactPoint = smallestDistanceToAxisIntercept;
			returnHitInfo.pointOfImpact = currentPos;
			return returnHitInfo;
		}
		
		if( smallestDistanceToAxisIntercept != FLT_MAX )
			rayTraveledDistance += smallestDistanceToAxisIntercept;
		else
			rayTraveledDistance += 1.f;
	}
	returnHitInfo.faceOfTileImpacted = NONE;
	return returnHitInfo;
}

void Map::ResolveActorTileCollision()
{
	for( unsigned int i = 0; i < m_actorList.size(); i++ )
	{
		Actor* actor = m_actorList[i];
		WorldCoords2D position = actor->m_worldPosition;
		float radius = actor->m_radius;
		
		WorldCoords2D north = position + Vector2( 0.f, radius);
		WorldCoords2D south = position + Vector2( 0.f,-radius);
		WorldCoords2D west = position + Vector2( -radius, 0.f );
		WorldCoords2D east = position + Vector2( radius, 0.f );
		WorldCoords2D northEast = position + Vector2( radius, radius );
		WorldCoords2D northWest = position + Vector2( -radius, radius );
		WorldCoords2D southEast = position + Vector2( radius, -radius );
		WorldCoords2D southWest = position + Vector2(-radius, -radius);

		TileCoords2D northTileCoords = ConvertWorldCoords2DToTileCoords2D( north );
		TileCoords2D southTileCoords = ConvertWorldCoords2DToTileCoords2D( south );
		TileCoords2D westTileCoords = ConvertWorldCoords2DToTileCoords2D( west );
		TileCoords2D eastTileCoords = ConvertWorldCoords2DToTileCoords2D( east );
		TileCoords2D northEastTileCoords = ConvertWorldCoords2DToTileCoords2D( northEast );
		TileCoords2D northWestTileCoords = ConvertWorldCoords2DToTileCoords2D( northWest );
		TileCoords2D southEastTileCoords = ConvertWorldCoords2DToTileCoords2D( southEast );
		TileCoords2D southWestTileCoords = ConvertWorldCoords2DToTileCoords2D( southWest );

		TileIndex northTileIndex = ConvertTileCoords2DToTileIndex( northTileCoords, m_mapWidthX );
		TileIndex southTileIndex = ConvertTileCoords2DToTileIndex( southTileCoords, m_mapWidthX );
		TileIndex westTileIndex = ConvertTileCoords2DToTileIndex( westTileCoords, m_mapWidthX );
		TileIndex eastTileIndex = ConvertTileCoords2DToTileIndex( eastTileCoords, m_mapWidthX );
		TileIndex northEastTileIndex = ConvertTileCoords2DToTileIndex( northEastTileCoords, m_mapWidthX );
		TileIndex northWestTileIndex = ConvertTileCoords2DToTileIndex( northWestTileCoords, m_mapWidthX );
		TileIndex southEastTileIndex = ConvertTileCoords2DToTileIndex( southEastTileCoords, m_mapWidthX );
		TileIndex southWestTileIndex = ConvertTileCoords2DToTileIndex( southWestTileCoords, m_mapWidthX );
		
		if( m_tileList[northTileIndex].m_tileBlueprint->m_isSolid )
		{
			actor->m_worldPosition.y = northTileCoords.y - radius;
		}
		if( m_tileList[southTileIndex].m_tileBlueprint->m_isSolid )
		{
			actor->m_worldPosition.y = (southTileCoords.y + 1) + radius;
		}
		if( m_tileList[westTileIndex].m_tileBlueprint->m_isSolid )
		{
			actor->m_worldPosition.x = (westTileCoords.x + 1) + radius;
		}
		if( m_tileList[eastTileIndex].m_tileBlueprint->m_isSolid )
		{
			actor->m_worldPosition.x = (eastTileCoords.x ) - radius;
		}

		TileCoords2D corner;
		corner = TileCoords2D( northEastTileCoords.x ,northEastTileCoords.y );
		ConstraintActorPosition( actor,radius, northEastTileIndex,corner );

		corner = TileCoords2D( northWestTileCoords.x + 1, northWestTileCoords.y );
		ConstraintActorPosition( actor,radius, northWestTileIndex,corner );

		corner = TileCoords2D( southEastTileCoords.x, southEastTileCoords.y + 1 );
		ConstraintActorPosition( actor, radius, southEastTileIndex,corner );

		corner = TileCoords2D( southWestTileCoords.x + 1, southWestTileCoords.y + 1 );
		ConstraintActorPosition( actor,radius, southWestTileIndex,corner );
	}
}

void Map::ConstraintActorPosition(Actor* actor, float actorRadius, const TileIndex& tileIndex, const TileCoords2D& corner)
{
	float distanceToCorner;
	Vector2 cornerToCenterVector;
	float depth = 0.f;
	Vector2 displacement;
	float radius = actorRadius;

	if( m_tileList[tileIndex].m_tileBlueprint->m_isSolid )
	{
		cornerToCenterVector = actor->m_worldPosition - Vector2( (float)corner.x, (float)corner.y );
		distanceToCorner = cornerToCenterVector.CalcLength();
		if(distanceToCorner < radius)
		{
			depth = radius - distanceToCorner;
		}
		cornerToCenterVector.SetLength(depth);
		displacement = cornerToCenterVector;
		actor->m_worldPosition += displacement;
	}
}

void Map::ResolveActorActorCollision()
{
	float stiffness = 0.5;
	for( unsigned int i = 0; i < m_actorList.size(); i++ )
	{
		Actor* currentActor = m_actorList[i];

		for( unsigned int j = i + 1; j < m_actorList.size(); j++ )
		{
			Actor* checkedActor = m_actorList[j];
			float distanceBetweenTwoActors = ( currentActor->m_worldPosition - checkedActor->m_worldPosition ).CalcLength();
			float sumOfTwoRadius = currentActor->m_radius + checkedActor->m_radius;
			if( sumOfTwoRadius > distanceBetweenTwoActors )
			{
				float depth =  sumOfTwoRadius - distanceBetweenTwoActors;
				Vector2 displacement = currentActor->m_worldPosition - checkedActor->m_worldPosition;
				displacement = displacement.Normalize();
				
				currentActor->m_worldPosition += displacement * depth * 0.5f * stiffness ; 
				checkedActor->m_worldPosition -= displacement * depth * 0.5f * stiffness ; 

				if( currentActor->m_velocity == Vector2( 0.f, 0.f ) )
					currentActor->m_goalWorldPosition= currentActor->m_worldPosition;

				if( checkedActor->m_velocity == Vector2( 0.f, 0.f ) )
					checkedActor->m_goalWorldPosition= checkedActor->m_worldPosition;
			}
		}
	}
}

void Map::ProcessWaitingForPathList()
{
	if( s_actorWaitingForPathList.size() != 0 )
	{
		bool knowAboutMap;
		Actor* actor = s_actorWaitingForPathList.front();
		if( actor->m_actorBlueprint->m_isPlayer )
			knowAboutMap = false;
		else
			knowAboutMap = true;

		CalculateAStarPath( actor->m_worldPosition, actor->m_goalWorldPosition,actor->m_path, knowAboutMap, nullptr );
		s_actorWaitingForPathList.erase( s_actorWaitingForPathList.begin() );
	}
}

void Map::AddActorToWaitingForPathList(Actor* actor)
{
	for( unsigned int i = 0; i < s_actorWaitingForPathList.size(); i++ )
	{
		Actor* actorInList = s_actorWaitingForPathList[i];
		if( actor->m_playerID == actorInList->m_playerID)
		{
			actorInList = actor;
			return;
		}
	}
	s_actorWaitingForPathList.push_back(actor);
}

void Map::ResolveActorItemCollision()
{
	float toleranceSquare = 0.1f * 0.1f;
	for( unsigned int i = 0; i < m_actorList.size(); i++ )
	{
		Actor* actor = m_actorList[i];
		if( actor->m_actorBlueprint->m_isPlayer )
		{
			for( unsigned int j = 0; j < m_itemList.size(); j++ )
			{
				Item* item = m_itemList[j];
				float distanceSquare = ( item->m_worldPosition - actor->m_worldPosition ).CalcLengthSquare();
				if( distanceSquare <= toleranceSquare )
				{
					actor->m_inventory->m_itemList.push_back( item );
					m_itemList.erase( m_itemList.begin() + j );
				}
			}
		}
	}
}

void Map::CheckAndSetIfTileIsAlreadySeenButNotVisibleNow()
{
	std::vector<Actor*> playerList;

	for( unsigned int i = 0; i < m_actorList.size(); i++ )
	{
		Actor* actor = m_actorList[i];
		if( actor->m_actorBlueprint->m_isPlayer )
			playerList.push_back( actor );
	}

	int notInRangeCounter = 0;

	for( unsigned int i = 0; i < m_tileList.size(); i++ )
	{
		Tile& tile = m_tileList[i];
		if( tile.m_isVisibleNow )
		{
			for( unsigned int playerIndex = 0; playerIndex < playerList.size(); playerIndex++ )
			{
				Actor* player = playerList[playerIndex];
				WorldCoords2D tileCenter = Vector2( tile.m_tileCoords.x + 0.5f, tile.m_tileCoords.y + 0.5f );
				float distanceSquareFromPlayerToCenterTile = ( player->m_worldPosition - tileCenter ).CalcLengthSquare();
				float visionSquare = player->m_visionRange * player->m_visionRange;
				if( distanceSquareFromPlayerToCenterTile > visionSquare )
				{
					notInRangeCounter++;
				}
			}

			if( notInRangeCounter >= static_cast<int>( playerList.size() ) )
			{
				tile.m_isVisibleNow = false;
				tile.m_isVisibleLastTime = true;
				m_isVBOtoDrawTileDirty = true;
			}
			notInRangeCounter = 0;
		}
	}
}

void Map::VisualizeTileStatus()
{
// 	for( unsigned int i = 0; i < m_tileList.size(); i++ )
// 	{
// 		Tile& tile = m_tileList[i];
// 		WorldCoords2D tileCenter = Vector2( tile.m_tileCoords.x + 0.5f, tile.m_tileCoords.y + 0.5f );
// 		std::string status;
// 
// 		if( tile.m_isVisibleNow )
// 			status = "VISIBLE";
// 		else if( tile.m_isVisibleLastTime )
// 			status = "KNOWN";
// 		else
// 			status = "UNKNOWN";
// 		g_glRender->RenderText( tileCenter, RGBColor(0.f,1.f,0.f,1.f), nullptr, nullptr, 0.1f, status );
// 	}

// 	for( unsigned int index = 0; index < m_dijkstraMapFromPlayer.size(); index++ )
// 	{
// 		std::string num = std::to_string( static_cast<long double>( m_dijkstraMapFromPlayer[index] ) );
// 		Vector2 center = Vector2( m_tileList[index].m_tileCoords.x + 0.2f, m_tileList[index].m_tileCoords.y + 0.2f );
// 		g_glRender->RenderText( center, RGBColor::Black(), nullptr, nullptr, 0.3f, num );
// 	}

// 	for( unsigned int index = 0; index < m_safetyMapFromPlayer.size(); index++ )
// 	{
// 		std::string num = std::to_string( static_cast<long double>( m_safetyMapFromPlayer[index] ) );
// 		Vector2 center = Vector2( m_tileList[index].m_tileCoords.x + 0.2f, m_tileList[index].m_tileCoords.y + 0.2f );
// 		g_glRender->RenderText( center, RGBColor::Black(), nullptr, nullptr, 0.3f, num );
// 	}

	for( unsigned int index = 0; index < m_test.size(); index++ )
	{
		std::string num = std::to_string( static_cast<long double>( m_test[index] ) );
		Vector2 center = Vector2( m_tileList[index].m_tileCoords.x + 0.2f, m_tileList[index].m_tileCoords.y + 0.2f );
		g_glRender->RenderText( center, RGBColor::Black(), nullptr, nullptr, 0.3f, num );
	}
}

void Map::GenerateRoomLikeSubMap()
{
	GenerateRooms();
	ChangeMapTileToRoomTile();
	GenerateHallWays();
	CreateOutlineOfHallway();
}

void Map::GenerateRooms()
{
	m_tileList.resize( m_mapSize );

 	Room room;

	for( unsigned int tileIndex = 0; tileIndex < m_tileList.size(); tileIndex++ )
	{
		TileCoords2D tileCoords = ConvertTileIndexToTileCoords2D( tileIndex, m_mapWidthX, m_mapHeightY );
		Tile tile( "Larva", tileCoords );
		m_tileList[tileIndex] = tile;
	}

	int numberOfRoom = MathUtilities::GetRandomNumber( m_mapBlueprint->m_minMaxNumRoom.x, m_mapBlueprint->m_minMaxNumRoom.y );

	for( int roomIndex = 0; roomIndex < numberOfRoom; roomIndex++ )
	{
		int roomWidth = MathUtilities::GetRandomNumber( m_mapBlueprint->m_roomMinMaxWidth.x, m_mapBlueprint->m_roomMinMaxWidth.y );
		int roomHeight = MathUtilities::GetRandomNumber( m_mapBlueprint->m_roomMinMaxHeight.x, m_mapBlueprint->m_roomMinMaxHeight.y );

		TileIndex	 roomCenterIndex = MathUtilities::GetRandomNumber( 0, m_mapSize );
		TileCoords2D roomCenter = ConvertTileIndexToTileCoords2D( roomCenterIndex, m_mapWidthX, m_mapHeightY );

		TileCoords2D roomMaxs;
		TileCoords2D roomMins;
		TileCoords2D offsetMin;
		TileCoords2D offsetMax;

		if( roomWidth % 2 != 0 )
		{
			offsetMin.x = static_cast<int>( roomWidth * 0.5f );
			offsetMax.x = static_cast<int>( roomWidth * 0.5f );
		}
		else
		{
			offsetMin.x = static_cast<int>( roomWidth * 0.5f - 1 );
			offsetMax.x = static_cast<int>( roomWidth * 0.5f );
		}

		if( roomHeight % 2 != 0 )
		{
			offsetMin.y = static_cast<int>( roomHeight * 0.5f );
			offsetMax.y = static_cast<int>( roomHeight * 0.5f );
		}
		else
		{
			offsetMin.y = static_cast<int>( roomHeight * 0.5f - 1 );
			offsetMax.y = static_cast<int>( roomHeight * 0.5f );
		}

		roomMaxs = roomCenter + offsetMax;
		roomMins = roomCenter - offsetMin;

		AABB2 roomAABB( roomMins, roomMaxs );

		int tryCount = 0;
		bool skipRoom = false;

		while( roomMaxs.x >= m_mapWidthX || roomMaxs.y >= m_mapHeightY || 
			   roomMins.x <= 0 || roomMins.y <= 0 ||
			   IsRoomOverlapExistedRoom(roomAABB) )
		{
			if( tryCount >= MAX_TRY_COUNT_TO_CREATE_ROOM )
			{
				skipRoom = true;
				break;
			}

			roomCenterIndex = MathUtilities::GetRandomNumber( 0, m_mapSize );
			roomCenter = ConvertTileIndexToTileCoords2D( roomCenterIndex, m_mapWidthX, m_mapHeightY );
			if( roomWidth % 2 != 0 )
			{
				offsetMin.x = static_cast<int>( roomWidth * 0.5f );
				offsetMax.x = static_cast<int>( roomWidth * 0.5f );
			}
			else
			{
				offsetMin.x = static_cast<int>( roomWidth * 0.5f - 1 );
				offsetMax.x = static_cast<int>( roomWidth * 0.5f );
			}

			if( roomHeight % 2 != 0 )
			{
				offsetMin.y = static_cast<int>( roomHeight * 0.5f );
				offsetMax.y = static_cast<int>( roomHeight * 0.5f );
			}
			else
			{
				offsetMin.y = static_cast<int>( roomHeight * 0.5f - 1 );
				offsetMax.y = static_cast<int>( roomHeight * 0.5f );
			}
			room.m_roomCenter = Vector2( roomCenter.x + 0.5f, roomCenter.y + 0.5f );
			roomMaxs = roomCenter + offsetMax;
			roomMins = roomCenter - offsetMin;
			roomAABB = AABB2( roomMins, roomMaxs );
			tryCount++;
		}

 		if( !skipRoom )
 		{
			room.m_roomCenter = Vector2( roomCenter.x + 0.5f, roomCenter.y + 0.5f );
			room.m_roomTileCoordsMins = roomMins;
			room.m_roomTileCoordsMaxs = roomMaxs;
			room.m_width = roomWidth;
			room.m_height = roomHeight;
			room.m_roomArea = roomWidth * roomHeight;
			m_roomList.push_back(room);
		}
	}
}

void Map::GenerateHallWays()
{
	std::vector<WorldCoords2D> hallway;
	
	std::vector<int> connectedRoomList;
	std::vector<int> unconnectedRoomList;

	unsigned int firstRoomIndex = MathUtilities::GetRandomNumber( 0, m_roomList.size() - 1 );
	connectedRoomList.push_back( firstRoomIndex );

	for( unsigned int roomIndex = 0; roomIndex < m_roomList.size(); roomIndex++ )
	{
		if( roomIndex != firstRoomIndex )
		{
			unconnectedRoomList.push_back( roomIndex );
		}
	}

	while( unconnectedRoomList.size() > 0 )
	{
		hallway.clear();
		int connectedRoomIndex = MathUtilities::GetRandomNumber( 0, connectedRoomList.size() - 1 );
		int closestRoomIndex = GetIndexOfClosestRoomToRoom( connectedRoomIndex, unconnectedRoomList );
		Room connectedRoom = m_roomList[connectedRoomList[connectedRoomIndex]];
		Room unconnectedRoom = m_roomList[unconnectedRoomList[closestRoomIndex]];

		CalculateAStarPath( connectedRoom.m_roomCenter, unconnectedRoom.m_roomCenter, hallway, false, nullptr );
		for( unsigned int i = 0; i < hallway.size(); i++ )
		{
			TileCoords2D hallwayTileCoords = ConvertWorldCoords2DToTileCoords2D( hallway[i] );
			TileCoords2D north = hallwayTileCoords + Vector2i( 0, 1 );
			TileCoords2D south = hallwayTileCoords - Vector2i( 0, 1 );
			TileCoords2D west = hallwayTileCoords - Vector2i( 1, 0 );
			TileCoords2D east = hallwayTileCoords + Vector2i( 1, 0 );

			TileIndex tileIndex = ConvertTileCoords2DToTileIndex( hallwayTileCoords, m_mapWidthX );
			TileIndex northIndex = ConvertTileCoords2DToTileIndex( north, m_mapWidthX );
			TileIndex southIndex = ConvertTileCoords2DToTileIndex( south, m_mapWidthX );
			TileIndex eastIndex = ConvertTileCoords2DToTileIndex( east, m_mapWidthX );
			TileIndex westIndex = ConvertTileCoords2DToTileIndex( west, m_mapWidthX );

			ChangeMapTileToHallwayTile( hallwayTileCoords, tileIndex );
			ChangeMapTileToHallwayTile( north, northIndex );
			ChangeMapTileToHallwayTile( south, southIndex );
			ChangeMapTileToHallwayTile( east, eastIndex );
			ChangeMapTileToHallwayTile( west, westIndex );
		}

		connectedRoomList.push_back(unconnectedRoomList[closestRoomIndex]);
		unconnectedRoomList.erase( unconnectedRoomList.begin() + closestRoomIndex );
	}

}

void Map::ChangeMapTileToHallwayTile( const TileCoords2D& tileCoords, TileIndex tileIndex )
{
	if( m_tileList[ tileIndex ].m_tileBlueprint->m_name.compare("Wall") == 0 )
	{
		ChangeTypeOfSingleTile( "Rock", tileCoords );
		return;
	}

	if( m_tileList[ tileIndex ].m_tileBlueprint->m_name.compare("Rock") != 0 )
	{
		ChangeTypeOfSingleTile( "Rock", tileCoords );
	}
}

int Map::GetIndexOfClosestRoomToRoom( int roomIndex, const std::vector<int>& unconnectedList )
{
	std::vector<float> distanceSquareList;

	Room room = m_roomList[roomIndex];

	for( unsigned int i = 0; i < unconnectedList.size(); i++ )
	{
		Room otherRoom = m_roomList[unconnectedList[i]];
		float distanceSquare = ( room.m_roomCenter - otherRoom.m_roomCenter ).CalcLengthSquare();
		distanceSquareList.push_back( distanceSquare );
	}

	int minIndex = 0;

	for( unsigned int i = 0; i < distanceSquareList.size(); i++ )
	{
		if( distanceSquareList[minIndex] < distanceSquareList[i] )
			minIndex = i;
	}

	return minIndex;
}

bool Map::IsRoomOverlapExistedRoom( AABB2 room )
{
	for( unsigned int createdRoomIndex = 0; createdRoomIndex < m_roomList.size(); createdRoomIndex++ )
	{
		const Room& createdRoom = m_roomList[createdRoomIndex];
		AABB2 createdRoomAABB( createdRoom.m_roomTileCoordsMins, createdRoom.m_roomTileCoordsMaxs );

		if( room.IsCollideWithAABB( createdRoomAABB ) )
			return true;
	}
	return false;
}

void Map::ChangeMapTileToRoomTile()
{
	for( unsigned int i = 0; i < m_roomList.size(); i++ )
	{
		Room& room = m_roomList[i];

		for( int roomHeightY = room.m_roomTileCoordsMins.y; roomHeightY < room.m_roomTileCoordsMins.y + room.m_height; roomHeightY++  )
		{
			for( int roomWidthX = room.m_roomTileCoordsMins.x; roomWidthX < room.m_roomTileCoordsMins.x + room.m_width; roomWidthX++ )
			{
				Vector2i tileCoors( roomWidthX, roomHeightY );
				ChangeTypeOfSingleTile( "Rock", tileCoors );
			}
		}

		//bottom and top wall
 		int index;
		TileIndex minIndex = ConvertTileCoords2DToTileIndex( room.m_roomTileCoordsMins, m_mapWidthX );

		for( index = minIndex; index < minIndex + room.m_width; index++ )
		{
			int tileAtBottom = index;
			int tileAtTop = index + ( room.m_height - 1 ) * m_mapWidthX;
			TileCoords2D tileCoordsAtBottom = ConvertTileIndexToTileCoords2D( tileAtBottom,  m_mapWidthX, m_mapHeightY ); 
			TileCoords2D tileCoordsAtTop = ConvertTileIndexToTileCoords2D( tileAtTop,  m_mapWidthX, m_mapHeightY ); 
			ChangeTypeOfSingleTile( "Wall", tileCoordsAtTop );
			ChangeTypeOfSingleTile( "Wall", tileCoordsAtBottom );
		}

		for( index = 0; index < room.m_height; index++ )
		{
			int tileAtLeft = minIndex + index * m_mapWidthX;;
			int tileAtRight = ( minIndex + room.m_width - 1 ) + index * m_mapWidthX;
			TileCoords2D tileCoordsAtLeft = ConvertTileIndexToTileCoords2D( tileAtLeft,  m_mapWidthX, m_mapHeightY ); 
			TileCoords2D tileCoordsAtRight = ConvertTileIndexToTileCoords2D( tileAtRight,  m_mapWidthX, m_mapHeightY ); 
			ChangeTypeOfSingleTile( "Wall", tileCoordsAtLeft );
			ChangeTypeOfSingleTile( "Wall", tileCoordsAtRight );
		}
	}
}


void Map::CreateOutlineOfHallway()
{
	std::vector<Tile> copyList = m_tileList;
	int neighborIndexList[8];

	for( unsigned tileIndex = 0; tileIndex < m_tileList.size(); tileIndex++ )
	{
		Tile copyTile = copyList[tileIndex];
		TileCoords2D tileCoords = ConvertTileIndexToTileCoords2D( tileIndex, m_mapWidthX, m_mapHeightY );

		Get8NeighborTileIndex( tileIndex, neighborIndexList );

		for( int index = 0; index < 8; index++ )
		{
			if(	copyTile.m_tileBlueprint->m_name.compare("Rock") == 0 
				&& neighborIndexList[index] != -1
				&& copyList[ neighborIndexList[index] ].m_tileBlueprint->m_name.compare( "Larva" ) == 0 )
			{
					TileCoords2D tileToChange = ConvertTileIndexToTileCoords2D( neighborIndexList[index],m_mapWidthX,m_mapHeightY );
					ChangeTypeOfSingleTile( "Wall", tileToChange );
			}
		}	
	}
}

void Map::Get8NeighborTileIndex( TileIndex tileIndex, int indexList[8] )
{
	TileCoords2D tileCoords = ConvertTileIndexToTileCoords2D( tileIndex, m_mapWidthX, m_mapHeightY );

	if( tileCoords.y <= 0 )
		indexList[SOUTH_CARDINAL_DIRECTION] = -1;
	else 
		indexList[SOUTH_CARDINAL_DIRECTION] = tileIndex - m_mapWidthX;

	if( tileCoords.y >= m_mapHeightY - 1)
		indexList[NORTH_CARDINAL_DIRECTION] = -1;
	else
		indexList[NORTH_CARDINAL_DIRECTION] = tileIndex + m_mapWidthX;

	if( tileCoords.x <= 0 )
		indexList[WEST_CARDINAL_DIRECTION] = -1;
	else
		indexList[WEST_CARDINAL_DIRECTION] = tileIndex - 1;

	if( tileCoords.x >= m_mapWidthX - 1 )
		indexList[EAST_CARDINAL_DIRECTION] = -1;
	else
		indexList[EAST_CARDINAL_DIRECTION] = tileIndex + 1;

	if( indexList[NORTH_CARDINAL_DIRECTION] == -1 )
	{	
		indexList[NORTH_EAST_SEMICARDINAL_DIRECTION] = -1;
		indexList[NORTH_WEST_SEMICARDINAL_DIRECTION] = -1;
	}
	else
	{
		if( indexList[EAST_CARDINAL_DIRECTION] == -1 )
			indexList[NORTH_EAST_SEMICARDINAL_DIRECTION] = -1;
		else
			indexList[NORTH_EAST_SEMICARDINAL_DIRECTION] = indexList[NORTH_CARDINAL_DIRECTION] + 1;

		if( indexList[WEST_CARDINAL_DIRECTION] == -1 )
			indexList[NORTH_WEST_SEMICARDINAL_DIRECTION] = -1;
		else
			indexList[NORTH_WEST_SEMICARDINAL_DIRECTION] = indexList[NORTH_CARDINAL_DIRECTION] - 1;
	}

	if( indexList[SOUTH_CARDINAL_DIRECTION] == -1 )
	{	
		indexList[SOUTH_EAST_SEMICARDINAL_DIRECTION] = -1;
		indexList[SOUTH_WEST_SEMICARDINAL_DIRECTION] = -1;
	}
	else
	{
		if( indexList[EAST_CARDINAL_DIRECTION] == -1 )
			indexList[SOUTH_EAST_SEMICARDINAL_DIRECTION] = -1;
		else
			indexList[SOUTH_EAST_SEMICARDINAL_DIRECTION] = indexList[SOUTH_CARDINAL_DIRECTION] + 1;

		if( indexList[WEST_CARDINAL_DIRECTION] == -1 )
			indexList[SOUTH_WEST_SEMICARDINAL_DIRECTION] = -1;
		else
			indexList[SOUTH_WEST_SEMICARDINAL_DIRECTION] = indexList[SOUTH_CARDINAL_DIRECTION] - 1;
	}
}

void Map::Get4NeighborTileIndex(TileIndex tileIndex, int indexList[4])
{
	TileCoords2D tileCoords = ConvertTileIndexToTileCoords2D( tileIndex, m_mapWidthX, m_mapHeightY );

	if( tileCoords.y <= 0 )
		indexList[SOUTH_CARDINAL_DIRECTION] = -1;
	else 
		indexList[SOUTH_CARDINAL_DIRECTION] = tileIndex - m_mapWidthX;

	if( tileCoords.y >= m_mapHeightY - 1)
		indexList[NORTH_CARDINAL_DIRECTION] = -1;
	else
		indexList[NORTH_CARDINAL_DIRECTION] = tileIndex + m_mapWidthX;

	if( tileCoords.x <= 0 )
		indexList[WEST_CARDINAL_DIRECTION] = -1;
	else
		indexList[WEST_CARDINAL_DIRECTION] = tileIndex - 1;

	if( tileCoords.x >= m_mapWidthX - 1 )
		indexList[EAST_CARDINAL_DIRECTION] = -1;
	else
		indexList[EAST_CARDINAL_DIRECTION] = tileIndex + 1;

}

void Map::GenerateCaveLikeSubMap()
{
	std::vector<Tile> copyList;
	m_tileList.reserve( m_mapSize );
	int neighborIndexList[8];
	int tileIndex;

	//pre-filled map with random tile
	for( tileIndex = 0; tileIndex < m_mapSize; tileIndex++ )
	{
		TileCoords2D tileCoords = ConvertTileIndexToTileCoords2D( tileIndex, m_mapWidthX, m_mapHeightY );
		float randNum = MathUtilities::GetRandomFloatZeroToOne();
		if( randNum < 0.45f )
		{
			Tile tile( "Wall",tileCoords );
			m_tileList.push_back(tile);
		}
		else
		{
			Tile tile( "Sand",tileCoords );
			m_tileList.push_back(tile);
		}
	}

	//Surround map with solid tile
	for( tileIndex = 0; tileIndex < m_mapWidthX; tileIndex++ )
	{
		int tileAtBottom = tileIndex;
		int tileAtTop = tileIndex + ( m_mapWidthX - 1 ) * m_mapWidthX;
		TileCoords2D tileCoordsAtBottom = ConvertTileIndexToTileCoords2D( tileAtBottom,  m_mapWidthX, m_mapHeightY ); 
		TileCoords2D tileCoordsAtTop = ConvertTileIndexToTileCoords2D( tileAtTop,  m_mapWidthX, m_mapHeightY ); 
		ChangeTypeOfSingleTile( "Wall", tileCoordsAtTop );
		ChangeTypeOfSingleTile( "Wall", tileCoordsAtBottom );
	}

	for( tileIndex = 0; tileIndex < m_mapHeightY; tileIndex++ )
	{
		int tileAtLeft = tileIndex * m_mapWidthX;;
		int tileAtRight = ( m_mapWidthX - 1 ) + tileIndex * m_mapWidthX;
		TileCoords2D tileCoordsAtLeft = ConvertTileIndexToTileCoords2D( tileAtLeft,  m_mapWidthX, m_mapHeightY ); 
		TileCoords2D tileCoordsAtRight = ConvertTileIndexToTileCoords2D( tileAtRight,  m_mapWidthX, m_mapHeightY ); 
		ChangeTypeOfSingleTile( "Wall", tileCoordsAtLeft );
		ChangeTypeOfSingleTile( "Wall", tileCoordsAtRight );
	}

	copyList = m_tileList;

	for( int iterationCount = 0; iterationCount < CELLULAR_AUTOMATA_ITERATION_COUNT; iterationCount++ )
	{
		for( tileIndex = 0; tileIndex < m_tileList.size(); tileIndex++ )
		{
			int wallTileCount = 0;
			int emptyTileCount = 0;
			Tile copyTile = copyList[tileIndex];
			TileCoords2D tileCoords = ConvertTileIndexToTileCoords2D( tileIndex, m_mapWidthX, m_mapHeightY );

			Get8NeighborTileIndex( tileIndex, neighborIndexList );

			for( int index = 0; index < 8; index++ )
			{
				if( neighborIndexList[index] != -1 )
				{
					if( copyList[ neighborIndexList[index] ].m_tileBlueprint->m_isSolid )
					{
						wallTileCount++;
					}
					else
					{
						emptyTileCount++;
					}
				} 
			}

			if( wallTileCount >= 5 || emptyTileCount <= 1 )
			{
				ChangeTypeOfSingleTile( "Wall", tileCoords );
			}

			if( copyTile.m_tileBlueprint->m_isSolid && emptyTileCount >= 5 )
			{
				ChangeTypeOfSingleTile( "Sand", tileCoords );
			}
		}
		copyList = m_tileList;
	}

	ConnectDisjointSectionOfCaveMap();
}

void Map::SpawnItemWhenActorDie( Actor* actor )
{
	ActorBlueprint* bp = actor->m_actorBlueprint;
	std::string itemName;
	float percentDrop;
	Vector2 offset;// avoid item to overlap

	for( unsigned int i = 0; i < bp->m_dropList.size(); i++ )
	{
		itemName = bp->m_dropList[i]->name;
		percentDrop = bp->m_dropList[i]->dropRate;

		float rollDice = MathUtilities::GetRandomFloatZeroToOne();
		if( rollDice <= percentDrop )
		{
			offset.x = MathUtilities::GetRandomFloatZeroToOne() - 0.5f;
			offset.y = MathUtilities::GetRandomFloatZeroToOne() - 0.5f;

			Item* newItem = new Item( itemName, actor->m_worldPosition + offset );
			m_itemList.push_back( newItem );
		}
	}
}

void Map::CalDijkstraMapFromPlayer()
{
	for( unsigned int index = 0; index < m_tileList.size(); index++ )
	{
		m_dijkstraMapFromPlayer[index] = VERY_HIGH_INT;
	}

	for( unsigned int actorIndex = 0; actorIndex < m_actorList.size(); actorIndex++ )
	{
		if( m_actorList[actorIndex]->m_actorBlueprint->m_isPlayer )
		{
			TileIndex actorTileIndex = ConvertWorldCoords2DToTileIndex( m_actorList[actorIndex]->m_worldPosition, m_mapWidthX );
			m_dijkstraMapFromPlayer[actorTileIndex] = 0;
		}
	}

	bool isDijkstraMapChange = true;

	while( isDijkstraMapChange )
	{
		isDijkstraMapChange = false;
		for( unsigned int index = 0; index < m_tileList.size(); index++ )
		{
			if( !m_tileList[index].m_tileBlueprint->m_isSolid )
			{
				int smallestNeighbourIndex = GetSmallestDijkstraValueIn8NeighborTileIndex( index, m_dijkstraMapFromPlayer );

				if( ( m_dijkstraMapFromPlayer[index] - m_dijkstraMapFromPlayer[smallestNeighbourIndex] >= 2 ) )
				{
					m_dijkstraMapFromPlayer[index] = m_dijkstraMapFromPlayer[smallestNeighbourIndex] + 1;
					isDijkstraMapChange = true;
				}
			}
		}
	}
}

void Map::CalSafetyMapFromPlayer()
{
	for( unsigned int i = 0; i < m_safetyMapFromPlayer.size(); i++ )
	{
		if( m_dijkstraMapFromPlayer[i] == VERY_HIGH_INT )
			continue;

		m_safetyMapFromPlayer[i] = static_cast<int>( m_dijkstraMapFromPlayer[i] * ( -1.2f ) ) ;
	}

	bool isDijkstraMapChange = true;

	while( isDijkstraMapChange )
	{
		isDijkstraMapChange = false;
		for( unsigned int index = 0; index < m_tileList.size(); index++ )
		{
			if( !m_tileList[index].m_tileBlueprint->m_isSolid )
			{
				int smallestNeighbourIndex = GetSmallestDijkstraValueIn8NeighborTileIndex( index, m_dijkstraMapFromPlayer );

				if( ( m_safetyMapFromPlayer[index] - m_safetyMapFromPlayer[smallestNeighbourIndex] >= 2 ) )
				{
					m_safetyMapFromPlayer[index] = m_safetyMapFromPlayer[smallestNeighbourIndex] + 1;
					isDijkstraMapChange = true;
				}
			}
		}
	}
}

int Map::GetSmallestDijkstraValueIn8NeighborTileIndex( TileIndex centerTileindex, const std::vector<int>& map )
{
	int smallestNeighbourIndex;
	int neighborIndexList[8];

	Get8NeighborTileIndex( centerTileindex, neighborIndexList );

	for( int i = 0; i < 8; i++ )
	{
		if( neighborIndexList[i] != -1 )
		{
			smallestNeighbourIndex = neighborIndexList[i];
			break;
		}
	}

	for( int i = 0; i < 8; i++ )
	{
		if( neighborIndexList[i] >= 0 && map[smallestNeighbourIndex] > map[neighborIndexList[i]] )
			smallestNeighbourIndex = neighborIndexList[i];
	}

	return smallestNeighbourIndex;
}

int Map::GetSmallestDijkstraValueIn4NeighborTileIndex( TileIndex centerTileIndex, const std::vector<int>& map)
{
	int smallestNeighbourIndex = -1;
	int neighborIndexList[4];

	Get4NeighborTileIndex( centerTileIndex, neighborIndexList );

	for( int i = 0; i < 4; i++ )
	{
		if( neighborIndexList[i] != -1 )
		{
			smallestNeighbourIndex = neighborIndexList[i];
			break;
		}
	}

	for( int i = 0; i < 4; i++ )
	{
		if( neighborIndexList[i] >= 0 && map[smallestNeighbourIndex] > map[neighborIndexList[i]] )
			smallestNeighbourIndex = neighborIndexList[i];
	}

	return smallestNeighbourIndex;
}

void Map::ConnectDisjointSectionOfCaveMap()
{
	std::vector<int> testMap;
	testMap.resize( m_mapSize );
	bool isClosedSpace = true;
	unsigned int index;
	int randTile = MathUtilities::GetRandomNumber( 0, m_mapSize - 1 );

	while( m_tileList[randTile].m_tileBlueprint->m_isSolid )
	{
		randTile = MathUtilities::GetRandomNumber( 0, m_mapSize - 1 );
	}

	for( index = 0; index < m_tileList.size(); index++ )
	{
		if( index != randTile )
			testMap[index] = VERY_HIGH_INT;
	}

	while( isClosedSpace )
	{
		isClosedSpace = false;

		bool isDijkstraMapChange = true;

		while( isDijkstraMapChange )
		{
			isDijkstraMapChange = false;
			for( unsigned int index = 0; index < m_tileList.size(); index++ )
			{
				if( !m_tileList[index].m_tileBlueprint->m_isSolid )
				{
					int smallestNeighbourIndex = GetSmallestDijkstraValueIn4NeighborTileIndex( index, testMap );

					if( ( testMap[index] - testMap[smallestNeighbourIndex] >= 2 ) )
					{
						testMap[index] = testMap[smallestNeighbourIndex] + 1;
						isDijkstraMapChange = true;
					}
				}
			}
		}

		for( index = 0; index < testMap.size(); index++ )
		{
			if( testMap[index] == VERY_HIGH_INT && !m_tileList[index].m_tileBlueprint->m_isSolid )
			{
				isClosedSpace = true;
				break;
			}
		}

		if( isClosedSpace )
		{
			int neighborIndexList[4];

			Get4NeighborTileIndex( index, neighborIndexList );

			for( int i = 0; i < 4; i++ )
			{
				TileCoords2D tileCoords = ConvertTileIndexToTileCoords2D( neighborIndexList[i], m_mapWidthX, m_mapHeightY );
				ChangeTypeOfSingleTile( "Sand", tileCoords );
			}
		}
	}
	m_test = testMap;
}

void Map::PlaceRandomEnemyOnTheMap( const MapBlueprint& blueprint )
{
	unsigned int index = 0;
	for( index = 0; index < blueprint.m_encounterInfoList.size(); index++ )
	{
		EncounterInfo* info = blueprint.m_encounterInfoList[index];
		int amount = MathUtilities::GetRandomNumber( info->m_minMaxNum.x, info->m_minMaxNum.y );

		for( int amountCount = 0; amountCount < amount; amountCount++ )
		{
			float randX = MathUtilities::GetRandomFloatInRange( 0.f, static_cast<float>( m_mapWidthX ) );
			float randY = MathUtilities::GetRandomFloatInRange( 0.f, static_cast<float>( m_mapHeightY ) );
			WorldCoords2D worldCoords = WorldCoords2D( randX, randY );
			TileIndex tileIndex = ConvertWorldCoords2DToTileIndex( worldCoords, m_mapWidthX );

			while( m_tileList[tileIndex].m_tileBlueprint->m_isSolid 
				  || m_tileList[tileIndex].m_tileBlueprint->m_name.compare( "Larva" ) == 0 )
			{
				randX = MathUtilities::GetRandomFloatInRange( 0.f, static_cast<float>( m_mapWidthX ) );
				randY = MathUtilities::GetRandomFloatInRange( 0.f, static_cast<float>( m_mapHeightY ) );
				worldCoords = WorldCoords2D( randX, randY );
				tileIndex = ConvertWorldCoords2DToTileIndex( worldCoords, m_mapWidthX );
			}

			Actor* encounter = new Actor( info->m_name, worldCoords );
			m_actorList.push_back( encounter );
		}
	}
}

void Map::SetRandomStartPositionForPlayer()
{
	int openNeighborCount = 0;
	int indexList[8];
	int openIndex[4];

	for( unsigned int tileIndex = 0; tileIndex < m_tileList.size(); tileIndex++ )
	{
		Tile tile = m_tileList[tileIndex];

		if( tile.m_tileBlueprint->m_isSolid )
			continue;

		openIndex[openNeighborCount] = tileIndex;

		Get8NeighborTileIndex( tileIndex, indexList );

		for( int i = 0; i < 8; i++ )
		{
			if( indexList[i] == -1 )
				continue;

			Tile neighborTile = m_tileList[ indexList[i] ];
			if( !neighborTile.m_tileBlueprint->m_isSolid )
			{
				openNeighborCount++;
				openIndex[openNeighborCount] = indexList[i];
			}

			if( openNeighborCount == 3 )
				break;
		}
		if( openNeighborCount == 3 )
			break;
	}

	for( int i = 0; i < 4; i++ )
	{
		TileCoords2D tileCoords = ConvertTileIndexToTileCoords2D( openIndex[i], m_mapWidthX, m_mapHeightY );
		if( i == 0 )
		{
			ChangeTypeOfSingleTile( "ToWorld", tileCoords );
			continue;
		}
		WorldCoords2D center = WorldCoords2D( tileCoords.x + 0.5f, tileCoords.y + 0.5f );
		m_startPositionList.push_back( center );
	}
}

void Map::RenderProjectiles()
{
	for( unsigned int i = 0; i < m_projectileList.size(); i++ )
	{
		m_projectileList[i]->Render();
	}
}










