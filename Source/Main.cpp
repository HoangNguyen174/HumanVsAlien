#ifndef MAIN
#define MAIN
#define WIN32_LEAN_AND_MEAN


#include <math.h>
#include <stdlib.h>
#include <cassert>
#include <crtdbg.h>
#include "SoulStoneEngine/Utilities/GameCommon.hpp"
#include "SoulStoneEngine/Render/GLRender.hpp"
#include "SoulStoneEngine/Utilities/Time.hpp"
#include "SoulStoneEngine/Utilities/Clock.hpp"
#include "SoulStoneEngine/Utilities/ProfileSection.hpp"
#include "World.hpp"
#include "SoulStoneEngine/JobSystem/MemoryPoolManager.hpp"

#pragma comment( lib, "opengl32" ) // Link in the OpenGL32.lib static library
#pragma comment( lib, "glu32")
//-----------------------------------------------------------------------------------------------
#define UNUSED(x) (void)(x);

//-----------------------------------------------------------------------------------------------	
struct AppCommand
{
	std::string commandName;
	std::vector<std::string> argList;
};

HWND g_hWnd = nullptr;
HDC g_displayDeviceContext = nullptr;
HGLRC g_openGLRenderingContext = nullptr;
const char* APP_NAME = "HumanVsAlien";
const int OFFSET_FROM_WINDOW_DESKTOP = 50;
const int WINDOW_PHYSICAL_WIDTH = 1600;
const int WINDOW_PHYSICAL_HEIGHT = 900;
const float WINDOW_VIRTUAL_HEIGHT = 9.0f;
const float WINDOW_VIRTUAL_WIDTH = 16.f;
const double VIEW_LEFT = 0.0;
const double VIEW_RIGHT = 1600.0;
const double VIEW_BOTTOM = 0.0;
const double VIEW_TOP = VIEW_RIGHT * static_cast<double>(WINDOW_PHYSICAL_HEIGHT) / static_cast<double>(WINDOW_PHYSICAL_WIDTH);
std::vector<AppCommand> g_commandList;

void ParseCommandString( std::string& command );
int ProcessCommandString( LPSTR commandLineString );

//-----------------------------------------------------------------------------------------------
LRESULT CALLBACK WindowsMessageHandlingProcedure( HWND windowHandle, UINT wmMessageCode, WPARAM wParam, LPARAM lParam )
{
	unsigned char asKey = (unsigned char) wParam;
	switch( wmMessageCode )
	{
		case WM_CLOSE:
		case WM_DESTROY:
		case WM_QUIT:
			g_isQuitting = true;
			return 0;
			break;
		case WM_KEYDOWN:
// 			if( asKey == VK_ESCAPE && !g_theConsole->m_isOpen )
// 			{
// 				g_isQuitting = true;
// 				return 0;
// 			}
			if( !g_theConsole->m_isOpen && theWorld->ProcessKeyDownEvent(windowHandle,wmMessageCode,wParam,lParam) )
			{
				return 0;
			}
			if( g_theConsole->m_isOpen )
			{
				g_theConsole->ProcessSpecialKeyboard(asKey);
				return 0;
			}

		case WM_KEYUP:
			if( theWorld->ProcessKeyDownEvent(windowHandle,wmMessageCode,wParam,lParam) )
			{
				return 0;
			}
			break;
		case WM_CHAR:
			if( g_theConsole->m_isOpen )
			{
				g_theConsole->ProcessKeyboard(asKey);
				return 0;
			}
			break;
		case WM_LBUTTONDOWN:
			if( theWorld->ProcessMouseDownEvent(windowHandle,wmMessageCode,wParam,lParam) )
			{
				return 0;
			}
			break;
		case WM_LBUTTONUP:
			if( theWorld->ProcessMouseDownEvent(windowHandle,wmMessageCode,wParam,lParam) )
			{
				return 0;
			}
			break;
		case WM_RBUTTONDOWN:
			if( theWorld->ProcessMouseDownEvent(windowHandle,wmMessageCode,wParam,lParam) )
			{
				return 0;
			}
			break;
	}

	return DefWindowProc( windowHandle, wmMessageCode, wParam, lParam );
}


