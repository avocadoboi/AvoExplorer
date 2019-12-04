#pragma once

#include "../AvoExplorer.hpp"
#include "../ScrollContainer/ScrollContainer.hpp"

#undef DialogBox
#include "../DialogBox/DialogBox.hpp"

//------------------------------

#include <filesystem>

//------------------------------

class FileBrowserPathEditor;
class FileBrowserItems;
class FileBrowserItem;

class FileBrowser :
	public AvoGUI::View,
	public DialogBoxListener
{
private:
	AvoExplorer* m_avoExplorer;

	FileBrowserPathEditor* m_pathEditor;
	AvoGUI::Button* m_button_changeView;
	AvoGUI::Button* m_button_add;

	FileBrowserItems* m_items;

	DialogBox* m_dialog;

public:
	FileBrowser(AvoExplorer* p_parent);
	~FileBrowser()
	{
	}

	//------------------------------

	void handleDialogBoxClose()
	{
		getGUI()->getWindow()->enableUserInteraction();
	}
	void handleDialogBoxChoice(std::string const& p_text)
	{
		if (p_text == Strings::restart)
		{

		}
	}

	void setWorkingDirectory(std::filesystem::path p_path);

	//------------------------------

	void handleSizeChange() override;
};
