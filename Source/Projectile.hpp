#ifndef PROJECTILE_H
#define PROJECTILE_H
#include "SoulStoneEngine/Utilities/GameCommon.hpp"
#include "Actor.hpp"

class Projectile
{
	public:
		Vector2			m_velocity;
		WorldCoords2D	m_position;
		float			m_speed;
		RGBColor		m_color;
		float			m_radius;
		float			m_remainingTimeSec;
		Actor*			m_owner;

	public:
		Projectile( Actor* owner );
		void Update( float elapsedTime );
		void Render();
};


#endif