#include "FileBrowserItems.hpp"

#include "FileBrowserItem.hpp"

//------------------------------

float const FILE_BROWSER_ITEMS_PADDING = 1 * 8.f;
float const FILE_BROWSER_ITEMS_MARGIN_HORIZONTAL = 1 * 8.f;
float const FILE_BROWSER_ITEMS_MARGIN_VERTICAL = 2 * 8.f;
float const FILE_BROWSER_ITEMS_LABEL_MARGIN_TOP = 2 * 8.f;
float const FILE_BROWSER_ITEMS_LABEL_MARGIN_BOTTOM = 1 * 8.f;

//------------------------------

void FileBrowserItems::setWorkingDirectory(std::filesystem::path const& p_path)
{
	m_directoryItems.clear();
	m_fileItems.clear();
	removeAllChildren();

	std::vector<std::filesystem::path> directoryPaths;
	directoryPaths.reserve(128);
	std::vector<std::filesystem::path> filePaths;
	filePaths.reserve(256);

	for (auto item : std::filesystem::directory_iterator(p_path))
	{
		if (item.is_regular_file())
		{
			filePaths.push_back(item.path());
		}
		else
		{
			directoryPaths.push_back(item.path());
		}
	}

	// Sort files and directories separately, because they will be displayed separately.
	std::sort(filePaths.begin(), filePaths.end());
	std::sort(directoryPaths.begin(), directoryPaths.end());

	m_directoryItems.reserve(directoryPaths.size());
	m_fileItems.reserve(filePaths.size());
	for (auto path : directoryPaths)
	{
		m_directoryItems.push_back(new FileBrowserItem(this, path, false));
	}
	for (auto path : filePaths)
	{
		m_fileItems.push_back(new FileBrowserItem(this, path, true));
	}
}

void FileBrowserItems::updateLayout()
{
	FileBrowserItem* lastItem = 0;

	float width = 0.f;
	float height = 0.f;

	m_text_directories->setTopLeft(FILE_BROWSER_ITEMS_PADDING);
	for (FileBrowserItem* item : m_directoryItems)
	{
		if (lastItem)
		{
			if (lastItem->getRight() + FILE_BROWSER_ITEMS_MARGIN_HORIZONTAL + item->getWidth() > getParent()->getWidth() - FILE_BROWSER_ITEMS_PADDING)
			{
				item->setTopLeft(FILE_BROWSER_ITEMS_PADDING, lastItem->getBottom() + FILE_BROWSER_ITEMS_MARGIN_VERTICAL);
			}
			else
			{
				item->setTopLeft(lastItem->getRight() + FILE_BROWSER_ITEMS_MARGIN_HORIZONTAL, lastItem->getTop());
				if (item->getBottom() > height)
				{
					height = item->getBottom();
				}
			}
			if (item->getRight() > width)
			{
				width = item->getRight();
			}
		}
		else
		{
			item->setTopLeft(FILE_BROWSER_ITEMS_PADDING, m_text_directories->getBottom() + FILE_BROWSER_ITEMS_LABEL_MARGIN_BOTTOM);
			height = item->getBottom();
			width = item->getWidth();
		}
		lastItem = item;
	}
	m_text_files->setTopLeft(FILE_BROWSER_ITEMS_PADDING, m_directoryItems.size() ? m_directoryItems.back()->getBottom() + FILE_BROWSER_ITEMS_LABEL_MARGIN_TOP : FILE_BROWSER_ITEMS_PADDING);
	for (FileBrowserItem* item : m_fileItems)
	{
		if (lastItem)
		{
			if (lastItem->getRight() + FILE_BROWSER_ITEMS_MARGIN_HORIZONTAL + item->getWidth() > getParent()->getWidth() - FILE_BROWSER_ITEMS_PADDING)
			{
				item->setTopLeft(FILE_BROWSER_ITEMS_PADDING, lastItem->getBottom() + FILE_BROWSER_ITEMS_MARGIN_VERTICAL);
				if (item->getBottom() > height)
				{
					height = item->getBottom();
				}

			}
			else if (item == m_fileItems[0])
			{
				item->setTopLeft(FILE_BROWSER_ITEMS_PADDING, m_text_files->getBottom() + FILE_BROWSER_ITEMS_LABEL_MARGIN_BOTTOM);
				if (item->getBottom() > height)
				{
					height = item->getBottom();
				}
			}
			else
			{
				item->setTopLeft(lastItem->getRight() + FILE_BROWSER_ITEMS_MARGIN_HORIZONTAL, lastItem->getTop());
			}
		}
		else
		{
			item->setTopLeft(FILE_BROWSER_ITEMS_PADDING, m_text_files->getBottom() + FILE_BROWSER_ITEMS_LABEL_MARGIN_BOTTOM);
			height = item->getBottom();
		}
		if (item->getRight() > width)
		{
			width = item->getRight();
		}
		lastItem = item;
	}
	setSize(width, height);
}