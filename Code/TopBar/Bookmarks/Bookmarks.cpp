#include "Bookmarks.hpp"

#include "../../FileBrowser/FileBrowserItems/FileBrowserItem.hpp"

//------------------------------

void Bookmarks::addBookmark(std::filesystem::path const& p_path)
{
	FileBrowserItem* bookmark = new FileBrowserItem(m_bookmarksScrollContainer, p_path, true);

	float padding = 0.5f*(getHeight() - bookmark->getHeight());
	bookmark->setTopLeft((m_bookmarks.size() ? m_bookmarks.back()->getLeft() : 0) + padding, padding);

	//getGui()->getViewById<FileBrowserItems>(Ids::fileBrowserItems)

	m_bookmarks.push_back(bookmark);

	bookmark->invalidate();
}
