#include "AvoExplorer.hpp"

#include "TopBar/TopBar.hpp"

//------------------------------

uint32 const WINDOW_WIDTH_MIN = 100;
uint32 const WINDOW_HEIGHT_MIN = 100;
uint32 const WINDOW_WIDTH_START = 750;
uint32 const WINDOW_HEIGHT_START = 600;

//------------------------------

AvoExplorer::AvoExplorer()
{
	create("AvoExplorer", WINDOW_WIDTH_START, WINDOW_HEIGHT_START, AvoGUI::WindowStyleFlags::Default);
}

void AvoExplorer::createContent()
{
	m_topBar = new TopBar(this);
}
