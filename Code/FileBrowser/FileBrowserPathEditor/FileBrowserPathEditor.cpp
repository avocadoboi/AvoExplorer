#include "FileBrowserPathEditor.hpp"

#include "../../utilities.hpp"
#include "../../TopBar/Bookmarks/Bookmarks.hpp"

//------------------------------

float constexpr HEIGHT = 6 * 8.f;

float constexpr DIRECTORY_BUTTON_PADDING_HORIZONTAL = 1		* 8.f;
float constexpr DIRECTORY_BUTTON_PADDING_VERTICAL = 1.f		* 8.f;

float constexpr PATH_EDITOR_DIRECTORY_SEPARATOR_MARGIN = -0.25f	* 8.f;
float constexpr PATH_EDITOR_DIRECTORY_SEPARATOR_SIZE = 2.f		* 8.f;

float constexpr PATH_EDITOR_BOOKMARK_ICON_SIZE = 2.75f	* 8.f;
float constexpr PATH_FADE_GRADIENT_WIDTH = 2.f			* 8.f;

//------------------------------
// class FileBrowserPathEditorDirectoryButton
//------------------------------

FileBrowserPathEditorDirectoryButton::FileBrowserPathEditorDirectoryButton(AvoGUI::View* p_parent, FileBrowserPathEditor* p_editor, std::filesystem::path const& p_path, std::string const& p_name) :
	View(p_parent), m_pathEditor(p_editor),
	m_path(p_path), m_text(0), m_ripple(0)
{
	m_text = getGui()->getDrawingContext()->createText(p_name.c_str(), 15.f);
	m_text->setIsTopTrimmed(false);
	m_text->fitSizeToText();

	setSize(m_text->getWidth() + 2.f*DIRECTORY_BUTTON_PADDING_HORIZONTAL, 15 + 2.f*DIRECTORY_BUTTON_PADDING_VERTICAL);

	m_text->setCenterX(getCenterX());
	m_text->setTop(DIRECTORY_BUTTON_PADDING_VERTICAL);

	m_ripple = new AvoGUI::Ripple(this, AvoGUI::Color(getThemeColor("on background"), 0.2f));

	setCornerRadius(5.f);
	enableMouseEvents();
	setCursor(AvoGUI::Cursor::Hand);
}

//------------------------------

void FileBrowserPathEditorDirectoryButton::handleMouseUp(AvoGUI::MouseEvent const& p_event)
{
	if (p_event.mouseButton == AvoGUI::MouseButton::Left && p_event.x > 0.f && p_event.x < getWidth() && p_event.y > 0.f && p_event.y < getHeight())
	{
		m_pathEditor->getFileBrowser()->setWorkingDirectory(m_path);
	}
}

//------------------------------
// class FileBrowserPathEditorPath
//------------------------------

FileBrowserPathEditorPath::FileBrowserPathEditorPath(FileBrowserPathEditor* p_parent) :
	View(p_parent), 
	m_pathEditor(p_parent), m_directorySeparatorIcon(0)
{
	enableMouseEvents();

	m_directorySeparatorIcon = getGui()->getDrawingContext()->createText(MaterialIcons::CHEVRON_RIGHT, PATH_EDITOR_DIRECTORY_SEPARATOR_SIZE);
	m_directorySeparatorIcon->setFontFamily(AvoGUI::FONT_FAMILY_MATERIAL_ICONS);
	m_directorySeparatorIcon->setIsTopTrimmed(true);
	m_directorySeparatorIcon->fitSizeToText();
	m_directorySeparatorIcon->setCenterY(HEIGHT * 0.5f);
}

void FileBrowserPathEditorPath::setWorkingDirectory(std::filesystem::path const& p_path)
{
	std::string pathString = p_path.u8string();

	getGui()->excludeAnimationThread();
	removeAllChildren();
	getGui()->includeAnimationThread();

	m_directoryButtons.clear();

	uint32 directoryStartIndex = 0;
	for (uint32 a = 0; a <= pathString.size(); a++)
	{
		if (a == pathString.size() || pathString[a] == '/' || pathString[a] == '\\')
		{
			FileBrowserPathEditorDirectoryButton* button = new FileBrowserPathEditorDirectoryButton(this, m_pathEditor, std::filesystem::u8path(pathString.substr(0, a) + '/'), pathString.substr(directoryStartIndex, a - directoryStartIndex));
			button->setCenterY(getHeight() * 0.5f);
			if (m_directoryButtons.size())
			{
				button->setLeft(m_directoryButtons.back()->getRight() + 2.f * PATH_EDITOR_DIRECTORY_SEPARATOR_MARGIN + m_directorySeparatorIcon->getWidth());
			}
			else
			{
				button->setLeft(button->getTop());
			}
			m_directoryButtons.push_back(button);

			if (a == pathString.size() - 1)
			{
				break;
			}
			directoryStartIndex = a + 1;
		}
	}
	setWidth(m_directoryButtons.back()->getRight());
}

