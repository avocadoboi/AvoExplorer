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
	AvoGUI::View* m_bookmarksContainer = 0;
	ScrollContainer* m_bookmarksScrollContainer = 0;
	void updateLayout();

public:
	Bookmarks(TopBar* p_topBar) :
		View(p_topBar, Ids::bookmarks), m_topBar(p_topBar)
	{
		enableMouseEvents();

		setCornerRadius(6.f);

		setThemeColor("background", Colors::topBarBookmarksBackground);

		m_bookmarksScrollContainer = new ScrollContainer(this);
		m_bookmarksScrollContainer->setScrollbarMargin(2.f);
		m_bookmarksContainer = m_bookmarksScrollContainer->getContent();

		loadBookmarks();
	}

	void handleSizeChange(float p_previousWidth, float p_previousHeight) override;

	//------------------------------

	void handleBookmarkDrag(FileBrowserItem* p_bookmark);
	AvoGUI::View* getBookmarksContainer()
	{
		return m_bookmarksContainer;
	}
	std::vector<FileBrowserItem*>& getBookmarks()
	{
		return m_bookmarks;
	}

	//------------------------------

	void loadBookmarks();
	void saveBookmarks();

	void addBookmark(std::filesystem::path const& p_path);
	void removeBookmark(uint32 p_index);
	void removeBookmark(std::filesystem::path const& p_path);
	void removeBookmark(FileBrowserItem* p_bookmark);
	bool getIsPathBookmarked(std::filesystem::path const& p_path);

	//------------------------------

	void draw(AvoGUI::DrawingContext* p_context) override
	{
		p_context->setColor(getThemeColor("background"));
		p_context->fillRectangle(getSize());

		p_context->setColor(Colors::topBarBookmarksBorder);
		p_context->strokeGeometry(getClipGeometry(), 3.f);
	}
};
