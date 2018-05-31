#include "HUD.hpp"
#include "World.hpp"
#include "SoulStoneEngine/JobSystem/MemoryPoolManager.hpp"

bool	IsInCharacterHud;
bool	IsInItemHud;
bool	IsInAbilityHud;
bool	IsInventoryAndAbilityShowing;

HUD::HUD( const Map& map )
{
	m_currentMap = map;
	m_selectedActor = nullptr;
	m_texture = Texture::CreateOrGetTexture( "./Data/Texture/ItemSheet.png" );

	m_veteran = nullptr;
	m_doctor = nullptr;
	m_engineer = nullptr;

	IsInCharacterHud = false;
	IsInItemHud = false;
	IsInAbilityHud = false;

	IsInventoryAndAbilityShowing = false;

	for ( unsigned int i = 0; i < map.m_actorList.size(); i++ )
	{
		Actor* actor = map.m_actorList[i];
		if( actor->m_actorBlueprint->m_isPlayer == true )
		{
			std::string actorName = actor->m_actorBlueprint->m_name;
			if( actorName.compare( "Veteran") == 0 )
			{
				m_veteran = actor;
			}
			if ( actorName.compare( "Engineer") == 0 )
			{
				m_engineer = actor;
			}
			if ( actorName.compare( "Doctor" ) == 0 )
			{
				m_doctor = actor;
			}
		}
	}
}

void HUD::Update()
{
	ProcessMouseClick();
}

void HUD::Render()
{
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0.f, 16.f, 0.f, 9.f, 0.f, 1.f );

	RenderPlayerPotrait();
	RenderPlayerHealthAndManaBar();
	
	if( m_playerController->m_selectedPlayerList.size() == 1)
		RenderInventory( m_playerController->m_selectedPlayerList[0] );

	glPopMatrix();
}