//-----------------------------------------------------------------------------------------------
void CreateOpenGLWindow( HINSTANCE applicationInstanceHandle )
{
	// Define a window class
	WNDCLASSEX windowClassDescription;
	memset( &windowClassDescription, 0, sizeof( windowClassDescription ) );
	windowClassDescription.cbSize = sizeof( windowClassDescription );
	windowClassDescription.style = CS_OWNDC; // Redraw on move, request own Display Context
	windowClassDescription.lpfnWndProc = static_cast< WNDPROC >( WindowsMessageHandlingProcedure ); // Assign a win32 message-handling function
	windowClassDescription.hInstance = GetModuleHandle( NULL );
	windowClassDescription.hIcon = NULL;
	windowClassDescription.hCursor = NULL;
	windowClassDescription.lpszClassName = TEXT( "Simple Window Class" );
	RegisterClassEx( &windowClassDescription );

	const DWORD windowStyleFlags = WS_CAPTION | WS_BORDER | WS_THICKFRAME | WS_SYSMENU | WS_OVERLAPPED;
	const DWORD windowStyleExFlags = WS_EX_APPWINDOW;

	RECT desktopRect;
	HWND desktopWindowHandle = GetDesktopWindow();
	GetClientRect( desktopWindowHandle, &desktopRect );

	RECT windowRect = { OFFSET_FROM_WINDOW_DESKTOP + 0, OFFSET_FROM_WINDOW_DESKTOP + 0, OFFSET_FROM_WINDOW_DESKTOP + WINDOW_PHYSICAL_WIDTH, 50 + WINDOW_PHYSICAL_HEIGHT };
	AdjustWindowRectEx( &windowRect, windowStyleFlags, FALSE, windowStyleExFlags );

	WCHAR windowTitle[ 1024 ];
	MultiByteToWideChar( GetACP(), 0, APP_NAME, -1, windowTitle, sizeof(windowTitle)/sizeof(windowTitle[0]) );
	g_hWnd = CreateWindowEx(
		windowStyleExFlags,
		windowClassDescription.lpszClassName,
		windowTitle,
		windowStyleFlags,
		windowRect.left,
		windowRect.top,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		NULL,
		NULL,
		applicationInstanceHandle,
		NULL );

	ShowWindow( g_hWnd, SW_SHOW );
	SetForegroundWindow( g_hWnd );
	SetFocus( g_hWnd );
	
	g_displayDeviceContext = GetDC( g_hWnd );
	HCURSOR cursor = LoadCursor( NULL, NULL );
	SetCursor( cursor );

	PIXELFORMATDESCRIPTOR pixelFormatDescriptor;
	memset( &pixelFormatDescriptor, 0, sizeof( pixelFormatDescriptor ) );
	pixelFormatDescriptor.nSize			= sizeof( pixelFormatDescriptor );
	pixelFormatDescriptor.nVersion		= 1;
	pixelFormatDescriptor.dwFlags		= PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pixelFormatDescriptor.iPixelType	= PFD_TYPE_RGBA;
	pixelFormatDescriptor.cColorBits	= 24;
	pixelFormatDescriptor.cDepthBits	= 24;
	pixelFormatDescriptor.cAccumBits	= 0;
	pixelFormatDescriptor.cStencilBits	= 8;

	int pixelFormatCode = ChoosePixelFormat( g_displayDeviceContext, &pixelFormatDescriptor );
	SetPixelFormat( g_displayDeviceContext, pixelFormatCode, &pixelFormatDescriptor );
	g_openGLRenderingContext = wglCreateContext( g_displayDeviceContext );
	wglMakeCurrent( g_displayDeviceContext, g_openGLRenderingContext );
	g_glRender->Enable(GL_LINE_SMOOTH);
	g_glRender->Enable(GL_BLEND);
	g_glRender->BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

//-----------------------------------------------------------------------------------------------
void RunMessagePump()
{
	//ShowCursor( FALSE );
	MSG queuedMessage;
	for( ;; )
	{
		const BOOL wasMessagePresent = PeekMessage( &queuedMessage, NULL, 0, 0, PM_REMOVE );
		if( !wasMessagePresent )
		{
			break;
		}

		TranslateMessage( &queuedMessage );
		DispatchMessage( &queuedMessage );
	}
}

//-----------------------------------------------------------------------------------------------
void Update(float elapsedTime)
{
	theWorld->Update(elapsedTime);
}


//-----------------------------------------------------------------------------------------------
void Render()
{
	g_glRender->ClearColor( 0.f, 0.f, 0.f, 1.f );
	g_glRender->Clear( GL_COLOR_BUFFER_BIT );
	theWorld->Render();

	

	const double minimumFrameSeconds = (1.0 / 60.0);
	static double lastTimeHere = GetCurrentTimeSeconds();
	while( GetCurrentTimeSeconds() - lastTimeHere < minimumFrameSeconds )
	{
	}
	lastTimeHere = GetCurrentTimeSeconds();

	SwapBuffers( g_displayDeviceContext );
}


//-----------------------------------------------------------------------------------------------
void RunFrame(float elapsedTime)
{
	RunMessagePump();
	Update(elapsedTime);
	Render();

	theWorld->ClearWorld();
}

void Initialize(HINSTANCE applicationInstanceHandle)
{
	InitializeTime();
	CreateOpenGLWindow(applicationInstanceHandle);
	ArchiveManager::Initialize();
	Clock::s_masterClock.m_timeScale = 1.0;
	g_glRender = new GLRender();
	g_theConsole = new DeveloperConsole();
	theWorld = new World();
}

void Shutdown()
{
	delete g_glRender;
	delete g_theConsole;
	delete theWorld;
}

//-----------------------------------------------------------------------------------------------
int WINAPI WinMain( HINSTANCE applicationInstanceHandle, HINSTANCE, LPSTR commandLineString, int )
{
	UNUSED( commandLineString );
	int result = ProcessCommandString( commandLineString );

	#ifdef USE_MEMORY_MANAGER
		g_memoryManager = GetMemoryPoolManager();
	#endif

	if( result == -1 )
	{
		DebuggerPrintf( "Error Executing Command Line. Exit Program!.\n" );
		exit(0);
	}

	Initialize(applicationInstanceHandle);
	const double minimumFrameSeconds = (1.0 / 60.0);
	while( !g_isQuitting )	
	{
		static float lastTimeHere = static_cast<float>( GetCurrentTimeSeconds() );
		while( GetCurrentTimeSeconds() - lastTimeHere < minimumFrameSeconds )
		{
		}	
		//lastTimeHere = static_cast<float>( GetCurrentTimeSeconds() );
		//float elapsedTime = static_cast<float>( GetCurrentTimeSeconds() ) - lastTimeHere;
		Clock::s_masterClock.AdvanceTime( minimumFrameSeconds );

		ProfileSection::s_mainLoopProfileSection.StartProfileSection();
		RunFrame( static_cast<float>( Clock::s_masterClock.m_lastDeltaSecond ) );
		ProfileSection::s_mainLoopProfileSection.EndProfileSection();
	}
	
	// #if defined( _WIN32 ) && defined( _DEBUG )
	// 	assert( _CrtCheckMemory() );
	// 	_CrtDumpMemoryLeaks();
	// #endif
	
	#ifdef USE_MEMORY_MANAGER
		g_memoryManager->CheckMemoryLeaks();
	#endif

	Shutdown();
	return 0;
}

void ParseCommandString( std::string& command )
{
	if( command.size() < 2 )
		return;

	if( command[0] != '-' )
		return;

	while( command.size() > 0 )
	{
		int spacePosition = command.find( " " );

		AppCommand commandlet;
		std::string commandName = command.substr( 1, spacePosition - 1 );
		commandlet.commandName = commandName;

		command.erase( 0, spacePosition + 1 );
		
		int nextHypenPosition = command.find( "-" );
		std::string argList;

		if( nextHypenPosition == -1 )
		{
			argList = command.substr( 0, command.size() );
			command.erase( 0, command.size() );
		}
		else
		{
			argList = command.substr( 0, nextHypenPosition );
			command.erase( 0, nextHypenPosition );
		}

		while( argList.size() > 0 )
		{
			std::string argument;
			if( argList[0] == '\"' )
			{
				argList.erase( 0, 1 );
				int endQuotePos = argList.find("\"");
				if( endQuotePos == -1 )
					return;
				argument = argList.substr( 0, endQuotePos );
				argList.erase( 0, endQuotePos + 2 );
				commandlet.argList.push_back( argument );
				continue;
			}

			spacePosition = argList.find( " " );


			if( spacePosition == -1 )
			{
				argument = argList.substr( 0, argList.size() );
				argList.clear();
			}
			else
			{
				argument = argList.substr( 0, spacePosition );
				argList.erase( 0, spacePosition + 1 );
			}

			commandlet.argList.push_back( argument );
		}

		g_commandList.push_back( commandlet );
	}
}

int GenerateFiles( const std::string& fileCountStr, const std::string& kbPerFileStr )
{
	int fileCount = std::stoi( fileCountStr );
	int kbPerFile = std::stoi( kbPerFileStr );

	std::string filePath = "./Data/Files/";
	std::string fileName;

	for( int fileIndex = 0; fileIndex < fileCount; fileIndex++ )
	{
		std::string fileIndexStr = std::to_string( static_cast< long long >( fileIndex ) );

		if( fileIndex < 10 )
			fileName = "generated_" + kbPerFileStr + "kb_file_00" + fileIndexStr + ".dat";
		else if( fileIndex >= 10 && fileIndex < 100 )
			fileName = "generated_" + kbPerFileStr + "kb_file_0" + fileIndexStr + ".dat";
		else 
			fileName = "generated_" + kbPerFileStr + "kb_file_" + fileIndexStr + ".dat";

		std::string fullName = filePath + fileName;

		char* buffer = nullptr;
		int sizeOfBuffer = kbPerFile * 1024;
		buffer = new char[ sizeOfBuffer ];

		ofstream outFile( fullName );

		if( !outFile.is_open() )
		{
			DebuggerPrintf( "Could not open file to write.\n" );
			return -1;
		}

		outFile.write( buffer, sizeOfBuffer );
		outFile.flush();
		outFile.close();

		delete[] buffer;
	}

	return 1;
}

int ProcessCommandString( LPSTR commandLineString )
{
	std::string command = commandLineString;
	ParseCommandString( command );

	for( unsigned int i = 0; i < g_commandList.size(); i++ )
	{
		AppCommand& command = g_commandList[i];

		//process generateFiles command
		if( command.commandName.compare( "generateFiles") == 0)
		{
			if( command.argList.size() != 2 )
			{
				DebuggerPrintf( "Invalid number of argument. Usage: -generateFiles <fileCount> <KBPerFile>.\n" );
				exit(0);
			}

			return GenerateFiles( command.argList[0], command.argList[1] );
		}
		if( command.commandName.compare( "loadMode" ) == 0 )
		{
			if( command.argList.size() != 1 )
			{
				DebuggerPrintf( "Invalid number of argument. Usage: -loadMode <mode>.\n" );
				DebuggerPrintf( "1. Load from disk.\n" );
				DebuggerPrintf( "2. Load from archive.\n" );
				DebuggerPrintf( "3. Prefer load from disk.\n" );
				DebuggerPrintf( "4. Prefer load from archive.\n" );
				exit(0);
			}

			int choice = atoi( command.argList[0].c_str() );
			switch( choice )
			{
				case 1: ArchiveManager::s_loadMode = LOAD_FROM_DISK;
						break;
				case 2: ArchiveManager::s_loadMode = LOAD_FROM_ARCHIVE;
						break;
				case 3: ArchiveManager::s_loadMode = PREFER_LOAD_FROM_DISK;
						break;
				case 4: ArchiveManager::s_loadMode = PREFER_LOAD_FROM_ARCHIVE;
						break;
			}

			return 1;
		}
	}
	return -1;
}


#endif