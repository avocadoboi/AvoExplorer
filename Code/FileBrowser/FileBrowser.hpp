#pragma once

#include "../AvoExplorer.hpp"
#include "../ScrollContainer/ScrollContainer.hpp"

#include "../DialogBox/ChoiceDialogBox.hpp"
#include "../ActionMenu/ActionMenu.hpp"

//------------------------------

#include <filesystem>

//------------------------------

class FileBrowserPathEditor;
class FileBrowserItems;
class FileBrowserItem;

class FileBrowser :
	public AvoGUI::View,
	public ChoiceDialogBoxListener,
	public AvoGUI::ButtonListener,
	public ActionMenuListener
{
private:
	AvoExplorer* m_avoExplorer;

	FileBrowserPathEditor* m_pathEditor = 0;
	AvoGUI::Button* m_button_changeView = 0;
	AvoGUI::Button* m_button_add = 0;
	ActionMenu* m_actionMenu_add = 0;

	FileBrowserItems* m_items = 0;

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

	void handleChoiceDialogBoxClose(ChoiceDialogBox* p_dialog) override
	{
		getGui()->getWindow()->enableUserInteraction();
	}
	void handleDialogBoxChoice(ChoiceDialogBox* p_dialog, std::string const& p_text) override;

	void setWorkingDirectory(std::filesystem::path p_path);
	std::filesystem::path const& getPath()
	{
		return m_path;
	}

	//------------------------------

	void handleSizeChange() override;
};
