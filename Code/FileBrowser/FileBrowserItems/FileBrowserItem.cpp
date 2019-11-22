#include "FileBrowserItem.hpp"

//------------------------------

float constexpr FILE_WIDTH = 18			* 8.f;
float constexpr FILE_HEIGHT = 16		* 8.f;
float constexpr FILE_NAME_PADDING = 1	* 8.f;

float constexpr FOLDER_WIDTH = 24		* 8.f;
float constexpr FOLDER_HEIGHT = 6		* 8.f;
float constexpr FOLDER_ICON_WIDTH = 4	* 8.f;

//------------------------------

FileBrowserItem::FileBrowserItem(FileBrowserItems* p_parent, std::filesystem::path const& p_path, bool p_isFile) :
	View(p_parent), m_fileBrowserItems(p_parent),
	m_icon(0), m_text_name(0), m_isFile(p_isFile), m_hasThumbnail(false)
{
	std::wstring pathString = p_path.native();
	for (uint32 a = 0; a < pathString.size(); a++)
	{
		if (pathString[a] == L'/')
		{
			pathString[a] = L'\\';
		}
	}
	m_path = pathString;

	std::string extension = m_path.extension().u8string();
	m_hasThumbnail = 
		m_isFile && (
			extension == u8".jpg" || extension == u8".JPG" || 
			extension == u8".png" || extension == u8".PNG"
		);

	//------------------------------

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
