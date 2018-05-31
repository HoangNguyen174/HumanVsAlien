#ifndef WORLD_H
#define WORLD_H
#include "SoulStoneEngine/Utilities/GameCommon.hpp"
#include "SoulStoneEngine/Render/GLRender.hpp"
#include "SoulStoneEngine/Render/OpenGLShader.hpp"
#include "SoulStoneEngine/Render/OpenGLShaderProgram.hpp"
#include "SoulStoneEngine/Debugger/DeveloperConsole.hpp"
#include "SoulStoneEngine/Render/BitmapFont.hpp"
#include "SoulStoneEngine/Utilities/Matrix44.hpp"
#include "SoulStoneEngine/Utilities/MatrixStack.hpp"
#include "SoulStoneEngine/Debugger/DebugPoint.hpp"
#include "SoulStoneEngine/Render/Material.hpp"
#include "SoulStoneEngine/Render/FBO.hpp"
#include "SoulStoneEngine/Utilities/XMLLoaderUtilities.hpp"
#include "SoulStoneEngine/Camera/Camera2D.hpp"
#include "SoulStoneEngine/Camera/Camera3D.hpp"
#include "SoulStoneEngine/Utilities/Clock.hpp"
#include "SoulStoneEngine/JobSystem/Thread.hpp"
#include "SoulStoneEngine/Audio/AudioSystem.hpp"
#include "SoulStoneEngine/JobSystem/ArchiveManager.hpp"
#include "MapBlueprint.hpp"
#include "Map.hpp"
#include "ActorBlueprint.hpp"
#include "TileBlueprint.hpp"
#include "PlayerController.hpp"
#include "HUD.hpp"
#include "AIController.hpp"
#include "ConstructBlueprint.hpp"
#include "AbilityBlueprint.hpp"
#include "SoulStoneEngine/JobSystem/MemoryPoolManager.hpp"

class World
{
	public:
		Camera3D						m_camera3D;
		Camera2D						m_camera2D;
		MatrixStack						m_matrixStack;
		bool							m_renderWorldOriginAxes;
		std::vector<Map*>				m_registeredMap;
		float							m_zoomFactor;
		PlayerController*				m_playerController;
		AIController*					m_AIController;
		HUD*							m_hud;
		int								m_mapIndex;
		int								m_previousMapIndex;
		int								m_press1Count;
		int								m_press2Count;
		int								m_press3Count;
		int								m_press4Count;
		Clock*							m_worldClock;
		unsigned long					m_hashValue;

	private:
		RGBColor						m_mouseClickColor;
		float							m_mouseClickRadius;
		float							m_mouseClickFlashTime;

	public:
		World();
		~World();
		void Initialize();
		void LoadMapAndBlueprint();
		bool ProcessKeyDownEvent(HWND windowHandle, UINT wmMessageCode, WPARAM wParam, LPARAM lParam );
		bool ProcessMouseDownEvent(HWND windowHandle, UINT wmMessageCode, WPARAM wParam, LPARAM lParam );
		bool ProcessKeyUpEnvent(HWND windowHandle, UINT wmMessageCode, WPARAM wParam, LPARAM lParam );
		void SetupPerspectiveProjection();
		void SetupOrthoProjection();
		void ApplyCameraTransform(Camera3D camera);
		void OpenOrCloseConsole();
		Vector2 GetMouseSinceLastChecked();
		void Update(float elapsedTime);
		void ClearWorld();
		void UpdateCommandFromKeyboard();
		void UpdateMouseWorldPosition();
		void Render();
		void RenderWorldAxes();
		void DrawCursor();
		void ProcessPlayerMovementAndAction();
		void SwitchMapIfNeed();
		void RenderFlashCursor();
};

extern Map*	g_currentMap;
extern World* theWorld;
extern bool isKeyDownKeyboard[256];
extern bool isKeyDownLastFrame[256];
extern WorldCoords2D g_mouseWorldPosition;
extern WorldCoords2D g_mouseScreenPosition;
extern bool g_isLeftMouseDown;
extern bool g_isRightMouseDown;
extern bool g_isHoldingShift;
extern bool g_isQuitting;	
extern bool g_isFOWoff;
extern AudioSystem* g_audioSystem;

#ifdef USE_MEMORY_MANAGER
	extern MemoryPoolManager* g_memoryManager;
#endif //USE_MEMORY_MANAGER

#endif