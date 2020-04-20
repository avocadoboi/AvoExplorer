#include "IconLoader.hpp"

#include "../Views/FileBrowserItem.hpp"

void IconLoader::loadIconForItem(FileBrowserItem* p_item)
{
	if (p_item->getHasLoadedIcon())
	{
		return;
	}

	auto context = getComponentById<AvoExplorer>(Ids::avoExplorer)->getDrawingContext();

	if (p_item->getIsFile())
	{
		if (p_item->getIsIconThumbnail())
		{
			IShellItem* item = 0;

			SHCreateItemFromParsingName(p_item->getPath().c_str(), 0, IID_PPV_ARGS(&item));

			ISharedBitmap* bitmap = 0;
			WTS_CACHEFLAGS flags;
			m_thumbnailCache->GetThumbnail(item, 100, WTS_EXTRACT, &bitmap, &flags, 0);

			if (bitmap)
			{
				HBITMAP bitmapHandle;
				bitmap->GetSharedBitmap(&bitmapHandle);

				AvoGUI::Image* newIcon = context->createImageFromHandle(bitmapHandle);
				p_item->setIcon(newIcon);
				newIcon->forget();

				DeleteObject(bitmapHandle);
				bitmap->Release();
			}

			item->Release();
		}
		else
		{
			SHFILEINFOW fileInfo = { 0 };
			SHGetFileInfoW(p_item->getPath().c_str(), 0, &fileInfo, sizeof(SHFILEINFOW), SHGFI_SYSICONINDEX);

			if (m_uniqueLoadedFileIcons.find(fileInfo.iIcon) == m_uniqueLoadedFileIcons.end())
			{
				HICON icon;
				m_windowsFileIconList->GetIcon(fileInfo.iIcon, 0, &icon);

				AvoGUI::Image* newIcon = context->createImageFromHandle(icon);
				p_item->setIcon(newIcon);
				m_uniqueLoadedFileIcons[fileInfo.iIcon] = newIcon;

				DestroyIcon(icon);
			}
			else
			{
				p_item->setIcon(m_uniqueLoadedFileIcons[fileInfo.iIcon]);
			}
		}
	}
	else
	{
		SHFILEINFOW fileInfo = { 0 };
		SHGetFileInfoW(p_item->getPath().c_str(), 0, &fileInfo, sizeof(SHFILEINFOW), SHGFI_SYSICONINDEX);

		if (m_uniqueLoadedDirectoryIcons.find(fileInfo.iIcon) == m_uniqueLoadedDirectoryIcons.end())
		{
			HICON icon;
			m_windowsDirectoryIconList->GetIcon(fileInfo.iIcon, 0, &icon);

			AvoGUI::Image* newIcon = context->createImageFromHandle(icon);
			p_item->setIcon(newIcon);
			m_uniqueLoadedDirectoryIcons[fileInfo.iIcon] = newIcon;

			DestroyIcon(icon);
		}
		else
		{
			p_item->setIcon(m_uniqueLoadedDirectoryIcons[fileInfo.iIcon]);
		}
	}
}