void HUD::RenderPlayerHealthAndManaBar()
{
	//veteran
	float ratio = 0.f;
	std::string health;
	std::string maxHealth;
	std::string energy;
	std::string maxEnergy;

	if( m_veteran != nullptr )
	{
		ratio = ( m_veteran->m_health / m_veteran->m_maximumHealth );
		ratio = MathUtilities::Clamp( 0.f, 1.f, ratio );

		g_glRender->Draw2DFilledRectangle( Vector2( 0.2f, 6.25f ), 1.2f * ratio, .2f, RGBColor( 0.f,1.f,0.f,0.5f ) );
		g_glRender->Draw2DHollowRectangle( Vector2( 0.2f, 6.25f ), 1.2f, .2f, RGBColor( 1.f,0.f,0.f,1.f ) );
		g_glRender->RenderText( Vector2( 0.2f, 6.25f), RGBColor::White() , nullptr, nullptr, 0.19f, "Health" );
		
		health = std::to_string( static_cast<long double>( m_veteran->m_health ) );
		maxHealth = std::to_string( static_cast<long double>( m_veteran->m_maximumHealth ) );
		g_glRender->RenderText( Vector2( 0.75f, 6.25f), RGBColor::White() , nullptr, nullptr, 0.19f, health + "/" + maxHealth );

		ratio = ( m_veteran->m_energy / m_veteran->m_maximumEnergy );
		ratio = MathUtilities::Clamp( 0.f, 1.f, ratio );

		g_glRender->Draw2DFilledRectangle( Vector2( 0.2f, 6.f ), 1.2f * ratio, .2f, RGBColor(0.f,0.f,1.f,0.5f) );
		g_glRender->Draw2DHollowRectangle( Vector2( 0.2f, 6.f ), 1.2f, .2f, RGBColor(1.f,0.f,0.f,1.f));
		g_glRender->RenderText( Vector2( 0.2f, 6.f), RGBColor(1.f,1.f,1.f,1.f) , nullptr, nullptr, 0.2f, "Energy" );

		energy = std::to_string( static_cast<long double>( m_veteran->m_energy ) );
		maxEnergy = std::to_string( static_cast<long double>( m_veteran->m_maximumEnergy ) );
		g_glRender->RenderText( Vector2( 0.75f, 6.f), RGBColor::White() , nullptr, nullptr, 0.19f, energy + "/" + maxEnergy );
	}

	if( m_engineer != nullptr )
	{
		ratio = ( m_engineer->m_health / m_engineer->m_maximumHealth );
		ratio = MathUtilities::Clamp( 0.f, 1.f, ratio );

		g_glRender->Draw2DFilledRectangle( Vector2( 0.2f, 4.25f), 1.2f * ratio, .2f, RGBColor(0.f,1.f,0.f,0.5f));
		g_glRender->Draw2DHollowRectangle( Vector2( 0.2f, 4.25f), 1.2f, .2f, RGBColor(1.f,0.f,0.f,1.f));
		g_glRender->RenderText( Vector2( 0.2f, 4.25f), RGBColor(1.f,1.f,1.f,1.f) , nullptr, nullptr, 0.2f, "Health" );
		
		health = std::to_string( static_cast<long double>( m_engineer->m_health ) );
		maxHealth = std::to_string( static_cast<long double>( (int)m_engineer->m_maximumHealth ) );
		g_glRender->RenderText( Vector2( 0.75f, 4.25f), RGBColor::White() , nullptr, nullptr, 0.19f, health + "/" + maxHealth );


		ratio = ( m_engineer->m_energy / m_engineer->m_maximumEnergy );
		ratio = MathUtilities::Clamp( 0.f, 1.f, ratio );

		g_glRender->Draw2DFilledRectangle( Vector2( 0.2f, 4.0f), 1.2f * ratio, .2f, RGBColor(0.f,0.f,1.f,0.5f));
		g_glRender->Draw2DHollowRectangle( Vector2( 0.2f, 4.0f), 1.2f, .2f, RGBColor(1.f,0.f,0.f,1.f));
		g_glRender->RenderText( Vector2( 0.2f, 4.0f), RGBColor(1.f,1.f,1.f,1.f) , nullptr, nullptr, 0.2f, "Energy" );

		energy = std::to_string( static_cast<long double>( m_engineer->m_energy ) );
		maxEnergy = std::to_string( static_cast<long double>( m_engineer->m_maximumEnergy ) );
		g_glRender->RenderText( Vector2( 0.75f, 4.0f), RGBColor::White() , nullptr, nullptr, 0.19f, energy + "/" + maxEnergy );
	}

	if( m_doctor != nullptr )
	{
		ratio = ( m_doctor->m_health / m_doctor->m_maximumHealth );
		ratio = MathUtilities::Clamp( 0.f, 1.f, ratio );

		g_glRender->Draw2DFilledRectangle( Vector2( 0.2f, 2.25f), 1.2f * ratio, .2f, RGBColor(0.f,1.f,0.f,0.5f));
		g_glRender->Draw2DHollowRectangle( Vector2( 0.2f, 2.25f), 1.2f, .2f, RGBColor(1.f,0.f,0.f,1.f));
		g_glRender->RenderText( Vector2( 0.2f, 2.25f), RGBColor(1.f,1.f,1.f,1.f) , nullptr, nullptr, 0.2f, "Health" );

		health = std::to_string( static_cast<long double>( m_doctor->m_health ) );
		maxHealth = std::to_string( static_cast<long double>( m_doctor->m_maximumHealth ) );
		g_glRender->RenderText( Vector2( 0.75f, 2.25f), RGBColor::White() , nullptr, nullptr, 0.19f, health + "/" + maxHealth );

		ratio = ( m_doctor->m_energy / m_doctor->m_maximumEnergy );
		ratio = MathUtilities::Clamp( 0.f, 1.f, ratio );

		g_glRender->Draw2DFilledRectangle( Vector2( 0.2f, 2.0f), 1.2f * ratio, .2f, RGBColor(0.f,0.f,1.f,0.5f));
		g_glRender->Draw2DHollowRectangle( Vector2( 0.2f, 2.0f), 1.2f, .2f, RGBColor(1.f,0.f,0.f,1.f));
		g_glRender->RenderText( Vector2( 0.2f, 2.0f), RGBColor(1.f,1.f,1.f,1.f) , nullptr, nullptr, 0.2f, "Energy" );

		energy = std::to_string( static_cast<long double>( m_doctor->m_energy ) );
		maxEnergy = std::to_string( static_cast<long double>( m_doctor->m_maximumEnergy ) );
		g_glRender->RenderText( Vector2( 0.75f, 2.f), RGBColor::White() , nullptr, nullptr, 0.19f, energy + "/" + maxEnergy );
	}
}

