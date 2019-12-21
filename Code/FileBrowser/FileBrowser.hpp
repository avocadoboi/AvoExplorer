#pragma once

#include "../AvoExplorer.hpp"
#include "../ScrollContainer/ScrollContainer.hpp"

#undef DialogBox
#include "../DialogBox/DialogBox.hpp"
#include "../ActionMenu/ActionMenu.hpp"

//------------------------------

#include <filesystem>

//------------------------------

class FileBrowserPathEditor;
class FileBrowserItems;
class FileBrowserItem;

class FileBrowser :
	public AvoGUI::View,
	public DialogBoxListener,
	public AvoGUI::ButtonListener,
	public ActionMenuListener
{
private:
	AvoExplorer* m_avoExplorer;

	FileBrowserPathEditor* m_pathEditor;
	AvoGUI::Button* m_button_changeView;
	AvoGUI::Button* m_button_add;
	ActionMenu* m_actionMenu_add;

	FileBrowserItems* m_items;

	DialogBox* m_dialog;

	std::filesystem::path m_path;

public:
	FileBrowser(AvoExplorer* p_parent);
	~FileBrowser()
	{
	}

	//------------------------------

	void handleButtonClick(AvoGUI::Button* p_button) override
	{
		m_actionMenu_add->open(p_button->getCenter());
	}
	void handleActionMenuItemChoice(std::string const& p_action, uint32 p_index) override;

	//------------------------------

	void handleDialogBoxClose()
	{
		getGui()->getWindow()->enableUserInteraction();
	}
	void handleDialogBoxChoice(std::string const& p_text);

	void setWorkingDirectory(std::filesystem::path p_path);
	std::filesystem::path const& getPath()
	{
		return m_path;
	}

	//------------------------------

	void handleSizeChange() override;
};
