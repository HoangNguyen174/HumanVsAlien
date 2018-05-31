#include "World.hpp"
#include "SoulStoneEngine/Utilities/ProfileSection.hpp"
#include "SoulStoneEngine/JobSystem/JobManager.hpp"
#include "SoulStoneEngine/JobSystem/TestJob.hpp"
#include "SoulStoneEngine/JobSystem/HashBufferJob.hpp"
#include "SoulStoneEngine/JobSystem/LoadFileJob.hpp"
#include "SoulStoneEngine/Utilities/FixedPointMath.hpp"

World* theWorld = nullptr;
bool isKeyDownKeyboard[256];
bool isKeyDownLastFrame[256];
WorldCoords2D g_mouseWorldPosition;
WorldCoords2D g_mouseScreenPosition;
bool g_isLeftMouseDown;
bool g_isRightMouseDown;
bool g_isHoldingShift;
Map* g_currentMap = nullptr;
bool g_isQuitting = false;
AudioSystem* g_audioSystem = nullptr;
bool g_isFOWoff = false;

#ifdef USE_MEMORY_MANAGER
	MemoryPoolManager* g_memoryManager = nullptr;
#endif

World::World()
{
	Initialize();		
}

World::~World()
{
	JobManager::ExitJobManager();
}

void* ThreadTestFunc( void* /*args*/ )
{
	std::string* str = new std::string( Stringf( "Thread ID: %i.", Thread::GetCurrentThreadID() ) );

	return str;
}



