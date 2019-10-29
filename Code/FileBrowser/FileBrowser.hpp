#pragma once

#include "../AvoExplorer.hpp"
#include "../ScrollContainer/ScrollContainer.hpp"

//------------------------------

#include <filesystem>

//------------------------------

class FileBrowserPathEditor;
class FileBrowserItems;
class FileBrowserItem;

class FileBrowser :
	public AvoGUI::View
{
private:
	AvoExplorer* m_avoExplorer;

	FileBrowserPathEditor* m_pathEditor;
	AvoGUI::Button* m_button_changeView;
	AvoGUI::Button* m_button_add;

	FileBrowserItems* m_items;

public:
	FileBrowser(AvoExplorer* p_parent);
	~FileBrowser()
	{
		return;
	}

	//------------------------------

	void setWorkingDirectory(std::filesystem::path p_path);

	//------------------------------

	void handleSizeChange() override;
};
