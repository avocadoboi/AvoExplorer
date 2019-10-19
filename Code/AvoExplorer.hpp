#pragma once

#include <AvoGUI.hpp>
#include "../Resources/Colors.hpp"
#include "../Resources/Strings.hpp"

//------------------------------

class TopBar;
class ActionMenu;

class AvoExplorer : 
	public AvoGUI::GUI
{
private:
	TopBar* m_topBar;
	ActionMenu* m_actionMenu;

public:
	AvoExplorer();

	void createContent() override;

	void handleSizeChange() override;

	//------------------------------

	void handleMouseDown(AvoGUI::MouseEvent const& p_event) override;
};
