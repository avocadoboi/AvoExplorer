#pragma once

#include "AvoExplorer.hpp"
#include "ScrollContainer.hpp"

#include "ChoiceDialogBox.hpp"
#include "ActionMenu.hpp"

//------------------------------

#include <filesystem>

//------------------------------

class FileBrowserPathEditor;
class FileBrowserItems;
class FileBrowserItem;

class FileBrowser : public AvoGUI::View
{
public:
	static constexpr float PADDING_HORIZONTAL = 2 * 8.f;
	static constexpr float PADDING_TOP = 2 * 8.f;

private:
	AvoExplorer* m_avoExplorer;

	FileBrowserPathEditor* m_pathEditor{ nullptr };
	AvoGUI::Button* m_button_add{ new AvoGUI::Button(this, MaterialIcons::ADD, AvoGUI::Button::Emphasis::High, true) };

	FileBrowserItems* m_items{ nullptr };

	std::filesystem::path m_path;

public:
	FileBrowser(AvoExplorer* p_parent);

	//------------------------------

	void setWorkingDirectory(std::filesystem::path p_path);
	std::filesystem::path const& getPath()
	{
		return m_path;
	}

	//------------------------------

	void handleSizeChange() override;
};