void World::Initialize()
{	
	//JobManager::Initalize();
	std::string str = "ABC";
	m_hashValue = -1;
	char* buffer = nullptr;
	//HashBufferJob* hashJob = new HashBufferJob( str, &m_hashValue );

	//LoadFileJob* loadFileJob = new LoadFileJob( "ReadMe.txt", buffer );

	//JobManager::AddJob( hashJob );
	//JobManager::AddJob( loadFileJob );

	m_worldClock = new Clock( Clock::s_masterClock );
	m_worldClock->m_timeScale = 1.0;

	FixedPoint<long long,16> a(-3);
	FixedPoint<long long,16> b(6);
	FixedPoint<long long,16> e(1.14f);
	FixedPoint<long long,16> c;
	FixedPoint<long long,16> d(4);

	c = a + b;
	DebuggerPrintf( "fix + fix: %f.\n", fixToFloat<long long,16>( c ) );

	c = a - b;
	DebuggerPrintf( "fix - fix: %f.\n", fixToFloat<long long,16>( c ) );

	c = a * b;
	DebuggerPrintf( "fix * fix: %f.\n", fixToFloat<long long,16>( c ) );

	c = a / b;
	DebuggerPrintf( "fix / fix: %f.\n", fixToFloat<long long,16>( c ) );

	c = b % d;
	DebuggerPrintf( "fix mod fix: %i.\n", fixToInt<long long,16>( c ) );

	c = a + 1.2f;
	DebuggerPrintf( "fix + float: %f.\n", fixToFloat<long long,16>( c ) );

	c = a - 1.2f;
	DebuggerPrintf( "fix - float: %f.\n", fixToFloat<long long,16>( c ) );

	c = a * 1.2f;
	DebuggerPrintf( "fix * float: %f.\n", fixToFloat<long long,16>( c ) );

	c = a / 1.2f;
	DebuggerPrintf( "fix / float: %f.\n", fixToFloat<long long,16>( c ) );

	c = 1.2f + a;
	DebuggerPrintf( "float + fix: %f.\n", fixToFloat<long long,16>( c ) );

	c = 1.2f - a;
	DebuggerPrintf( "float - fix: %f.\n", fixToFloat<long long,16>( c ) );

	c = 1.2f * a;
	DebuggerPrintf( "float * fix: %f.\n", fixToFloat<long long,16>( c ) );

	c = 1.2f / a;
	DebuggerPrintf( "float / fix: %f.\n", fixToFloat<long long,16>( c ) );

	c = a + 6;
	DebuggerPrintf( "fix + int: %i.\n", fixToInt<long long,16>( c ) );

	c = a - 6;
	DebuggerPrintf( "fix - int: %i.\n", fixToInt<long long,16>( c ) );

	c = a * 6;
	DebuggerPrintf( "fix * int: %i.\n", fixToInt<long long,16>( c ) );

	c = a / 6;
	DebuggerPrintf( "fix / int: %i.\n", fixToInt<long long,16>( c ) );

	c = 6 + a;
	DebuggerPrintf( "int + fix: %i.\n", fixToInt<long long,16>( c ) );

	c = 6 - a;
	DebuggerPrintf( "int - fix: %i.\n", fixToInt<long long,16>( c ) );

	c = 6 * a;
	DebuggerPrintf( "int * fix: %i.\n", fixToInt<long long,16>( c ) );

	c = 6 / a;
	DebuggerPrintf( "int / fix: %i.\n", fixToInt<long long,16>( c ) );

	c = factorial( b );
	DebuggerPrintf( "factorial: %i.\n", fixToInt<long long,16>( c ) );

	c = power( b, 3 );
	DebuggerPrintf( "power: %i.\n", fixToInt<long long,16>( c ) );

	c = sin( e );
	DebuggerPrintf( "sin: %f.\n", fixToFloat<long long,16>( c ) );

	c = cos( e );
	DebuggerPrintf( "cos: %f.\n", fixToFloat<long long,16>( c ) );

	float result = fixToFloat<long long,16>( c );
	
	//m_worldClock->AddAlarmEvent( "Test Alarm Event", 100.0, "", true, TestAlarmEvent, "my test event" );
	//m_worldClock->AddAlarmEvent( "Test Alarm Event 2", 4.0, "", true, TestAlarmEvent, "my test event" );
	m_mapIndex = 0;
	m_previousMapIndex = 0;
	g_theConsole->InitializeConsole();
	g_audioSystem = new AudioSystem;
	g_audioSystem->Initialize();
	m_mouseClickColor = RGBColor( 0.f,0.f,0.f, 0.f );
	m_mouseClickRadius = 0.f;
	m_mouseClickFlashTime = 0.f;

	for(int i = 0; i < 256; i++)
	{
		isKeyDownKeyboard[i] = false;
		isKeyDownLastFrame[i] = false;
	}
	m_renderWorldOriginAxes = false;
	m_camera3D.m_cameraPosition = Vector3(-10.f,0.f,0.f);
	LoadMapAndBlueprint();
	m_playerController = new PlayerController( g_currentMap );
	m_AIController = new AIController( g_currentMap );
	m_hud = new HUD( *g_currentMap );
	m_hud->m_playerController = m_playerController;
	m_zoomFactor = 1.f;
	m_camera2D.m_viewportWidth = WINDOW_VIRTUAL_WIDTH;
	m_camera2D.m_viewportHeight = WINDOW_VIRTUAL_HEIGHT;
	g_isLeftMouseDown = false;
	g_isRightMouseDown = false;

	m_press1Count = 0;
	m_press2Count = 0;
	m_press3Count = 0;
	m_press4Count = 0;
}

