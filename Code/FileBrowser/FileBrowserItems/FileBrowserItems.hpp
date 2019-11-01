#pragma once

#include "../FileBrowser.hpp"

//------------------------------

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

	IThumbnailCache* m_thumbnailCache;
	IImageList2* m_iconList_large;
	IImageList2* m_iconList_jumbo;

	std::vector<FileBrowserItem*> m_fileItems;
	std::vector<FileBrowserItem*> m_directoryItems;
	FileBrowserItem* m_selectedItem;

	AvoGUI::Text* m_text_directories;
	AvoGUI::Text* m_text_files;

	bool m_isIconLoadingThreadRunning;
	bool m_needsToLoadMoreIcons;

public:
	FileBrowserItems(ScrollContainer* p_parent, FileBrowser* p_fileBrowser) :
		View(p_parent), m_fileBrowser(p_fileBrowser),
		m_thumbnailCache(0), m_iconList_large(0), m_iconList_jumbo(0),
		m_selectedItem(0),
		m_text_directories(0), m_text_files(0),
		m_isIconLoadingThreadRunning(false), m_needsToLoadMoreIcons(false)
	{
		CoCreateInstance(CLSID_LocalThumbnailCache, 0, CLSCTX_INPROC, IID_IThumbnailCache, (void**)&m_thumbnailCache);

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

	void tellIconLoadingThreadToLoadMoreIcons()
	{
		if (m_isIconLoadingThreadRunning)
		{
			m_needsToLoadMoreIcons = true;
		}
		else
		{
			m_needsToLoadMoreIcons = true;
			std::thread(&FileBrowserItems::loadIcons, this).detach();
		}
	}
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
