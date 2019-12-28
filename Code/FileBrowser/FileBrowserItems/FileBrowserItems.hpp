#pragma once

#include "../FileBrowser.hpp"
#include "../../DialogBox/InputDialogBox.hpp"

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

class FileBrowserItems :
	public AvoGUI::View,
	public AvoGUI::KeyboardListener,
	public InputDialogBoxListener,
	public ChoiceDialogBoxListener
{
private:
	FileBrowser* m_fileBrowser;

	std::vector<FileBrowserItem*> m_fileItems;
	std::vector<FileBrowserItem*> m_directoryItems;

	/*
		When selecting between two items (shift + click, shift + arrow key), m_firstSelectedItem is the start of the selection and m_lastSelectedItem is the end of the selection.
		m_lastSelectedItem is equal to m_firstSelectedItem when the user is just selecting individual items, and they are equal to the last selected item.
	*/
	std::vector<FileBrowserItem*> m_selectedItems;
	FileBrowserItem* m_firstSelectedItem = 0;
	FileBrowserItem* m_lastSelectedItem = 0;

	AvoGUI::Rectangle<float> m_selectionRectangle;
	AvoGUI::Point<float> m_selectionRectangleAnchor;
	bool m_isDraggingSelectionRectangle;
	
	bool m_isMouseOnBackground;

	AvoGUI::Text* m_text_directories = 0;
	AvoGUI::Text* m_text_files = 0;
	AvoGUI::Text* m_text_directoryIsEmpty = 0;

	AvoGUI::Geometry* m_fileGeometry = 0;
	AvoGUI::Geometry* m_directoryGeometry = 0;
	AvoGUI::LinearGradient* m_fileNameEndGradient = 0;

	//------------------------------
	// Icon loading

	IThumbnailCache* m_thumbnailCache = 0;
	IImageList2* m_windowsDirectoryIconList = 0;
	IImageList2* m_windowsFileIconList = 0;

	// Icon cache
	std::unordered_map<uint32, AvoGUI::Image*> m_uniqueLoadedDirectoryIcons;
	std::unordered_map<uint32, AvoGUI::Image*> m_uniqueLoadedFileIcons;
	std::vector<FileBrowserItem*> m_itemsToLoadIconFor;

	std::condition_variable m_needsToLoadMoreIconsConditionVariable;
	std::mutex m_needsToLoadMoreIconsMutex;
	std::atomic<bool> m_needsToLoadMoreIcons;

	std::filesystem::path m_lastPath;
	bool m_needsToChangeDirectory;

	std::mutex m_fileItemsMutex;
	std::mutex m_directoryItemsMutex;
	std::atomic<bool> m_needsToExitIconLoadingThread;
	std::thread m_iconLoadingThread;
	void thread_loadIcons();

	//------------------------------

	void loadIconForItem(FileBrowserItem* p_item);

	uint32 getNumberOfDirectoriesPerRow();
	uint32 getNumberOfFilesPerRow();

	void scrollToShowLastSelectedItem();

	FileBrowserItem* getItemFromAbsoluteIndex(uint32 p_index);
	uint32 getAbsoluteIndexFromItem(FileBrowserItem* p_item);

	void insertNewFileItem(std::filesystem::path const& p_path);
	void insertNewDirectoryItem(std::filesystem::path const& p_path);

public:
	FileBrowserItems(ScrollContainer* p_parent, FileBrowser* p_fileBrowser) :
		View(p_parent, Ids::fileBrowserItems), m_fileBrowser(p_fileBrowser),
		m_isDraggingSelectionRectangle(false),
		m_isMouseOnBackground(false),
		m_needsToLoadMoreIcons(false), 
		m_needsToChangeDirectory(false),
		m_needsToExitIconLoadingThread(false)
	{
		enableMouseEvents();
		getGui()->setKeyboardFocus(this);

		AvoGUI::DrawingContext* context = getGui()->getDrawingContext();

		m_text_directories = context->createText(Strings::directories, 16.f);
		m_text_files = context->createText(Strings::files, 16.f);

		m_fileNameEndGradient = context->createLinearGradient(
			{ 
				{ getThemeColor("on background"), 0.f }, 
				{ AvoGUI::Color(0.f, 0.f), 1.f } 
			}, 
			-25.f, 0.f, 0.f, 0.f
		);

		m_iconLoadingThread = std::thread(&FileBrowserItems::thread_loadIcons, this);
	}
	~FileBrowserItems();

	//------------------------------

	AvoGUI::Geometry* getFileGeometry()
	{
		return m_fileGeometry;
	}
	AvoGUI::Geometry* getDirectoryGeometry()
	{
		return m_directoryGeometry;
	}

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
		dialog->setInputDialogBoxListener(this);
		dialog->detachFromParent();
	}
	void letUserAddFile()
	{
		InputDialogBox* dialog = new InputDialogBox(getGui(), Strings::newFileDialogTitle, Strings::newFileDialogMessage);
		dialog->setId(Ids::createFileDialog);
		dialog->setInputDialogBoxListener(this);
		dialog->detachFromParent();
	}
	void handleDialogBoxInput(InputDialogBox* p_dialog, std::string const& p_input) override
	{
		switch (p_dialog->getId())
		{
		case Ids::createFileDialog:
		{
			createFile(p_input);
			break;
		}
		case Ids::createDirectoryDialog:
		{
			createDirectory(p_input);
			break;
		}
		}
	}
	void handleDialogBoxChoice(ChoiceDialogBox* p_dialog, std::string const& p_choice) override
	{
		switch (p_dialog->getId())
		{
		case Ids::newFileAccessDeniedDialog:
			if (p_choice == Strings::restart)
			{
				getGui<AvoExplorer>()->restartWithElevatedPrivileges();
			}
			break;
		case Ids::newFileAlreadyExistsDialog:
			if (p_choice == Strings::replace)
			{
				createFile(p_dialog->getDialogArgument(0), true);
			}
			break;
		case Ids::newDirectoryAlreadyExistsDialog:
			if (p_choice == Strings::replace)
			{
				createDirectory(p_dialog->getDialogArgument(0), true);
			}
			break;
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

	AvoGUI::LinearGradient* getFileNameEndGradient()
	{
		return m_fileNameEndGradient;
	}

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
	void drawOverlay(AvoGUI::DrawingContext* p_context) override
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
};