void HUD::RenderPlayerPotrait()
{
	g_glRender->Draw2DHollowRectangle( Vector2( 0.f, 1.65f ), 1.55f, 6.4f, RGBColor::Red() );
	g_glRender->Draw2DFilledRectangle( Vector2( 0.f, 1.7f ), 1.5f, 6.3f, RGBColor( 1.f,0.5f, 0.f, 0.3f) );

	if( m_veteran != nullptr )
	{
		RenderPotraitWithTexCoords( Vector2( .7f ,  7.f ), 0.5f, Vector2i( 9,8 ), m_veteran->m_isSelected, m_veteran->m_isCursorOnActor );
		g_glRender->RenderText( Vector2( .25f, 7.5f ), RGBColor::White() , nullptr, nullptr, 0.3f, "Veteran" );
	}

	if( m_engineer != nullptr )
	{
		RenderPotraitWithTexCoords( Vector2( .7f ,  5.f ), 0.5f, Vector2i( 7,8 ), m_engineer->m_isSelected, m_engineer->m_isCursorOnActor );
		g_glRender->RenderText( Vector2( .20f, 5.5f ), RGBColor::White() , nullptr, nullptr, 0.3f, "Engineer" );
	}

	if( m_doctor != nullptr )
	{
		RenderPotraitWithTexCoords( Vector2( .7f ,  3.f ), 0.5f, Vector2i( 5,8 ), m_doctor->m_isSelected, m_doctor->m_isCursorOnActor  );
		g_glRender->RenderText( Vector2( .30f, 3.5f ), RGBColor::White() , nullptr, nullptr, 0.3f, "Doctor" );
	}
}

void HUD::RenderPotraitWithTexCoords( const Vector2& center, float radius, const Vector2i texCoords, bool isSelected, bool isMouseOn )
{
	float texFrameWidthX = 1.f / 12.f;
	float texFrameHeightY = 1.f / 12.f;

	Vector2 topLeftTexCoords;
	topLeftTexCoords.x = texCoords.x * texFrameWidthX;
	topLeftTexCoords.y = texCoords.y * texFrameHeightY + 0.01f;

	g_glRender->Enable( GL_TEXTURE_2D );
	g_glRender->BindTexture( GL_TEXTURE_2D, m_texture->m_openglTextureID );

	float alpha = 0.0f;
	if( isMouseOn || isSelected )
		alpha = 1.f;
	else
		alpha = 0.7f;

	g_glRender->Color4f( 1.f,1.f,1.f,alpha );
	g_glRender->BeginDraw( GL_POLYGON );

	g_glRender->TexCoord2f( topLeftTexCoords.x, topLeftTexCoords.y + texFrameHeightY );
	g_glRender->Vertex3f( center.x - radius, center.y - radius, 0.f );

	g_glRender->TexCoord2f( topLeftTexCoords.x + texFrameWidthX, topLeftTexCoords.y + texFrameHeightY );
	g_glRender->Vertex3f( center.x + radius, center.y - radius, 0.f );

	g_glRender->TexCoord2f( topLeftTexCoords.x + texFrameWidthX , topLeftTexCoords.y );
	g_glRender->Vertex3f( center.x + radius, center.y + radius, 0.f );

	g_glRender->TexCoord2f( topLeftTexCoords.x, topLeftTexCoords.y );
	g_glRender->Vertex3f( center.x - radius, center.y + radius, 0.f );

	g_glRender->EndDraw();
	g_glRender->Disable( GL_TEXTURE_2D );
	
	if( isSelected )
		g_glRender->Draw2DHollowRectangle( center - Vector2( radius, radius ), radius * 2.f, radius * 2.f, RGBColor::White() );

}

