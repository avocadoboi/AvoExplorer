#include "FileBrowser.hpp"

#include "FileBrowserPathEditor.hpp"
#include "FileBrowserItems.hpp"
#include "../utilities.hpp"

//------------------------------

FileBrowser::FileBrowser(AvoExplorer* p_parent) :
	View(p_parent, Ids::fileBrowser), 
	m_avoExplorer(p_parent)
{
	enableMouseEvents();

	m_pathEditor = new FileBrowserPathEditor(this);

	auto actionMenu = new ActionMenu(this);
	actionMenu->addAction(Strings::file, "Ctrl N");
	actionMenu->addAction(Strings::directory, "Ctrl Shift N");
	actionMenu->actionMenuItemChoiceListeners += [this](ActionMenuItem* p_item) {
		if (p_item->getAction() == Strings::directory)
		{
			m_items->letUserAddDirectory();
		}
		else
		{
			m_items->letUserAddFile();
		}
	};

	m_button_add->setSize(36.f);
	m_button_add->setCornerRadius(m_button_add->getWidth() * 0.5f);
	m_button_add->getText().setFontFamily(AvoGUI::FONT_FAMILY_MATERIAL_ICONS);
	m_button_add->getText().setFontSize(24.f);
	m_button_add->getText().fitSizeToText();
	m_button_add->getText().setCenter(m_button_add->getSize() * 0.5f);
	m_button_add->buttonClickListeners += [this, actionMenu](auto p_button) {
		actionMenu->open(p_button->getCenter());
	};

	ScrollContainer* scrollContainer = new ScrollContainer(this);
	scrollContainer->enableMouseEvents();
	m_items = new FileBrowserItems(scrollContainer, this);
	scrollContainer->setContentView(m_items);
}

//------------------------------

void FileBrowser::setWorkingDirectory(std::filesystem::path p_path)
{
	if (!std::filesystem::exists(p_path))
	{
		return;
	}

	p_path.make_preferred();
	if (p_path.u8string().back() != '\\')
	{
		p_path += '\\';
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
			ChoiceDialogBox* dialog = new ChoiceDialogBox(getGui(), Strings::openDirectoryAccessDeniedDialogTitle, Strings::openDirectoryAccessDeniedDialogMessage);
			dialog->addButton(Strings::restart, AvoGUI::Button::Emphasis::High);
			dialog->addButton(Strings::no, AvoGUI::Button::Emphasis::Medium);
			dialog->dialogBoxChoiceListeners += [this](std::string const& choice) {
				if (choice == Strings::restart)
				{
					getGui<AvoExplorer>()->restartWithElevatedPrivileges();
				}
			};
			dialog->run();
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

	m_items->getParent<View>()->setBounds(0, m_pathEditor->getBottom(), getWidth(), getHeight());
	m_items->updateLayout();
}
