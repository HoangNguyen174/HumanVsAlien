#ifndef CONSTRUCT_H
#define CONSTRUCT_H
#include "ConstructBlueprint.hpp"
#include "SoulStoneEngine/Utilities/GameCommon.hpp"
#include "Ability.hpp"

class Construct
{
	public:
		static int					s_ID;
		int							m_constructID;
		ConstructBlueprint*			m_constructBlueprint;
		WorldCoords2D				m_worldPosition;
		Texture*					m_texture;
		bool						m_isVBODirty;
		float						m_radius;
		std::vector<Vertex2D>		m_vertexList;
		GLuint						m_vboID;
		int							m_numVertex;
		std::vector<Ability*>		m_abilityList;
		bool						m_isOverlappable;
	
	public:
		Construct( const std::string& blueprintName, const WorldCoords2D& worldPosition );
		void Render();
		void Update( float elapsedTime );

	private:
		void PopulateFromBlueprint( const ConstructBlueprint& blueprint );
		void CreateVBO();
		void RenderVBO();
};


#endif