void World::LoadMapAndBlueprint()
{
	TileBlueprint::LoadBlueprint( "./Data/XML/TileDefinitions.xml" );
	ActorBlueprint::LoadBlueprint( "./Data/XML/ActorDefinitions.xml" );
	ItemBlueprint::LoadBlueprint( "./Data/XML/ItemDefinitions.xml" );
	MapBlueprint::LoadBlueprint( "./Data/XML/MapDefinitions.xml" );
	ConstructBlueprint::LoadBlueprint( "./Data/XML/ConstructDefinitions.xml" );
	AbilityBlueprint::LoadBlueprint( "./Data/XML/AbilityDefinitions.xml" );

	//Load World Map First
	Map* worldMap = new Map( "./Data/XML/MapLayout1.xml", "./Data/XML/MapDetail1.xml" );
	worldMap->LoadTexture( "Tile", "./Data/Texture/DugeonTileSet.png", 2048, 1536 );
	worldMap->LoadTexture( "Actor", "./Data/Texture\\ActorSheet.png", 384, 272 );

	m_registeredMap.push_back( worldMap );

	Map* caveMap = new Map( "Cave" );
	caveMap->LoadTexture( "Tile", "./Data/Texture/DugeonTileSet.png", 2048, 1536 );
	caveMap->LoadTexture( "Actor", "./Data/Texture/ActorSheet.png", 384, 272 );
	m_registeredMap.push_back( caveMap );

	Map* scoutShipMap = new Map( "ScoutShip" );
	scoutShipMap->LoadTexture( "Tile", "./Data/Texture/DugeonTileSet.png", 2048, 1536 );
	scoutShipMap->LoadTexture( "Actor", "./Data/Texture/ActorSheet.png", 384, 272 );
	m_registeredMap.push_back( scoutShipMap );

	Map* battleShipMap = new Map( "BattleShip" );
	battleShipMap->LoadTexture( "Tile", "./Data/Texture/DugeonTileSet.png", 2048, 1536 );
	battleShipMap->LoadTexture( "Actor", "./Data/Texture/ActorSheet.png", 384, 272 );
	m_registeredMap.push_back( battleShipMap );

	g_currentMap = m_registeredMap[0];
	
	g_audioSystem->PlaySoundByName( g_currentMap->m_backgroundMusic.c_str(),1,true );
}

void World::SetupPerspectiveProjection()
{
	float aspect = (16.f/9.f);
	float fovX = 70.f;
	float fovY = (fovX / aspect);
	float zNear = 0.1f;
	float zFar = 1000.f;
	g_glRender->LoadIdentityMatrix();
	glLoadMatrixf(m_matrixStack.StackTop());
	gluPerspective(fovY,aspect,zNear,zFar);
// 	m_matrixStack.LoadIdentity();
// 	m_matrixStack.PushMatrix(Matrix44::CreatePerspectiveMatrix(fovY,aspect,zNear,zFar));
}

void World::ApplyCameraTransform(Camera3D camera)
{
	g_glRender->Rotatef(-90.f,1.f,0.f,0.f);
	g_glRender->Rotatef(90.f, 0.f,0.f,1.f);

	g_glRender->Rotatef(-camera.m_rollDegreesAboutX, 1.f,0.f,0.f);
	g_glRender->Rotatef(-camera.m_pitchDegreesAboutY, 0.f,1.f,0.f);
	g_glRender->Rotatef(-camera.m_yawDegreesAboutZ , 0.f,0.f,1.f);

	g_glRender->Translatef(-camera.m_cameraPosition.x, -camera.m_cameraPosition.y, -camera.m_cameraPosition.z);

	m_matrixStack.PushMatrix(Matrix44::CreateRotationAboutAxisDegrees(Vector3(1.f,0.f,0.f),-90.f));
	m_matrixStack.PushMatrix(Matrix44::CreateRotationAboutAxisDegrees(Vector3(0.f,0.f,1.f), 90.f));

	m_matrixStack.PushMatrix(Matrix44::CreateRotationAboutAxisDegrees(Vector3(1.f,0.f,0.f), -camera.m_rollDegreesAboutX));
	m_matrixStack.PushMatrix(Matrix44::CreateRotationAboutAxisDegrees(Vector3(0.f,1.f,0.f), -camera.m_pitchDegreesAboutY));
	m_matrixStack.PushMatrix(Matrix44::CreateRotationAboutAxisDegrees(Vector3(0.f,0.f,1.f), -camera.m_yawDegreesAboutZ));

	m_matrixStack.PushMatrix(Matrix44::CreateTranslationMatrix(Vector3(-camera.m_cameraPosition.x, -camera.m_cameraPosition.y, -camera.m_cameraPosition.z)));
	glLoadMatrixf(m_matrixStack.StackTop());
}

