#pragma once

#include "../TopBar.hpp"

//------------------------------

class Bookmark;

class Bookmarks :
	public AvoGUI::View
{
private:
	TopBar* m_topBar;

	std::vector<Bookmark*> m_bookmarks;

	AvoGUI::Geometry* m_borderGeometry;

public:
	Bookmarks(TopBar* p_topBar) :
		View(p_topBar, Ids::bookmarks), m_topBar(p_topBar)
	{
		enableMouseEvents();

		setCornerRadius(6.f);
		m_borderGeometry = getGui()->getDrawingContext()->createRoundedRectangleGeometry(getSize(), getCorners().topLeftSizeX);

		setThemeColor("background", Colors::topBarBookmarksBackground);
	}
	~Bookmarks()
	{
		m_borderGeometry->forget();
	}

	void addBookmark(std::filesystem::path const& p_path);

	void draw(AvoGUI::DrawingContext* p_context) override
	{
		p_context->setColor(getThemeColor("background"));
		p_context->fillRectangle(getSize());

		p_context->setColor(Colors::topBarBookmarksBorder);
		p_context->strokeGeometry(m_borderGeometry, 3.f);
	}
};