void FileBrowserPathEditorPath::draw(AvoGUI::DrawingContext* p_context)
{
	p_context->setColor(AvoGUI::Color(getThemeColor("on background"), 0.7f));
	for (uint32 a = 0; a < m_directoryButtons.size() - 1; a++)
	{
		m_directorySeparatorIcon->setLeft(m_directoryButtons[a]->getRight() + PATH_EDITOR_DIRECTORY_SEPARATOR_MARGIN);
		p_context->drawText(m_directorySeparatorIcon);
	}
}

//------------------------------
// class FileBrowserPathEditor
//------------------------------

//
// Private
//

void FileBrowserPathEditor::updateBookmarkButtonIcon()
{
	m_bookmarkButton->setString(m_isBookmark ? MaterialIcons::BOOKMARK : MaterialIcons::BOOKMARK_BORDER);
	m_bookmarkButton->setSize(32.f);
	m_bookmarkButton->getText()->setFontFamily(AvoGUI::FONT_FAMILY_MATERIAL_ICONS);
	m_bookmarkButton->getText()->setFontSize(PATH_EDITOR_BOOKMARK_ICON_SIZE);
	m_bookmarkButton->getText()->fitSizeToText();
	m_bookmarkButton->getText()->setCenter(m_bookmarkButton->getSize() * 0.5f);
}

//
// Public
//

FileBrowserPathEditor::FileBrowserPathEditor(FileBrowser* p_parent) :
	View(p_parent), m_fileBrowser(p_parent), m_path(0), 
	m_pathFadeGradient(0), 
	m_bookmarkButton(0), m_isBookmark(false)
{
	enableMouseEvents();

	setThemeColor("background", Colors::fileBrowserPathEditorBackground);

	m_path = new FileBrowserPathEditorPath(this);
	m_path->setHeight(HEIGHT);

	m_bookmarkButton = new AvoGUI::Button(this, "", AvoGUI::Button::Emphasis::Low);
	m_bookmarkButton->setThemeColor("primary on background", getThemeColor("on background"));
	m_bookmarkButton->setCornerRadius(16.f);
	m_bookmarkButton->addButtonListener(this);
	updateBookmarkButtonIcon();

	m_pathFadeGradient = getGui()->getDrawingContext()->createLinearGradient(
		{ { getThemeColor("background"), 0.f }, { AvoGUI::Color(getThemeColor("background"), 0.f), 1.f } },
		0.f, 0.f, PATH_FADE_GRADIENT_WIDTH, 0
	);

	setCornerRadius(4.f);
	setElevation(3.f);
	setHeight(HEIGHT);
}

void FileBrowserPathEditor::handleSizeChange() 
{
	m_bookmarkButton->setRight(getWidth() - m_bookmarkButton->getTop());
	m_bookmarkButton->setCenterY(getHeight() * 0.5f);

	updateLayout();
}

//------------------------------

void FileBrowserPathEditor::handleButtonClick(AvoGUI::Button* p_button)
{
	if (m_isBookmark)
	{
		getGui()->getViewById<Bookmarks>(Ids::bookmarks)->removeBookmark(m_fileBrowser->getPath());
	}
	else
	{
		getGui()->getViewById<Bookmarks>(Ids::bookmarks)->addBookmark(m_fileBrowser->getPath());
	}

	m_isBookmark = !m_isBookmark;
	updateBookmarkButtonIcon();
}

void FileBrowserPathEditor::setWorkingDirectory(std::filesystem::path const& p_path)
{
	m_isBookmark = getGui()->getViewById<Bookmarks>(Ids::bookmarks)->getIsPathBookmarked(p_path);
	updateBookmarkButtonIcon();

	m_path->setWorkingDirectory(p_path);
	updateLayout();
}

//------------------------------

void FileBrowserPathEditor::draw(AvoGUI::DrawingContext* p_context)
{
	p_context->setColor(getThemeColor("background"));
	p_context->fillRectangle(getSize());
}
void FileBrowserPathEditor::drawOverlay(AvoGUI::DrawingContext* p_context)
{
	if (m_path->getLeft() < 0.f)
	{
		p_context->setGradient(m_pathFadeGradient);
		p_context->fillRectangle(0, m_path->getTop(), PATH_FADE_GRADIENT_WIDTH, m_path->getBottom());
	}

	p_context->setColor(Colors::fileBrowserPathEditorBorder);
	p_context->strokeGeometry(getClipGeometry(), 3.f);
}