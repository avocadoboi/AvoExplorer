#include "FileBrowserItem.hpp"

//------------------------------

float const FILE_WIDTH = 24			* 8.f;
float const FILE_HEIGHT = 16		* 8.f;

float const FOLDER_WIDTH = 24		* 8.f;
float const FOLDER_HEIGHT = 6		* 8.f;

//------------------------------

FileBrowserItem::FileBrowserItem(AvoGUI::View* p_parent, std::filesystem::path const& p_path, bool p_isFile) :
	View(p_parent), m_path(p_path), m_isFile(p_isFile)
{
	setCornerRadius(6.f);

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
	m_text_name = getGUI()->getDrawingContext()->createText(m_name.u8string().c_str(), 12.f);
	m_text_name->setBottomLeft(0.5f * (FOLDER_HEIGHT - m_text_name->getHeight()), getHeight() - 0.5f * (FOLDER_HEIGHT - m_text_name->getHeight()));
}
