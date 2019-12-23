#include "FileBrowserItems.hpp"

#include "FileBrowserItem.hpp"

//------------------------------

constexpr float PADDING_TOP = 2				* 8.f;
constexpr float PADDING = 3					* 8.f;
constexpr float MARGIN_HORIZONTAL = 1		* 8.f;
constexpr float MARGIN_VERTICAL = 1			* 8.f;
constexpr float LABEL_MARGIN_TOP = 3		* 8.f;
constexpr float LABEL_MARGIN_BOTTOM = 2		* 8.f;
constexpr float MIN_FILE_WIDTH = 20			* 8.f;
constexpr float MIN_DIRECTORY_WIDTH = 20	* 8.f;
constexpr float DIRECTORY_HEIGHT = 6		* 8.f;

//------------------------------

bool getIsPathLessThan(std::filesystem::path const& p_a, std::filesystem::path const& p_b)
{
	std::wstring string_a = p_a.wstring();
	std::wstring string_b = p_b.wstring();
	
	return CSTR_LESS_THAN == CompareStringW(LOCALE_SYSTEM_DEFAULT, LINGUISTIC_IGNORECASE | SORT_DIGITSASNUMBERS, string_a.c_str(), string_a.size(), string_b.c_str(), string_b.size());
}

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

			deselectAllItems();
			m_firstSelectedItem = 0;
			m_lastSelectedItem = 0;

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
			std::sort(filePaths.begin(), filePaths.end(), getIsPathLessThan);
			std::sort(directoryPaths.begin(), directoryPaths.end(), getIsPathLessThan);

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
				getParent()->invalidate();
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
			int32 numberOfColumns = getNumberOfDirectoriesPerRow();
			int32 firstVisibleDirectoryItemIndex = numberOfColumns * floor((-getTop() - m_text_directories->getBottom() - LABEL_MARGIN_BOTTOM) / (m_directoryItems[0]->getHeight() + MARGIN_VERTICAL));
			int32 lastVisibleDirectoryItemIndex = numberOfColumns * floor(1 + (-getTop() + getParent()->getHeight() - m_text_directories->getBottom() - LABEL_MARGIN_BOTTOM) / (m_directoryItems[0]->getHeight() + MARGIN_VERTICAL));
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
			int32 numberOfColumns = getNumberOfFilesPerRow();
			int32 firstVisibleFileItemIndex = numberOfColumns * floor((-getTop() - m_text_files->getBottom() - LABEL_MARGIN_BOTTOM) / (m_fileItems[0]->getHeight() + MARGIN_VERTICAL));
			int32 lastVisibleFileItemIndex = numberOfColumns * floor(1 + (-getTop() + getParent()->getHeight() - m_text_files->getBottom() - LABEL_MARGIN_BOTTOM) / (m_fileItems[0]->getHeight() + MARGIN_VERTICAL));
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
				m_thumbnailCache->GetThumbnail(item, 100, WTS_EXTRACT, &bitmap, &flags, 0);

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

uint32 FileBrowserItems::getNumberOfDirectoriesPerRow()
{
	return AvoGUI::max(1u, (uint32)floor((getParent()->getWidth() - PADDING + MARGIN_HORIZONTAL) / (MIN_DIRECTORY_WIDTH + MARGIN_HORIZONTAL)));
}
uint32 FileBrowserItems::getNumberOfFilesPerRow()
{
	return AvoGUI::max(1u, (uint32)floor((getParent()->getWidth() - PADDING + MARGIN_HORIZONTAL) / (MIN_FILE_WIDTH + MARGIN_HORIZONTAL)));
}

