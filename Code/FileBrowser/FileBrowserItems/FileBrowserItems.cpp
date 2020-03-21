#include "FileBrowserItems.hpp"

#include "FileBrowserItem.hpp"

#include <fstream>

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

/*
	Returns true if p_a is less than p_b.
*/
bool getIsPathStringLessThan(std::wstring const& p_a, std::wstring const& p_b)
{
	return CSTR_LESS_THAN == CompareStringW(LOCALE_SYSTEM_DEFAULT, LINGUISTIC_IGNORECASE | SORT_DIGITSASNUMBERS, p_a.c_str(), p_a.size(), p_b.c_str(), p_b.size());
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

			std::filesystem::path const& newPath = m_fileBrowser->getPath();
			for (auto item : std::filesystem::directory_iterator(newPath))
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
			std::sort(filePaths.begin(), filePaths.end(), getIsPathStringLessThan);
			std::sort(directoryPaths.begin(), directoryPaths.end(), getIsPathStringLessThan);

			m_directoryItems.reserve(directoryPaths.size());
			m_fileItems.reserve(filePaths.size());

			/*
				Find the name of the child directory that was last visited, it will be used to find the right item to later be selected.
			*/
			int32 lastVisitedDirectoryItemIndex = -1;
			std::string lastWorkingDirectoryName = m_lastPath.native().size() > newPath.native().size() ? m_lastPath.parent_path().u8string() : "";
			std::string newPathString = newPath.u8string();
			if (lastWorkingDirectoryName.size() && newPathString == lastWorkingDirectoryName.substr(0, newPathString.size()))
			{
				for (uint32 a = newPathString.size(); a <= lastWorkingDirectoryName.size(); a++)
				{
					if (a == lastWorkingDirectoryName.size() || lastWorkingDirectoryName[a] == '\\')
					{
						lastWorkingDirectoryName = lastWorkingDirectoryName.substr(newPathString.size(), a - newPathString.size());
					}
				}
			}
			else
			{
				lastWorkingDirectoryName = "";
			}

			getGui()->excludeAnimationThread();
			for (uint32 a = 0; a < directoryPaths.size(); a++)
			{
				m_directoryItems.push_back(new FileBrowserItem(this, directoryPaths[a], false));
				m_directoryItems.back()->setItemIndex(a);
				if (lastWorkingDirectoryName.size() && lastVisitedDirectoryItemIndex == -1 &&
					m_directoryItems.back()->getName() == lastWorkingDirectoryName)
				{
					lastVisitedDirectoryItemIndex = a;
				}
			}
			for (uint32 a = 0; a < filePaths.size(); a++)
			{
				m_fileItems.push_back(new FileBrowserItem(this, filePaths[a], false));
				m_fileItems.back()->setItemIndex(a);
			}

			if (getParent()->getWidth() && getParent()->getHeight())
			{
				updateLayout();
				getParent()->invalidate();
			}
			getGui()->includeAnimationThread();

			if (lastVisitedDirectoryItemIndex >= 0)
			{
				setSelectedItem(m_directoryItems[lastVisitedDirectoryItemIndex]);
			}
			m_lastPath = newPath;
		}

		if (m_needsToExitIconLoadingThread)
		{
			break;
		}

		m_needsToLoadMoreIcons = false;

		if (m_directoryItems.size())
		{
			std::lock_guard<std::mutex> lock(m_directoryItemsMutex);
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
			std::lock_guard<std::mutex> lock(m_fileItemsMutex);
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

FileBrowserItem* FileBrowserItems::getItemFromAbsoluteIndex(uint32 p_index)
{
	return p_index >= m_directoryItems.size() ? m_fileItems[p_index - m_directoryItems.size()] : m_directoryItems[p_index];
}
uint32 FileBrowserItems::getAbsoluteIndexFromItem(FileBrowserItem* p_item)
{
	return p_item ? (p_item->getIsFile() ? p_item->getItemIndex() + m_directoryItems.size() : p_item->getItemIndex()) : 0;
}

void FileBrowserItems::insertNewFileItem(std::filesystem::path const& p_path)
{
	std::lock_guard<std::mutex> lock(m_fileItemsMutex);

	FileBrowserItem* newItem = new FileBrowserItem(this, p_path, false);
	FileBrowserItem* lastItem = 0;
	for (uint32 a = 0; a < m_fileItems.size(); a++)
	{
		if (lastItem)
		{
			std::swap(lastItem, m_fileItems[a]);
			lastItem->incrementItemIndex();
		}
		else if (getIsPathStringLessThan(p_path, m_fileItems[a]->getPath()))
		{
			lastItem = m_fileItems[a];
			lastItem->incrementItemIndex();
			m_fileItems[a] = newItem;
			newItem->setItemIndex(a);
		}
	}
	m_fileItems.push_back(lastItem ? lastItem : newItem);
	updateLayout();
	setSelectedItem(newItem);
	getParent()->invalidate();
	tellIconLoadingThreadToLoadMoreIcons();
}
void FileBrowserItems::insertNewDirectoryItem(std::filesystem::path const& p_path)
{
	std::lock_guard<std::mutex> lock(m_directoryItemsMutex);

	FileBrowserItem* newItem = new FileBrowserItem(this, p_path, false);
	FileBrowserItem* lastItem = 0;
	for (uint32 a = 0; a < m_directoryItems.size(); a++)
	{
		if (lastItem)
		{
			std::swap(lastItem, m_directoryItems[a]);
			lastItem->incrementItemIndex();
		}
		else if (getIsPathStringLessThan(p_path, m_directoryItems[a]->getPath()))
		{
			lastItem = m_directoryItems[a];
			lastItem->incrementItemIndex();
			m_directoryItems[a] = newItem;
			newItem->setItemIndex(a);
		}
	}
	m_directoryItems.push_back(lastItem ? lastItem : newItem);
	updateLayout();
	setSelectedItem(newItem);
	getParent()->invalidate();
	tellIconLoadingThreadToLoadMoreIcons();
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

void FileBrowserItems::setSelectedItem(FileBrowserItem* p_item, bool p_willScrollToShowItem)
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

	if (p_willScrollToShowItem)
	{
		scrollToShowLastSelectedItem();
	}
}
void FileBrowserItems::addSelectedItem(FileBrowserItem* p_item, bool p_willScrollToShowItem)
{
	if (p_item)
	{
		m_selectedItems.push_back(p_item);
		p_item->select();
		m_firstSelectedItem = p_item;
		m_lastSelectedItem = p_item;

		if (p_willScrollToShowItem)
		{
			scrollToShowLastSelectedItem();
		}
	}
}
void FileBrowserItems::selectItemsTo(FileBrowserItem* p_item, bool p_isAdditive, bool p_willScrollToShowItem)
{
	if (!p_item)
	{
		return;
	}

	uint32 firstIndex = 0;
	uint32 lastIndex = 0;
	if (m_firstSelectedItem)
	{
		if (getAbsoluteIndexFromItem(m_firstSelectedItem) < getAbsoluteIndexFromItem(p_item))
		{
			firstIndex = getAbsoluteIndexFromItem(m_firstSelectedItem);
			lastIndex = getAbsoluteIndexFromItem(p_item);
		}
		else
		{
			firstIndex = getAbsoluteIndexFromItem(p_item);
			lastIndex = getAbsoluteIndexFromItem(m_firstSelectedItem);
		}
	}
	else
	{
		lastIndex = getAbsoluteIndexFromItem(p_item);
	}

	if (!p_isAdditive)
	{
		deselectAllItems();
	}

	for (uint32 a = firstIndex; a <= lastIndex; a++)
	{
		FileBrowserItem* item = getItemFromAbsoluteIndex(a);
		if (!item->getIsSelected())
		{
			item->select();
			m_selectedItems.push_back(item);
		}
	}
	m_lastSelectedItem = p_item;
	if (p_willScrollToShowItem)
	{
		scrollToShowLastSelectedItem();
	}
}
void FileBrowserItems::removeSelectedItem(FileBrowserItem* p_item)
{
	p_item->deselect();
	AvoGUI::removeVectorElementWithoutKeepingOrder(m_selectedItems, p_item);
	m_firstSelectedItem = p_item;
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

void FileBrowserItems::createFile(std::string const& p_name, bool p_willReplaceExisting)
{
	if (p_name == "")
	{
		return;
	}

	std::filesystem::path path = m_fileBrowser->getPath().native() + AvoGUI::convertUtf8ToUtf16(p_name);
	path.make_preferred();
	bool existsPath = std::filesystem::exists(path);
	bool wasDirectory = existsPath ? std::filesystem::is_directory(path) : false;

	if (existsPath)
	{
		if (p_willReplaceExisting)
		{
			// Move old file to recycle bin

			std::wstring doubleNullTerminatedPath = path.wstring() + L'\0';

			SHFILEOPSTRUCTW fileOperation = { 0 };
			fileOperation.fFlags = FOF_ALLOWUNDO;
			fileOperation.wFunc = FO_DELETE;
			fileOperation.pFrom = doubleNullTerminatedPath.data();
			SHFileOperationW(&fileOperation);

			if (fileOperation.fAnyOperationsAborted)
			{
				return;
			}
		}
		else
		{
			ChoiceDialogBox* dialog = new ChoiceDialogBox(getGui(), Strings::newDirectoryOrFileAlreadyExistsDialogTitle, Strings::newDirectoryOrFileAlreadyExistsDialogMessage);
			dialog->addButton(Strings::replace, AvoGUI::Button::Emphasis::High);
			dialog->addButton(Strings::no, AvoGUI::Button::Emphasis::Medium);
			dialog->addDialogArgument(p_name);
			dialog->setId(Ids::newFileAlreadyExistsDialog);
			dialog->setChoiceDialogBoxListener(this);
			dialog->detachFromThread();
			return;
		}
	}


	/*
		If the user wrote for example "parent/child/file.txt", then create those directories first, then the file.
		In that example, a new directory item "parent" will be inserted.
	*/

	// Will be used later to determine whether to insert new file item
	bool isNewFileInSameDirectory = !std::filesystem::path(p_name).has_parent_path() && std::filesystem::exists(path.parent_path());
	if (!isNewFileInSameDirectory)
	{
		std::filesystem::path newItemPath;
		for (uint32 a = 0; a < p_name.size(); a++)
		{
			if (p_name[a] == '/' || p_name[a] == '\\')
			{
				newItemPath = m_fileBrowser->getPath().native() + AvoGUI::convertUtf8ToUtf16(p_name.substr(0, a));
				break;
			}
		}
		newItemPath.make_preferred();
		bool existedNewItemPath = std::filesystem::exists(newItemPath);

		std::error_code errorCode;
		std::filesystem::create_directories(path.parent_path(), errorCode);
		if (errorCode)
		{
			ChoiceDialogBox* dialog = new ChoiceDialogBox(getGui(), Strings::newDirectoryFailedDialogTitle, (Strings::newDirectoryFailedDialogMessage + errorCode.message()).c_str());
			dialog->addButton(Strings::ok, AvoGUI::Button::Emphasis::High);
			dialog->detachFromThread();
			return;
		}
		else if (!existedNewItemPath)
		{
			insertNewDirectoryItem(newItemPath);
		}
	}

	//------------------------------
	// Create the file

	HANDLE fileHandle = CreateFileW(path.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, 0, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, 0);
	CloseHandle(fileHandle);

	if (fileHandle == INVALID_HANDLE_VALUE)
	{
		DWORD error = GetLastError();
		switch (error)
		{
		case ERROR_ACCESS_DENIED:
		{
			ChoiceDialogBox* dialog = new ChoiceDialogBox(getGui(), Strings::newFileAccessDeniedDialogTitle, Strings::newFileAccessDeniedDialogMessage);
			dialog->addButton(Strings::restart, AvoGUI::Button::Emphasis::High);
			dialog->addButton(Strings::no, AvoGUI::Button::Emphasis::Medium);
			dialog->setId(Ids::newFileAccessDeniedDialog);
			dialog->setChoiceDialogBoxListener(this);
			dialog->detachFromThread();
			break;
		}
		default:
		{
			ChoiceDialogBox* dialog = new ChoiceDialogBox(getGui(), Strings::newFileFailedDialogTitle, Strings::newFileFailedDialogMessage);
			dialog->addButton(Strings::ok, AvoGUI::Button::Emphasis::High);
			dialog->detachFromThread();
		}
		}
	}
	else if (isNewFileInSameDirectory && wasDirectory == p_willReplaceExisting)
	{
		if (wasDirectory)
		{
			for (uint32 a = 0; a < m_directoryItems.size(); a++)
			{
				if (m_directoryItems[a]->getPath() == path)
				{
					deselectAllItems();
					removeChild(m_directoryItems[a]);
					m_directoryItems.erase(m_directoryItems.begin() + a);
					break;
				}
			}
		}
		
		insertNewFileItem(path);
	}
}
void FileBrowserItems::createDirectory(std::string const& p_name, bool p_willReplaceExisting)
{
	std::filesystem::path path = m_fileBrowser->getPath().native() + AvoGUI::convertUtf8ToUtf16(p_name);
	path.make_preferred();

	std::filesystem::path newItemPath;
	if (std::filesystem::path(p_name).has_parent_path())
	{
		for (uint32 a = 0; a < p_name.size(); a++)
		{
			if (p_name[a] == '/' || p_name[a] == '\\')
			{
				newItemPath = m_fileBrowser->getPath().native() + AvoGUI::convertUtf8ToUtf16(p_name.substr(0, a));
				break;
			}
		}
		newItemPath.make_preferred();
	}
	else
	{
		newItemPath = path;
	}

	bool existedNewItemPath = std::filesystem::exists(newItemPath);
	bool wasRegularFile = existedNewItemPath ? std::filesystem::is_regular_file(newItemPath) : false;

	if (existedNewItemPath)
	{
		if (p_willReplaceExisting)
		{
			std::wstring doubleNullTerminatedPath = path.wstring() + L'\0';

			SHFILEOPSTRUCTW fileOperation = { 0 };
			fileOperation.fFlags = FOF_ALLOWUNDO;
			fileOperation.wFunc = FO_DELETE;
			fileOperation.pFrom = doubleNullTerminatedPath.data();
			SHFileOperationW(&fileOperation);
		}
		else
		{
			ChoiceDialogBox* dialog = new ChoiceDialogBox(getGui(), Strings::newDirectoryOrFileAlreadyExistsDialogTitle, Strings::newDirectoryOrFileAlreadyExistsDialogMessage);
			dialog->addButton(Strings::replace, AvoGUI::Button::Emphasis::High);
			dialog->addButton(Strings::no, AvoGUI::Button::Emphasis::Medium);
			dialog->addDialogArgument(p_name);
			dialog->setId(Ids::newDirectoryAlreadyExistsDialog);
			dialog->setChoiceDialogBoxListener(this);
			dialog->detachFromThread();
			return;
		}
	}

	std::error_code errorCode;
	std::filesystem::create_directories(path, errorCode);
	if (errorCode)
	{
		ChoiceDialogBox* dialog = new ChoiceDialogBox(getGui(), Strings::newDirectoryFailedDialogTitle, (Strings::newDirectoryFailedDialogMessage + errorCode.message()).c_str());
		dialog->addButton(Strings::ok, AvoGUI::Button::Emphasis::High);
		dialog->detachFromThread();
	}
	else if (p_willReplaceExisting == wasRegularFile)
	{
		if (wasRegularFile)
		{
			for (uint32 a = 0; a < m_fileItems.size(); a++)
			{
				if (m_fileItems[a]->getPath() == path)
				{
					deselectAllItems();
					removeChild(m_fileItems[a]);
					m_fileItems.erase(m_fileItems.begin() + a);
					break;
				}
			}
		}

		insertNewDirectoryItem(newItemPath);
	}
}

//------------------------------

void FileBrowserItems::dragSelectedItems()
{
	std::vector<std::string> paths;
	paths.resize(m_selectedItems.size());
	for (uint32 a = 0; a < m_selectedItems.size(); a++)
	{
		paths[a] = m_selectedItems[a]->getPath().string();
	}

	getGui()->getWindow()->dragAndDropFiles(paths, m_selectedItems[0]->getIcon(), m_selectedItems[0]->getIcon()->getOriginalSize()*0.5f);
}
void FileBrowserItems::dropItems(AvoGUI::ClipboardData* p_data, NameCollisionOption p_collisionOption)
{
	std::vector<std::wstring> paths = p_data->getUtf16ItemNames();

	if (!paths.size())
	{
		return;
	}

	std::filesystem::path directoryPath = m_fileBrowser->getPath();

	std::vector<std::filesystem::path> targetDirectoryPaths;
	targetDirectoryPaths.reserve(paths.size());
	std::vector<std::filesystem::path> targetFilePaths;
	targetFilePaths.reserve(paths.size());

	std::wstring pathsString;
	pathsString.reserve(MAX_PATH * paths.size());
	
	uint32 numberOfPathsThatAlreadyExist = 0;
	for (uint32 a = 0; a < paths.size(); a++)
	{
		std::filesystem::path path = paths[a];
		if (std::filesystem::is_regular_file(path))
		{
			path = directoryPath / path.filename();
			targetFilePaths.push_back(path);
		}
		else if (std::filesystem::is_directory(path))
		{
			path = directoryPath / path.filename();
			targetDirectoryPaths.push_back(path);
		}
		else
		{
			continue;
		}

		if (std::filesystem::exists(path) && (p_collisionOption == NameCollisionOption::Skip || p_collisionOption == NameCollisionOption::None))
		{
			if (p_collisionOption == NameCollisionOption::None)
			{
				numberOfPathsThatAlreadyExist++;
			}
		}
		else
		{
			pathsString += paths[a] + L'\0';
		}
	}
	pathsString += L'\0';
	if (numberOfPathsThatAlreadyExist)
	{
		ChoiceDialogBox* dialog = new ChoiceDialogBox(getGui(), Strings::directoriesOrFilesAlreadyExistDialogTitle, AvoGUI::createFormattedString(Strings::directoriesOrFilesAlreadyExistDialogMessage, { numberOfPathsThatAlreadyExist }));
		dialog->addButton(Strings::replace, AvoGUI::Button::Emphasis::High);
		dialog->addButton(Strings::addSuffixes, AvoGUI::Button::Emphasis::High);
		dialog->addButton(Strings::skipDuplicates, AvoGUI::Button::Emphasis::High);
		dialog->addButton(Strings::cancel, AvoGUI::Button::Emphasis::Medium);
		dialog->setWidth(900.f);
		dialog->setId(Ids::directoriesOrFilesAlreadyExistDialog);
		dialog->setChoiceDialogBoxListener(this);
		dialog->detachFromThread();
		return;
	}

	SHFILEOPSTRUCTW fileOperation = { 0 };
	fileOperation.fFlags = FOF_ALLOWUNDO;
	if (p_collisionOption == NameCollisionOption::Rename)
	{
		fileOperation.fFlags |= FOF_RENAMEONCOLLISION;
	}
	else if (p_collisionOption == NameCollisionOption::Replace)
	{
		fileOperation.fFlags |= FOF_NOCONFIRMATION;
	}
	fileOperation.wFunc = FO_COPY;
	fileOperation.pFrom = pathsString.data();

	std::wstring directoryPathString = directoryPath.wstring() + L'\0';
	fileOperation.pTo = directoryPathString.data();

	SHFileOperationW(&fileOperation);

	if (!fileOperation.fAnyOperationsAborted)
	{
		deselectAllItems();
		if (targetDirectoryPaths.size())
		{
			if (targetDirectoryPaths.size() > 1)
			{
				std::sort(targetDirectoryPaths.begin(), targetDirectoryPaths.end(), getIsPathStringLessThan);
			}
			m_directoryItems.resize(m_directoryItems.size() + targetDirectoryPaths.size(), 0);
			int32 insertionIndex = m_directoryItems.size() - 1;
			for (int32 a = m_directoryItems.size() - targetDirectoryPaths.size() - 1; a >= 0; a--)
			{
				while (targetDirectoryPaths.size() && getIsPathStringLessThan(m_directoryItems[a]->getPath(), targetDirectoryPaths.back()))
				{
					m_directoryItems[insertionIndex] = new FileBrowserItem(this, targetDirectoryPaths.back(), false);
					m_directoryItems[insertionIndex]->setItemIndex(insertionIndex);
					targetDirectoryPaths.pop_back();
					insertionIndex--;
				}
				m_directoryItems[insertionIndex] = m_directoryItems[a];
				m_directoryItems[insertionIndex]->setItemIndex(insertionIndex);
				insertionIndex--;
			}
			while (insertionIndex >= 0)
			{
				m_directoryItems[insertionIndex] = new FileBrowserItem(this, targetDirectoryPaths.back(), false);
				m_directoryItems[insertionIndex]->setItemIndex(insertionIndex);
				targetDirectoryPaths.pop_back();
				insertionIndex--;
			}
		}
		if (targetFilePaths.size())
		{
			if (targetFilePaths.size() > 1)
			{
				std::sort(targetFilePaths.begin(), targetFilePaths.end(), getIsPathStringLessThan);
			}
			m_fileItems.resize(m_fileItems.size() + targetFilePaths.size(), 0);
			int32 insertionIndex = m_fileItems.size() - 1;
			for (int32 a = m_fileItems.size() - targetFilePaths.size() - 1; a >= 0; a--)
			{
				while (targetFilePaths.size() && getIsPathStringLessThan(m_fileItems[a]->getPath(), targetFilePaths.back()))
				{
					m_fileItems[insertionIndex] = new FileBrowserItem(this, targetFilePaths.back(), false);
					m_fileItems[insertionIndex]->setItemIndex(insertionIndex);
					targetFilePaths.pop_back();
					insertionIndex--;
				}
				m_fileItems[insertionIndex] = m_fileItems[a];
				m_fileItems[insertionIndex]->setItemIndex(insertionIndex);
				insertionIndex--;
			}
			while (insertionIndex >= 0)
			{
				m_fileItems[insertionIndex] = new FileBrowserItem(this, targetFilePaths.back(), false);
				m_fileItems[insertionIndex]->setItemIndex(insertionIndex);
				targetFilePaths.pop_back();
				insertionIndex--;
			}
		}
		updateLayout();
		getParent()->invalidate();
		tellIconLoadingThreadToLoadMoreIcons();
	}
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
	}
	getGui()->setKeyboardFocus(this);
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
		AvoGUI::Rectangle<float> selectionRectangleBefore = m_selectionRectangle;
		// To keep the correct relationship between left and right and top and bottom.
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

		if (!getGui()->getWindow()->getIsKeyDown(AvoGUI::KeyboardKey::Control))
		{
			deselectAllItems();
		}

		/*
			Here we first do a broad phase and then a narrow phase, like collision detection in games 🤔
		*/

		FileBrowserItem* itemToScrollTo = 0;
		int32 leftIndex = 0;
		int32 rightIndex = 0;
		int32 topIndex = 0;
		int32 bottomIndex = 0;
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
							if (!itemToScrollTo || p_event.y < m_selectionRectangleAnchor.y && m_directoryItems[index]->getTop() < itemToScrollTo->getTop() || p_event.y > m_selectionRectangleAnchor.y && m_directoryItems[index]->getBottom() > itemToScrollTo->getBottom())
							{
								itemToScrollTo = m_directoryItems[index];
							}
							addSelectedItem(m_directoryItems[index], false);
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
							if (!itemToScrollTo || p_event.y < m_selectionRectangleAnchor.y && m_fileItems[index]->getTop() < itemToScrollTo->getTop() || p_event.y > m_selectionRectangleAnchor.y && m_fileItems[index]->getBottom() > itemToScrollTo->getBottom())
							{
								itemToScrollTo = m_fileItems[index];
							}
							addSelectedItem(m_fileItems[index], false);
						}
					}
				}
			}
		}
		if (itemToScrollTo)
		{
			if (itemToScrollTo->getAbsoluteTop() < getParent()->getAbsoluteTop())
			{
				getParent<ScrollContainer>()->setScrollPosition(0.f, itemToScrollTo->getTop() - MARGIN_VERTICAL);
				getParent()->invalidate();
			}
			else if (itemToScrollTo->getAbsoluteBottom() > getParent()->getAbsoluteBottom())
			{
				getParent<ScrollContainer>()->setScrollPosition(0.f, itemToScrollTo->getBottom() + MARGIN_VERTICAL - getParent()->getHeight());
				getParent()->invalidate();
			}
		}

		getGui()->invalidateRectangle(selectionRectangleBefore + getAbsoluteTopLeft());
		getGui()->invalidateRectangle(m_selectionRectangle + getAbsoluteTopLeft());
	}
}

