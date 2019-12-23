#include "FileBrowser.hpp"

#include "FileBrowserPathEditor/FileBrowserPathEditor.hpp"
#include "FileBrowserItems/FileBrowserItems.hpp"
#include "../utilities.hpp"

//------------------------------

float constexpr PADDING_HORIZONTAL = 2	* 8.f;
float constexpr PADDING_TOP = 2			* 8.f;

//------------------------------

FileBrowser::FileBrowser(AvoExplorer* p_parent) :
	View(p_parent), m_avoExplorer(p_parent)
{
	enableMouseEvents();

	m_pathEditor = new FileBrowserPathEditor(this);

	m_button_add = new AvoGUI::Button(this, MaterialIcons::ADD, AvoGUI::Button::Emphasis::High, true);
	m_button_add->setSize(36.f);
	m_button_add->setCornerRadius(m_button_add->getWidth() * 0.5f);
	m_button_add->getText()->setFontFamily(AvoGUI::FONT_FAMILY_MATERIAL_ICONS);
	m_button_add->getText()->setFontSize(24.f);
	m_button_add->getText()->fitSizeToText();
	m_button_add->getText()->setCenter(m_button_add->getSize() * 0.5f);
	m_button_add->addButtonListener(this);

	m_actionMenu_add = new ActionMenu(this);
	m_actionMenu_add->addAction(Strings::file, "Ctrl N");
	m_actionMenu_add->addAction(Strings::directory, "Ctrl Shift N");
	m_actionMenu_add->addActionMenuListener(this);

	ScrollContainer* scrollContainer = new ScrollContainer(this);
	scrollContainer->enableMouseEvents();
	m_items = new FileBrowserItems(scrollContainer, this);
	scrollContainer->setContentView(m_items);
}

//------------------------------

void FileBrowser::handleActionMenuItemChoice(std::string const& p_action, uint32 p_index)
{
	if (p_action == Strings::directory)
	{
		m_items->letUserAddDirectory();
	}
	else
	{
		m_items->letUserAddFile();
	}
}

//------------------------------

void FileBrowser::handleDialogBoxChoice(ChoiceDialogBox* p_dialog, std::string const& p_text)
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
			ChoiceDialogBox* dialog = new ChoiceDialogBox(getGui(), Strings::accessDeniedDialogTitle, Strings::accessDeniedDialogMessage);
			dialog->addButton(Strings::restart, AvoGUI::Button::Emphasis::High);
			dialog->addButton(Strings::no, AvoGUI::Button::Emphasis::Medium);
			dialog->setDialogBoxListener(this);
			dialog->setId(Ids::accessDeniedDialogBox);
			dialog->detachFromParent();
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
	float buttonMargin = (2.f * PADDING_TOP + m_pathEditor->getHeight() - m_button_add->getHeight()) * 0.5f;
	m_button_add->setTopRight(getWidth() - PADDING_HORIZONTAL, buttonMargin);

	m_pathEditor->setTopLeft(PADDING_HORIZONTAL, PADDING_TOP);
	m_pathEditor->setRight(m_button_add->getLeft() - PADDING_HORIZONTAL, false);

	m_items->getParent()->setBounds(0, m_pathEditor->getBottom(), getWidth(), getHeight());
	m_items->updateLayout();
}
