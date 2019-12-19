#include "FileBrowser.hpp"

#include "FileBrowserPathEditor/FileBrowserPathEditor.hpp"
#include "FileBrowserItems/FileBrowserItems.hpp"

//------------------------------

float constexpr FILE_BROWSER_PADDING_HORIZONTAL = 2			* 8.f;
float constexpr FILE_BROWSER_PADDING_TOP = 2				* 8.f;

//------------------------------

FileBrowser::FileBrowser(AvoExplorer* p_parent) :
	View(p_parent), m_avoExplorer(p_parent),
	m_pathEditor(0), m_button_changeView(0), m_button_add(0),
	m_items(0), m_dialog(0)
{
	enableMouseEvents();

	m_pathEditor = new FileBrowserPathEditor(this);

	ScrollContainer* scrollContainer = new ScrollContainer(this);
	scrollContainer->enableMouseEvents();
	m_items = new FileBrowserItems(scrollContainer, this);
	scrollContainer->setContentView(m_items);
}

//------------------------------

void FileBrowser::handleDialogBoxChoice(std::string const& p_text)
{
	if (p_text == Strings::restart)
	{
		wchar_t executablePath[MAX_PATH];
		GetModuleFileNameW(0, executablePath, MAX_PATH);
		ShellExecuteW(0, L"runas", executablePath, m_path.c_str(), 0, SW_SHOWNORMAL);

		getGui()->getWindow()->close();
	}
}

void FileBrowser::setWorkingDirectory(std::filesystem::path p_path)
{
	if (p_path.u8string().back() != '/' && p_path.u8string().back() != '\\')
	{
		p_path += '/';
	}

	try
	{
		std::filesystem::directory_iterator iterator(p_path);
		if (!iterator._At_end())
		{
			iterator++;
		}
	}
	catch (std::filesystem::filesystem_error error)
	{
		if (error.code().value() == 5)
		{
			m_dialog = new DialogBox(getGui(), Strings::accessDeniedDialogTitle, Strings::accessDeniedDialogText);
			m_dialog->addButton(Strings::restart, AvoGUI::Button::Emphasis::High);
			m_dialog->addButton(Strings::no, AvoGUI::Button::Emphasis::Medium);
			m_dialog->setDialogBoxListener(this);
			m_dialog->detachFromParent();
			getGui()->getWindow()->disableUserInteraction();
			return;
		}
	}

	m_path = p_path;
	m_pathEditor->setWorkingDirectory(p_path);
	m_items->setWorkingDirectory(p_path);
	invalidate();
}

//------------------------------

void FileBrowser::handleSizeChange()
{
	m_pathEditor->setTopLeft(FILE_BROWSER_PADDING_HORIZONTAL, FILE_BROWSER_PADDING_TOP);
	m_pathEditor->setWidth(getRight() - FILE_BROWSER_PADDING_HORIZONTAL * 2.f);

	m_items->getParent()->setBounds(0, m_pathEditor->getBottom(), getWidth(), getHeight());
	m_items->updateLayout();
}