bool World::ProcessKeyDownEvent(HWND , UINT wmMessageCode, WPARAM wParam, LPARAM )
{
	unsigned char asKey = (unsigned char) wParam;
	switch( wmMessageCode )
	{
		case WM_KEYDOWN:
			isKeyDownKeyboard[asKey] = true;
			return true;
			break;

		case WM_KEYUP:
			isKeyDownKeyboard[asKey] = false;
			return true;
			break;
	}
	return true;
}

Vector2 World::GetMouseSinceLastChecked()
{
	POINT centerCursorPos = { 800, 450 };
	POINT cursorPos;
	GetCursorPos( &cursorPos );
	SetCursorPos( centerCursorPos.x, centerCursorPos.y );
	Vector2i mouseDeltaInts( cursorPos.x - centerCursorPos.x, cursorPos.y - centerCursorPos.y );
	Vector2 mouseDeltas( (float) mouseDeltaInts.x, (float) mouseDeltaInts.y );
	return mouseDeltas;
}

void World::OpenOrCloseConsole()
{
	if( isKeyDownKeyboard[ VK_OEM_3 ] && isKeyDownKeyboard[ VK_OEM_3 ] != isKeyDownLastFrame[ VK_OEM_3 ] )
	{
		if(g_theConsole->m_isOpen == false)
			g_theConsole->m_isOpen = true;
		else
			g_theConsole->m_isOpen = false;
	}
}

