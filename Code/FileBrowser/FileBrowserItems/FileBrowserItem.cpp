#include "FileBrowserItem.hpp"

//------------------------------

float constexpr FILE_WIDTH = 18			* 8.f;
float constexpr FILE_HEIGHT = 16		* 8.f;
float constexpr FILE_NAME_PADDING = 1	* 8.f;

float constexpr FOLDER_WIDTH = 24		* 8.f;
float constexpr FOLDER_HEIGHT = 6		* 8.f;
float constexpr FOLDER_ICON_WIDTH = 4	* 8.f;

float constexpr BOOKMARK_HEIGHT = 5		* 8.f;

//------------------------------

FileBrowserItem::FileBrowserItem(AvoGUI::View* p_parent, std::filesystem::path const& p_path, bool p_isBookmark) :
	View(p_parent), m_fileBrowserItems(0),
	m_icon(0), m_text_name(0), m_isFile(false), m_hasThumbnail(false), m_isBookmark(p_isBookmark),
	m_hoverAnimationTime(0), m_hoverAnimationValue(0), m_isHovering(false), 
	m_isSelected(false)
{
	m_isFile = !std::filesystem::is_directory(p_path);

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

	if (m_isBookmark)
	{
		setHeight(BOOKMARK_HEIGHT);
	}
	else
	{
		if (m_isFile)
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
	}

	//------------------------------

	m_fileBrowserItems = getGui()->getViewById<FileBrowserItems>(Ids::fileBrowserItems);
	m_bookmarks = getGui()->getViewById<Bookmarks>(Ids::bookmarks);

	//------------------------------

	m_text_name = getGui()->getDrawingContext()->createText(m_name.u8string().c_str(), 11.f);
	m_text_name->setIsTopTrimmed(true);
	m_text_name->fitHeightToText();
	if (m_isBookmark)
	{
		setWidth(m_text_name->getRight() + 0.5f * (BOOKMARK_HEIGHT - m_text_name->getHeight()));
	}
	else
	{
		if (m_isFile)
		{
			m_text_name->setBottomLeft(FILE_NAME_PADDING*1.1f, getHeight() - FILE_NAME_PADDING);
		}
		else
		{
			m_text_name->setCenterY(FOLDER_HEIGHT*0.5f);
			m_text_name->setLeft(FOLDER_HEIGHT);
		}
	}
}

void FileBrowserItem::setIcon(AvoGUI::Image* p_image)
{
	p_image->remember();
	m_icon = p_image;
	invalidate();
}

void FileBrowserItem::draw(AvoGUI::DrawingContext* p_context) 
{
	p_context->setColor(Colors::fileBrowserItemBackground);
	p_context->fillRectangle(getSize());

	if (m_isSelected)
	{
		p_context->setColor(AvoGUI::Color(getThemeColor("selection")));
		p_context->fillRectangle(getSize());
	}

	if (!m_isBookmark)
	{
		AvoGUI::LinearGradient* gradient = m_fileBrowserItems->getFileNameEndGradient();
		if (m_text_name->getRight() - getWidth() > gradient->getStartPositionX())
		{
			gradient->setOffsetX(getWidth());
			p_context->setGradient(gradient);
		}
		else
		{
			p_context->setColor(getThemeColor("on background"));
		}
		p_context->drawText(m_text_name);
	}

	if (m_icon)
	{
		if (m_isFile)
		{
			if (m_icon)
			{
				m_icon->setBounds(0.f, FILE_NAME_PADDING, getWidth(), m_text_name->getTop() - FILE_NAME_PADDING);
				m_icon->setBoundsPositioning(0.5f, 0.5f);
				m_icon->setBoundsSizing(AvoGUI::ImageBoundsSizing::Contain);
			}
		}
		else
		{
			if (m_icon)
			{
				m_icon->setSize(FOLDER_ICON_WIDTH);
				m_icon->setBoundsPositioning(0.5f, 0.5f);
				m_icon->setCenterY(FOLDER_HEIGHT * 0.5f);
				m_icon->setLeft(m_icon->getTop());
			}
		}

		p_context->drawImage(m_icon);
	}

	p_context->setColor(AvoGUI::Color(getThemeColor("on background"), m_hoverAnimationValue * 0.15f));
	p_context->fillRectangle(getSize());
}
