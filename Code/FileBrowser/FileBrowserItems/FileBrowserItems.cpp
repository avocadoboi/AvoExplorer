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
	CoInitialize(0);
	CoCreateInstance(CLSID_LocalThumbnailCache, 0, CLSCTX_INPROC, IID_IThumbnailCache, (void**)&m_thumbnailCache);

	SHGetImageList(SHIL_LARGE, IID_IImageList2, (void**)&m_windowsDirectoryIconList);
	SHGetImageList(SHIL_JUMBO, IID_IImageList2, (void**)&m_windowsFileIconList);

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
			m_needsToChangeDirectory = false;

			m_selectedItems.clear();
			m_directoryItems.clear();
			m_fileItems.clear();
			getGui()->excludeAnimationThread();
			removeAllChildren();
			getGui()->includeAnimationThread();

			std::vector<std::filesystem::path> directoryPaths;
			directoryPaths.reserve(128);
			std::vector<std::filesystem::path> filePaths;
			filePaths.reserve(256);

			for (auto item : std::filesystem::directory_iterator(m_fileBrowser->getPath()))
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

			getGui()->excludeAnimationThread();
			for (auto path : directoryPaths)
			{
				m_directoryItems.push_back(new FileBrowserItem(this, path, false));
			}
			for (auto path : filePaths)
			{
				m_fileItems.push_back(new FileBrowserItem(this, path, false));
			}

			if (getParent()->getWidth() && getParent()->getHeight())
			{
				updateLayout();
				invalidate();
			}
			getGui()->includeAnimationThread();
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
				loadIconForItem(m_directoryItems[a]);
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
				loadIconForItem(m_fileItems[a]);
			}
		}

		if (m_itemsToLoadIconFor.size())
		{
			std::vector<FileBrowserItem*> items = std::move(m_itemsToLoadIconFor);
			for (FileBrowserItem* item : items)
			{
				loadIconForItem(item);
			}
		}
	}
	m_thumbnailCache->Release();
	m_windowsDirectoryIconList->Release();
	m_windowsFileIconList->Release();
}

