#pragma once

#include "../TopBar.hpp"
#include "../../ScrollContainer/ScrollContainer.hpp"

//------------------------------

class FileBrowserItem;

class Bookmarks :
	public AvoGUI::View
{
private:
	TopBar* m_topBar;

	std::vector<FileBrowserItem*> m_bookmarks;
	ScrollContainer* m_bookmarksScrollContainer;
	FileBrowserItem* m_draggedBookmark;

	AvoGUI::Geometry* m_borderGeometry;

public:
	Bookmarks(TopBar* p_topBar) :
		View(p_topBar, Ids::bookmarks), m_topBar(p_topBar),
		m_draggedBookmark(0), m_borderGeometry(0)
	{
		enableMouseEvents();

		setCornerRadius(6.f);
		m_borderGeometry = getGui()->getDrawingContext()->createRoundedRectangleGeometry(getSize(), getCorners().topLeftSizeX);

		setThemeColor("background", Colors::topBarBookmarksBackground);

		m_bookmarksScrollContainer = new ScrollContainer(this);
	}
	~Bookmarks()
	{
		m_borderGeometry->forget();
	}


	void handleSizeChange()
	{
		m_bookmarksScrollContainer->setSize(getSize());
	}

	//------------------------------

	void addBookmark(std::filesystem::path const& p_path);

	//------------------------------

	void draw(AvoGUI::DrawingContext* p_context) override
	{
		p_context->setColor(getThemeColor("background"));
		p_context->fillRectangle(getSize());

		p_context->setColor(Colors::topBarBookmarksBorder);
		p_context->strokeGeometry(m_borderGeometry, 3.f);
	}
};
