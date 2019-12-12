#include "Bookmarks.hpp"

#include "../../FileBrowser/FileBrowserItems/FileBrowserItem.hpp"

//
// Private
//

void Bookmarks::updateLayout()
{
	for (uint32 a = 0; a < m_bookmarks.size(); a++)
	{
		float padding = 0.5f * (getHeight() - m_bookmarks[a]->getHeight());
		m_bookmarks[a]->setTopLeft((a ? m_bookmarks[a - 1]->getRight() : 0) + padding, padding);
		m_bookmarks[a]->invalidate();
	}
}

//
// Public
//

void Bookmarks::handleSizeChange(float p_previousWidth, float p_previousHeight)
{
	m_bookmarksScrollContainer->setSize(getSize());
	if (p_previousHeight != getHeight())
	{
		updateLayout();
	}
}

//------------------------------

void Bookmarks::addBookmark(std::filesystem::path const& p_path)
{
	m_bookmarks.push_back(new FileBrowserItem(m_bookmarksScrollContainer, p_path, true));
	updateLayout();
}
void Bookmarks::removeBookmark(uint32 p_index)
{
	m_bookmarks[p_index]->invalidate();
	m_bookmarksScrollContainer->removeChild(m_bookmarks[p_index]);
	m_bookmarks.erase(m_bookmarks.begin() + p_index);
	updateLayout();
}
