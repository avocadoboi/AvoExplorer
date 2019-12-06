#include "FileBrowserItems.hpp"

#include "FileBrowserItem.hpp"

//------------------------------

float constexpr FILE_BROWSER_ITEMS_PADDING_TOP = 2			* 8.f;
float constexpr FILE_BROWSER_ITEMS_PADDING = 3				* 8.f;
float constexpr FILE_BROWSER_ITEMS_MARGIN_HORIZONTAL = 1	* 8.f;
float constexpr FILE_BROWSER_ITEMS_MARGIN_VERTICAL = 1		* 8.f;
float constexpr FILE_BROWSER_ITEMS_LABEL_MARGIN_TOP = 3		* 8.f;
float constexpr FILE_BROWSER_ITEMS_LABEL_MARGIN_BOTTOM = 2	* 8.f;

//
// Private
//

void FileBrowserItems::thread_loadIcons()
{
	IThumbnailCache* thumbnailCache = 0;

	while (true)
	{
	start:
		if (m_needsToExitIconLoadingThread)
		{
			break;
		}
		
		if (!m_needsToLoadMoreIcons)
		{
			std::unique_lock<std::mutex> mutexLock(m_needsToLoadMoreIconsMutex);
			m_needsToLoadMoreIconsConditionVariable.wait(mutexLock, [=] { return (bool)m_needsToLoadMoreIcons; });
		}

		if (m_needsToChangeDirectory)
		{
			getGUI()->excludeAnimationThread();
			setWorkingDirectory(m_path);
			getGUI()->includeAnimationThread();
		}

		if (m_needsToExitIconLoadingThread)
		{
			break;
		}

		m_needsToLoadMoreIcons = false;

		if (m_directoryItems.size())
		{
			int32 numberOfColumns = floor((getWidth() - FILE_BROWSER_ITEMS_PADDING + FILE_BROWSER_ITEMS_MARGIN_HORIZONTAL) / (m_directoryItems[0]->getWidth() + FILE_BROWSER_ITEMS_MARGIN_HORIZONTAL));
			int32 firstVisibleDirectoryItemIndex = numberOfColumns * floor((-getTop() - m_text_directories->getBottom() - FILE_BROWSER_ITEMS_LABEL_MARGIN_BOTTOM) / (m_directoryItems[0]->getHeight() + FILE_BROWSER_ITEMS_MARGIN_VERTICAL));
			int32 lastVisibleDirectoryItemIndex = numberOfColumns * floor(1 + (-getTop() + getParent()->getHeight() - m_text_directories->getBottom() - FILE_BROWSER_ITEMS_LABEL_MARGIN_BOTTOM) / (m_directoryItems[0]->getHeight() + FILE_BROWSER_ITEMS_MARGIN_VERTICAL));
			for (int32 a = AvoGUI::max(0, firstVisibleDirectoryItemIndex); a < lastVisibleDirectoryItemIndex && a < m_directoryItems.size(); a++)
			{
				if (m_needsToLoadMoreIcons)
				{
					goto start;
				}
				FileBrowserItem* directoryItem = m_directoryItems[a];
				if (!directoryItem->getHasLoadedIcon())
				{
					SHFILEINFOW fileInfo = { 0 };
					DWORD_PTR result = SHGetFileInfoW(directoryItem->getPath().c_str(), 0, &fileInfo, sizeof(SHFILEINFOW), SHGFI_SYSICONINDEX);

					if (m_uniqueLoadedDirectoryIcons.find(fileInfo.iIcon) == m_uniqueLoadedDirectoryIcons.end())
					{
						HICON icon;
						m_windowsDirectoryIconList->GetIcon(fileInfo.iIcon, 0, &icon);

						AvoGUI::Image* newIcon = getGUI()->getDrawingContext()->createImage(icon);
						directoryItem->setIcon(newIcon);
						m_uniqueLoadedDirectoryIcons[fileInfo.iIcon] = newIcon;

						DestroyIcon(icon);
					}
					else
					{
						directoryItem->setIcon(m_uniqueLoadedDirectoryIcons[fileInfo.iIcon]);
					}
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
				if (m_needsToLoadMoreIcons)
				{
					goto start;
				}
				FileBrowserItem* fileItem = m_fileItems[a];
				if (!fileItem->getHasLoadedIcon())
				{
					if (fileItem->getIsIconThumbnail())
					{
						if (!thumbnailCache)
						{
							// CoInitialize is on current thread.
							CoInitialize(0);
							CoCreateInstance(CLSID_LocalThumbnailCache, 0, CLSCTX_INPROC, IID_IThumbnailCache, (void**)&thumbnailCache);
						}
						IShellItem* item = 0;

						HRESULT result = SHCreateItemFromParsingName(fileItem->getPath().c_str(), 0, IID_PPV_ARGS(&item));

						ISharedBitmap* bitmap = 0;
						WTS_CACHEFLAGS flags;
						thumbnailCache->GetThumbnail(item, 128, WTS_EXTRACT, &bitmap, &flags, 0);

						HBITMAP bitmapHandle;
						bitmap->GetSharedBitmap(&bitmapHandle);

						AvoGUI::Image* newIcon = getGUI()->getDrawingContext()->createImage(bitmapHandle);
						fileItem->setIcon(newIcon);
						newIcon->forget();

						DeleteObject(bitmapHandle);
						bitmap->Release();
						item->Release();
					}
					else
					{
						SHFILEINFOW fileInfo = { 0 };
						DWORD_PTR result = SHGetFileInfoW(fileItem->getPath().c_str(), 0, &fileInfo, sizeof(SHFILEINFOW), SHGFI_SYSICONINDEX);

						if (m_uniqueLoadedFileIcons.find(fileInfo.iIcon) == m_uniqueLoadedFileIcons.end())
						{
							HICON icon;
							m_windowsFileIconList->GetIcon(fileInfo.iIcon, 0, &icon);

							AvoGUI::Image* newIcon = getGUI()->getDrawingContext()->createImage(icon);
							fileItem->setIcon(newIcon);
							m_uniqueLoadedFileIcons[fileInfo.iIcon] = newIcon;

							DestroyIcon(icon);
						}
						else
						{
							fileItem->setIcon(m_uniqueLoadedFileIcons[fileInfo.iIcon]);
						}
					}
				}
			}
		}
	}

	if (thumbnailCache)
	{
		thumbnailCache->Release();
	}
}

//
// Public
//

FileBrowserItems::~FileBrowserItems()
{
	m_needsToExitIconLoadingThread = true;
	m_needsToLoadMoreIcons = true;
	m_needsToLoadMoreIconsMutex.lock();
	m_needsToLoadMoreIconsConditionVariable.notify_one();
	m_needsToLoadMoreIconsMutex.unlock();
	m_iconLoadingThread.join();

	m_fileNameEndGradient->forget();
	for (auto& icon : m_uniqueLoadedFileIcons)
	{
		icon.second->forget();
	}
	for (auto& icon : m_uniqueLoadedDirectoryIcons)
	{
		icon.second->forget();
	}
	if (m_text_directories)
	{
		m_text_directories->forget();
	}
	if (m_text_files)
	{
		m_text_files->forget();
	}
	if (m_windowsDirectoryIconList)
	{
		m_windowsDirectoryIconList->Release();
	}
	if (m_windowsFileIconList)
	{
		m_windowsFileIconList->Release();
	}
}

//------------------------------

void FileBrowserItems::setSelectedItem(FileBrowserItem* p_item)
{
	if (p_item && p_item->getIsSelected())
	{
		for (FileBrowserItem* item : m_selectedItems)
		{
			if (item != p_item)
			{
				item->deselect();
			}
		}
		m_selectedItems.clear();
		m_selectedItems.push_back(p_item);
	}
	else
	{
		for (FileBrowserItem* item : m_selectedItems)
		{
			item->deselect();
		}
		m_selectedItems.clear();
		if (p_item)
		{
			m_selectedItems.push_back(p_item);
			p_item->select();
		}
	}
}
void FileBrowserItems::addSelectedItem(FileBrowserItem* p_item)
{
	m_selectedItems.push_back(p_item);
	p_item->select();
}

//------------------------------

void FileBrowserItems::tellIconLoadingThreadToLoadMoreIcons()
{
	if (!m_needsToLoadMoreIcons)
	{
		m_needsToLoadMoreIconsMutex.lock();
		m_needsToLoadMoreIcons = true;
		m_needsToLoadMoreIconsMutex.unlock();
		m_needsToLoadMoreIconsConditionVariable.notify_one();
	}
}

void FileBrowserItems::setWorkingDirectory(std::filesystem::path const& p_path)
{
	m_path = p_path;

	if (!m_needsToChangeDirectory)
	{
		m_needsToChangeDirectory = true;
		tellIconLoadingThreadToLoadMoreIcons();
		return;
	}
	else
	{
		m_needsToChangeDirectory = false;
	}

	m_selectedItems.clear();
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