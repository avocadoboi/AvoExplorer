#include "FileBrowserPathEditor.hpp"

#include "../../utilities.hpp"

//------------------------------

float constexpr HEIGHT = 6 * 8.f;

float constexpr DIRECTORY_BUTTON_PADDING_HORIZONTAL = 1		* 8.f;
float constexpr DIRECTORY_BUTTON_PADDING_VERTICAL = 1.f		* 8.f;

float constexpr PATH_EDITOR_DIRECTORY_SEPARATOR_MARGIN = 0.5	* 8.f;
float constexpr PATH_EDITOR_DIRECTORY_SEPARATOR_HEIGHT = 1.5f	* 8.f;

float constexpr PATH_EDITOR_PATH_PADDING_RIGHT = 80;

//------------------------------
// class FileBrowserPathEditorDirectoryButton
//------------------------------

FileBrowserPathEditorDirectoryButton::FileBrowserPathEditorDirectoryButton(AvoGUI::View* p_parent, FileBrowserPathEditor* p_editor, std::filesystem::path const& p_path, std::string const& p_name) :
	View(p_parent), m_pathEditor(p_editor),
	m_path(p_path), m_text(0), m_ripple(0)
{
	m_text = getGui()->getDrawingContext()->createText(p_name.c_str(), 16.f);
	m_text->setIsTopTrimmed(false);
	m_text->fitSizeToText();

	setSize(m_text->getWidth() + 2.f*DIRECTORY_BUTTON_PADDING_HORIZONTAL, 16.f + 2.f*DIRECTORY_BUTTON_PADDING_VERTICAL);

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
	if (p_event.x > 0.f && p_event.x < getWidth() && p_event.y > 0.f && p_event.y < getHeight())
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
	m_directorySeparatorIcon = loadImageFromResource(RESOURCE_ICON_CHEVRON, getGui()->getDrawingContext());
	m_directorySeparatorIcon->setBoundsSizing(AvoGUI::ImageBoundsSizing::Fill);
	m_directorySeparatorIcon->setSize(0.f, PATH_EDITOR_DIRECTORY_SEPARATOR_HEIGHT);
	m_directorySeparatorIcon->setWidth(m_directorySeparatorIcon->getInnerWidth());
	m_directorySeparatorIcon->setCenterY(HEIGHT * 0.5f);
	m_directorySeparatorIcon->setOpacity(0.7f);
}

void FileBrowserPathEditorPath::setWorkingDirectory(std::filesystem::path const& p_path)
{
	std::string pathString = p_path.u8string();

	removeAllChildren();
	//for (uint32 a = 0; a < m_directoryButtons.size(); a++)
	//{
	//	removeChild(m_directoryButtons[a]);
	//}
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
}

void FileBrowserPathEditorPath::draw(AvoGUI::DrawingContext* p_context)
{
	for (uint32 a = 0; a < m_directoryButtons.size() - 1; a++)
	{
		m_directorySeparatorIcon->setLeft(m_directoryButtons[a]->getRight() + PATH_EDITOR_DIRECTORY_SEPARATOR_MARGIN);
		p_context->drawImage(m_directorySeparatorIcon);
	}
}

//------------------------------
// class FileBrowserPathEditor
//------------------------------

FileBrowserPathEditor::FileBrowserPathEditor(FileBrowser* p_parent) :
	View(p_parent), m_fileBrowser(p_parent)
{
	enableMouseEvents();

	m_path = new FileBrowserPathEditorPath(this);
	m_path->setHeight(HEIGHT);

	m_bookmarkIcon_filled = loadImageFromResource(RESOURCE_ICON_BOOKMARK_FILLED, getGui()->getDrawingContext());
	m_bookmarkIcon_hollow = loadImageFromResource(RESOURCE_ICON_BOOKMARK_HOLLOW, getGui()->getDrawingContext());
	m_bookmarkButton = new AvoGUI::Button(this, "", AvoGUI::Button::Emphasis::Low);

	setCornerRadius(4.f);
	setElevation(3.f);
	setHeight(HEIGHT);
}

void FileBrowserPathEditor::handleSizeChange() 
{
	if (getWidth())
	{
		m_path->setWidth(getWidth() - PATH_EDITOR_PATH_PADDING_RIGHT);
	}
}

//------------------------------

void FileBrowserPathEditor::setWorkingDirectory(std::filesystem::path const& p_path)
{
	m_path->setWorkingDirectory(p_path);
}

void FileBrowserPathEditor::draw(AvoGUI::DrawingContext* p_context)
{
	p_context->setColor(Colors::fileBrowserPathEditorBackground);
	p_context->fillRectangle(getSize());
	p_context->setColor(Colors::fileBrowserPathEditorBorder);
	p_context->strokeRoundedRectangle(getSize(), getCorners().topLeftSizeX, 3.f);
}