void HUD::RenderInventory( Actor* actor )
{
	if( actor == nullptr || m_playerController->m_selectedPlayerList.size() > 1 || m_playerController->m_selectedPlayerList.size() == 0 )
	{
		IsInventoryAndAbilityShowing = false;
		return;
	}

	IsInventoryAndAbilityShowing = true;
	g_glRender->Draw2DHollowRectangle( Vector2( 14.45f, 1.65f ), 2.5, 6.1f, RGBColor::Red() );
	g_glRender->Draw2DFilledRectangle( Vector2( 14.5f, 1.7f ), 1.5, 6.0f, RGBColor( 1.f,0.5f, 0.f, 0.3f) );

	g_glRender->Draw2DHollowCircle( Vector2( 15.3f, 7.f), 0.5f, RGBColor::White() );
	g_glRender->RenderText( Vector2( 14.8f, 6.25f), RGBColor::White() , nullptr, nullptr, 0.25f, "Main Hand" );

	g_glRender->Draw2DHollowCircle( Vector2( 15.3f, 5.5f), 0.5f, RGBColor::White() );
	g_glRender->RenderText( Vector2( 14.8f, 4.75f), RGBColor::White() , nullptr, nullptr, 0.25f, "Off Hand" );

	g_glRender->Draw2DHollowCircle( Vector2( 15.3f, 4.0f), 0.5f, RGBColor::White() );
	g_glRender->RenderText( Vector2( 15.0f, 3.25f), RGBColor::White() , nullptr, nullptr, 0.25f, "Armor" );

	g_glRender->Draw2DHollowCircle( Vector2( 15.3f, 2.5f), 0.5f, RGBColor::White() );
	g_glRender->RenderText( Vector2( 15.05f, 1.75f), RGBColor::White() , nullptr, nullptr, 0.25f, "Head" );
	
	g_glRender->Draw2DHollowRectangle( Vector2( 5.37f, 0.f ), 5.76f, 1.03f, RGBColor::Red() );
	g_glRender->Draw2DFilledRectangle( Vector2( 5.4f, 0.f ), 5.7f, 1.f, RGBColor( 1.f,0.5f,0.f, 0.3f) );

	g_glRender->Draw2DHollowCircle( Vector2( 6.25f, 0.5f), 0.4f, RGBColor::White() );
	int numPortion = actor->m_inventory->GetNumberOfEnergyPortion();
	std::string numStr = std::to_string( static_cast<long double>( numPortion) );
	g_glRender->RenderText( Vector2( 5.5f, 0.3f), RGBColor::White() , nullptr, nullptr, 0.3f, numStr + "x" );

	numPortion = actor->m_inventory->GetNumberOfHealthPortion();
	numStr = std::to_string( static_cast<long double>( numPortion) );
	g_glRender->RenderText( Vector2( 10.75f, 0.3f), RGBColor::White() , nullptr, nullptr, 0.3f, "x" + numStr );
	g_glRender->Draw2DHollowCircle( Vector2( 10.25f, 0.5f), 0.4f, RGBColor::White() );

	float percent = actor->m_abilityList[0]->m_remainingTime / actor->m_abilityList[0]->m_coolDown;

	g_glRender->Draw2DPartOfHollowCircle( Vector2( 7.75f, 0.5f ), 0.4f, RGBColor::White(), percent );
	g_glRender->RenderText( Vector2( 8.2f, 0.1f ), RGBColor::White() , nullptr, nullptr, 0.3f, "F" );

	percent = actor->m_abilityList[1]->m_remainingTime  / actor->m_abilityList[1]->m_coolDown;

	g_glRender->Draw2DPartOfHollowCircle( Vector2( 8.75f, 0.5f ), 0.4f, RGBColor::White(), percent );
	g_glRender->RenderText( Vector2( 9.2f, 0.1f ), RGBColor::White() , nullptr, nullptr, 0.3f, "G" );

	if( actor->m_currentEquipedItemSlot[MAIN_HAND] != nullptr )
		RenderItem( actor->m_currentEquipedItemSlot[MAIN_HAND], Vector2( 15.3f, 7.f) );

	if( actor->m_currentEquipedItemSlot[OFF_HAND] != nullptr )
		RenderItem( actor->m_currentEquipedItemSlot[OFF_HAND], Vector2( 15.3f, 5.5f) );

	if( actor->m_currentEquipedItemSlot[BODY] != nullptr  )
		RenderItem( actor->m_currentEquipedItemSlot[BODY], Vector2( 15.3f, 4.0f) );

	if( actor->m_currentEquipedItemSlot[HEAD] != nullptr  )
		RenderItem( actor->m_currentEquipedItemSlot[HEAD], Vector2( 15.3f, 2.5f) );

	if( actor->m_currentEquipedItemSlot[HEALTH] != nullptr  )
		RenderItem( actor->m_currentEquipedItemSlot[HEALTH], Vector2( 10.25f, 0.5f ) );

	if( actor->m_currentEquipedItemSlot[ENERGY] != nullptr  )
		RenderItem( actor->m_currentEquipedItemSlot[ENERGY], Vector2( 6.25f, 0.5f ) );
}