void World::UpdateCommandFromKeyboard()
{
	if( isKeyDownKeyboard[ 'W' ] )
	{
		m_camera2D.m_center.y += MOUSE_MOVE_RATE;
	}

	if( isKeyDownKeyboard[ 'S' ] )
	{
		m_camera2D.m_center.y -= MOUSE_MOVE_RATE;
	}

	if( isKeyDownKeyboard[ 'D' ] )
	{
		m_camera2D.m_center.x += MOUSE_MOVE_RATE;
	}

	if( isKeyDownKeyboard[ 'A' ] )
	{
		m_camera2D.m_center.x -= MOUSE_MOVE_RATE;
	}

	if( isKeyDownKeyboard[ 'Q' ] && isKeyDownKeyboard[ 'Q' ] != isKeyDownLastFrame[ 'Q' ] )
	{
		m_camera2D.m_viewportWidth *= ZOOM_FACTOR;
		m_camera2D.m_viewportHeight *= ZOOM_FACTOR;
	}

	if( isKeyDownKeyboard[ 'E' ] && isKeyDownKeyboard[ 'E' ] != isKeyDownLastFrame[ 'E' ] )
	{
		m_camera2D.m_viewportWidth /= ZOOM_FACTOR;
		m_camera2D.m_viewportHeight /= ZOOM_FACTOR;
	}

	if( isKeyDownKeyboard[ 'I' ] && isKeyDownKeyboard[ 'I' ] != isKeyDownLastFrame[ 'I' ] )
	{
		if( g_currentMap->m_visualizeMapInfo == false )
			g_currentMap->m_visualizeMapInfo = true;
		else
			g_currentMap->m_visualizeMapInfo = false;
	}

	if( isKeyDownKeyboard[ VK_SHIFT ] )
		g_isHoldingShift = true;
	else
		g_isHoldingShift = false;

	if( isKeyDownKeyboard[ 'F' ] && isKeyDownKeyboard[ 'F' ] != isKeyDownLastFrame[ 'F' ] )
	{
		if( m_playerController->m_selectedPlayerList.size() == 1 )
		{
			m_playerController->m_isFirstAbilitySelected = true;
			m_playerController->m_isSecondAbilitySelected = false;
		}
	}

	if( isKeyDownKeyboard[ 'G' ] && isKeyDownKeyboard[ 'G' ] != isKeyDownLastFrame[ 'G' ] )
	{
		if( m_playerController->m_selectedPlayerList.size() == 1 )
		{
			m_playerController->m_isFirstAbilitySelected = false;
			m_playerController->m_isSecondAbilitySelected = true;
		}
	}

	if( isKeyDownKeyboard[ '1' ] && isKeyDownKeyboard[ '1' ] != isKeyDownLastFrame[ '1' ] )
	{
		m_press1Count++;
		m_playerController->GetSelectedPlayerWithKeyboard(1);	
	}

	if( isKeyDownKeyboard[ '2' ] && isKeyDownKeyboard[ '2' ] != isKeyDownLastFrame[ '2' ] )
	{
		m_press2Count++;
		m_playerController->GetSelectedPlayerWithKeyboard(2);
	}

	if( isKeyDownKeyboard[ '3' ] && isKeyDownKeyboard[ '3' ] != isKeyDownLastFrame[ '3' ] )
	{
		m_press3Count++;
		m_playerController->GetSelectedPlayerWithKeyboard(3);
	}

	if( m_press1Count == 2 || m_press2Count == 2 || m_press3Count == 2 )
	{
		m_camera2D.m_center = m_playerController->m_selectedPlayerList[0]->m_worldPosition;
		m_press1Count = 0;
		m_press2Count = 0;
		m_press3Count = 0;
	}

	if( isKeyDownKeyboard[ '4' ] && isKeyDownKeyboard[ '4' ] != isKeyDownLastFrame[ '4' ] )
	{
		m_press4Count++;
		m_playerController->GetAllPlayerSelected();
	}

	if( isKeyDownKeyboard[ 'M' ] && isKeyDownKeyboard[ 'M' ] != isKeyDownLastFrame[ 'M' ] )
	{
		#ifdef USE_MEMORY_MANAGER
			GetMemoryPoolManager()->CheckMemoryLeaks();
		#endif // USE_MEMORY_MANAGER
	}

	if( isKeyDownKeyboard[ 'K' ] && isKeyDownKeyboard[ 'K' ] != isKeyDownLastFrame[ 'K' ] )
	{
		if( g_isFOWoff )
			g_isFOWoff = false;
		else
			g_isFOWoff = true;
	}

	if( m_press4Count == 2 )
	{
		WorldCoords2D averageWorldPos;
		for( unsigned int i = 0; i < m_playerController->m_selectedPlayerList.size(); i++ )
		{
			averageWorldPos += m_playerController->m_selectedPlayerList[i]->m_worldPosition; 
		}
		averageWorldPos = averageWorldPos / static_cast<float>( m_playerController->m_selectedPlayerList.size() );
		m_camera2D.m_center = averageWorldPos;
		m_press4Count = 0;
	}

	if( isKeyDownKeyboard[ '0' ] && isKeyDownKeyboard[ '0' ] != isKeyDownLastFrame[ '0' ] )
	{
		m_playerController->ClearPlayerList();
	}

	if( isKeyDownKeyboard[ VK_ESCAPE ] && isKeyDownKeyboard[ VK_ESCAPE ] != isKeyDownLastFrame[ VK_ESCAPE ] )
	{
		if( m_playerController->m_selectedPlayerList.size() > 0 )
			m_playerController->ClearPlayerList();
		else
			g_isQuitting = true;
	}
}

