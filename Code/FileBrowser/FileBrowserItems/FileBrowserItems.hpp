#pragma once

#include "../FileBrowser.hpp"
#include "../../DialogBox/InputDialogBox.hpp"
#include "../../../Resources/Strings.hpp"

//------------------------------

#include <unordered_map>
#include <deque>
#include <atomic>
#include <condition_variable>

#include <CommCtrl.h>
#include <commoncontrols.h>
#include <thumbcache.h>
#undef max
#undef min

//------------------------------

class FileBrowserItem;

class FileBrowserItems : public AvoGUI::View
{
public:
	static constexpr float PADDING_TOP = 2          * 8.f;
	static constexpr float PADDING = 3              * 8.f;
	static constexpr float MARGIN_HORIZONTAL = 1    * 8.f;
	static constexpr float MARGIN_VERTICAL = 1      * 8.f;
	static constexpr float LABEL_MARGIN_TOP = 3     * 8.f;
	static constexpr float LABEL_MARGIN_BOTTOM = 2  * 8.f;
	static constexpr float MIN_FILE_WIDTH = 20      * 8.f;
	static constexpr float MIN_DIRECTORY_WIDTH = 20	* 8.f;
	static constexpr float DIRECTORY_HEIGHT = 6	    * 8.f;

private:
	FileBrowser* m_fileBrowser;

	std::vector<FileBrowserItem*> m_fileItems;
	std::vector<FileBrowserItem*> m_directoryItems;

	/*
		When selecting between two items (shift + click or shift + arrow key), m_firstSelectedItem is the start of the selection and m_lastSelectedItem is the end of the selection.
		m_lastSelectedItem is equal to m_firstSelectedItem when the user is just selecting individual items, and they are equal to the last selected item.
	*/
	std::vector<FileBrowserItem*> m_selectedItems;
	FileBrowserItem* m_firstSelectedItem{ nullptr };
	FileBrowserItem* m_lastSelectedItem{ nullptr };

	AvoGUI::Rectangle<float> m_selectionRectangle;
	AvoGUI::Point<float> m_selectionRectangleAnchor;
	bool m_isDraggingSelectionRectangle{ false };
	
	bool m_isMouseOnBackground{ false };
	bool m_isDraggingDataOnBackground{ false };

	//------------------------------
	// Icon loading

	IThumbnailCache* m_thumbnailCache{ nullptr };
	IImageList2* m_windowsDirectoryIconList{ nullptr };
	IImageList2* m_windowsFileIconList{ nullptr };

	// Icon cache
	std::unordered_map<uint32, AvoGUI::Image*> m_uniqueLoadedDirectoryIcons;
	std::unordered_map<uint32, AvoGUI::Image*> m_uniqueLoadedFileIcons;
	std::vector<FileBrowserItem*> m_itemsToLoadIconFor;

	std::condition_variable m_needsToLoadMoreIconsConditionVariable;
	std::mutex m_needsToLoadMoreIconsMutex;
	std::atomic<bool> m_needsToLoadMoreIcons;

	std::filesystem::path m_lastPath;
	bool m_needsToChangeDirectory{ false };

	std::mutex m_fileItemsMutex;
	std::mutex m_directoryItemsMutex;
	std::atomic<bool> m_needsToExitIconLoadingThread;
	std::thread m_iconLoadingThread;
	void thread_loadIcons()
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
				removeAllChildViews();
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

