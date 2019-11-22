#pragma once

#include "../FileBrowser.hpp"

//------------------------------

#include <hash_map>
#include <deque>
#include <atomic>

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

	IImageList2* m_iconList_large;
	IImageList2* m_iconList_jumbo;

	std::vector<FileBrowserItem*> m_fileItems;
	std::vector<FileBrowserItem*> m_directoryItems;
	FileBrowserItem* m_selectedItem;

	std::hash_map<uint32, AvoGUI::Image*> m_uniqueLoadedDirectoryIcons;
	std::hash_map<uint32, AvoGUI::Image*> m_uniqueLoadedFileIcons;
	std::deque<FileBrowserItem*> m_filesToLoadIconFor;
	std::deque<FileBrowserItem*> m_directoriesToLoadIconFor;

	AvoGUI::Text* m_text_directories;
	AvoGUI::Text* m_text_files;

	std::filesystem::path m_path;
	/*std::atomic<*/bool m_wantsToChangeDirectory;
	/*std::atomic<*/bool m_isIconLoadingThreadRunning;
	/*std::atomic<*/bool m_needsToLoadMoreIcons;

public:
	FileBrowserItems(ScrollContainer* p_parent, FileBrowser* p_fileBrowser) :
		View(p_parent), m_fileBrowser(p_fileBrowser),
		m_iconList_large(0), m_iconList_jumbo(0),
		m_selectedItem(0),
		m_text_directories(0), m_text_files(0),
		m_wantsToChangeDirectory(false), m_isIconLoadingThreadRunning(false), m_needsToLoadMoreIcons(false)
	{
		SHGetImageList(SHIL_LARGE, IID_IImageList2, (void**)&m_iconList_large);
		SHGetImageList(SHIL_JUMBO, IID_IImageList2, (void**)&m_iconList_jumbo);

		m_text_directories = getGUI()->getDrawingContext()->createText(Strings::directories, 16.f);
		m_text_files = getGUI()->getDrawingContext()->createText(Strings::files, 16.f);
		enableMouseEvents();
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
	void loadIcons();
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
