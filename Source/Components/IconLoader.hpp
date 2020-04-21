#pragma once

#include "Worker.hpp"
#include "../Resources/Ids.hpp"

//------------------------------

#include <AvoGUI.hpp>

//#include <Windows.h>
#include <wincodec.h>
#include <CommCtrl.h>
#include <commoncontrols.h>
#include <thumbcache.h>
#undef max
#undef min

//------------------------------

class FileBrowserItem;

class IconLoader : public AvoGUI::Component
{
private:
	IWICImagingFactory2* m_windowsImagingFactory{ nullptr };

public:
	IWICImagingFactory2* getWindowsImagingFactory()
	{
		return m_windowsImagingFactory;
	}

private:
	IThumbnailCache* m_thumbnailCache{ nullptr };
	IImageList2* m_windowsDirectoryIconList{ nullptr };
	IImageList2* m_windowsFileIconList{ nullptr };

	// Icon cache
	std::unordered_map<uint32, AvoGUI::Image*> m_uniqueLoadedDirectoryIcons;
	std::unordered_map<uint32, AvoGUI::Image*> m_uniqueLoadedFileIcons;
	std::vector<FileBrowserItem*> m_itemsToLoadIconFor;

public:
	/*
		MUST be called from worker thread!
	*/
	void loadIconForItem(FileBrowserItem* p_item);
	void requestIconLoadingForItem(FileBrowserItem* p_item)
	{
		getComponentById<Worker>(Ids::worker)->requestCallback([this, p_item]() {
			loadIconForItem(p_item);
		});
	}

public:
	IconLoader(Component* p_parent) :
		Component(p_parent)
	{
		getComponentById<Worker>(Ids::worker)->requestCallback([this]() {
			CoInitialize(0);
			CoCreateInstance(CLSID_WICImagingFactory2, 0, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&m_windowsImagingFactory));
			CoCreateInstance(CLSID_LocalThumbnailCache, 0, CLSCTX_INPROC, IID_IThumbnailCache, (void**)&m_thumbnailCache);

			SHGetImageList(SHIL_LARGE, IID_IImageList2, (void**)&m_windowsDirectoryIconList);
			SHGetImageList(SHIL_JUMBO, IID_IImageList2, (void**)&m_windowsFileIconList);
		});
	}
	~IconLoader()
	{
		for (auto& icon : m_uniqueLoadedFileIcons)
		{
			icon.second->forget();
		}
		for (auto& icon : m_uniqueLoadedDirectoryIcons)
		{
			icon.second->forget();
		}
		if (m_thumbnailCache)
		{
			m_thumbnailCache->Release();
		}
		if (m_windowsDirectoryIconList)
		{
			m_windowsDirectoryIconList->Release();
		}
		if (m_windowsFileIconList)
		{
			m_windowsFileIconList->Release();
		}
		if (m_windowsImagingFactory)
		{
			m_windowsImagingFactory->Release();
		}
	}
};