void HUD::RenderItem( Item* item, const WorldCoords2D& position )
{
	if( item != nullptr )
	{
		item->m_worldPosition = position;
		item->m_isVBODirty = true;
		item->Render();
	}
}

void HUD::RenderAbility()
{

}

bool HUD::IsMouseIsInHudArea()
{
	if( m_mouseScreenPos.x >= 14.45f && m_mouseScreenPos.x <= 16.f )
	{
		if( m_mouseScreenPos.y >= 1.65f && m_mouseScreenPos.y <= 7.75 )
		{
			IsInItemHud = true;
			IsInAbilityHud = false;
			IsInCharacterHud = false;
			return true;
		}
	}
	else if( m_mouseScreenPos.x >= 0.f && m_mouseScreenPos.x <= 1.55f )
	{
		if( m_mouseScreenPos.y >= 1.65f && m_mouseScreenPos.y <= 8.05f )
		{
			IsInCharacterHud = true;
			IsInAbilityHud = false;
			IsInItemHud = false;
			return true;
		}
	}
	else if( m_mouseScreenPos.x >= 5.37f && m_mouseScreenPos.x <= 11.13f )
	{
		if( m_mouseScreenPos.y >= 0.f && m_mouseScreenPos.y <= 1.03f )
		{
			IsInAbilityHud = true;
			IsInItemHud = false;
			IsInCharacterHud = false;
			return true;
		}
	}

	IsInCharacterHud = false;
	IsInItemHud = false;
	IsInAbilityHud = false;
	return false;
}

void HUD::ProcessMouseClick()
{
	ProcessIfPlayerClickOnPlayerPotrait();
	ProcessIfPlayerClickOnItem();
	ProcessIfPlayerClickOnPortionOrAbility();
}

void HUD::ProcessIfPlayerClickOnPlayerPotrait()
{
	//check if mouse is on veteran
	if( m_mouseScreenPos.x >= 0.2f &&  m_mouseScreenPos.x <= 1.2f )
	{
		if( m_mouseScreenPos.y >= 6.5f && m_mouseScreenPos.y <= 7.5f )
		{
			m_veteran->m_isCursorOnActor = true;
		}
		else if( m_mouseScreenPos.y >= 4.5f && m_mouseScreenPos.y <= 5.5f )
		{
			m_engineer->m_isCursorOnActor = true;
		}
		else if( m_mouseScreenPos.y >= 2.5f && m_mouseScreenPos.y <= 3.5f )
		{
			m_doctor->m_isCursorOnActor = true;
		}
	}
	else
	{
		if( g_isLeftMouseDown && !IsInventoryAndAbilityShowing && !IsInAbilityHud && !IsInItemHud )
		{
			m_playerController->ClearPlayerList();
		}
	}

	if( g_isLeftMouseDown )
	{
		if( m_veteran->m_isCursorOnActor )
		{
			m_veteran->m_isSelected = true;
			m_veteran->m_isCursorOnActor = false;
			m_selectedActor = m_veteran;		
		}
		if( m_engineer->m_isCursorOnActor )
		{
			m_engineer->m_isSelected = true;
			m_engineer->m_isCursorOnActor = false;
			m_selectedActor = m_engineer;
		}
		if( m_doctor->m_isCursorOnActor )
		{
			m_doctor->m_isSelected = true;
			m_doctor->m_isCursorOnActor = false;
			m_selectedActor = m_doctor;
		}

		if( m_selectedActor != nullptr )
		{
			if( g_isHoldingShift )
			{
				if( !m_playerController->IsPlayerAlreadySelected( m_selectedActor ) )
				{
					m_playerController->m_selectedPlayerList.push_back( m_selectedActor );
				}
			}
			else
			{
				if( !m_playerController->IsPlayerAlreadySelected( m_selectedActor ) )
				{
					m_playerController->ClearPlayerList();
					m_playerController->m_selectedPlayerList.push_back( m_selectedActor );
				}
			}
		}
	}
}

