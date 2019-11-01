#include "FileBrowserPathEditor.hpp"

#include "../../utilities.hpp"

//------------------------------

float constexpr HEIGHT = 6 * 8.f;

float constexpr DIRECTORY_BUTTON_PADDING_HORIZONTAL = 1		* 8.f;
float constexpr DIRECTORY_BUTTON_PADDING_VERTICAL = 1.f		* 8.f;

float constexpr PATH_EDITOR_DIRECTORY_SEPARATOR_MARGIN = 0.5	* 8.f;
float constexpr PATH_EDITOR_DIRECTORY_SEPARATOR_HEIGHT = 1.5f	* 8.f;

//------------------------------
// class FileBrowserPathEditorDirectoryButton
//------------------------------

FileBrowserPathEditorDirectoryButton::FileBrowserPathEditorDirectoryButton(FileBrowserPathEditor* p_parent, std::filesystem::path const& p_path, std::string const& p_name) :
	View(p_parent), m_pathEditor(p_parent),
	m_path(p_path), m_text(0), m_ripple(0)
{
	m_text = getGUI()->getDrawingContext()->createText(p_name.c_str(), 16.f);
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
// class FileBrowserPathEditor
//------------------------------

FileBrowserPathEditor::FileBrowserPathEditor(FileBrowser* p_parent) :
	View(p_parent), m_fileBrowser(p_parent),
	m_directorySeparatorIcon(0)
{
	setHeight(HEIGHT);
	setCornerRadius(4.f);
	setElevation(3.f);

	enableMouseEvents();

	m_directorySeparatorIcon = loadImageFromResource(RESOURCE_ICON_CHEVRON, getGUI()->getDrawingContext());
	m_directorySeparatorIcon->setBoundsSizing(AvoGUI::ImageBoundsSizing::Fill);
	m_directorySeparatorIcon->setSize(0.f, PATH_EDITOR_DIRECTORY_SEPARATOR_HEIGHT);
	m_directorySeparatorIcon->setWidth(m_directorySeparatorIcon->getInnerWidth());
	m_directorySeparatorIcon->setCenterY(getHeight() * 0.5f);
	m_directorySeparatorIcon->setOpacity(0.7f);
}

//------------------------------

void FileBrowserPathEditor::setWorkingDirectory(std::filesystem::path const& p_path)
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
			FileBrowserPathEditorDirectoryButton* button = new FileBrowserPathEditorDirectoryButton(this, std::filesystem::u8path(pathString.substr(0, a) + '/'), pathString.substr(directoryStartIndex, a - directoryStartIndex));
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

void FileBrowserPathEditor::draw(AvoGUI::DrawingContext* p_context)
{
	p_context->setColor(Colors::fileBrowserPathEditorBackground);
	p_context->fillRectangle(getSize());
	p_context->setColor(Colors::fileBrowserPathEditorBorder);
	p_context->strokeRoundedRectangle(getSize(), getCorners().topLeftSizeX, 3.f);

	for (uint32 a = 0; a < m_directoryButtons.size() - 1; a++)
	{
		m_directorySeparatorIcon->setLeft(m_directoryButtons[a]->getRight() + PATH_EDITOR_DIRECTORY_SEPARATOR_MARGIN);
		p_context->drawImage(m_directorySeparatorIcon);
	}
}