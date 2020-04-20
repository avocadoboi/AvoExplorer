#include "FileBrowserItems.hpp"

#include "FileBrowserItem.hpp"

#include <fstream>

//------------------------------

//
// Private
//

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

					AvoGUI::Image* newIcon = getGui()->getDrawingContext()->createImageFromHandle(bitmapHandle);
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

					AvoGUI::Image* newIcon = getDrawingContext()->createImageFromHandle(icon);
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

				AvoGUI::Image* newIcon = getDrawingContext()->createImageFromHandle(icon);
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

void FileBrowserItems::insertNewItem(std::filesystem::path const& p_path, std::vector<FileBrowserItem*>& p_targetList)
{
	std::lock_guard<std::mutex> lock(m_fileItemsMutex);

	FileBrowserItem* newItem = new FileBrowserItem(this, p_path, false);
	FileBrowserItem* lastItem = nullptr;
	for (uint32 a = 0; a < p_targetList.size(); a++)
	{
		if (lastItem)
		{
			std::swap(lastItem, p_targetList[a]);
			lastItem->incrementItemIndex();
		}
		else if (getIsPathStringLessThan(p_path, p_targetList[a]->getPath()))
		{
			lastItem = p_targetList[a];
			lastItem->incrementItemIndex();
			p_targetList[a] = newItem;
			newItem->setItemIndex(a);
		}
	}
	p_targetList.push_back(lastItem ? lastItem : newItem);
	updateLayout();
	setSelectedItem(newItem);
	getParent()->invalidate();
	tellIconLoadingThreadToLoadMoreIcons();
}
void FileBrowserItems::insertNewItems(std::vector<std::filesystem::path>& p_pathsToInsert, std::vector<FileBrowserItem*>& p_targetList)
{
	if (p_pathsToInsert.size())
	{
		if (p_pathsToInsert.size() > 1)
		{
			std::sort(p_pathsToInsert.begin(), p_pathsToInsert.end(), getIsPathStringLessThan);
		}
		p_targetList.resize(p_targetList.size() + p_pathsToInsert.size(), 0);
		int32 insertionIndex = p_targetList.size() - 1;
		for (int32 a = p_targetList.size() - p_pathsToInsert.size() - 1; a >= 0; a--)
		{
			while (p_pathsToInsert.size() && getIsPathStringLessThan(p_targetList[a]->getPath(), p_pathsToInsert.back()))
			{
				p_targetList[insertionIndex] = new FileBrowserItem(this, p_pathsToInsert.back(), false);
				p_targetList[insertionIndex]->setItemIndex(insertionIndex);
				p_pathsToInsert.pop_back();
				insertionIndex--;
			}
			p_targetList[insertionIndex] = p_targetList[a];
			p_targetList[insertionIndex]->setItemIndex(insertionIndex);
			insertionIndex--;
		}
		while (insertionIndex >= 0)
		{
			p_targetList[insertionIndex] = new FileBrowserItem(this, p_pathsToInsert.back(), false);
			p_targetList[insertionIndex]->setItemIndex(insertionIndex);
			p_pathsToInsert.pop_back();
			insertionIndex--;
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
			dialog->dialogBoxChoiceListeners += [this, dialog](std::string const& choice) {
				if (choice == Strings::replace)
				{
					createFile(dialog->getDialogArgument(0), true);
				}
			};
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
			insertNewItem(newItemPath, m_directoryItems);
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
			dialog->dialogBoxChoiceListeners += [this](std::string const& choice) {
				if (choice == Strings::restart)
				{
					getGui<AvoExplorer>()->restartWithElevatedPrivileges();
				}
			};
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
					removeChildView(m_directoryItems[a]);
					m_directoryItems.erase(m_directoryItems.begin() + a);
					break;
				}
			}
		}
		
		insertNewItem(path, m_fileItems);
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
			dialog->dialogBoxChoiceListeners += [this, dialog](std::string const& choice) {
				if (choice == Strings::replace)
				{
					createDirectory(dialog->getDialogArgument(0), true);
				}
			};
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
					removeChildView(m_fileItems[a]);
					m_fileItems.erase(m_fileItems.begin() + a);
					break;
				}
			}
		}

		insertNewItem(newItemPath, m_fileItems);
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
void FileBrowserItems::tryDroppingItems(AvoGUI::ClipboardData* p_data, std::filesystem::path const& p_targetDirectory, ItemDrop::Operation p_operation)
{
	std::vector<std::wstring> paths = p_data->getUtf16ItemNames();

	if (paths.empty()) // The user tried pasting or dropping text or something, just  i g n o r e  the dumb boy
	{
		return;
	}

	// Check for duplicates and concatenate the paths to prepare for the Windows API.

	std::wstring pathsString;
	pathsString.reserve(MAX_PATH * paths.size());
	
	uint32 numberOfPathsThatAlreadyExist = 0;
	bool isSingleDuplicatePathDirectory = false;
	for (uint32 a = 0; a < paths.size(); a++)
	{
		std::filesystem::path path = paths[a];
		bool isDirectory;
		if (!(isDirectory = std::filesystem::is_directory(path)) && !std::filesystem::is_regular_file(path))
		{
			continue;
		}
		path = p_targetDirectory / path.filename();

		if (std::filesystem::exists(path))
		{
			if (!numberOfPathsThatAlreadyExist)
			{
				isSingleDuplicatePathDirectory = isDirectory;
			}
			numberOfPathsThatAlreadyExist++;
		}
		pathsString += paths[a] + L'\0';
	}
	pathsString += L'\0';

	m_currentItemDrop.operation = p_operation;
	m_currentItemDrop.nameCollisionOption = ItemDrop::None;
	m_currentItemDrop.targetDirectory = p_targetDirectory;
	m_currentItemDrop.pathsString = std::move(pathsString);

	if (numberOfPathsThatAlreadyExist)
	{
		ChoiceDialogBox* dialog = nullptr;
		if (numberOfPathsThatAlreadyExist == 1)
		{
			dialog = new ChoiceDialogBox(getGui(), isSingleDuplicatePathDirectory ? Strings::directoryAlreadyExistsDialogTitle : Strings::fileAlreadyExistsDialogTitle, isSingleDuplicatePathDirectory ? Strings::directoryAlreadyExistsDialogMessage : Strings::fileAlreadyExistsDialogMessage);
			dialog->addButton(Strings::replace, AvoGUI::Button::Emphasis::High);
			dialog->addButton(Strings::addSuffixes, AvoGUI::Button::Emphasis::High);
			dialog->addButton(Strings::cancel, AvoGUI::Button::Emphasis::Medium);
			dialog->setWidth(500.f);
		}
		else
		{
			dialog = new ChoiceDialogBox(getGui(), Strings::directoriesOrFilesAlreadyExistDialogTitle, AvoGUI::createFormattedString(Strings::directoriesOrFilesAlreadyExistDialogMessage, numberOfPathsThatAlreadyExist));
			dialog->addButton(Strings::replace, AvoGUI::Button::Emphasis::High);
			dialog->addButton(Strings::addSuffixes, AvoGUI::Button::Emphasis::High);
			dialog->addButton(Strings::skipDuplicates, AvoGUI::Button::Emphasis::High);
			dialog->addButton(Strings::cancel, AvoGUI::Button::Emphasis::Medium);
			dialog->setWidth(600.f);
		}
		dialog->setId(Ids::directoriesOrFilesAlreadyExistDialog);
		dialog->dialogBoxChoiceListeners += [this](std::string const& choice) {
			if (choice == Strings::replace)
			{
				m_currentItemDrop.nameCollisionOption = ItemDrop::Replace;
			}
			else if (choice == Strings::addSuffixes)
			{
				m_currentItemDrop.nameCollisionOption = ItemDrop::Rename;
			}
			else if (choice == Strings::skipDuplicates)
			{
				m_currentItemDrop.nameCollisionOption = ItemDrop::Skip;
			}
			finishDroppingItems();
		};
		dialog->detachFromThread();
		return;
	}

	finishDroppingItems();
}

