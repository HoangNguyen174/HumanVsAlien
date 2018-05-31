#include "Projectile.hpp"
#include "World.hpp"

Projectile::Projectile( Actor* owner )
{
	m_owner = owner;
	m_position = m_owner->m_worldPosition;
	m_color = RGBColor::Red();
	m_speed = 3.f;
	m_radius = 0.1f;
	m_remainingTimeSec = 3.f;
}

void Projectile::Update( float elapsedTime )
{
	if( m_remainingTimeSec > 0 )
		m_remainingTimeSec -= elapsedTime;

	if( m_velocity.CalcLengthSquare() != 0 )
	{
		m_velocity = m_velocity.Normalize();
		m_position += m_velocity * elapsedTime * m_speed;
	}
}

void Projectile::Render()
{
	g_glRender->Draw2DFilledCircle( m_position, m_radius, m_color );
}