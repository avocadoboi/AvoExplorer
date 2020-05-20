#include "FileBrowserItems.hpp"

#include "FileBrowserItem.hpp"
#include "../Components/Worker.hpp"
#include "../Components/IconLoader.hpp"

#include <fstream>

//------------------------------

uint32 FileBrowserItems::getAbsoluteIndexFromItem(FileBrowserItem* p_item)
{
	return p_item ? (p_item->getIsFile() ? p_item->getItemIndex() + m_directoryItems.size() : p_item->getItemIndex()) : 0;
}

void FileBrowserItems::insertNewItem(std::filesystem::path const& p_path, std::vector<FileBrowserItem*>& p_targetList)
{
	std::scoped_lock lock(m_itemsMutex);

	auto newItem = new FileBrowserItem(this, p_path, false);
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
	getParent<View>()->invalidate();
	requestIconLoading();
}
void FileBrowserItems::insertNewItems(std::vector<std::filesystem::path>& p_pathsToInsert, std::vector<FileBrowserItem*>& p_targetList)
{
	if (p_pathsToInsert.empty())
	{
		return;
	}
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

void FileBrowserItems::createFile(std::string const& p_name, bool p_willReplaceExisting)
{
	if (p_name == "")
	{
		return;
	}

	std::filesystem::path path = m_fileBrowser->getPath().native() + Avo::convertUtf8ToUtf16(p_name);
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
			dialog->addButton(Strings::replace, Avo::Button::Emphasis::High);
			dialog->addButton(Strings::no, Avo::Button::Emphasis::Medium);
			dialog->addDialogArgument(p_name);
			dialog->dialogBoxChoiceListeners += [this, dialog](std::string const& choice) {
				if (choice == Strings::replace)
				{
					createFile(dialog->getDialogArgument(0), true);
				}
			};
			dialog->run();
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
				newItemPath = m_fileBrowser->getPath().native() + Avo::convertUtf8ToUtf16(p_name.substr(0, a));
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
			dialog->addButton(Strings::ok, Avo::Button::Emphasis::High);
			dialog->run();
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
			dialog->addButton(Strings::restart, Avo::Button::Emphasis::High);
			dialog->addButton(Strings::no, Avo::Button::Emphasis::Medium);
			dialog->dialogBoxChoiceListeners += [this](std::string const& choice) {
				if (choice == Strings::restart)
				{
					getGui<AvoExplorer>()->restartWithElevatedPrivileges();
				}
			};
			dialog->run();
			break;
		}
		default:
		{
			ChoiceDialogBox* dialog = new ChoiceDialogBox(getGui(), Strings::newFileFailedDialogTitle, Strings::newFileFailedDialogMessage);
			dialog->addButton(Strings::ok, Avo::Button::Emphasis::High);
			dialog->run();
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
	std::filesystem::path path = m_fileBrowser->getPath().native() + Avo::convertUtf8ToUtf16(p_name);
	path.make_preferred();

	std::filesystem::path newItemPath;
	if (std::filesystem::path(p_name).has_parent_path())
	{
		for (uint32 a = 0; a < p_name.size(); a++)
		{
			if (p_name[a] == '/' || p_name[a] == '\\')
			{
				newItemPath = m_fileBrowser->getPath().native() + Avo::convertUtf8ToUtf16(p_name.substr(0, a));
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
			auto dialog = new ChoiceDialogBox(getGui(), Strings::newDirectoryOrFileAlreadyExistsDialogTitle, Strings::newDirectoryOrFileAlreadyExistsDialogMessage);
			dialog->addButton(Strings::replace, Avo::Button::Emphasis::High);
			dialog->addButton(Strings::no, Avo::Button::Emphasis::Medium);
			dialog->dialogBoxChoiceListeners += [this, &p_name, dialog](std::string const& choice) {
				if (choice == Strings::replace)
				{
					createDirectory(p_name, true);
				}
			};
			dialog->run();
			return;
		}
	}

	std::error_code errorCode;
	std::filesystem::create_directories(path, errorCode);
	if (errorCode)
	{
		auto dialog = new ChoiceDialogBox{ getGui(), Strings::newDirectoryFailedDialogTitle, Strings::newDirectoryFailedDialogMessage + errorCode.message() };
		dialog->addButton(Strings::ok, Avo::Button::Emphasis::High);
		dialog->run();
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

		insertNewItem(newItemPath, m_directoryItems);
	}
}

void FileBrowserItems::setWorkingDirectory(std::filesystem::path const& p_path)
{
	m_isLoadingFiles = true;

	// Loading the files in a big directory can take some time so we do it in the worker thread.
	getComponentById<Worker>(Ids::worker)->requestCallback([this]() {
		{
			auto lock(getGui()->createThreadLock());

			deselectAllItems();
			m_firstSelectedItem = 0;
			m_lastSelectedItem = 0;

			m_selectedItems.clear();
			m_directoryItems.clear();
			m_fileItems.clear();
			removeAllChildViews();
		}

		std::vector<std::filesystem::path> directoryPaths;
		directoryPaths.reserve(128);
		std::vector<std::filesystem::path> filePaths;
		filePaths.reserve(256);

		auto const& newPath = m_fileBrowser->getPath();
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

		if (!directoryPaths.empty() || !filePaths.empty())
		{
			// Sort files and directories separately, because they will be displayed separately.
			std::sort(filePaths.begin(), filePaths.end(), getIsPathStringLessThan);
			std::sort(directoryPaths.begin(), directoryPaths.end(), getIsPathStringLessThan);

			m_directoryItems.reserve(directoryPaths.size());
			m_fileItems.reserve(filePaths.size());

			auto lock(getGui()->createThreadLock());
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
		}
		auto lock(getGui()->createThreadLock());
		if (getParent<View>()->getWidth() && getParent<View>()->getHeight())
		{
			updateLayout();
			getParent<View>()->invalidate();
		}

		if (lastVisitedDirectoryItemIndex >= 0)
		{
			setSelectedItem(m_directoryItems[lastVisitedDirectoryItemIndex]);
		}

		m_lastPath = newPath;

		m_isLoadingFiles = false;
	});
}

void FileBrowserItems::tryDroppingItems(std::unique_ptr<Avo::ClipboardData> const& p_data, std::filesystem::path const& p_targetDirectory, 
	ItemDrop::Operation p_operation)
{
	auto sourcePaths = p_data->getUtf16ItemNames();

	if (sourcePaths.empty()) // The user tried pasting or dropping text or something, just  i g n o r e  the dumb boy
	{
		return;
	}

	// Check for duplicates and concatenate the paths to prepare for the Windows API.

	m_itemDrop.sourcePathsString.clear();
	m_itemDrop.sourcePathsString.reserve(MAX_PATH * sourcePaths.size());

	m_itemDrop.targetFilePaths.clear();
	m_itemDrop.targetFilePaths.reserve(sourcePaths.size());
	m_itemDrop.targetDirectoryPaths.clear();
	m_itemDrop.targetDirectoryPaths.reserve(sourcePaths.size());

	uint32 numberOfPathsThatAlreadyExist = 0;
	bool isSingleDuplicatePathDirectory = false;
	for (uint32 a = 0; a < sourcePaths.size(); a++)
	{
		auto path = std::filesystem::path(sourcePaths[a]);
		bool isDirectory;
		if (!(isDirectory = std::filesystem::is_directory(path)) && !std::filesystem::is_regular_file(path))
		{
			continue;
		}
		auto newPath = p_targetDirectory / path.filename();
		if (p_operation == ItemDrop::Move && path == newPath)
		{
			continue;
		}

		if (std::filesystem::exists(newPath))
		{
			if (!numberOfPathsThatAlreadyExist)
			{
				isSingleDuplicatePathDirectory = isDirectory;
			}
			numberOfPathsThatAlreadyExist++;
		}
		m_itemDrop.sourcePathsString += sourcePaths[a] + L'\0';
		if (isDirectory)
		{
			m_itemDrop.targetDirectoryPaths.push_back(newPath);
		}
		else
		{
			m_itemDrop.targetFilePaths.push_back(newPath);
		}
	}
	if (m_itemDrop.sourcePathsString.empty())
	{
		return;
	}

	// Second null terminator.
	m_itemDrop.sourcePathsString += L'\0';

	m_itemDrop.operation = p_operation;
	m_itemDrop.nameCollisionOption = ItemDrop::None;
	m_itemDrop.targetDirectory = p_targetDirectory;

	if (numberOfPathsThatAlreadyExist)
	{
		ChoiceDialogBox* dialog = nullptr;
		if (numberOfPathsThatAlreadyExist == 1)
		{
			dialog = new ChoiceDialogBox{ 
				getGui(), 
				isSingleDuplicatePathDirectory ? Strings::directoryAlreadyExistsDialogTitle : Strings::fileAlreadyExistsDialogTitle,
				isSingleDuplicatePathDirectory ? Strings::directoryAlreadyExistsDialogMessage : Strings::fileAlreadyExistsDialogMessage 
			};
			dialog->addButton(Strings::replace, Avo::Button::Emphasis::High);
			dialog->addButton(Strings::addSuffixes, Avo::Button::Emphasis::High);
			dialog->addButton(Strings::cancel, Avo::Button::Emphasis::Medium);
			dialog->setWidth(500.f);
		}
		else
		{
			dialog = new ChoiceDialogBox{
				getGui(), Strings::directoriesOrFilesAlreadyExistDialogTitle,
				Avo::createFormattedString(Strings::directoriesOrFilesAlreadyExistDialogMessage, numberOfPathsThatAlreadyExist)
			};
			dialog->addButton(Strings::replace, Avo::Button::Emphasis::High);
			dialog->addButton(Strings::addSuffixes, Avo::Button::Emphasis::High);
			dialog->addButton(Strings::skipDuplicates, Avo::Button::Emphasis::High);
			dialog->addButton(Strings::cancel, Avo::Button::Emphasis::Medium);
			dialog->setWidth(600.f);
		}
		dialog->dialogBoxChoiceListeners += [this](std::string const& choice) {
			if (choice == Strings::cancel)
			{
				return;
			}
			else if (choice == Strings::replace)
			{
				m_itemDrop.nameCollisionOption = ItemDrop::Replace;
			}
			else if (choice == Strings::addSuffixes)
			{
				m_itemDrop.nameCollisionOption = ItemDrop::Rename;
			}
			else if (choice == Strings::skipDuplicates)
			{
				m_itemDrop.nameCollisionOption = ItemDrop::Skip;
			}
			finishDroppingItems();
		};
		dialog->run();
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
	if (m_itemDrop.nameCollisionOption == ItemDrop::Rename)
	{
		fileOperation.fFlags |= FOF_RENAMEONCOLLISION;
	}
	else if (m_itemDrop.nameCollisionOption == ItemDrop::Replace)
	{
		fileOperation.fFlags |= FOF_NOCONFIRMATION;
	}
	else if (m_itemDrop.nameCollisionOption == ItemDrop::Skip)
	{
		//auto exists = [](std::filesystem::path& p_path) { 
		//	return std::filesystem::exists(p_path); 
		//};
		//auto& directories = m_itemDrop.targetDirectoryPaths;
		//auto& files = m_itemDrop.targetFilePaths;
		//directories.resize(std::remove_if(directories.begin(), directories.end(), exists) - directories.begin());
		//files.resize(std::remove_if(files.begin(), files.end(), exists) - files.begin());
	}
	fileOperation.wFunc = m_itemDrop.operation == ItemDrop::Copy ? FO_COPY : FO_MOVE;
	fileOperation.pFrom = m_itemDrop.sourcePathsString.data();

	auto directoryPathString = m_itemDrop.targetDirectory.wstring() + L'\0';
	fileOperation.pTo = directoryPathString.data();

	SHFileOperationW(&fileOperation);

	if (!fileOperation.fAnyOperationsAborted && m_itemDrop.nameCollisionOption != ItemDrop::Replace && m_itemDrop.targetDirectory == m_fileBrowser->getPath())
	{
		deselectAllItems();

		if (fileOperation.hNameMappings)
		{
			std::vector<std::filesystem::path> targetDirectoryPaths;
			std::vector<std::filesystem::path> targetFilePaths;
			/*
				In case the user chose to let the OS rename the files because name collision occurred,
				we need to use the new names that the OS assigned them.
			*/
			auto nameMappings = *(SHNAMEMAPPINGW**)((UINT*)fileOperation.hNameMappings + 2);
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

			insertNewItems(targetDirectoryPaths, m_directoryItems);
			insertNewItems(targetFilePaths, m_fileItems);
		}
		else
		{
			insertNewItems(m_itemDrop.targetDirectoryPaths, m_directoryItems);
			insertNewItems(m_itemDrop.targetFilePaths, m_fileItems);
		}

		updateLayout();
		getParent<View>()->invalidate();
		requestIconLoading();
	}
}

void FileBrowserItems::dragSelectedItems()
{
	std::vector<std::string> paths;
	paths.resize(m_selectedItems.size());
	for (uint32 a = 0; a < m_selectedItems.size(); a++)
	{
		paths[a] = m_selectedItems[a]->getPath().string();
	}

	getWindow()->dragAndDropFiles(paths, m_selectedItems[0]->getIcon(), m_selectedItems[0]->getIcon().getOriginalSize() * 0.5f);
}

void FileBrowserItems::scrollToShowLastSelectedItem()
{
	if (!m_lastSelectedItem)
	{
		return;
	}
	if (m_lastSelectedItem->getAbsoluteTop() < getParent<View>()->getAbsoluteTop())
	{
		getParent<ScrollContainer>()->setScrollPosition(0.f, m_lastSelectedItem->getTop() - MARGIN_VERTICAL);
		getParent<View>()->invalidate();
	}
	else if (m_lastSelectedItem->getAbsoluteBottom() > getParent<View>()->getAbsoluteBottom())
	{
		getParent<ScrollContainer>()->setScrollPosition(0.f, m_lastSelectedItem->getBottom() - getParent<View>()->getHeight() + MARGIN_VERTICAL);
		getParent<View>()->invalidate();
	}
}

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
	Avo::removeVectorElementWithoutKeepingOrder(m_selectedItems, p_item);
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

void FileBrowserItems::handleMouseDown(Avo::MouseEvent const& p_event)
{
	if (m_isMouseOnBackground && p_event.modifierKeys & Avo::ModifierKeyFlags::LeftMouse)
	{
		if (!(p_event.modifierKeys & Avo::ModifierKeyFlags::Control))
		{
			deselectAllItems();
		}
		m_dragSelection.isDragging = true;
		m_dragSelection.rectangle.set(p_event.x, p_event.y, p_event.x, p_event.y);
		m_dragSelection.anchor.set(p_event.x, p_event.y);
	}
	getGui()->setKeyboardFocus(this);
}
void FileBrowserItems::handleMouseUp(Avo::MouseEvent const& p_event)
{
	if (m_dragSelection.isDragging)
	{
		m_dragSelection.isDragging = false;
		getGui()->invalidateRectangle(m_dragSelection.rectangle + getAbsoluteTopLeft());
	}
}

void FileBrowserItems::handleMouseMove(Avo::MouseEvent const& p_event)
{
	if (m_dragSelection.isDragging && 
		Avo::Point<float>::getDistanceSquared(p_event.x, p_event.y, m_dragSelection.anchor.x, m_dragSelection.anchor.y) > 36.f)
	{
		AvoGUI::Rectangle<float> selectionRectangleBefore = m_dragSelection.rectangle;
		// To keep the correct relationship between left and right and top and bottom.
		if (p_event.x < m_dragSelection.anchor.x)
		{
			m_dragSelection.rectangle.left = p_event.x;
			m_dragSelection.rectangle.right = m_dragSelection.anchor.x;
		}
		else
		{
			m_dragSelection.rectangle.left = m_dragSelection.anchor.x;
			m_dragSelection.rectangle.right = p_event.x;
		}
		if (p_event.y < m_dragSelection.anchor.y)
		{
			m_dragSelection.rectangle.top = p_event.y;
			m_dragSelection.rectangle.bottom = m_dragSelection.anchor.y;
		}
		else
		{
			m_dragSelection.rectangle.top = m_dragSelection.anchor.y;
			m_dragSelection.rectangle.bottom = p_event.y;
		}

		if (!getGui()->getWindow()->getIsKeyDown(Avo::KeyboardKey::Control))
		{
			deselectAllItems();
		}

		/*
			Here we first do a broad phase and then a narrow phase, like collision detection in games 🤔
		*/

		FileBrowserItem* itemToScrollTo = nullptr;
		int32 leftIndex = 0;
		int32 rightIndex = 0;
		int32 topIndex = 0;
		int32 bottomIndex = 0;
		static auto selectItemsWithRectangle = [&](std::vector<FileBrowserItem*> p_items, uint32 p_numberOfItemsPerRow)
		{
			if (p_items.size())
			{
				topIndex = int32(m_dragSelection.rectangle.top - p_items[0]->getTop()) / int32(p_items[0]->getHeight() + MARGIN_VERTICAL);
				bottomIndex = int32(m_dragSelection.rectangle.bottom - p_items[0]->getTop()) / int32(p_items[0]->getHeight() + MARGIN_VERTICAL);
				if (topIndex <= int32(p_items.size() / p_numberOfItemsPerRow) && bottomIndex >= 0)
				{
					if (topIndex < 0)
					{
						topIndex = 0;
					}

					leftIndex = Avo::constrain(int32(m_dragSelection.rectangle.left - PADDING) / int32(p_items[0]->getWidth() + MARGIN_HORIZONTAL), 0, (int32)p_numberOfItemsPerRow - 1);

					rightIndex = Avo::constrain(int32(m_dragSelection.rectangle.right - PADDING) / int32(p_items[0]->getWidth() + MARGIN_HORIZONTAL), leftIndex, (int32)p_numberOfItemsPerRow - 1);

					for (uint32 y = topIndex; y <= bottomIndex; y++)
					{
						for (uint32 x = leftIndex; x <= rightIndex; x++)
						{
							uint32 index = y * p_numberOfItemsPerRow + x;
							if (index >= p_items.size())
							{
								y = bottomIndex + 1;
								break;
							}
							if (p_items[index]->getIsIntersecting(m_dragSelection.rectangle) && !p_items[index]->getIsSelected())
							{
								if (!itemToScrollTo || p_event.y < m_dragSelection.anchor.y && p_items[index]->getTop() < itemToScrollTo->getTop() ||
									p_event.y > m_dragSelection.anchor.y && p_items[index]->getBottom() > itemToScrollTo->getBottom())
								{
									itemToScrollTo = p_items[index];
								}
								addSelectedItem(p_items[index], false);
							}
						}
					}
				}
			}
		};
		selectItemsWithRectangle(m_directoryItems, getNumberOfDirectoriesPerRow());
		selectItemsWithRectangle(m_fileItems, getNumberOfFilesPerRow());

		if (itemToScrollTo)
		{
			if (itemToScrollTo->getAbsoluteTop() < getParent<View>()->getAbsoluteTop())
			{
				getParent<ScrollContainer>()->setScrollPosition(0.f, itemToScrollTo->getTop() - MARGIN_VERTICAL);
				getParent<View>()->invalidate();
			}
			else if (itemToScrollTo->getAbsoluteBottom() > getParent<View>()->getAbsoluteBottom())
			{
				getParent<ScrollContainer>()->setScrollPosition(0.f, itemToScrollTo->getBottom() + MARGIN_VERTICAL - getParent<View>()->getHeight());
				getParent<View>()->invalidate();
			}
		}

		getGui()->invalidateRectangle(selectionRectangleBefore + getAbsoluteTopLeft());
		getGui()->invalidateRectangle(m_dragSelection.rectangle + getAbsoluteTopLeft());
	}
}

void FileBrowserItems::handleKeyboardKeyDown(Avo::KeyboardEvent const& p_event)
{
	bool isControlDown = getWindow()->getIsKeyDown(Avo::KeyboardKey::Control);
	bool isShiftDown = getWindow()->getIsKeyDown(Avo::KeyboardKey::Shift);
	bool isAltDown = getWindow()->getIsKeyDown(Avo::KeyboardKey::Alt);
	switch (p_event.key)
	{
		case Avo::KeyboardKey::Delete:
		{
			/*
				What happens here is
				1. Concatenate a string of the paths of all selected items.
				2. Remove all deleted items from their lists and update their indices.
				The best way to do this seems to be to just zero their values first and then 
				remove them afterwards if the operation was not aborted.
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
					m_fileItems[itemIndex] = nullptr;
				}
				else
				{
					if (firstErasedDirectoryItemIndex == -1 || itemIndex < firstErasedDirectoryItemIndex)
					{
						firstErasedDirectoryItemIndex = itemIndex;
					}
					m_directoryItems[itemIndex] = nullptr;
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
					Remove all deleted items
				*/
				auto index = firstErasedFileItemIndex;
				static auto getIsItemNull = [&](FileBrowserItem* p_item) {
					if (p_item)
					{
						p_item->setItemIndex(index++);
					}
					return !p_item;
				};
				if (index >= 0)
				{
					m_fileItems.resize(std::remove_if(m_fileItems.begin() + firstErasedFileItemIndex, m_fileItems.end(), getIsItemNull) - m_fileItems.begin());
				}

				index = firstErasedDirectoryItemIndex;
				if (index >= 0)
				{
					m_directoryItems.resize(std::remove_if(m_directoryItems.begin() + firstErasedDirectoryItemIndex, m_directoryItems.end(), getIsItemNull) - m_directoryItems.begin());
				}

				m_firstSelectedItem = nullptr;
				m_lastSelectedItem = nullptr;
				m_selectedItems.clear();
				updateLayout();
				getParent<View>()->invalidate();
			}
			break;
		}
		case Avo::KeyboardKey::A:
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
		case Avo::KeyboardKey::Left:
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
			else if (!m_directoryItems.empty() || !m_fileItems.empty())
			{
				setSelectedItem(getItemFromAbsoluteIndex(0));
			}
			break;
		}
		case Avo::KeyboardKey::Right:
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
			else if (!m_directoryItems.empty() || !m_fileItems.empty())
			{
				setSelectedItem(getItemFromAbsoluteIndex(0));
			}
			break;
		}
		case Avo::KeyboardKey::Up:
		{
			if (m_lastSelectedItem)
			{
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
			}
			else if (!m_directoryItems.empty() || !m_fileItems.empty())
			{
				setSelectedItem(getItemFromAbsoluteIndex(0));
			}
			break;
		}
		case Avo::KeyboardKey::Down:
		{
			if (m_lastSelectedItem)
			{
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
			}
			else if (!m_directoryItems.empty() || !m_fileItems.empty())
			{
				setSelectedItem(getItemFromAbsoluteIndex(0));
			}
			break;
		}
		case Avo::KeyboardKey::N:
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
		case Avo::KeyboardKey::Enter:
		{
			if (m_selectedItems.size() == 1)
			{
				m_selectedItems[0]->open();
			}
			break;
		}
		case Avo::KeyboardKey::V:
		{
			if (isControlDown)
			{
				auto clipboardData = getWindow()->getClipboardData();
				tryDroppingItems(clipboardData, m_fileBrowser->getPath(), clipboardData->getAdditionalData() ? ItemDrop::Move : ItemDrop::Copy);
			}
			break;
		}
		case Avo::KeyboardKey::X:
		case Avo::KeyboardKey::C:
		{
			if (isControlDown && m_selectedItems.size())
			{
				bool isCut = p_event.key == Avo::KeyboardKey::X;
				if (m_selectedItems.size() == 1)
				{
					if (isCut)
					{
						m_selectedItems[0]->setIsCut(true);
					}
					getWindow()->setClipboardFile(m_selectedItems[0]->getPath().wstring(), isCut);
				}
				else
				{
					std::vector<std::wstring> paths;
					paths.reserve(m_selectedItems.size());
					for (uint32 a = 0; a < m_selectedItems.size(); a++)
					{
						if (isCut)
						{
							m_selectedItems[a]->setIsCut(true);
						}
						paths.push_back(m_selectedItems[a]->getPath().wstring());
					}
					getWindow()->setClipboardFiles(paths, isCut);
				}
			}
			break;
		}
	}
}

