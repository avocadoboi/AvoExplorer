#include "FileBrowserItem.hpp"

//------------------------------

#include <Windows.h>
#include <CommCtrl.h>
#include <commoncontrols.h>

//------------------------------

float constexpr FILE_WIDTH = 18			* 8.f;
float constexpr FILE_HEIGHT = 16		* 8.f;
float constexpr FILE_NAME_PADDING = 2	* 8.f;

float constexpr FOLDER_WIDTH = 24		* 8.f;
float constexpr FOLDER_HEIGHT = 6		* 8.f;

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
	// Get icon

	if (p_isFile && (p_path.extension() == ".jpg" || p_path.extension() == ".png"))
	{
		m_icon = getGUI()->getDrawingContext()->createImage(p_path.u8string().c_str());
	}
	else
	{
		std::wstring filepath = p_path.native();
		for (uint32 a = 0; a < filepath.size(); a++)
		{
			if (filepath[a] == '/')
			{
				filepath[a] = '\\';
			}
		}
		SHFILEINFOW fileInfo = { 0 };
		DWORD_PTR result = SHGetFileInfoW(filepath.c_str(), 0, &fileInfo, sizeof(SHFILEINFOW), SHGFI_SYSICONINDEX);

		IImageList2* imageList = 0;
		SHGetImageList(p_isFile ? SHIL_JUMBO : SHIL_LARGE, IID_IImageList2, (void**)&imageList);

		HICON icon;
		imageList->GetIcon(fileInfo.iIcon, 0, &icon);

		m_icon = getGUI()->getDrawingContext()->createImage(icon);
	}

	//------------------------------

	m_text_name = getGUI()->getDrawingContext()->createText(m_name.u8string().c_str(), 11.f);
	if (p_isFile)
	{
		m_text_name->setBottomLeft(FILE_NAME_PADDING*1.1f, getHeight() - FILE_NAME_PADDING);

		m_icon->setBounds(0.f, FILE_NAME_PADDING, getWidth(), m_text_name->getTop() - FILE_NAME_PADDING);
		m_icon->setBoundsPositioning(0.5f, 0.5f);
		m_icon->setBoundsSizing(AvoGUI::ImageBoundsSizing::Contain);
	}
	else
	{
		m_icon->setSize(32);
		m_icon->setCenterY(FOLDER_HEIGHT * 0.5f);
		m_icon->setLeft(m_icon->getTop());
		m_text_name->setLeft(m_icon->getRight() + m_icon->getLeft());
		m_text_name->setCenterY(FOLDER_HEIGHT*0.5f);
	}
}
