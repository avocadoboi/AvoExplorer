#include "FileBrowserItem.hpp"

//------------------------------

float constexpr FILE_HEIGHT = 16		* 8.f;
float constexpr FILE_NAME_PADDING = 1	* 8.f;

float constexpr BOOKMARK_HEIGHT = 4		* 8.f;

//------------------------------

FileBrowserItem::FileBrowserItem(AvoGUI::View* p_parent, std::filesystem::path const& p_path, bool p_isBookmark) :
	ContextView(p_parent), m_fileBrowserItems(0), m_bookmarks(0), m_itemIndex(0),
	m_icon(0), m_text_name(0), m_isFile(false), m_hasThumbnail(false), 
	m_isBookmark(p_isBookmark), m_positionAnimationTime(0.f), m_isDragged(false),
	m_hoverAnimationTime(0.f), m_hoverAnimationValue(0.f), m_isHovering(false), 
	m_isSelected(false)
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

	// I got a _Sharing_Violation on some files for some reason, probaly a bug because it doesn't happen when iterating the directory
	try
	{
		m_isFile = std::filesystem::is_regular_file(m_path);
	}
	catch (std::filesystem::filesystem_error)
	{
		m_isFile = true;
	}

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

	if (m_isFile)
	{
		m_name = p_path.filename().u8string();
	}
	else
	{
		if (p_path.root_path() == p_path)
		{
			m_name = p_path.root_name().u8string();
		}
		else if (p_path.has_filename())
		{
			m_name = p_path.filename().u8string();
		}
		else if (p_path.has_parent_path())
		{
			m_name = p_path.parent_path().filename().u8string();
		}
	}

	//------------------------------

	m_fileBrowserItems = getGui()->getViewById<FileBrowserItems>(Ids::fileBrowserItems);
	m_bookmarks = getGui()->getViewById<Bookmarks>(Ids::bookmarks);

	//------------------------------

	m_text_name = getGui()->getDrawingContext()->createText(m_name.c_str(), 11.f);
	m_text_name->setIsTopTrimmed(true);
	m_text_name->fitHeightToText();

	if (m_isBookmark)
	{
		setHeight(BOOKMARK_HEIGHT);
		setWidth(m_text_name->getRight() + 0.5f * (BOOKMARK_HEIGHT - m_text_name->getHeight()));
		m_fileBrowserItems->tellIconLoadingThreadToLoadIconForItem(this);
	}

	//m_contextMenuItems.push_back(ActionMenuItemData("Copy", "Ctrl + C"));
	//m_contextMenuItems.push_back(ActionMenuItemData("Cut", "Ctrl + X"));
	//m_contextMenuItems.push_back(ActionMenuItemData("Remove", "del"));
}

void FileBrowserItem::handleSizeChange()
{
	if (m_isFile && !m_isBookmark)
	{
		m_text_name->setBottomLeft(FILE_NAME_PADDING * 1.1f, getHeight() - FILE_NAME_PADDING);
	}
	else
	{
		m_text_name->setCenterY(getHeight() * 0.5f);
		m_text_name->setLeft(getHeight() - 1.f);
	}
}

//------------------------------

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

	AvoGUI::LinearGradient* gradient = m_fileBrowserItems->getFileNameEndGradient();
	if (!m_isBookmark && m_text_name->getRight() - getWidth() > gradient->getStartPositionX())
	{
		gradient->setOffsetX(getWidth());
		p_context->setGradient(gradient);
	}
	else
	{
		p_context->setColor(getThemeColor("on background"));
	}
	p_context->drawText(m_text_name);

	if (m_icon)
	{
		if (m_isFile && !m_isBookmark)
		{
			m_icon->setBounds(0.f, FILE_NAME_PADDING, getWidth(), m_text_name->getTop() - FILE_NAME_PADDING);
			m_icon->setBoundsSizing(AvoGUI::ImageBoundsSizing::Contain);
		}
		else
		{
			m_icon->setSize(getHeight()*0.65f);
			m_icon->setCenterY(getHeight() * 0.5f);
			m_icon->setLeft(m_icon->getTop());
		}

		p_context->drawImage(m_icon);
	}

	p_context->setColor(AvoGUI::Color(getThemeColor("on background"), m_hoverAnimationValue * 0.15f));
	p_context->fillRectangle(getSize());
}
