#pragma once

#include "../AvoExplorer.hpp"

//------------------------------

class FileBrowserPathEditor;

class FileBrowser :
	public AvoGUI::View
{
private:
	AvoExplorer* m_avoExplorer;

	FileBrowserPathEditor* m_pathEditor;
	AvoGUI::Button* m_button_changeView;
	AvoGUI::Button* m_button_add;

public:
	FileBrowser(AvoExplorer* p_parent);

	//------------------------------

	void handleSizeChange() override;
};
