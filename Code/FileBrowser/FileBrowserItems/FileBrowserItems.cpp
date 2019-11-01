#include "FileBrowserItems.hpp"

#include "FileBrowserItem.hpp"

//------------------------------

float constexpr FILE_BROWSER_ITEMS_PADDING_TOP = 2			* 8.f;
float constexpr FILE_BROWSER_ITEMS_PADDING = 3				* 8.f;
float constexpr FILE_BROWSER_ITEMS_MARGIN_HORIZONTAL = 1	* 8.f;
float constexpr FILE_BROWSER_ITEMS_MARGIN_VERTICAL = 1		* 8.f;
float constexpr FILE_BROWSER_ITEMS_LABEL_MARGIN_TOP = 3		* 8.f;
float constexpr FILE_BROWSER_ITEMS_LABEL_MARGIN_BOTTOM = 2	* 8.f;

//------------------------------

FileBrowserItems::~FileBrowserItems()
{
	if (m_text_directories)
	{
		m_text_directories->forget();
	}
	if (m_text_files)
	{
		m_text_files->forget();
	}
	if (m_selectedItem)
	{
		m_selectedItem->forget();
	}
	if (m_thumbnailCache)
	{
		m_thumbnailCache->Release();
	}
	if (m_iconList_large)
	{
		m_iconList_large->Release();
	}
	if (m_iconList_jumbo)
	{
		m_iconList_jumbo->Release();
	}
}

//------------------------------

void FileBrowserItems::setSelectedItem(FileBrowserItem* p_item)
{
	if (m_selectedItem)
	{
		m_selectedItem->forget();
	}
	m_selectedItem = p_item;
	if (m_selectedItem)
	{
		m_selectedItem->remember();
	}
}

//------------------------------

void FileBrowserItems::loadIcons()
{
	m_isIconLoadingThreadRunning = true;

	while (m_needsToLoadMoreIcons)
	{
		m_needsToLoadMoreIcons = false;

		if (m_directoryItems.size())
		{
			int32 numberOfColumns = floor((getWidth() - FILE_BROWSER_ITEMS_PADDING + FILE_BROWSER_ITEMS_MARGIN_HORIZONTAL) / (m_directoryItems[0]->getWidth() + FILE_BROWSER_ITEMS_MARGIN_HORIZONTAL));
			int32 firstVisibleDirectoryItemIndex = numberOfColumns*floor((-getTop() - m_text_directories->getBottom() - FILE_BROWSER_ITEMS_LABEL_MARGIN_BOTTOM) / (m_directoryItems[0]->getHeight() + FILE_BROWSER_ITEMS_MARGIN_VERTICAL));
			int32 lastVisibleDirectoryItemIndex = numberOfColumns*floor(1 + (-getTop() + getParent()->getHeight() - m_text_directories->getBottom() - FILE_BROWSER_ITEMS_LABEL_MARGIN_BOTTOM) / (m_directoryItems[0]->getHeight() + FILE_BROWSER_ITEMS_MARGIN_VERTICAL));
			for (int32 a = AvoGUI::max(0, firstVisibleDirectoryItemIndex); a < lastVisibleDirectoryItemIndex && a < m_directoryItems.size(); a++)
			{
				if (!m_directoryItems[a]->getHasLoadedIcon())
				{
					m_directoryItems[a]->loadIcon(m_iconList_large, m_thumbnailCache);
				}
			}
		}

		if (m_fileItems.size())
		{
			int32 numberOfColumns = floor((getWidth() - FILE_BROWSER_ITEMS_PADDING + FILE_BROWSER_ITEMS_MARGIN_HORIZONTAL) / (m_fileItems[0]->getWidth() + FILE_BROWSER_ITEMS_MARGIN_HORIZONTAL));
			int32 firstVisibleFileItemIndex = numberOfColumns * floor((-getTop() - m_text_files->getBottom() - FILE_BROWSER_ITEMS_LABEL_MARGIN_BOTTOM) / (m_fileItems[0]->getHeight() + FILE_BROWSER_ITEMS_MARGIN_VERTICAL));
			int32 lastVisibleFileItemIndex = numberOfColumns * floor(1 + (-getTop() + getParent()->getHeight() - m_text_files->getBottom() - FILE_BROWSER_ITEMS_LABEL_MARGIN_BOTTOM) / (m_fileItems[0]->getHeight() + FILE_BROWSER_ITEMS_MARGIN_VERTICAL));
			for (int32 a = AvoGUI::max(0, firstVisibleFileItemIndex); a < lastVisibleFileItemIndex && a < m_fileItems.size(); a++)
			{
				if (!m_fileItems[a]->getHasLoadedIcon())
				{
					m_fileItems[a]->loadIcon(m_iconList_jumbo, m_thumbnailCache);
				}
			}
		}
	}

	m_isIconLoadingThreadRunning = false;
}

void FileBrowserItems::setWorkingDirectory(std::filesystem::path const& p_path)
{
	setSelectedItem(0);
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
		else if (item.is_directory())
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

	if (getParent()->getWidth() && getParent()->getHeight())
	{
		updateLayout();
	}
}

void FileBrowserItems::updateLayout()
{
	FileBrowserItem* lastItem = 0;

	float width = 0.f;
	float height = 0.f;

	m_text_directories->setTopLeft(FILE_BROWSER_ITEMS_PADDING, FILE_BROWSER_ITEMS_PADDING_TOP);
	for (FileBrowserItem* item : m_directoryItems)
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
			else
			{
				item->setTopLeft(lastItem->getRight() + FILE_BROWSER_ITEMS_MARGIN_HORIZONTAL, lastItem->getTop());
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
			width = item->getRight();
		}
		lastItem = item;
	}

	m_text_files->setTopLeft(FILE_BROWSER_ITEMS_PADDING, m_directoryItems.size() ? m_directoryItems.back()->getBottom() + FILE_BROWSER_ITEMS_LABEL_MARGIN_TOP : FILE_BROWSER_ITEMS_PADDING_TOP);
	for (FileBrowserItem* item : m_fileItems)
	{
		if (lastItem)
		{
			if (item == m_fileItems[0])
			{
				item->setTopLeft(FILE_BROWSER_ITEMS_PADDING, m_text_files->getBottom() + FILE_BROWSER_ITEMS_LABEL_MARGIN_BOTTOM);
				if (item->getBottom() > height)
				{
					height = item->getBottom();
				}
			}
			else if (lastItem->getRight() + FILE_BROWSER_ITEMS_MARGIN_HORIZONTAL + item->getWidth() > getParent()->getWidth() - FILE_BROWSER_ITEMS_PADDING)
			{
				item->setTopLeft(FILE_BROWSER_ITEMS_PADDING, lastItem->getBottom() + FILE_BROWSER_ITEMS_MARGIN_VERTICAL);
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
	setSize(width, height + FILE_BROWSER_ITEMS_PADDING);

	tellIconLoadingThreadToLoadMoreIcons();
}