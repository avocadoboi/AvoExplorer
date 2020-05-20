#pragma once

#include "TopBar.hpp"
#include "ScrollContainer.hpp"
#include "../Resources/Ids.hpp"
#include "../utilities.hpp"

//------------------------------

class FileBrowserItem;

class Bookmarks :
	public Avo::View
{
public:
	static constexpr char const* BOOKMARKS_DATA_PATH = "data";

private:
	std::vector<FileBrowserItem*> m_bookmarks;
public:
	std::vector<FileBrowserItem*> const& getBookmarks()
	{
		return m_bookmarks;
	}

private:
	ScrollContainer* m_bookmarksScrollContainer = nullptr;
	Avo::View* m_bookmarksContainer = nullptr;
public:
	Avo::View* getBookmarksContainer()
	{
		return m_bookmarksContainer;
	}

private:
	void updateLayout();
public:
	void handleSizeChange(float p_previousWidth, float p_previousHeight) override
	{
		m_bookmarksScrollContainer->setSize(getSize());
		if (p_previousHeight != getHeight() ||
			(p_previousWidth > m_bookmarksContainer->getWidth()) != (getWidth() < m_bookmarksContainer->getWidth()))
		{
			updateLayout();
		}
	}

	//------------------------------

	void handleBookmarkDrag(FileBrowserItem* p_bookmark);

	//------------------------------

	void loadBookmarks();
	void saveBookmarks();

	void addBookmark(std::filesystem::path const& p_path);

private:
	void removeBookmark(std::vector<FileBrowserItem*>::iterator const& p_iterator)
	{
		if (p_iterator != m_bookmarks.end())
		{
			removeBookmark(p_iterator - m_bookmarks.begin());
		}
	}
public:
	void removeBookmark(uint32 p_index);
	void removeBookmark(std::filesystem::path const& p_path);
	void removeBookmark(FileBrowserItem* p_bookmark)
	{
		removeBookmark(std::find(m_bookmarks.begin(), m_bookmarks.end(), p_bookmark));
	}
	bool getIsPathBookmarked(std::filesystem::path const& p_path);

	//------------------------------

	void draw(Avo::DrawingContext* p_context) override
	{
		p_context->setColor(getThemeColor(ThemeColors::background));
		p_context->fillRectangle(getSize());

		p_context->setColor(Colors::topBarBookmarksBorder);
		p_context->strokeGeometry(getClipGeometry(), 3.f);
	}

	Bookmarks(View* p_topBar) :
		View{ p_topBar, Ids::bookmarks }
	{
		enableMouseEvents();

		setCornerRadius(6.f);

		setThemeColor(ThemeColors::background, Colors::topBarBookmarksBackground);

		m_bookmarksScrollContainer = new ScrollContainer{ this };
		m_bookmarksScrollContainer->setScrollbarMargin(2.f);
		m_bookmarksContainer = m_bookmarksScrollContainer->getContent();

		loadBookmarks();
	}
};