//------------------------------

void FileBrowserItems::handleKeyboardKeyDown(AvoGUI::KeyboardEvent const& p_event)
{
	bool isControlDown = getGui()->getWindow()->getIsKeyDown(AvoGUI::KeyboardKey::Control);
	bool isShiftDown = getGui()->getWindow()->getIsKeyDown(AvoGUI::KeyboardKey::Shift);
	bool isAltDown = getGui()->getWindow()->getIsKeyDown(AvoGUI::KeyboardKey::Alt);
	switch (p_event.key)
	{
	case AvoGUI::KeyboardKey::Delete:
	{
		/*
			What happens here is
			1. Concatenate a string of the paths of all selected items.
			2. Remove all deleted items from their lists and update their indexes.
			The best way to do this seems to be to just zero their values first and then 
			remove them in a single sweep afterwards if the operation was not aborted.
		*/
		std::wstring paths;
		int32 firstErasedFileItemIndex = -1;
		int32 firstErasedDirectoryItemIndex = -1;
		for (uint32 a = 0; a < m_selectedItems.size(); a++)
		{
			paths += m_selectedItems[a]->getPath().wstring() + L'\0';

			uint32 itemIndex = m_selectedItems[a]->getItemIndex();
			if (m_selectedItems[a]->getIsFile())
			{
				if (firstErasedFileItemIndex == -1 || itemIndex < firstErasedFileItemIndex)
				{
					firstErasedFileItemIndex = itemIndex;
				}
				m_fileItems[itemIndex] = 0;
			}
			else
			{
				if (firstErasedDirectoryItemIndex == -1 || itemIndex < firstErasedDirectoryItemIndex)
				{
					firstErasedDirectoryItemIndex = itemIndex;
				}
				m_directoryItems[itemIndex] = 0;
			}
			removeChild(m_selectedItems[a]);
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
			/*
				Remove all deleted items in single sweep
			*/
			if (firstErasedFileItemIndex >= 0)
			{
				uint32 lastEmptyIndex = firstErasedFileItemIndex;
				for (uint32 a = firstErasedFileItemIndex + 1; a < m_fileItems.size(); a++)
				{
					if (m_fileItems[a])
					{
						m_fileItems[a]->setItemIndex(lastEmptyIndex);
						m_fileItems[lastEmptyIndex++] = m_fileItems[a];
						m_fileItems[a] = 0;
					}
				}
				m_fileItems.resize(lastEmptyIndex);
			}
			if (firstErasedDirectoryItemIndex >= 0)
			{
				uint32 lastEmptyIndex = firstErasedDirectoryItemIndex;
				for (uint32 a = firstErasedDirectoryItemIndex + 1; a < m_directoryItems.size(); a++)
				{
					if (m_directoryItems[a])
					{
						m_directoryItems[a]->setItemIndex(lastEmptyIndex);
						m_directoryItems[lastEmptyIndex++] = m_directoryItems[a];
						m_directoryItems[a] = 0;
					}
				}
				m_directoryItems.resize(lastEmptyIndex);
			}

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
		if (isControlDown)
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
		if (isAltDown)
		{
			m_fileBrowser->setWorkingDirectory(m_fileBrowser->getPath().parent_path().parent_path());
		}
		else if (m_lastSelectedItem)
		{
			uint32 index = getAbsoluteIndexFromItem(m_lastSelectedItem);
			if (getNumberOfChildren() && index)
			{
				FileBrowserItem* previousItem = getItemFromAbsoluteIndex(index - 1);
				if (isShiftDown)
				{
					selectItemsTo(previousItem, isControlDown);
				}
				else if (isControlDown)
				{
					addSelectedItem(previousItem);
				}
				else
				{
					setSelectedItem(previousItem);
				}
			}
		}
		else
		{
			setSelectedItem(m_directoryItems[0]);
		}
		break;
	}
	case AvoGUI::KeyboardKey::Right:
	{
		if (m_lastSelectedItem)
		{
			uint32 index = getAbsoluteIndexFromItem(m_lastSelectedItem);
			if (getNumberOfChildren() && index < m_fileItems.size() + m_directoryItems.size() - 1)
			{
				FileBrowserItem* nextItem = getItemFromAbsoluteIndex(index + 1);
				if (isShiftDown)
				{
					selectItemsTo(nextItem, isControlDown);
				}
				else if (isControlDown)
				{
					addSelectedItem(nextItem);
				}
				else
				{
					setSelectedItem(nextItem);
				}
			}
		}
		else
		{
			setSelectedItem(m_directoryItems[0]);
		}
		break;
	}
	case AvoGUI::KeyboardKey::Up:
	{
		if (!m_lastSelectedItem)
		{
			break;
		}
		uint32 index = getAbsoluteIndexFromItem(m_lastSelectedItem);
		for (int32 a = index - 1; a >= 0; a--)
		{
			FileBrowserItem* item = getItemFromAbsoluteIndex(a);
			if (item->getBottom() < m_lastSelectedItem->getTop() &&
				item->getRight() >= m_lastSelectedItem->getLeft() &&
				item->getLeft() <= m_lastSelectedItem->getRight())
			{
				if (isShiftDown)
				{
					selectItemsTo(item, isControlDown);
				}
				else if (isControlDown)
				{
					addSelectedItem(item);
				}
				else
				{
					setSelectedItem(item);
				}
				break;
			}
		}
		break;
	}
	case AvoGUI::KeyboardKey::Down:
	{
		if (!m_lastSelectedItem && getNumberOfChildren())
		{
			m_lastSelectedItem = m_directoryItems[0];
		}
		uint32 index = getAbsoluteIndexFromItem(m_lastSelectedItem);
		for (int32 a = index + 1; a < getNumberOfChildren(); a++)
		{
			FileBrowserItem* item = getItemFromAbsoluteIndex(a);
			if (item->getTop() > m_lastSelectedItem->getBottom() &&
				item->getRight() >= m_lastSelectedItem->getLeft() &&
				item->getLeft() <= m_lastSelectedItem->getRight())
			{
				if (isShiftDown)
				{
					selectItemsTo(item, isControlDown);
				}
				else if (isControlDown)
				{
					addSelectedItem(item);
				}
				else
				{
					setSelectedItem(item);
				}
				break;
			}
		}		
		break;
	}
	case AvoGUI::KeyboardKey::N:
	{
		if (isControlDown)
		{
			if (isShiftDown)
			{
				letUserAddDirectory();
			}
			else
			{
				letUserAddFile();
			}
		}
		break;
	}
	case AvoGUI::KeyboardKey::Enter:
	{
		if (m_selectedItems.size() == 1)
		{
			m_selectedItems[0]->open();
		}
		break;
	}
	case AvoGUI::KeyboardKey::V:
	{
		if (isControlDown)
		{
			AvoGUI::ClipboardData* clipboardData = getGui()->getWindow()->getClipboardData();
			dropItems(clipboardData);
			clipboardData->forget();
		}
		break;
	}
	case AvoGUI::KeyboardKey::C:
	{
		if (isControlDown && m_selectedItems.size())
		{
			std::vector<std::wstring> paths;
			paths.reserve(m_selectedItems.size());
			for (uint32 a = 0; a < m_selectedItems.size(); a++)
			{
				paths.push_back(m_selectedItems[a]->getPath().wstring());
			}
			getGui()->getWindow()->setClipboardFiles(paths);
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
	setSize(getParent()->getWidth() - PADDING, AvoGUI::max(m_fileBrowser->getHeight() - getParent()->getTop(), height + PADDING));

	tellIconLoadingThreadToLoadMoreIcons();
}