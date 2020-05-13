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
	std::vector<FileBrowserItem*> m_fileItems;
	std::vector<FileBrowserItem*> m_directoryItems;
	std::mutex m_itemsMutex;
	std::filesystem::path m_lastPath;

	uint32 getNumberOfDirectoriesPerRow()
	{
		return AvoGUI::max(1u, (uint32)floor((getParent<View>()->getWidth() - PADDING + MARGIN_HORIZONTAL) / (MIN_DIRECTORY_WIDTH + MARGIN_HORIZONTAL)));
	}
	uint32 getNumberOfFilesPerRow()
	{
		return AvoGUI::max(1u, (uint32)floor((getParent<View>()->getWidth() - PADDING + MARGIN_HORIZONTAL) / (MIN_FILE_WIDTH + MARGIN_HORIZONTAL)));
	}

	FileBrowserItem* getItemFromAbsoluteIndex(uint32 p_index)
	{
		return p_index >= m_directoryItems.size() ? m_fileItems[p_index - m_directoryItems.size()] : m_directoryItems[p_index];
	}
	uint32 getAbsoluteIndexFromItem(FileBrowserItem* p_item);

	void insertNewItem(std::filesystem::path const& p_path, std::vector<FileBrowserItem*>& p_targetList);
	void insertNewItems(std::vector<std::filesystem::path>& p_pathsToInsert, std::vector<FileBrowserItem*>& p_targetList);

	void createFile(std::string const& p_name, bool p_willReplaceExisting = false);
	void createDirectory(std::string const& p_name, bool p_willReplaceExisting = false);

	// Used to tell when to show "loading" text and when to show "no files" text.
	std::atomic<bool> m_isLoadingFiles{ false };

public:
	void setWorkingDirectory(std::filesystem::path const& p_path);

	void letUserAddDirectory()
	{
		InputDialogBox* dialog = new InputDialogBox(getGui(), Strings::newDirectoryDialogTitle, Strings::newDirectoryDialogMessage);
		dialog->dialogBoxInputListeners += [this](auto input) {
			createDirectory(input);
		};
		dialog->run();
	}
	void letUserAddFile()
	{
		InputDialogBox* dialog = new InputDialogBox(getGui(), Strings::newFileDialogTitle, Strings::newFileDialogMessage);
		dialog->dialogBoxInputListeners += [this](auto input) {
			createFile(input);
		};
		dialog->run();
	}

	//------------------------------

private:
	struct ItemDrop
	{
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
		std::wstring sourcePathsString;
		std::vector<std::filesystem::path> targetDirectoryPaths;
		std::vector<std::filesystem::path> targetFilePaths;
	} m_itemDrop;

	bool m_isDraggingDataOnBackground{ false };

	void tryDroppingItems(std::unique_ptr<AvoGUI::ClipboardData> const& p_data, std::filesystem::path const& p_targetDirectory, ItemDrop::Operation p_operation);
	void finishDroppingItems();

public:
	void dragSelectedItems();

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

private:
	/*
		When selecting between two items (shift + click or shift + arrow key), m_firstSelectedItem is the start of the selection and m_lastSelectedItem is the end of the selection.
		m_lastSelectedItem is equal to m_firstSelectedItem when the user is just selecting individual items, and they are equal to the last selected item.
	*/
	std::vector<FileBrowserItem*> m_selectedItems;
	FileBrowserItem* m_firstSelectedItem{ nullptr };
	FileBrowserItem* m_lastSelectedItem{ nullptr };

	void scrollToShowLastSelectedItem();

public:
	void setSelectedItem(FileBrowserItem* p_item, bool p_willScrollToShowItem = true);
	void addSelectedItem(FileBrowserItem* p_item, bool p_willScrollToShowItem = true);
	void selectItemsTo(FileBrowserItem* p_item, bool p_isAdditive = false, bool p_willScrollToShowItem = true);
	void removeSelectedItem(FileBrowserItem* p_item);
	void deselectAllItems();

	//------------------------------

private:
	bool m_isMouseOnBackground{ false };
	struct {
		AvoGUI::Rectangle<float> rectangle;
		AvoGUI::Point<float> anchor;
		bool isDragging{ false };
	} m_dragSelection;

public:
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

private:
	std::atomic<bool> m_needsToLoadMoreIcons{ false };
	void requestIconLoading();
public:
	void handleBoundsChange(AvoGUI::Rectangle<float> const& p_previousBounds) override
	{
		if (p_previousBounds.top != m_bounds.top && getWidth() && getHeight())
		{
			requestIconLoading();
		}
	}

	void updateLayout();

	//------------------------------

private:
	FileBrowser* m_fileBrowser;
public:
	FileBrowser* getFileBrowser()
	{
		return m_fileBrowser;
	}

	//------------------------------

private:
	AvoGUI::Geometry m_fileGeometry;
	AvoGUI::Geometry m_directoryGeometry;
	AvoGUI::LinearGradient m_fileNameEndGradient;
public:
	AvoGUI::Geometry const& getFileGeometry()
	{
		return m_fileGeometry;
	}
	AvoGUI::Geometry const& getDirectoryGeometry()
	{
		return m_directoryGeometry;
	}
	AvoGUI::LinearGradient& getFileNameEndGradient()
	{
		return m_fileNameEndGradient;
	}

private:
	AvoGUI::Text m_text_directories{ getDrawingContext()->createText(Strings::directories, 16.f) };
	AvoGUI::Text m_text_files{ getDrawingContext()->createText(Strings::files, 16.f) };
	AvoGUI::Text m_text_directoryIsEmpty{ getDrawingContext()->createText(Strings::thisDirectoryIsEmpty, 24.f) };
	AvoGUI::Text m_text_loading{ getDrawingContext()->createText(Strings::loading, 24.f) };

public:
	void draw(AvoGUI::DrawingContext* p_context) override
	{
		p_context->setColor(Colors::label);
		if (m_directoryItems.empty() && m_fileItems.empty())
		{
			if (m_isLoadingFiles)
			{
				p_context->drawText(m_text_loading);
			}
			else
			{
				p_context->drawText(m_text_directoryIsEmpty);
			}
		}
		else
		{
			if (!m_directoryItems.empty())
			{
				p_context->drawText(m_text_directories);
			}
			if (!m_fileItems.empty())
			{
				p_context->drawText(m_text_files);
			}
		}
	}
	void drawOverlay(AvoGUI::DrawingContext* p_context, AvoGUI::Rectangle<float> const& p_target) override
	{
		if (m_dragSelection.isDragging)
		{
			p_context->setColor(AvoGUI::Color(Colors::selection, 0.1f));
			p_context->fillRectangle(m_dragSelection.rectangle);
			p_context->setColor(AvoGUI::Color(Colors::selection, 0.9f));

			auto& rectangle = m_dragSelection.rectangle;
			if (rectangle.getWidth() > 1.f && rectangle.getHeight() > 1.f)
			{
				p_context->setLineDashStyle(AvoGUI::LineDashStyle::Dash);
				p_context->strokeRectangle(rectangle.left + 0.5f, rectangle.top + 0.5f, rectangle.right - 0.5f, rectangle.bottom - 0.5f, 1.f);
				p_context->setLineDashStyle(AvoGUI::LineDashStyle::Solid);
			}
		}
	}

	//------------------------------

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
};