void FileBrowserItems::requestIconLoading()
{
	static auto loadIconsForItems = [this](std::vector<FileBrowserItem*>& items, Avo::Text const& itemsTitle, int32 numberOfColumns) {
		if (items.size())
		{
			std::scoped_lock lock(m_itemsMutex);
			int32 firstVisibleItemIndex = numberOfColumns * floor((-getTop() - itemsTitle.getBottom() - LABEL_MARGIN_BOTTOM) / (items[0]->getHeight() + MARGIN_VERTICAL));
			int32 lastVisibleItemIndex = numberOfColumns * floor(1 + (-getTop() + getParent<View>()->getHeight() - itemsTitle.getBottom() - LABEL_MARGIN_BOTTOM) / (items[0]->getHeight() + MARGIN_VERTICAL));
			for (int32 a = Avo::max(0, firstVisibleItemIndex); a < lastVisibleItemIndex && a < items.size(); a++)
			{
				if (m_needsToLoadMoreIcons)
				{
					return true;
				}
				getComponentById<IconLoader>(Ids::iconLoader)->loadIconForItem(items[a]);
			}
		}
		return false;
	};

	m_needsToLoadMoreIcons = true;
	getComponentById<Worker>(Ids::worker)->requestCallback([this]() {
		m_needsToLoadMoreIcons = false;
		if (loadIconsForItems(m_directoryItems, m_text_directories, getNumberOfDirectoriesPerRow()))
		{
			return;
		}
		if (loadIconsForItems(m_fileItems, m_text_files, getNumberOfFilesPerRow()))
		{
			return;
		}
	});
}