void HUD::ProcessIfPlayerClickOnItem()
{
	if( !IsInventoryAndAbilityShowing || m_playerController->m_selectedPlayerList.size() == 0 )
		return;

	EquipSlot slot = UNEQUIPABLE;

	Actor* currentSelectedActor = nullptr;

	if( m_mouseScreenPos.x >= 14.8f && m_mouseScreenPos.x <= 15.8f )
	{
		currentSelectedActor = m_playerController->m_selectedPlayerList[0];
		if( m_mouseScreenPos.y >= 6.5f && m_mouseScreenPos.y <= 7.5f )
		{
			slot = MAIN_HAND;
		}
		else if( m_mouseScreenPos.y >= 5.f && m_mouseScreenPos.y <= 6.f )
		{
			slot = OFF_HAND;
		}
		else if( m_mouseScreenPos.y >= 3.5f && m_mouseScreenPos.y <= 4.5f )
		{
			slot = BODY;
		}
		else if( m_mouseScreenPos.y >= 2.f && m_mouseScreenPos.y <= 3.f )
		{
			slot = HEAD;
		}
	}

	if( currentSelectedActor == nullptr || slot == UNEQUIPABLE )
		return;

	if( g_isLeftMouseDown )
	{
		Item* currentItem = currentSelectedActor->m_currentEquipedItemSlot[slot];
		int nextItem = currentSelectedActor->m_inventory->GetIndexNextItemInEquipSlot( currentItem );
		if( nextItem != -1)
			currentSelectedActor->m_currentEquipedItemSlot[slot] = currentSelectedActor->m_inventory->m_itemList[nextItem];
	}
	if( g_isRightMouseDown )
	{
		Item* currentItem = currentSelectedActor->m_currentEquipedItemSlot[slot];
		int prevItem = currentSelectedActor->m_inventory->GetIndexNextItemInEquipSlot( currentItem );
		if( prevItem != -1 )
			currentSelectedActor->m_currentEquipedItemSlot[slot] = currentSelectedActor->m_inventory->m_itemList[prevItem];
	}
}

void HUD::ProcessIfPlayerClickOnPortionOrAbility()
{
	if( !IsInventoryAndAbilityShowing )
		return;

	if( g_isLeftMouseDown )
	{
		Actor* currentSelectedActor = m_playerController->m_selectedPlayerList[0];
		if( m_mouseScreenPos.y >= 0.1f && m_mouseScreenPos.y <= 0.9f )
		{
			if( m_mouseScreenPos.x >= 5.85f && m_mouseScreenPos.x <= 6.65f )
			{
				if( currentSelectedActor->m_energy < currentSelectedActor->m_maximumEnergy )
				{
					if( currentSelectedActor->m_currentEquipedItemSlot[ENERGY] != nullptr )
					{
						Item* currentItem = currentSelectedActor->m_currentEquipedItemSlot[ENERGY];
						currentSelectedActor->m_energy += currentItem->m_itemBlueprint->m_recoverAmount;
						currentSelectedActor->m_inventory->DeleteItem( currentItem );
						int nextItem = currentSelectedActor->m_inventory->GetIndexNextItemInEquipSlot( currentItem );
						if( nextItem != -1)
							currentSelectedActor->m_currentEquipedItemSlot[ENERGY] = currentSelectedActor->m_inventory->m_itemList[nextItem];
						else
							currentSelectedActor->m_currentEquipedItemSlot[ENERGY] = nullptr;
					}
				}
			}
			else if ( m_mouseScreenPos.x >= 7.35f && m_mouseScreenPos.x <= 8.15f )
			{

			}
			else if ( m_mouseScreenPos.x >= 8.35f && m_mouseScreenPos.x <= 9.15f )
			{

			}
			else if( m_mouseScreenPos.x >= 9.85f && m_mouseScreenPos.x <= 10.65f )
			{
				if( currentSelectedActor->m_health < currentSelectedActor->m_maximumHealth )
				{
					if( currentSelectedActor->m_currentEquipedItemSlot[HEALTH] != nullptr )
					{
						Item* currentItem = currentSelectedActor->m_currentEquipedItemSlot[HEALTH];
						currentSelectedActor->m_health += currentItem->m_itemBlueprint->m_recoverAmount;
						currentSelectedActor->m_inventory->DeleteItem( currentItem );
						int nextItem = currentSelectedActor->m_inventory->GetIndexNextItemInEquipSlot( currentItem );
						if( nextItem != -1)
							currentSelectedActor->m_currentEquipedItemSlot[HEALTH] = currentSelectedActor->m_inventory->m_itemList[nextItem];
						else
							currentSelectedActor->m_currentEquipedItemSlot[HEALTH] = nullptr;
					}
				}
			}
		}
	}
}
