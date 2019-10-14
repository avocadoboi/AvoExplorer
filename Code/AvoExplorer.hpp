#pragma once

#include <AvoGUI.hpp>
#include "../Resources/Colors.hpp"
#include "../Resources/Strings.hpp"

//------------------------------

class TopBar;

class AvoExplorer : 
	public AvoGUI::GUI
{
private:
	TopBar* m_topBar;

public:
	AvoExplorer();

	void createContent() override;

	void handleSizeChange() override;
};
