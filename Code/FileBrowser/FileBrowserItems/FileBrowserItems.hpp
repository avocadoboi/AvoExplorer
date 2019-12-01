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
	public AvoGUI::View
{
private:
	FileBrowser* m_fileBrowser;

	std::vector<FileBrowserItem*> m_fileItems;
	std::vector<FileBrowserItem*> m_directoryItems;
	FileBrowserItem* m_selectedItem;

	AvoGUI::Text* m_text_directories;
	AvoGUI::Text* m_text_files;

	std::filesystem::path m_path;

	//------------------------------
	// Icon loading

	IImageList2* m_windowsDirectoryIconList;
	IImageList2* m_windowsFileIconList;

	std::unordered_map<uint32, AvoGUI::Image*> m_uniqueLoadedDirectoryIcons;
	std::unordered_map<uint32, AvoGUI::Image*> m_uniqueLoadedFileIcons;
	std::deque<FileBrowserItem*> m_filesToLoadIconFor;
	std::deque<FileBrowserItem*> m_directoriesToLoadIconFor;

	std::condition_variable m_needsToLoadMoreIconsConditionVariable;
	std::mutex m_needsToLoadMoreIconsMutex;
	std::atomic<bool> m_needsToLoadMoreIcons;

	void thread_loadIcons();

public:
	FileBrowserItems(ScrollContainer* p_parent, FileBrowser* p_fileBrowser) :
		View(p_parent), m_fileBrowser(p_fileBrowser),
		m_windowsDirectoryIconList(0), m_windowsFileIconList(0),
		m_selectedItem(0),
		m_text_directories(0), m_text_files(0),
		m_needsToLoadMoreIcons(false)
	{
		SHGetImageList(SHIL_LARGE, IID_IImageList2, (void**)&m_windowsDirectoryIconList);
		SHGetImageList(SHIL_JUMBO, IID_IImageList2, (void**)&m_windowsFileIconList);

		m_text_directories = getGUI()->getDrawingContext()->createText(Strings::directories, 16.f);
		m_text_files = getGUI()->getDrawingContext()->createText(Strings::files, 16.f);
		enableMouseEvents();

		std::thread(&FileBrowserItems::thread_loadIcons, this).detach();
	}
	~FileBrowserItems();

	//------------------------------

	void setSelectedItem(FileBrowserItem* p_item);
	FileBrowserItem* getSelectedItem()
	{
		return m_selectedItem;
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