void World::Update( float elapsedTime )
{
	m_worldClock->AdvanceTime( static_cast<double>( elapsedTime ) );
	elapsedTime = static_cast<float>( m_worldClock->m_lastDeltaSecond );

	OpenOrCloseConsole();

	if( g_theConsole->m_isOpen )
	{
		m_worldClock->Pause();
		if( isKeyDownKeyboard[ VK_ESCAPE ] && isKeyDownKeyboard[ VK_ESCAPE ] != isKeyDownLastFrame[ VK_ESCAPE ] )
		{
			g_theConsole->m_isOpen = false;
 		}
	}
	else
	{
		UpdateCommandFromKeyboard();
		m_worldClock->Unpause();
	}

	//JobManager::Update();

	if( g_isRightMouseDown )
	{
		m_mouseClickFlashTime = 1.5f;
	}

	if( m_mouseClickFlashTime > 0.f)
	{
		m_mouseClickFlashTime -= elapsedTime;
		m_mouseClickColor = RGBColor( 1.f, 1.f, 1.f, m_mouseClickFlashTime / 1.5f );
		m_mouseClickRadius = 0.1f *  m_mouseClickFlashTime / 1.5f;
	}

	SwitchMapIfNeed();
	
	UpdateMouseWorldPosition();
	
	ProcessPlayerMovementAndAction();

	m_AIController->Update();

	elapsedTime = static_cast<float>( m_worldClock->m_lastDeltaSecond );

	m_playerController->Update( elapsedTime );

	g_currentMap->Update( elapsedTime );
	
	m_hud->Update();

	g_audioSystem->Update();

	for( unsigned int i = 0; i < g_currentMap->m_actorList.size(); i++ )
	{
		Actor* actor = g_currentMap->m_actorList[i];
		for( unsigned int abilityIndex = 0; abilityIndex < actor->m_abilityList.size(); abilityIndex++ )
		{
			actor->m_abilityList[abilityIndex]->Update(elapsedTime);
		}
	}

	for(int i = 0; i < 256; i++)
	{
		isKeyDownLastFrame[i] = isKeyDownKeyboard[i];
	}
	g_isLeftMouseDown = false;
	g_isRightMouseDown = false;
}

void World::Render()
{
	static ProfileSection renderProfile( "Render", TEST );
	renderProfile.StartProfileSection();

	g_glRender->ClearColor( 0.f,0.f,0.0f,1.f );
	g_glRender->ClearDepth( 1.f );
	g_glRender->Clear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	g_glRender->EnableDepthMask();
	g_glRender->Disable( GL_DEPTH_TEST );
	SetupOrthoProjection();

	g_currentMap->Render();
		
	m_playerController->RenderAbilityCursorForPlayer();

	for( unsigned int i = 0; i < g_currentMap->m_actorList.size(); i++ )
	{
		Actor* actor = g_currentMap->m_actorList[i];
		for( unsigned int abilityIndex = 0; abilityIndex < actor->m_abilityList.size(); abilityIndex++ )
		{
			actor->m_abilityList[abilityIndex]->RenderEffect();
		}
	}

	#ifdef USE_MEMORY_MANAGER
		g_memoryManager->PrintMemoryStatisticToScreen();
	#endif

	m_hud->Render();

	DrawCursor();

	RenderFlashCursor();

	if(g_theConsole->m_isOpen)
		g_theConsole->Render();

	renderProfile.EndProfileSection();
}


