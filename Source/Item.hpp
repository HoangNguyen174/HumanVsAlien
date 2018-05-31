#ifndef ITEM_H
#define ITEM_H
#include "ItemBlueprint.hpp"
#include "SoulStoneEngine/Utilities/GameCommon.hpp"

class Item
{
	public:
		static int				s_ID;
		int						m_itemID;
		ItemBlueprint*			m_itemBlueprint;
		WorldCoords2D			m_worldPosition;
		float					m_damage;
		float					m_attackRate;
		float					m_attackRange;
		float					m_toughness;
		Texture*				m_texture;
		bool					m_isVBODirty;
		float					m_radius;
		std::vector<Vertex2D>	m_vertexList;
		GLuint					m_vboID;
		int						m_numVertex;
		
	public:
		Item( const std::string& blueprintName, const WorldCoords2D& worldPosition );
		~Item();
		void Render();

	private:
		void PopulateFromBlueprint( const ItemBlueprint& blueprint );
		void CreateVBO();
		void RenderVBO();
};

#endif