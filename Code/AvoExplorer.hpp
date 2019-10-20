#pragma once

#include <AvoGUI.hpp>
#include "../Resources/Colors.hpp"
#include "../Resources/Strings.hpp"

//------------------------------

class TopBar;
class ActionMenu;
class ContextMenu;

class AvoExplorer : 
	public AvoGUI::GUI
{
private:
	TopBar* m_topBar;
	ContextMenu* m_contextMenu;

public:
	AvoExplorer();

	//------------------------------

	ContextMenu* getContextMenu()
	{
		return m_contextMenu;
	}

	//------------------------------

	void createContent() override;

	void handleSizeChange() override;
};
