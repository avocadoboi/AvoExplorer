#pragma once

#include <AvoGUI.hpp>

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
};
