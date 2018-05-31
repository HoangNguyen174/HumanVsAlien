#ifndef MAP_H
#define MAP_H
#include "SoulStoneEngine/Render/GLRender.hpp"
#include "SoulStoneEngine/Utilities/GameCommon.hpp"
#include "SoulStoneEngine/Utilities/Texture.hpp"
#include "SoulStoneEngine/Utilities/AABB2.hpp"
#include "Tile.hpp"
#include "Actor.hpp"
#include "Item.hpp"
#include "MapBlueprint.hpp"
#include "Room.hpp"
#include "Construct.hpp"
#include "Projectile.hpp"

const int TILETEXTURE_WIDTH_IN_FRAME = 64;
const int TILETEXTURE_HEIGHT_IN_FRAME = 48;
const int ACTORTEXTURE_WIDTH_IN_FRAME = 12;
const int ACTORTEXTURE_HEIGHT_IN_FRAME = 8;
const float ACTOR_HALF_WIDTH = .5f;
const int MAX_TRY_COUNT_TO_CREATE_ROOM = 10;
const int CELLULAR_AUTOMATA_ITERATION_COUNT = 5;
const int VERY_HIGH_INT = 9999;

struct HitInfo
{
	WorldCoords2D			pointOfImpact;
	float					distanceToImpactPoint;
	TileCoords2D			tileCoordsImpacted;
	CARDINAL_DIRECTION		faceOfTileImpacted;
};

struct PathNode 
{
	PathNode*		parent;
	TileCoords2D	tileCoords;
	float			gCost;
	float			hCost;
	float			fCost;
};

class Map
{
	public: 
		std::string					m_name;
		int							m_mapWidthX;
		int						    m_mapHeightY;
		int							m_mapSize;
		MapBlueprint*				m_mapBlueprint;

		//used for render
		std::vector<Vertex2D>		m_vertexListToDrawTile;
		Texture*				    m_tileTexture;
		Texture*				    m_actorTexture;
		GLuint						m_vboIDOfTileArray;
		int							m_numVertexToDrawTiles;
		bool						m_isVBOtoDrawTileDirty;

		//map contents actor, tile, etc
		static std::vector<Actor*>	s_actorWaitingForPathList;				
		std::vector<Tile>			m_tileList;
		std::vector<Actor*>			m_actorList;
		std::vector<Item*>			m_itemList;
		std::vector<Room>			m_roomList;
		std::vector<TileCoords2D>	m_hallway;
		std::vector<Construct*>		m_constructList;
		std::vector<WorldCoords2D>	m_startPositionList;
		std::vector<Projectile*>	m_projectileList;

		// Dijkstra map
		std::vector<int>			m_dijkstraMapFromPlayer;
		std::vector<int>			m_safetyMapFromPlayer;
		std::vector<int>			m_dijkstraMapFromAlienShip;
		std::vector<int>			m_test;
		bool						m_visualizeMapInfo;

		std::string					m_backgroundMusic;

		std::set<PathNode*>			m_openNodeList;
		std::set<PathNode*>			m_closedNodeList;

	public:
		Map() {};
		~Map();
		Map( const std::string& mapLayoutFilePath, const std::string& mapDetailFilePath );
		Map( const std::string& blueprintName );
		void ChangeTypeOfSingleTile( const std::string& newTileType, const TileCoords2D& tileCoords );
		void ChangeTypeOfAllTile( const std::string& oldType, const std::string& newType );
		void LoadTexture( const std::string& texType, const std::string& filePath, int texWidth, int texHeight );
		void SpawnNewActor( const std::string& name , const WorldCoords2D& position );
		void CalculateAStarPath( const WorldCoords2D& startPosition, const WorldCoords2D& goalPosition, std::vector<WorldCoords2D>& path, bool knowAboutMap, int* dijkstraMapPointer );
		void Render();
		void Update( float elapsedTime );
		void AddActorToWaitingForPathList( Actor* actor );
		static HitInfo GetImpactedInfoFromRaycast( const WorldCoords2D& rayStartPos, const Vector2& rayDirection, float maxRayLength, const Map& map );
		void Get8NeighborTileIndex( TileIndex tileIndex, int neighborIndexList[8] );
		void Get4NeighborTileIndex( TileIndex tileIndex, int neighborIndexList[4] );
		int GetSmallestDijkstraValueIn8NeighborTileIndex( TileIndex centerTile, const std::vector<int>& map );
		int GetSmallestDijkstraValueIn4NeighborTileIndex( TileIndex centerTile, const std::vector<int>& map );

	private:
		bool IsTileSolid( const TileCoords2D& tile );
		void LoadMapDetailFromFile( const std::string& mapFilePath );
		void LoadMapLayOutFromFile( const std::string& mapFilePath );
		void CreateVertexArrayForTiles();
		PathNode* GetNeighborNodeByCardinalDirection( PathNode* parentNode, CARDINAL_DIRECTION direction, const TileCoords2D& goalTile, bool knowAboutMap );
		PathNode* GetPathNodeWithSmallestfCostAndPopOutOfOpenList( std::set<PathNode*>& nodeList );
		void RenderTiles();
		void RenderActors();
		void RenderItems();
		void RenderConstructs();
		void RenderProjectiles();
		void ResolveProjectTileCollision();
		void ResolveActorTileCollision();
		void ConstraintActorPosition( Actor* actor, float actorRadius, const TileIndex& tileIndex, const TileCoords2D& corner );
		void ResolveActorActorCollision();
		void ResolveActorItemCollision();
		void ProcessWaitingForPathList();
		void CheckAndSetIfTileIsAlreadySeenButNotVisibleNow();
		void VisualizeTileStatus();
		void PopulateFromBluePrint( const MapBlueprint& blueprint );
		void GenerateRooms();
		void GenerateRoomLikeSubMap();
		void GenerateRandomSubMapContent();
		bool IsRoomOverlapExistedRoom( AABB2 room );
		void ChangeMapTileToRoomTile();
		void ChangeMapTileToHallwayTile( const TileCoords2D& tileCoords, TileIndex tileIndex );
		void GenerateHallWays();
		int GetIndexOfClosestRoomToRoom( int roomIndex, const std::vector<int>& unconnectedList );
		void CreateOutlineOfHallway();
		void GenerateCaveLikeSubMap();
		void SpawnItemWhenActorDie( Actor* actor );
		void CalDijkstraMapFromPlayer();
		void CalSafetyMapFromPlayer();
		void AllocateDijkstraMap();
		void ConnectDisjointSectionOfCaveMap();
		void PlaceRandomEnemyOnTheMap( const MapBlueprint& blueprint );
		void SetRandomStartPositionForPlayer();
};

#endif