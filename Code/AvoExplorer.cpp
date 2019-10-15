#include "AvoExplorer.hpp"

#include "TopBar/TopBar.hpp"
#include "FileExplorer/FileExplorer.hpp"

#include "ActionMenu/ActionMenu.hpp"

//------------------------------

uint32 const WINDOW_WIDTH_MIN = 260;
uint32 const WINDOW_HEIGHT_MIN = 200;
uint32 const WINDOW_WIDTH_START = 800;
uint32 const WINDOW_HEIGHT_START = 560;

//------------------------------

AvoExplorer::AvoExplorer() :
	m_topBar(0)
{
	create("AvoExplorer", WINDOW_WIDTH_START, WINDOW_HEIGHT_START, AvoGUI::WindowStyleFlags::Default);
}

void AvoExplorer::createContent()
{
	AvoGUI::DrawingContext* context = getDrawingContext();

	//------------------------------

	getWindow()->setMinSize(WINDOW_WIDTH_MIN, WINDOW_HEIGHT_MIN);

	context->setBackgroundColor(Colors::avoExplorerBackground);
	
	setThemeColor("background", Colors::avoExplorerBackground);
	setThemeColor("on background", Colors::avoExplorerOnBackground);
	setThemeColor("shadow", Colors::avoExplorerShadow);

	//------------------------------

	AvoGUI::TextProperties textProperties = context->getDefaultTextProperties();
	textProperties.fontFamilyName = "Roboto";
	textProperties.fontSize = 16.f;
	textProperties.fontWeight = AvoGUI::FontWeight::Light;
	textProperties.lineHeight = 1.1f;
	textProperties.characterSpacing = 0.3f;
	context->setDefaultTextProperties(textProperties);

	//------------------------------

	m_topBar = new TopBar(this);

	//------------------------------

	enableMouseEvents();
	m_actionMenu = new ActionMenu(this);
}

void AvoExplorer::handleSizeChange()
{
	m_topBar->setWidth(getWidth());
}

//------------------------------

void AvoExplorer::handleMouseDown(AvoGUI::MouseEvent const& p_event) 
{
	if (p_event.mouseButton == AvoGUI::MouseButton::Right)
	{
		m_actionMenu->open(p_event.x, p_event.y);
	}
}
