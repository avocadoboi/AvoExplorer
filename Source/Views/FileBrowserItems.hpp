#pragma once

#include "FileBrowser.hpp"
#include "InputDialogBox.hpp"
#include "ChoiceDialogBox.hpp"
#include "../Resources/Strings.hpp"
#include "../Resources/Ids.hpp"

//------------------------------

#include <unordered_map>
#include <deque>
#include <atomic>

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

	std::filesystem::path m_lastPath;

	std::mutex m_itemsMutex;
	std::atomic<bool> m_needsToLoadMoreIcons{ false };

	//------------------------------

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

	uint32 getNumberOfDirectoriesPerRow()
	{
		return AvoGUI::max(1u, (uint32)floor((getParent<View>()->getWidth() - PADDING + MARGIN_HORIZONTAL) / (MIN_DIRECTORY_WIDTH + MARGIN_HORIZONTAL)));
	}
	uint32 getNumberOfFilesPerRow()
	{
		return AvoGUI::max(1u, (uint32)floor((getParent<View>()->getWidth() - PADDING + MARGIN_HORIZONTAL) / (MIN_FILE_WIDTH + MARGIN_HORIZONTAL)));
	}

	void scrollToShowLastSelectedItem();

	FileBrowserItem* getItemFromAbsoluteIndex(uint32 p_index)
	{
		return p_index >= m_directoryItems.size() ? m_fileItems[p_index - m_directoryItems.size()] : m_directoryItems[p_index];
	}
	uint32 getAbsoluteIndexFromItem(FileBrowserItem* p_item);

	void insertNewItem(std::filesystem::path const& p_path, std::vector<FileBrowserItem*>& p_targetList);
	void insertNewItems(std::vector<std::filesystem::path>& p_pathsToInsert, std::vector<FileBrowserItem*>& p_targetList);

public:
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
		dialog->dialogBoxInputListeners += [this](auto input) {
			createDirectory(input);
		};
		dialog->detachFromThread();
	}
	void letUserAddFile()
	{
		InputDialogBox* dialog = new InputDialogBox(getGui(), Strings::newFileDialogTitle, Strings::newFileDialogMessage);
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

	void requestIconLoading();
	void handleBoundsChange(AvoGUI::Rectangle<float> const& p_previousBounds) override
	{
		if (p_previousBounds.top != m_bounds.top && getWidth() && getHeight())
		{
			requestIconLoading();
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
	}
	~FileBrowserItems()
	{
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

		if (m_text_directories)
		{
			m_text_directories->forget();
		}
		if (m_text_files)
		{
			m_text_files->forget();
		}
	}
};
