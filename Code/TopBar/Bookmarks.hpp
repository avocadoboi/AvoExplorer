#pragma once

#include "TopBar.hpp"
#include "../ActionMenu/ContextMenu.hpp"

//------------------------------

class Bookmarks :
	public ContextView
{
private:
	TopBar* m_topBar;

public:
	Bookmarks(TopBar* p_topBar) :
		ContextView(p_topBar), m_topBar(p_topBar)
	{
		setElevation(3.f);
		setCornerRadius(6.f);

		setThemeColor("background", Colors::topBarBookmarksBackground);

		enableMouseEvents();

		//------------------------------

		m_contextMenuWidth = 150.f;
		m_contextMenuItems.push_back(ActionMenuItemData("Test 1", "Ctrl+N"));
		m_contextMenuItems.push_back(ActionMenuItemData("Test 2", "Ctrl+Shift+N"));
	}

	void draw(AvoGUI::DrawingContext* p_context) override
	{
		p_context->setColor(getThemeColor("background"));
		p_context->fillRectangle(getSize());

		p_context->setColor(Colors::topBarBookmarksBorder);
		p_context->strokeRoundedRectangle(getSize(), getCorners().topLeftSizeX, 3.f);
	}
};