void FileBrowserItems::finishDroppingItems()
{
	/*
		This happens after the name collision check.
		If there was one or more name collisions, the user has now chosen an action to take.
	*/

	SHFILEOPSTRUCTW fileOperation = { 0 };
	fileOperation.fFlags = FOF_ALLOWUNDO | FOF_WANTMAPPINGHANDLE;
	if (m_currentItemDrop.nameCollisionOption == ItemDrop::Rename)
	{
		fileOperation.fFlags |= FOF_RENAMEONCOLLISION;
	}
	else if (m_currentItemDrop.nameCollisionOption == ItemDrop::Replace)
	{
		fileOperation.fFlags |= FOF_NOCONFIRMATION;
	}
	fileOperation.wFunc = m_currentItemDrop.operation == ItemDrop::Copy ? FO_COPY : FO_MOVE;
	fileOperation.pFrom = m_currentItemDrop.pathsString.data();

	std::wstring directoryPathString = m_currentItemDrop.targetDirectory.wstring() + L'\0';
	fileOperation.pTo = directoryPathString.data();

	SHFileOperationW(&fileOperation);

	if (!fileOperation.fAnyOperationsAborted && m_currentItemDrop.targetDirectory == m_fileBrowser->getPath())
	{
		deselectAllItems();

		std::vector<std::filesystem::path> targetDirectoryPaths;
		std::vector<std::filesystem::path> targetFilePaths;

		if (fileOperation.hNameMappings)
		{
			/*
				In case the user chose to let the OS rename the files because name collision occurred,
				we need to use the new names that the OS assigned them.
			*/
			SHNAMEMAPPINGW* nameMappings = *(SHNAMEMAPPINGW**)((UINT*)fileOperation.hNameMappings + 2);
			uint32 numberOfNameMappings = *(UINT*)fileOperation.hNameMappings;
			for (uint32 a = 0; a < numberOfNameMappings; a++)
			{
				std::filesystem::path path(nameMappings[a].pszNewPath, nameMappings[a].pszNewPath + nameMappings[a].cchNewPath);
				if (std::filesystem::is_directory(path))
				{
					targetDirectoryPaths.push_back(std::move(path));
				}
				else
				{
					targetFilePaths.push_back(std::move(path));
				}
			}
			SHFreeNameMappings(fileOperation.hNameMappings);
		}
		else
		{
			std::wstring& pathsString = m_currentItemDrop.pathsString;
			uint32 pathStartIndex = 0u;
			for (uint32 a = 0; a < pathsString.size(); a++)
			{
				if (pathsString[a] == '\0')
				{

				}
			}
		}

		insertNewItems(targetDirectoryPaths, m_directoryItems);
		insertNewItems(targetFilePaths, m_fileItems);

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
				removeChildView(m_selectedItems[a]);
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
				if (getNumberOfChildViews() && index)
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
				if (getNumberOfChildViews() && index < m_fileItems.size() + m_directoryItems.size() - 1)
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
			if (!m_lastSelectedItem && getNumberOfChildViews())
			{
				m_lastSelectedItem = m_directoryItems[0];
			}
			uint32 index = getAbsoluteIndexFromItem(m_lastSelectedItem);
			for (int32 a = index + 1; a < getNumberOfChildViews(); a++)
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
				
				for (uint32 a = 0; a < clipboardData->formats.size(); a++)
				{
					AvoGUI::DragDropFormatData data = clipboardData->getDataForFormat(a);
					std::cout << clipboardData->getFormatName(clipboardData->formats[a]) << ": " << data.buffer << '\n';
				}

				tryDroppingItems(clipboardData, m_fileBrowser->getPath(), ItemDrop::Copy);
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
			m_directoryGeometry = getGui()->getDrawingContext()->createCornerRectangleGeometry(directoryWidth, DIRECTORY_HEIGHT, m_directoryItems[0]->getCorners());
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
			m_fileGeometry = getGui()->getDrawingContext()->createCornerRectangleGeometry(fileWidth, fileWidth, m_fileItems[0]->getCorners());
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