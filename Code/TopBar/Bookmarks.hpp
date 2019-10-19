#pragma once

#include "TopBar.hpp"

//------------------------------

class Bookmarks :
	public AvoGUI::View
{
private:
	TopBar* m_topBar;

public:
	Bookmarks(TopBar* p_topBar) :
		View(p_topBar), m_topBar(0)
	{
		setElevation(3.f);
		setCornerRadius(6.f);

		setThemeColor("background", Colors::topBarBookmarksBackground);
	}

	void draw(AvoGUI::DrawingContext* p_context) override
	{
		p_context->setColor(getThemeColor("background"));
		p_context->fillRectangle(getSize());

		p_context->setColor(Colors::topBarBookmarksBorder);
		p_context->strokeRoundedRectangle(getSize(), getCorners().topLeftSizeX, 3.f);
	}
};