void FileBrowserItems::scrollToShowLastSelectedItem()
{
	if (!m_lastSelectedItem)
	{
		return;
	}
	if (m_lastSelectedItem->getAbsoluteTop() < getParent()->getAbsoluteTop())
	{
		getParent<ScrollContainer>()->setScrollPosition(0.f, m_lastSelectedItem->getTop() - MARGIN_VERTICAL);
		getParent()->invalidate();
	}
	else if (m_lastSelectedItem->getAbsoluteBottom() > getParent()->getAbsoluteBottom())
	{
		getParent<ScrollContainer>()->setScrollPosition(0.f, m_lastSelectedItem->getBottom() - getParent()->getHeight() + MARGIN_VERTICAL);
		getParent()->invalidate();
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

	if (m_directoryGeometry)
	{
		m_directoryGeometry->forget();
	}
	if (m_fileGeometry)
	{
		m_fileGeometry->forget();
	}
	if (m_fileNameEndGradient)
	{
		m_fileNameEndGradient->forget();
	}
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
	m_firstSelectedItem = p_item;
	m_lastSelectedItem = p_item;
}
void FileBrowserItems::addSelectedItem(FileBrowserItem* p_item)
{
	m_selectedItems.push_back(p_item);
	p_item->select();
	m_firstSelectedItem = p_item;
	m_lastSelectedItem = p_item;
}
void FileBrowserItems::removeSelectedItem(FileBrowserItem* p_item)
{
	p_item->deselect();
	AvoGUI::removeVectorElementWithoutKeepingOrder(m_selectedItems, p_item);
	m_firstSelectedItem = p_item;
	m_lastSelectedItem = p_item;
}
void FileBrowserItems::selectItemsTo(FileBrowserItem* p_item, bool p_isAdditive)
{
	if (!p_item)
	{
		return;
	}

	uint32 firstIndex = 0;
	uint32 lastIndex = 0;
	if (m_firstSelectedItem)
	{
		if (m_firstSelectedItem->getIndex() < p_item->getIndex())
		{
			firstIndex = m_firstSelectedItem->getIndex();
			lastIndex = p_item->getIndex();
		}
		else
		{
			firstIndex = p_item->getIndex();
			lastIndex = m_firstSelectedItem->getIndex();
		}
	}
	else
	{
		lastIndex = p_item->getIndex();
	}

	if (!p_isAdditive)
	{
		deselectAllItems();
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
void FileBrowserItems::deselectAllItems()
{
	for (FileBrowserItem* item : m_selectedItems)
	{
		item->deselect();
	}
	m_selectedItems.clear();
}

//------------------------------

void FileBrowserItems::handleMouseDown(AvoGUI::MouseEvent const& p_event)
{
	if (m_isMouseOnBackground && p_event.modifierKeys & AvoGUI::ModifierKeyFlags::LeftMouse)
	{
		if (!(p_event.modifierKeys & AvoGUI::ModifierKeyFlags::Control))
		{
			deselectAllItems();
		}
		m_isDraggingSelectionRectangle = true;
		m_selectionRectangle.set(p_event.x, p_event.y, p_event.x, p_event.y);
		m_selectionRectangleAnchor.set(p_event.x, p_event.y);
		getGui()->setKeyboardFocus(this);
	}
}
void FileBrowserItems::handleMouseUp(AvoGUI::MouseEvent const& p_event)
{
	if (m_isDraggingSelectionRectangle)
	{
		m_isDraggingSelectionRectangle = false;
		getGui()->invalidateRectangle(m_selectionRectangle + getAbsoluteTopLeft());
	}
}
void FileBrowserItems::handleMouseMove(AvoGUI::MouseEvent const& p_event)
{
	if (m_isDraggingSelectionRectangle && AvoGUI::Point<float>::getDistanceSquared(p_event.x, p_event.y, m_selectionRectangleAnchor.x, m_selectionRectangleAnchor.y) > 36.f)
	{
		getGui()->invalidateRectangle(m_selectionRectangle + getAbsoluteTopLeft());
		if (p_event.x < m_selectionRectangleAnchor.x)
		{
			m_selectionRectangle.left = p_event.x;
			m_selectionRectangle.right = m_selectionRectangleAnchor.x;
		}
		else
		{
			m_selectionRectangle.left = m_selectionRectangleAnchor.x;
			m_selectionRectangle.right = p_event.x;
		}
		if (p_event.y < m_selectionRectangleAnchor.y)
		{
			m_selectionRectangle.top = p_event.y;
			m_selectionRectangle.bottom = m_selectionRectangleAnchor.y;
		}
		else
		{
			m_selectionRectangle.top = m_selectionRectangleAnchor.y;
			m_selectionRectangle.bottom = p_event.y;
		}

		getGui()->invalidateRectangle(m_selectionRectangle + getAbsoluteTopLeft());

		if (!getGui()->getWindow()->getIsKeyDown(AvoGUI::KeyboardKey::Control))
		{
			deselectAllItems();
		}

		int32 leftIndex = 0;
		int32 rightIndex = 0;
		int32 topIndex = 0;
		int32 bottomIndex = 0;
		if (m_directoryItems.size())
		{
			uint32 numberOfDirectoriesPerRow = getNumberOfDirectoriesPerRow();
			topIndex = (m_selectionRectangle.top - m_directoryItems[0]->getTop()) / (m_directoryItems[0]->getHeight() + MARGIN_VERTICAL);
			if (topIndex <= m_directoryItems.size() / numberOfDirectoriesPerRow)
			{
				if (topIndex < 0)
				{
					topIndex = 0;
				}

				bottomIndex = int32(m_selectionRectangle.bottom - m_directoryItems[0]->getTop()) / int32(m_directoryItems[0]->getHeight() + MARGIN_VERTICAL);

				leftIndex = AvoGUI::constrain(int32(m_selectionRectangle.left - PADDING) / int32(m_directoryItems[0]->getWidth() + MARGIN_HORIZONTAL), 0, (int32)numberOfDirectoriesPerRow - 1);

				rightIndex = AvoGUI::constrain(int32(m_selectionRectangle.right - PADDING) / int32(m_directoryItems[0]->getWidth() + MARGIN_HORIZONTAL), leftIndex, (int32)numberOfDirectoriesPerRow - 1);

				for (uint32 y = topIndex; y <= bottomIndex; y++)
				{
					for (uint32 x = leftIndex; x <= rightIndex; x++)
					{
						uint32 index = y * numberOfDirectoriesPerRow + x;
						if (index >= m_directoryItems.size())
						{
							y = bottomIndex + 1;
							break;
						}
						if (m_directoryItems[index]->getIsIntersecting(m_selectionRectangle) && !m_directoryItems[index]->getIsSelected())
						{
							addSelectedItem(m_directoryItems[index]);
						}
					}
				}
			}
		}
		if (m_directoryItems.size())
		{
			uint32 numberOfDirectoriesPerRow = getNumberOfDirectoriesPerRow();
			topIndex = int32(m_selectionRectangle.top - m_directoryItems[0]->getTop()) / int32(m_directoryItems[0]->getHeight() + MARGIN_VERTICAL);
			bottomIndex = int32(m_selectionRectangle.bottom - m_directoryItems[0]->getTop()) / int32(m_directoryItems[0]->getHeight() + MARGIN_VERTICAL);
			if (topIndex <= int32(m_directoryItems.size() / numberOfDirectoriesPerRow) && bottomIndex >= 0)
			{
				if (topIndex < 0)
				{
					topIndex = 0;
				}

				leftIndex = AvoGUI::constrain(int32(m_selectionRectangle.left - PADDING) / int32(m_directoryItems[0]->getWidth() + MARGIN_HORIZONTAL), 0, (int32)numberOfDirectoriesPerRow - 1);

				rightIndex = AvoGUI::constrain(int32(m_selectionRectangle.right - PADDING) / int32(m_directoryItems[0]->getWidth() + MARGIN_HORIZONTAL), leftIndex, (int32)numberOfDirectoriesPerRow - 1);

				for (uint32 y = topIndex; y <= bottomIndex; y++)
				{
					for (uint32 x = leftIndex; x <= rightIndex; x++)
					{
						uint32 index = y * numberOfDirectoriesPerRow + x;
						if (index >= m_directoryItems.size())
						{
							y = bottomIndex + 1;
							break;
						}
						if (m_directoryItems[index]->getIsIntersecting(m_selectionRectangle) && !m_directoryItems[index]->getIsSelected())
						{
							addSelectedItem(m_directoryItems[index]);
						}
					}
				}
			}
		}
		if (m_fileItems.size())
		{
			uint32 numberOfFilesPerRow = getNumberOfFilesPerRow();
			topIndex = (m_selectionRectangle.top - m_fileItems[0]->getTop()) / (m_fileItems[0]->getHeight() + MARGIN_VERTICAL);
			bottomIndex = int32(m_selectionRectangle.bottom - m_fileItems[0]->getTop()) / int32(m_fileItems[0]->getHeight() + MARGIN_VERTICAL);
			if (topIndex <= int32(m_fileItems.size() / numberOfFilesPerRow) && bottomIndex >= 0)
			{
				if (topIndex < 0)
				{
					topIndex = 0;
				}

				leftIndex = AvoGUI::constrain(int32(m_selectionRectangle.left - PADDING) / int32(m_fileItems[0]->getWidth() + MARGIN_HORIZONTAL), 0, (int32)numberOfFilesPerRow - 1);

				rightIndex = AvoGUI::constrain(int32(m_selectionRectangle.right - PADDING) / int32(m_fileItems[0]->getWidth() + MARGIN_HORIZONTAL), leftIndex, (int32)numberOfFilesPerRow - 1);

				for (uint32 y = topIndex; y <= bottomIndex; y++)
				{
					for (uint32 x = leftIndex; x <= rightIndex; x++)
					{
						uint32 index = y * numberOfFilesPerRow + x;
						if (index >= m_fileItems.size())
						{
							y = bottomIndex + 1;
							break;
						}
						if (m_fileItems[index]->getIsIntersecting(m_selectionRectangle) && !m_fileItems[index]->getIsSelected())
						{
							addSelectedItem(m_fileItems[index]);
						}
					}
				}
			}
		}
	}
}

//------------------------------

void FileBrowserItems::handleKeyboardKeyDown(AvoGUI::KeyboardEvent const& p_event)
{
	switch (p_event.key)
	{
	case AvoGUI::KeyboardKey::Delete:
	{
		std::wstring paths;
		for (uint32 a = 0; a < m_selectedItems.size(); a++)
		{
			paths += m_selectedItems[a]->getPath().wstring() + L'\0';

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
		paths += L'\0';

		SHFILEOPSTRUCTW fileOperation = { 0 };
		fileOperation.fFlags = FOF_ALLOWUNDO;
		fileOperation.wFunc = FO_DELETE;
		fileOperation.pFrom = paths.data();
		SHFileOperationW(&fileOperation);

		if (fileOperation.fAnyOperationsAborted)
		{
			setWorkingDirectory(m_fileBrowser->getPath());
		}
		else
		{
			m_firstSelectedItem = 0;
			m_lastSelectedItem = 0;
			m_selectedItems.clear();
			updateLayout();
			getParent()->invalidate();
		}
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
	case AvoGUI::KeyboardKey::Left:
	{
		uint32 index = m_lastSelectedItem ? m_lastSelectedItem->getIndex() : 0;
		if (getNumberOfChildren() && index)
		{
			bool isCtrlDown = getGui()->getWindow()->getIsKeyDown(AvoGUI::KeyboardKey::Control);
			if (getGui()->getWindow()->getIsKeyDown(AvoGUI::KeyboardKey::Shift))
			{
				selectItemsTo(getChild<FileBrowserItem>(index - 1), isCtrlDown);
			}
			else if (isCtrlDown)
			{
				addSelectedItem(getChild<FileBrowserItem>(index - 1));
			}
			else
			{
				setSelectedItem(getChild<FileBrowserItem>(index - 1));
			}
			scrollToShowLastSelectedItem();
		}
		break;
	}
	case AvoGUI::KeyboardKey::Right:
	{
		uint32 index = m_lastSelectedItem ? m_lastSelectedItem->getIndex() : 0;
		if (getNumberOfChildren() && (int32)index < (int32)getNumberOfChildren() - 1)
		{
			bool isCtrlDown = getGui()->getWindow()->getIsKeyDown(AvoGUI::KeyboardKey::Control);
			if (getGui()->getWindow()->getIsKeyDown(AvoGUI::KeyboardKey::Shift))
			{
				selectItemsTo(getChild<FileBrowserItem>(index + 1), isCtrlDown);
			}
			else if (isCtrlDown)
			{
				addSelectedItem(getChild<FileBrowserItem>(index + 1));
			}
			else
			{
				setSelectedItem(getChild<FileBrowserItem>(index + 1));
			}
			scrollToShowLastSelectedItem();
		}
		break;
	}
	case AvoGUI::KeyboardKey::Up:
	{
		if (!m_lastSelectedItem)
		{
			break;
		}
		uint32 index = m_lastSelectedItem->getIndex();
		for (int32 a = index - 1; a >= 0; a--)
		{
			if (getChild(a)->getBottom() < m_lastSelectedItem->getTop() &&
				getChild(a)->getRight() >= m_lastSelectedItem->getLeft() &&
				getChild(a)->getLeft() <= m_lastSelectedItem->getRight() || !a)
			{
				bool isCtrlDown = getGui()->getWindow()->getIsKeyDown(AvoGUI::KeyboardKey::Control);
				if (getGui()->getWindow()->getIsKeyDown(AvoGUI::KeyboardKey::Shift))
				{
					selectItemsTo(getChild<FileBrowserItem>(a), isCtrlDown);
				}
				else if (isCtrlDown)
				{
					addSelectedItem(getChild<FileBrowserItem>(a));
				}
				else
				{
					setSelectedItem(getChild<FileBrowserItem>(a));
				}
				scrollToShowLastSelectedItem();
				break;
			}
		}
		break;
	}
	case AvoGUI::KeyboardKey::Down:
	{
		if (!m_lastSelectedItem && getNumberOfChildren())
		{
			m_lastSelectedItem = getChild<FileBrowserItem>(0);
		}
		uint32 index = m_lastSelectedItem->getIndex();
		for (int32 a = index + 1; a < getNumberOfChildren(); a++)
		{
			if (getChild(a)->getTop() > m_lastSelectedItem->getBottom() &&
				getChild(a)->getRight() >= m_lastSelectedItem->getLeft() &&
				getChild(a)->getLeft() <= m_lastSelectedItem->getRight() || a == getNumberOfChildren() - 1)
			{
				bool isCtrlDown = getGui()->getWindow()->getIsKeyDown(AvoGUI::KeyboardKey::Control);
				if (getGui()->getWindow()->getIsKeyDown(AvoGUI::KeyboardKey::Shift))
				{
					selectItemsTo(getChild<FileBrowserItem>(a), isCtrlDown);
				}
				else if (isCtrlDown)
				{
					addSelectedItem(getChild<FileBrowserItem>(a));
				}
				else
				{
					setSelectedItem(getChild<FileBrowserItem>(a));
				}
				scrollToShowLastSelectedItem();
				break;
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

	uint32 directoriesPerRow = getNumberOfDirectoriesPerRow();
	uint32 filesPerRow = getNumberOfFilesPerRow();

	if (m_directoryItems.size())
	{
		float directoryWidth = (getParent()->getWidth() - 2.f*PADDING - MARGIN_HORIZONTAL* (getNumberOfDirectoriesPerRow() - 1.f))/getNumberOfDirectoriesPerRow();
		if (std::abs(directoryWidth - m_directoryItems[0]->getWidth()) > 0.1f)
		{
			if (m_directoryGeometry)
			{
				m_directoryGeometry->forget();
			}
			m_directoryGeometry = getGui()->getDrawingContext()->createCornerRectangleGeometry(directoryWidth, DIRECTORY_HEIGHT, m_directoryItems[0]->getCorners().topLeftSizeX);
		}
		m_text_directories->setTopLeft(PADDING, PADDING_TOP);
		for (uint32 a = 0; a < m_directoryItems.size(); a++)
		{
			m_directoryItems[a]->setSize(directoryWidth, DIRECTORY_HEIGHT);
			m_directoryItems[a]->setTopLeft(
				PADDING + (directoryWidth + MARGIN_HORIZONTAL) * (a % directoriesPerRow), 
				m_text_directories->getBottom() + LABEL_MARGIN_BOTTOM + (DIRECTORY_HEIGHT + MARGIN_VERTICAL) * (a / directoriesPerRow)
			);
		}
	}

	if (m_fileItems.size())
	{
		float fileWidth = (getParent()->getWidth() - 2.f * PADDING - MARGIN_HORIZONTAL * (getNumberOfFilesPerRow() - 1.f)) / getNumberOfFilesPerRow();
		if (std::abs(fileWidth - m_fileItems[0]->getWidth()) > 0.1f)
		{
			if (m_fileGeometry)
			{
				m_fileGeometry->forget();
			}
			m_fileGeometry = getGui()->getDrawingContext()->createCornerRectangleGeometry(fileWidth, fileWidth, m_fileItems[0]->getCorners().topLeftSizeX);
		}
		m_text_files->setTopLeft(PADDING, m_directoryItems.size() ? m_directoryItems.back()->getBottom() + LABEL_MARGIN_TOP : PADDING_TOP);
		for (uint32 a = 0; a < m_fileItems.size(); a++)
		{
			m_fileItems[a]->setSize(fileWidth);
			m_fileItems[a]->setTopLeft(
				PADDING + (fileWidth + MARGIN_HORIZONTAL) * (a % filesPerRow),
				m_text_files->getBottom() + LABEL_MARGIN_BOTTOM + (fileWidth + MARGIN_VERTICAL) * (a / filesPerRow)
			);
		}
	}

	float height = 0.f;
	if (m_fileItems.size())
	{
		height = m_fileItems.back()->getBottom();
	}
	else if (m_directoryItems.size())
	{
		height = m_directoryItems.back()->getBottom();
	}
	setSize(getParent()->getWidth() - PADDING, height + PADDING);

	tellIconLoadingThreadToLoadMoreIcons();
}