void World::RenderWorldAxes()
{
	g_glRender->PushMatrix();
	g_glRender->Disable(GL_DEPTH_TEST);
	g_glRender->Disable(GL_TEXTURE_2D);
	g_glRender->Scalef(2.f,2.f,2.f);
 	g_glRender->LineWidth(1.f);
 	g_glRender->Enable(GL_LINE_SMOOTH);
	g_glRender->BeginDraw(GL_LINES);
	{
		g_glRender->Color4f(1.f,0.f,0.f,1.f);
		g_glRender->Vertex3f(0.f,0.f,0.f);
		g_glRender->Vertex3f(1.f,0.f,0.f);

		g_glRender->Color4f(0.f,1.f,0.f,1.f);
		g_glRender->Vertex3f(0.f,0.f,0.f);
		g_glRender->Vertex3f(0.f,1.f,0.f);

		g_glRender->Color4f(0.f,0.f,1.f,1.f);
		g_glRender->Vertex3f(0.f,0.f,0.f);
		g_glRender->Vertex3f(0.f,0.f,1.f);
	}
	g_glRender->EndDraw();
	
	g_glRender->Enable(GL_DEPTH_TEST);
	g_glRender->LineWidth(3.f);
	g_glRender->BeginDraw(GL_LINES);
	{
		g_glRender->Color4f(1.f,0.f,0.f,1.f);
		g_glRender->Vertex3f(0.f,0.f,0.f);
		g_glRender->Vertex3f(1.f,0.f,0.f);

		g_glRender->Color4f(0.f,1.f,0.f,1.f);
		g_glRender->Vertex3f(0.f,0.f,0.f);
		g_glRender->Vertex3f(0.f,1.f,0.f);

		g_glRender->Color4f(0.f,0.f,1.f,1.f);
		g_glRender->Vertex3f(0.f,0.f,0.f);
		g_glRender->Vertex3f(0.f,0.f,1.f);
	}
	g_glRender->EndDraw();

	g_glRender->LineWidth(1.f);
	g_glRender->Color4f(1.f,1.f,1.f,1.f);
 	g_glRender->Enable(GL_TEXTURE_2D);
	g_glRender->PopMatrix();
}

void World::ClearWorld()
{
	m_matrixStack.ClearMatrixStack();
}

void World::SetupOrthoProjection()
{
	glLoadIdentity();
	glOrtho( 0, m_camera2D.m_viewportWidth, 0, m_camera2D.m_viewportHeight , 0.0, 1.0 );
	glTranslatef( -( m_camera2D.m_center.x - m_camera2D.m_viewportWidth * 0.5f ), -( m_camera2D.m_center.y - m_camera2D.m_viewportHeight * 0.5f ), 0.f );
}

void World::UpdateMouseWorldPosition()
{
	POINT cursorScreenPos;
	GetCursorPos( &cursorScreenPos );

	POINT cursorClientPos = cursorScreenPos;
	ScreenToClient( g_hWnd, &cursorClientPos );
	Vector2 mouseClientPos( (float)cursorClientPos.x, (float)cursorClientPos.y );
	Vector2 invertedMouseClientPos( mouseClientPos.x, WINDOW_PHYSICAL_HEIGHT - mouseClientPos.y );
	float scaleFactor = m_camera2D.m_viewportWidth / WINDOW_PHYSICAL_WIDTH;
	Vector2 scaleMouseClientPos = scaleFactor * invertedMouseClientPos;
	m_hud->m_mouseScreenPos = 0.01f * invertedMouseClientPos;
	g_mouseScreenPosition = 0.01f * invertedMouseClientPos;
	scaleMouseClientPos += Vector2( m_camera2D.m_center.x - m_camera2D.m_viewportWidth * 0.5f , m_camera2D.m_center.y - m_camera2D.m_viewportHeight * 0.5f );
	g_mouseWorldPosition = scaleMouseClientPos;
}

void World::DrawCursor()
{
	g_glRender->Disable( GL_TEXTURE_2D );
	g_glRender->PushMatrix();
	g_glRender->Translated( g_mouseWorldPosition.x, g_mouseWorldPosition.y, 0.f );
	g_glRender->Scalef( .1f, .1f, .1f );
	g_glRender->LineWidth( 3.f );
	g_glRender->Color3f( 1.f, 0.3f, 0.9f );
	g_glRender->BeginDraw( GL_LINES );
	g_glRender->Vertex3f( 1.f, 1.f, 0.f );
	g_glRender->Vertex3f( -1.f, -1.f, 0.f );
	g_glRender->Vertex3f( -1.f, 1.f, 0.f );
	g_glRender->Vertex3f( 1.f, -1.f, 0.f );
	g_glRender->EndDraw();
	g_glRender->PopMatrix();
}

