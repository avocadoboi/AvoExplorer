#include "AvoExplorer.hpp"

#include "TopBar/TopBar.hpp"
#include "FileBrowser/FileBrowser.hpp"

#include "ActionMenu/ContextMenu.hpp"

//------------------------------

uint32 constexpr WINDOW_WIDTH_MIN = 260;
uint32 constexpr WINDOW_HEIGHT_MIN = 200;
uint32 constexpr WINDOW_WIDTH_START = 800;
uint32 constexpr WINDOW_HEIGHT_START = 560;

//------------------------------

AvoExplorer::AvoExplorer() :
	m_topBar(0), m_fileBrowser(0), m_contextMenu(0)
{
	CoInitialize(0);
	CoCreateInstance(CLSID_WICImagingFactory2, 0, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&m_windowsImagingFactory));

	create("AvoExplorer", WINDOW_WIDTH_START, WINDOW_HEIGHT_START, AvoGUI::WindowStyleFlags::DefaultCustom);
}
AvoExplorer::~AvoExplorer()
{
	m_windowsImagingFactory->Release();
}

//------------------------------

void AvoExplorer::createContent()
{
	AvoGUI::DrawingContext* context = getDrawingContext();

	//------------------------------

	getWindow()->setMinSize(WINDOW_WIDTH_MIN, WINDOW_HEIGHT_MIN);

	context->setBackgroundColor(Colors::avoExplorerBackground);
	
	setThemeColor("background", Colors::avoExplorerBackground);
	setThemeColor("on background", Colors::avoExplorerOnBackground);
	setThemeColor("shadow", Colors::avoExplorerShadow);
	setThemeColor("selection", Colors::selection);

	setThemeValue("hover animation speed", 0.4f);

	//------------------------------

	AvoGUI::TextProperties textProperties = context->getDefaultTextProperties();
	textProperties.fontFamilyName = "Roboto";
	textProperties.fontSize = 16.f;
	textProperties.fontWeight = AvoGUI::FontWeight::Light;
	textProperties.lineHeight = 1.1f;
	textProperties.characterSpacing = 0.3f;
	context->setDefaultTextProperties(textProperties);

	//------------------------------

	m_contextMenu = new ContextMenu(this);

	//------------------------------

	m_topBar = new TopBar(this);
	m_fileBrowser = new FileBrowser(this);

	//------------------------------

	enableMouseEvents();
}

void AvoExplorer::handleSizeChange()
{
	m_topBar->setWidth(getWidth());
	m_fileBrowser->setBounds(0.f, m_topBar->getBottom(), getWidth(), getHeight());
}
