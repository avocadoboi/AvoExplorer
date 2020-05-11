#include "Bookmarks.hpp"

#include "FileBrowserItem.hpp"

#include <fstream>

//------------------------------

//
// Private
//

void Bookmarks::updateLayout()
{
	if (m_bookmarks.size() && getHeight())
	{
		float padding = 0.5f * (getHeight() - m_bookmarks[0]->getHeight());
		for (uint32 a = 0; a < m_bookmarks.size(); a++)
		{
			m_bookmarks[a]->setTargetPosition((a ? m_bookmarks[a - 1]->getTargetPosition().x + m_bookmarks[a - 1]->getWidth() : 0) + padding, padding);
		}
		m_bookmarksContainer->setWidth(m_bookmarks.back()->getTargetPosition().x + m_bookmarks.back()->getWidth() + padding);
		m_bookmarksContainer->setHeight(padding + m_bookmarks[0]->getHeight());
	}
}

//
// Public
//

void Bookmarks::handleSizeChange(float p_previousWidth, float p_previousHeight)
{
	m_bookmarksScrollContainer->setSize(getSize());
	if (p_previousHeight != getHeight() || (p_previousWidth > m_bookmarksContainer->getWidth()) != (getWidth() < m_bookmarksContainer->getWidth()))
	{
		updateLayout();
	}
}

void Bookmarks::handleBookmarkDrag(FileBrowserItem* p_bookmark)
{
	if (m_bookmarks.size() > 1)
	{
		float x = getGui()->getWindow()->getMousePosition().x;

		AvoGUI::removeVectorElementWhileKeepingOrder(m_bookmarks, p_bookmark);
		for (uint32 a = 0; a < m_bookmarks.size(); a++)
		{
			if (x < m_bookmarks[a]->getAbsoluteCenterX())
			{
				m_bookmarks.insert(m_bookmarks.begin() + a, p_bookmark);
				break;
			}
			else if (a == m_bookmarks.size() - 1)
			{
				m_bookmarks.push_back(p_bookmark);
				break;
			}
		}
		updateLayout();
	}
}

//------------------------------

void Bookmarks::loadBookmarks()
{
	if (std::filesystem::exists(BOOKMARKS_DATA_PATH))
	{
		std::wifstream fileStream(BOOKMARKS_DATA_PATH, std::ios::binary);

		while (!fileStream.eof())
		{
			std::wstring pathString;
			std::getline(fileStream, pathString, (wchar_t)0);
			if (pathString != L"" && std::filesystem::exists(pathString))
			{
				m_bookmarks.push_back(new FileBrowserItem(m_bookmarksContainer, pathString, true));
			}
		}
		updateLayout();

		fileStream.close();
	}
}
void Bookmarks::saveBookmarks()
{
	std::wofstream fileStream(BOOKMARKS_DATA_PATH, std::ios::binary);

	for (FileBrowserItem* bookmark : m_bookmarks)
	{
		fileStream.write(bookmark->getPath().c_str(), bookmark->getPath().wstring().size() + 1);
	}

	fileStream.close();
}

void Bookmarks::addBookmark(std::filesystem::path const& p_path)
{
	m_bookmarks.push_back(new FileBrowserItem(m_bookmarksContainer, p_path, true));
	updateLayout();
	m_bookmarks.back()->invalidate();
	saveBookmarks();
}
void Bookmarks::removeBookmark(uint32 p_index)
{
	m_bookmarks[p_index]->invalidate();
	m_bookmarksContainer->removeChildView(m_bookmarks[p_index]);
	m_bookmarks.erase(m_bookmarks.begin() + p_index);
	updateLayout();
	saveBookmarks();
}
void Bookmarks::removeBookmark(std::filesystem::path const& p_path)
{
	for (uint32 a = 0; a < m_bookmarks.size(); a++)
	{
		if (m_bookmarks[a]->getPath() == p_path)
		{
			removeBookmark(a);
			break;
		}
	}
}
void Bookmarks::removeBookmark(FileBrowserItem* p_bookmark)
{
	for (uint32 a = 0; a < m_bookmarks.size(); a++)
	{
		if (m_bookmarks[a] == p_bookmark)
		{
			removeBookmark(a);
			break;
		}
	}
}
bool Bookmarks::getIsPathBookmarked(std::filesystem::path const& p_path)
{
	for (uint32 a = 0; a < m_bookmarks.size(); a++)
	{
		if (m_bookmarks[a]->getPath() == p_path)
		{
			return true;
		}
	}
	return false;
}
