#include "FileBrowserItem.hpp"

//------------------------------

float constexpr FILE_WIDTH = 18			* 8.f;
float constexpr FILE_HEIGHT = 16		* 8.f;
float constexpr FILE_NAME_PADDING = 2	* 8.f;

float constexpr FOLDER_WIDTH = 24		* 8.f;
float constexpr FOLDER_HEIGHT = 6		* 8.f;
float constexpr FOLDER_ICON_WIDTH = 4	* 8.f;

//------------------------------

FileBrowserItem::FileBrowserItem(FileBrowserItems* p_parent, std::filesystem::path const& p_path, bool p_isFile) :
	View(p_parent), m_fileBrowserItems(p_parent),
	m_icon(0),
	m_path(p_path), m_text_name(0), m_isFile(p_isFile)
{
	setCornerRadius(6.f);
	enableMouseEvents();

	//------------------------------

	if (p_isFile)
	{
		m_name = p_path.filename();
		setSize(FILE_WIDTH, FILE_HEIGHT);
	}
	else
	{
		if (p_path.has_filename())
		{
			m_name = p_path.filename();
		}
		else if (p_path.has_parent_path())
		{
			m_name = p_path.parent_path().filename();
		}
		setSize(FOLDER_WIDTH, FOLDER_HEIGHT);
	}

	//------------------------------

	m_text_name = getGUI()->getDrawingContext()->createText(m_name.u8string().c_str(), 11.f);
	m_text_name->setIsTopTrimmed(true);
	m_text_name->fitHeightToText();
	if (p_isFile)
	{
		m_text_name->setBottomLeft(FILE_NAME_PADDING*1.1f, getHeight() - FILE_NAME_PADDING);
	}
	else
	{
		m_text_name->setCenterY(FOLDER_HEIGHT*0.5f);
		m_text_name->setLeft(FOLDER_HEIGHT);
	}
}

void FileBrowserItem::loadIcon(IImageList2* p_imageList, IThumbnailCache* p_thumbnailCache)
{
	if (m_icon)
	{
		return;
	}

	std::string extension = m_path.extension().u8string();

	AvoGUI::Image* newIcon = 0;

	std::wstring filepath = m_path.native();
	for (uint32 a = 0; a < filepath.size(); a++)
	{
		if (filepath[a] == '/')
		{
			filepath[a] = '\\';
		}
	}

	if (m_isFile && (m_path.extension() == u8".jpg" || m_path.extension() == u8".JPG" ||
		m_path.extension() == u8".png" || m_path.extension() == u8".PNG"))
	{
		IShellItem* item = 0;

		HRESULT result = SHCreateItemFromParsingName(filepath.c_str(), 0, IID_PPV_ARGS(&item));

		ISharedBitmap* bitmap = 0;
		WTS_CACHEFLAGS flags;
		p_thumbnailCache->GetThumbnail(item, 128, WTS_EXTRACT, &bitmap, &flags, 0);

		HBITMAP bitmapHandle;
		bitmap->GetSharedBitmap(&bitmapHandle);

		newIcon = getGUI()->getDrawingContext()->createImage(bitmapHandle);

		DeleteObject(bitmapHandle);
		bitmap->Release();
		item->Release();
	}
	else
	{
		SHFILEINFOW fileInfo = { 0 };
		DWORD_PTR result = SHGetFileInfoW(filepath.c_str(), 0, &fileInfo, sizeof(SHFILEINFOW), SHGFI_SYSICONINDEX);

		HICON icon;
		p_imageList->GetIcon(fileInfo.iIcon, 0, &icon);

		newIcon = getGUI()->getDrawingContext()->createImage(icon);
	}

	if (m_isFile)
	{
		m_text_name->setBottomLeft(FILE_NAME_PADDING * 1.1f, getHeight() - FILE_NAME_PADDING);

		if (newIcon)
		{
			newIcon->setBounds(0.f, FILE_NAME_PADDING, getWidth(), m_text_name->getTop() - FILE_NAME_PADDING);
			newIcon->setBoundsPositioning(0.5f, 0.5f);
			newIcon->setBoundsSizing(AvoGUI::ImageBoundsSizing::Contain);
		}
	}
	else
	{
		if (newIcon)
		{
			newIcon->setSize(FOLDER_ICON_WIDTH);
			newIcon->setBoundsPositioning(0.5f, 0.5f);
			newIcon->setCenterY(FOLDER_HEIGHT * 0.5f);
			newIcon->setLeft(newIcon->getTop());
		}
		m_text_name->setCenterY(FOLDER_HEIGHT * 0.5f);
	}
	m_icon = newIcon;

	// This method is called from an icon loading thread seperate from both the event thread and the animation/drawing thread.
	// The mutex used by those threads needs to be locked when invalidating here, since the resources used during invalidation 
	// in the GUI are also used in the animation thread when drawing.
	getGUI()->excludeAnimationThread();
	invalidate();
	getGUI()->includeAnimationThread();
}
