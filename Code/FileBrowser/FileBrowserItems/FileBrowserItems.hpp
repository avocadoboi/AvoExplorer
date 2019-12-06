#pragma once

#include "../FileBrowser.hpp"

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
	public AvoGUI::KeyboardListener
{
private:
	FileBrowser* m_fileBrowser;

	std::vector<FileBrowserItem*> m_fileItems;
	std::vector<FileBrowserItem*> m_directoryItems;
	std::vector<FileBrowserItem*> m_selectedItems;
	FileBrowserItem* m_lastSelectedItem;
	bool m_isMouseOnBackground;

	AvoGUI::Text* m_text_directories;
	AvoGUI::Text* m_text_files;
	AvoGUI::Text* m_text_directoryIsEmpty;

	AvoGUI::LinearGradient* m_fileNameEndGradient;

	//------------------------------
	// Icon loading

	IImageList2* m_windowsDirectoryIconList;
	IImageList2* m_windowsFileIconList;

	std::unordered_map<uint32, AvoGUI::Image*> m_uniqueLoadedDirectoryIcons;
	std::unordered_map<uint32, AvoGUI::Image*> m_uniqueLoadedFileIcons;

	std::condition_variable m_needsToLoadMoreIconsConditionVariable;
	std::mutex m_needsToLoadMoreIconsMutex;
	std::atomic<bool> m_needsToLoadMoreIcons;

	bool m_needsToChangeDirectory;

	std::mutex m_itemsMutex;
	std::atomic<bool> m_needsToExitIconLoadingThread;
	std::thread m_iconLoadingThread;
	void thread_loadIcons();

public:
	FileBrowserItems(ScrollContainer* p_parent, FileBrowser* p_fileBrowser) :
		View(p_parent), m_fileBrowser(p_fileBrowser),
		m_windowsDirectoryIconList(0), m_windowsFileIconList(0),
		m_lastSelectedItem(0),
		m_isMouseOnBackground(false),
		m_text_directories(0), m_text_files(0),
		m_fileNameEndGradient(0),
		m_needsToLoadMoreIcons(false), 
		m_needsToChangeDirectory(false),
		m_needsToExitIconLoadingThread(false)
	{
		enableMouseEvents();

		SHGetImageList(SHIL_LARGE, IID_IImageList2, (void**)&m_windowsDirectoryIconList);
		SHGetImageList(SHIL_JUMBO, IID_IImageList2, (void**)&m_windowsFileIconList);

		AvoGUI::DrawingContext* context = getGUI()->getDrawingContext();

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

	void setSelectedItem(FileBrowserItem* p_item);
	void addSelectedItem(FileBrowserItem* p_item);
	void removeSelectedItem(FileBrowserItem* p_item);
	void selectItemsTo(FileBrowserItem* p_item);

	//------------------------------

	void handleMouseBackgroundEnter(AvoGUI::MouseEvent const& p_event) override
	{
		m_isMouseOnBackground = true;
	}
	void handleMouseBackgroundLeave(AvoGUI::MouseEvent const& p_event) override
	{
		m_isMouseOnBackground = false;
	}
	void handleMouseDown(AvoGUI::MouseEvent const& p_event) override;

	//------------------------------

	void handleKeyboardKeyDown(AvoGUI::KeyboardEvent const& p_event) override;

	//------------------------------

	AvoGUI::LinearGradient* getFileNameEndGradient()
	{
		return m_fileNameEndGradient;
	}

	//------------------------------

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

	void draw(AvoGUI::DrawingContext* p_context)
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
};