void FileBrowserItems::updateLayout()
{
	FileBrowserItem* lastItem = 0;

	uint32 directoriesPerRow = getNumberOfDirectoriesPerRow();
	uint32 filesPerRow = getNumberOfFilesPerRow();

	if (!m_directoryItems.empty())
	{
		float directoryWidth = (getParent<View>()->getWidth() - 2.f*PADDING - MARGIN_HORIZONTAL* (getNumberOfDirectoriesPerRow() - 1.f))/getNumberOfDirectoriesPerRow();
		if (std::abs(directoryWidth - m_directoryItems[0]->getWidth()) > 0.1f)
		{
			m_directoryGeometry = getGui()->getDrawingContext()->createCornerRectangleGeometry(directoryWidth, DIRECTORY_HEIGHT, m_directoryItems[0]->getCorners());
		}
		m_text_directories.setTopLeft(PADDING, PADDING_TOP);
		for (uint32 a = 0; a < m_directoryItems.size(); a++)
		{
			m_directoryItems[a]->setSize(directoryWidth, DIRECTORY_HEIGHT);
			m_directoryItems[a]->setTopLeft(
				PADDING + (directoryWidth + MARGIN_HORIZONTAL) * (a % directoriesPerRow), 
				m_text_directories.getBottom() + LABEL_MARGIN_BOTTOM + (DIRECTORY_HEIGHT + MARGIN_VERTICAL) * (a / directoriesPerRow)
			);
		}
	}

	if (!m_fileItems.empty())
	{
		float fileWidth = (getParent<View>()->getWidth() - 2.f * PADDING - MARGIN_HORIZONTAL * (getNumberOfFilesPerRow() - 1.f)) / getNumberOfFilesPerRow();
		if (std::abs(fileWidth - m_fileItems[0]->getWidth()) > 0.1f)
		{
			m_fileGeometry = getDrawingContext()->createCornerRectangleGeometry(fileWidth, fileWidth, m_fileItems[0]->getCorners());
		}
		m_text_files.setTopLeft(PADDING, m_directoryItems.size() ? m_directoryItems.back()->getBottom() + LABEL_MARGIN_TOP : PADDING_TOP);
		for (uint32 a = 0; a < m_fileItems.size(); a++)
		{
			m_fileItems[a]->setSize(fileWidth);
			m_fileItems[a]->setTopLeft(
				PADDING + (fileWidth + MARGIN_HORIZONTAL) * (a % filesPerRow),
				m_text_files.getBottom() + LABEL_MARGIN_BOTTOM + (fileWidth + MARGIN_VERTICAL) * (a / filesPerRow)
			);
		}
	}

	if (m_fileItems.empty() && m_directoryItems.empty())
	{
		setSize(getParent<View>()->getSize());
		m_text_directoryIsEmpty.setCenter(getSize() * 0.5f);
		m_text_loading.setCenter(getSize() * 0.5f);
	}
	else
	{
		float height = 0.f;
		if (!m_fileItems.empty())
		{
			height = m_fileItems.back()->getBottom();
		}
		else
		{
			height = m_directoryItems.back()->getBottom();
		}
		setSize(getParent<View>()->getWidth() - PADDING, Avo::max(m_fileBrowser->getHeight() - getParent<View>()->getTop(), height + PADDING));
	}

	requestIconLoading();
}