void FileBrowserItems::loadIconForItem(FileBrowserItem* p_item)
{
	if (!p_item->getHasLoadedIcon())
	{
		if (p_item->getIsFile())
		{
			if (p_item->getIsIconThumbnail())
			{
				IShellItem* item = 0;

				HRESULT result = SHCreateItemFromParsingName(p_item->getPath().c_str(), 0, IID_PPV_ARGS(&item));

				ISharedBitmap* bitmap = 0;
				WTS_CACHEFLAGS flags;
				m_thumbnailCache->GetThumbnail(item, 128, WTS_EXTRACT, &bitmap, &flags, 0);

				if (bitmap)
				{
					HBITMAP bitmapHandle;
					bitmap->GetSharedBitmap(&bitmapHandle);

					AvoGUI::Image* newIcon = getGui()->getDrawingContext()->createImage(bitmapHandle);
					p_item->setIcon(newIcon);
					newIcon->forget();

					DeleteObject(bitmapHandle);
					bitmap->Release();
				}

				item->Release();
			}
			else
			{
				SHFILEINFOW fileInfo = { 0 };
				DWORD_PTR result = SHGetFileInfoW(p_item->getPath().c_str(), 0, &fileInfo, sizeof(SHFILEINFOW), SHGFI_SYSICONINDEX);

				if (m_uniqueLoadedFileIcons.find(fileInfo.iIcon) == m_uniqueLoadedFileIcons.end())
				{
					HICON icon;
					m_windowsFileIconList->GetIcon(fileInfo.iIcon, 0, &icon);

					AvoGUI::Image* newIcon = getGui()->getDrawingContext()->createImage(icon);
					p_item->setIcon(newIcon);
					m_uniqueLoadedFileIcons[fileInfo.iIcon] = newIcon;

					DestroyIcon(icon);
				}
				else
				{
					p_item->setIcon(m_uniqueLoadedFileIcons[fileInfo.iIcon]);
				}
			}
		}
		else
		{
			SHFILEINFOW fileInfo = { 0 };
			DWORD_PTR result = SHGetFileInfoW(p_item->getPath().c_str(), 0, &fileInfo, sizeof(SHFILEINFOW), SHGFI_SYSICONINDEX);

			if (m_uniqueLoadedDirectoryIcons.find(fileInfo.iIcon) == m_uniqueLoadedDirectoryIcons.end())
			{
				HICON icon;
				m_windowsDirectoryIconList->GetIcon(fileInfo.iIcon, 0, &icon);

				AvoGUI::Image* newIcon = getGui()->getDrawingContext()->createImage(icon);
				p_item->setIcon(newIcon);
				m_uniqueLoadedDirectoryIcons[fileInfo.iIcon] = newIcon;
				DestroyIcon(icon);
			}
			else
			{
				p_item->setIcon(m_uniqueLoadedDirectoryIcons[fileInfo.iIcon]);
			}
		}
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
	if (p_item)
	{
		m_lastSelectedItem = p_item;
	}
}
void FileBrowserItems::addSelectedItem(FileBrowserItem* p_item)
{
	m_selectedItems.push_back(p_item);
	p_item->select();
	m_lastSelectedItem = p_item;
}
void FileBrowserItems::selectItemsTo(FileBrowserItem* p_item)
{
	if (!p_item || p_item == m_lastSelectedItem)
	{
		return;
	}

	uint32 firstIndex = 0;
	uint32 lastIndex = 0;
	if (m_lastSelectedItem)
	{
		if (m_lastSelectedItem->getIndex() < p_item->getIndex())
		{
			firstIndex = m_lastSelectedItem->getIndex();
			lastIndex = p_item->getIndex();
		}
		else
		{
			firstIndex = p_item->getIndex();
			lastIndex = m_lastSelectedItem->getIndex();
		}
	}
	else
	{
		lastIndex = p_item->getIndex();
	}

	for (uint32 a = firstIndex; a <= lastIndex; a++)
	{
		FileBrowserItem* item = (FileBrowserItem*)getChild(a);
		if (!item->getIsSelected())
		{
			item->select();
			m_selectedItems.push_back(item);
		}
	}
	m_lastSelectedItem = p_item;
}
void FileBrowserItems::removeSelectedItem(FileBrowserItem* p_item)
{
	p_item->deselect();
	AvoGUI::removeVectorElementWithoutKeepingOrder(m_selectedItems, p_item);
	m_lastSelectedItem = p_item;
}

//------------------------------

void FileBrowserItems::handleMouseDown(AvoGUI::MouseEvent const& p_event)
{
	if (m_isMouseOnBackground && p_event.modifierKeys == AvoGUI::ModifierKeyFlags::LeftMouse)
	{
		for (FileBrowserItem* item : m_selectedItems)
		{
			item->deselect();
		}
		m_selectedItems.clear();
	}
	getGui()->setKeyboardFocus(this);
}

//------------------------------

void FileBrowserItems::handleKeyboardKeyDown(AvoGUI::KeyboardEvent const& p_event)
{
	switch (p_event.key)
	{
	case AvoGUI::KeyboardKey::Delete:
	{
		wchar_t pathBuffer[MAX_PATH + 1]; // MAX_PATH includes first 1 terminator, we want 2 null terminators
		for (uint32 a = 0; a < m_selectedItems.size(); a++)
		{
			std::wstring pathString = m_selectedItems[a]->getPath();

			memcpy(pathBuffer, pathString.data(), pathString.size() * 2);
			pathBuffer[pathString.size()] = 0;
			pathBuffer[pathString.size() + 1] = 0;

			SHFILEOPSTRUCTW fileOperation = { 0 };
			fileOperation.fFlags = FOF_ALLOWUNDO;
			fileOperation.wFunc = FO_DELETE;
			fileOperation.pFrom = pathBuffer;
			SHFileOperationW(&fileOperation);

			if (fileOperation.fAnyOperationsAborted)
			{
				continue;
			}

			if (m_selectedItems[a]->getIsFile())
			{
				m_fileItems.erase(m_fileItems.begin() + (m_selectedItems[a]->getIndex() - m_directoryItems.size()));
			}
			else
			{
				m_directoryItems.erase(m_directoryItems.begin() + m_selectedItems[a]->getIndex());
			}
			removeChild(m_selectedItems[a]->getIndex());
		}
		m_selectedItems.clear();
		updateLayout();
		invalidate();
		break;
	}
	case AvoGUI::KeyboardKey::A:
	{
		if (getGui()->getWindow()->getIsKeyDown(AvoGUI::KeyboardKey::Control))
		{
			for (uint32 a = 0; a < m_directoryItems.size(); a++)
			{
				if (!m_directoryItems[a]->getIsSelected())
				{
					m_directoryItems[a]->select();
					m_selectedItems.push_back(m_directoryItems[a]);
				}
			}
			for (uint32 a = 0; a < m_fileItems.size(); a++)
			{
				if (!m_fileItems[a]->getIsSelected())
				{
					m_fileItems[a]->select();
					m_selectedItems.push_back(m_fileItems[a]);
				}
			}
		}
		break;
	}
	}
}

//------------------------------

void FileBrowserItems::tellIconLoadingThreadToLoadIconForItem(FileBrowserItem* p_item)
{
	m_itemsToLoadIconFor.push_back(p_item);
	tellIconLoadingThreadToLoadMoreIcons();
}
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
	if (!m_needsToChangeDirectory)
	{
		m_needsToChangeDirectory = true;
		tellIconLoadingThreadToLoadMoreIcons();
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