bool World::ProcessMouseDownEvent(HWND, UINT wmMessageCode, WPARAM, LPARAM)
{
	switch( wmMessageCode )
	{
		case WM_LBUTTONDOWN:
			g_isLeftMouseDown = true;	
			break;

		case WM_LBUTTONUP:
			g_isLeftMouseDown = false;	
			break;

		case WM_RBUTTONDOWN:
			g_isRightMouseDown = true;
			break;

		case WM_RBUTTONUP:
			g_isRightMouseDown = false;	
			break;
	}
	return true;
}

void World::ProcessPlayerMovementAndAction()
{

	if( g_isLeftMouseDown && !m_hud->IsMouseIsInHudArea() 
		&& !m_playerController->m_isFirstAbilitySelected
		&& !m_playerController->m_isSecondAbilitySelected )
	{
		m_playerController->GetSelectedPlayerWithMouse();
		g_isLeftMouseDown = false;
	}

	if( g_isRightMouseDown && ( !m_hud->IsMouseIsInHudArea() || m_playerController->m_selectedPlayerList.size() > 1 ) )
	{
		m_playerController->SetPlayerDestinationToSpecifiedTile( g_mouseWorldPosition );
		m_playerController->SetTargetForSelectedPlayer();
		g_isRightMouseDown = false;
	}
}

void World::SwitchMapIfNeed()
{
	for( unsigned int i = 0; i < g_currentMap->m_actorList.size(); i++ )
	{
		Actor* actor = g_currentMap->m_actorList[i];
		if( actor->m_actorBlueprint->m_isPlayer )
		{
			TileIndex tileIndex = ConvertWorldCoords2DToTileIndex( actor->m_worldPosition, g_currentMap->m_mapWidthX );
			if( g_currentMap->m_tileList[tileIndex].m_tileBlueprint->m_name.compare("CaveEntrance") == 0 )
			{
				m_mapIndex = 1;
				break;
			}
			else if ( g_currentMap->m_tileList[tileIndex].m_tileBlueprint->m_name.compare("ScoutShipEntrance") == 0 )
			{
				m_mapIndex = 2;
				break;
			}
			else if ( g_currentMap->m_tileList[tileIndex].m_tileBlueprint->m_name.compare("BattleShipEntrance") == 0 )
			{
				m_mapIndex = 3;
				break;
			}
			else if ( g_currentMap->m_tileList[tileIndex].m_tileBlueprint->m_name.compare("ToWorld") == 0 )
			{
				m_mapIndex = 0;
				break;
			}
		}
	}

	if( m_mapIndex != m_previousMapIndex )
	{
		Map* mapToSwitch = m_registeredMap[m_mapIndex];

		int index;
		int playerCount = 0;

		for( index = 0; index < (int)g_currentMap->m_actorList.size(); index++ )
		{
			if( g_currentMap->m_actorList[index]->m_actorBlueprint->m_isPlayer )
			{
				Actor* player = g_currentMap->m_actorList[index];
				player->m_worldPosition = mapToSwitch->m_startPositionList[playerCount];
				m_camera2D.m_center = player->m_worldPosition;
				player->m_goalWorldPosition = player->m_worldPosition;
				player->m_path.clear();
				mapToSwitch->m_actorList.push_back(player);
				g_currentMap->m_actorList.erase( g_currentMap->m_actorList.begin() + index );
				index = -1;
				playerCount++;
			}
		}

		//std::map< 
		g_currentMap->s_actorWaitingForPathList.clear();
		g_currentMap = mapToSwitch;

		m_playerController->m_currentMap = g_currentMap;
		m_AIController->m_AIList.clear();
		m_AIController->m_currentMap =  g_currentMap;
		m_AIController->InitializeAIStrategy();
		m_hud->m_currentMap = *g_currentMap;

		m_previousMapIndex = m_mapIndex;
	}
}

void World::RenderFlashCursor()
{
	g_glRender->Draw2DFilledCircle( g_mouseWorldPosition, m_mouseClickRadius, m_mouseClickColor );
}