				if (getParent<View>()->getWidth() && getParent<View>()->getHeight())
				{
					updateLayout();
					getParent<View>()->invalidate();
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

	//------------------------------

	/*
		Stores information about an item drop/paste operation.
		In the case of file name collisions, we have dialog boxes which allow the user to choose what action to take.
		This requires us to exit the function while the user makes a choice because the parent window needs to respond to messages 
		for the dialog window to be responsive (due to the Windows API). I have yet to find a solution to this in AvoGUI, but 
		I kind of don't see it as a bug because it isn't really normal for event handlers to be asynchronous.
		dealing with the OS gets complicated sometimes smh
	*/
	class ItemDrop
	{
	public:
		enum NameCollisionOption
		{
			Replace,
			Skip,
			Rename,
			None
		} nameCollisionOption{ None };

		enum Operation
		{
			Copy,
			Move
		} operation{ Copy };

		std::filesystem::path targetDirectory;
		std::wstring pathsString;
	} m_currentItemDrop;

	//------------------------------

	void loadIconForItem(FileBrowserItem* p_item);

	uint32 getNumberOfDirectoriesPerRow()
	{
		return AvoGUI::max(1u, (uint32)floor((getParent()->getWidth() - PADDING + MARGIN_HORIZONTAL) / (MIN_DIRECTORY_WIDTH + MARGIN_HORIZONTAL)));
	}
	uint32 getNumberOfFilesPerRow()
	{
		return AvoGUI::max(1u, (uint32)floor((getParent()->getWidth() - PADDING + MARGIN_HORIZONTAL) / (MIN_FILE_WIDTH + MARGIN_HORIZONTAL)));
	}

	void scrollToShowLastSelectedItem()
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

	FileBrowserItem* getItemFromAbsoluteIndex(uint32 p_index)
	{
		return p_index >= m_directoryItems.size() ? m_fileItems[p_index - m_directoryItems.size()] : m_directoryItems[p_index];
	}
	uint32 getAbsoluteIndexFromItem(FileBrowserItem* p_item)
	{
		return p_item ? (p_item->getIsFile() ? p_item->getItemIndex() + m_directoryItems.size() : p_item->getItemIndex()) : 0;
	}

	void insertNewItem(std::filesystem::path const& p_path, std::vector<FileBrowserItem*>& p_targetList);
	void insertNewItems(std::vector<std::filesystem::path>& p_pathsToInsert, std::vector<FileBrowserItem*>& p_targetList);

	//------------------------------

	void setSelectedItem(FileBrowserItem* p_item, bool p_willScrollToShowItem = true);
	void addSelectedItem(FileBrowserItem* p_item, bool p_willScrollToShowItem = true);
	void selectItemsTo(FileBrowserItem* p_item, bool p_isAdditive = false, bool p_willScrollToShowItem = true);
	void removeSelectedItem(FileBrowserItem* p_item);
	void deselectAllItems();

	//------------------------------

	void createFile(std::string const& p_name, bool p_willReplaceExisting = false);
	void createDirectory(std::string const& p_name, bool p_willReplaceExisting = false);

	void letUserAddDirectory()
	{
		InputDialogBox* dialog = new InputDialogBox(getGui(), Strings::newDirectoryDialogTitle, Strings::newDirectoryDialogMessage);
		dialog->setId(Ids::createDirectoryDialog);
		dialog->dialogBoxInputListeners += [this](auto input) {
			createDirectory(input);
		};
		dialog->detachFromThread();
	}
	void letUserAddFile()
	{
		InputDialogBox* dialog = new InputDialogBox(getGui(), Strings::newFileDialogTitle, Strings::newFileDialogMessage);
		dialog->setId(Ids::createFileDialog);
		dialog->dialogBoxInputListeners += [this](auto input) {
			createFile(input);
		};
		dialog->detachFromThread();
	}

	//------------------------------

	void dragSelectedItems();
	void tryDroppingItems(AvoGUI::ClipboardData* p_data, std::filesystem::path const& p_targetDirectory, ItemDrop::Operation p_operation);
	void finishDroppingItems();

	AvoGUI::DragDropOperation getDragDropOperation(AvoGUI::DragDropEvent const& p_event) override
	{
		if (p_event.modifierKeys & AvoGUI::ModifierKeyFlags::Control)
		{
			return AvoGUI::DragDropOperation::Copy;
		}
		return AvoGUI::DragDropOperation::Move;
	}
	void handleDragDropBackgroundEnter(AvoGUI::DragDropEvent const& p_event) override
	{
		m_isDraggingDataOnBackground = true;
	}
	void handleDragDropBackgroundLeave(AvoGUI::DragDropEvent const& p_event) override
	{
		m_isDraggingDataOnBackground = false;
	}
	void handleDragDropFinish(AvoGUI::DragDropEvent const& p_event) override
	{
		if (m_isDraggingDataOnBackground)
		{
			tryDroppingItems(p_event.data, m_fileBrowser->getPath(), p_event.modifierKeys & AvoGUI::ModifierKeyFlags::Control ? ItemDrop::Copy : ItemDrop::Move);
		}
	}

	//------------------------------

	void handleMouseBackgroundEnter(AvoGUI::MouseEvent const& p_event) override
	{
		View::handleMouseBackgroundEnter(p_event);
		m_isMouseOnBackground = true;
	}
	void handleMouseBackgroundLeave(AvoGUI::MouseEvent const& p_event) override
	{
		m_isMouseOnBackground = false;
	}
	void handleMouseDown(AvoGUI::MouseEvent const& p_event) override;
	void handleMouseUp(AvoGUI::MouseEvent const& p_event) override;
	void handleMouseMove(AvoGUI::MouseEvent const& p_event) override;

	//------------------------------

	void handleKeyboardKeyDown(AvoGUI::KeyboardEvent const& p_event) override;

	//------------------------------

	void tellIconLoadingThreadToLoadIconForItem(FileBrowserItem* p_item);
	void tellIconLoadingThreadToLoadMoreIcons();
	void handleBoundsChange(AvoGUI::Rectangle<float> const& p_previousBounds) override
	{
		if (p_previousBounds.top != m_bounds.top && getWidth() && getHeight())
		{
			tellIconLoadingThreadToLoadMoreIcons();
		}
	}

	//------------------------------

	FileBrowser* getFileBrowser()
	{
		return m_fileBrowser;
	}

	void setWorkingDirectory(std::filesystem::path const& p_path);

	void updateLayout();

	//------------------------------

private:
	AvoGUI::Geometry* m_fileGeometry{ nullptr };
	AvoGUI::Geometry* m_directoryGeometry{ nullptr };
	AvoGUI::LinearGradient* m_fileNameEndGradient{ nullptr };
public:
	AvoGUI::Geometry* getFileGeometry()
	{
		return m_fileGeometry;
	}
	AvoGUI::Geometry* getDirectoryGeometry()
	{
		return m_directoryGeometry;
	}
	AvoGUI::LinearGradient* getFileNameEndGradient()
	{
		return m_fileNameEndGradient;
	}

private:
	AvoGUI::Text* m_text_directories{ getDrawingContext()->createText(Strings::directories, 16.f) };
	AvoGUI::Text* m_text_files{ getDrawingContext()->createText(Strings::files, 16.f) };
	AvoGUI::Text* m_text_directoryIsEmpty{ nullptr };

public:
	void draw(AvoGUI::DrawingContext* p_context) override
	{
		p_context->setColor(Colors::label);
		if (m_directoryItems.size())
		{
			p_context->drawText(m_text_directories);
		}
		if (m_fileItems.size())
		{
			p_context->drawText(m_text_files);
		}
	}
	void drawOverlay(AvoGUI::DrawingContext* p_context, AvoGUI::Rectangle<float> const& p_target) override
	{
		if (m_isDraggingSelectionRectangle)
		{
			p_context->setColor(AvoGUI::Color(Colors::selection, 0.1f));
			p_context->fillRectangle(m_selectionRectangle);
			p_context->setColor(AvoGUI::Color(Colors::selection, 0.9f));
			if (m_selectionRectangle.getWidth() > 1.f && m_selectionRectangle.getHeight() > 1.f)
			{
				p_context->setLineDashStyle(AvoGUI::LineDashStyle::Dash);
				p_context->strokeRectangle(m_selectionRectangle.left + 0.5f, m_selectionRectangle.top + 0.5f, m_selectionRectangle.right - 0.5f, m_selectionRectangle.bottom - 0.5f, 1.f);
				p_context->setLineDashStyle(AvoGUI::LineDashStyle::Solid);
			}
		}
	}

public:
	FileBrowserItems(ScrollContainer* p_parent, FileBrowser* p_fileBrowser) :
		View(p_parent, Ids::fileBrowserItems), m_fileBrowser(p_fileBrowser)
	{
		enableMouseEvents();
		enableDragDropEvents();
		getGui()->setKeyboardFocus(this);

		m_fileNameEndGradient = getDrawingContext()->createLinearGradient(
			{
				{ getThemeColor(ThemeColors::onBackground), 0.f },
				{ AvoGUI::Color(0.f, 0.f), 1.f }
			},
			-25.f, 0.f, 0.f, 0.f
		);

		m_iconLoadingThread = std::thread(&FileBrowserItems::thread_loadIcons, this);
	}
	~FileBrowserItems();
};
