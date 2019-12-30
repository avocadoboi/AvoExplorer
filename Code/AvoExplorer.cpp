#include "AvoExplorer.hpp"

#include "TitleBar/TitleBar.hpp"
#include "TopBar/TopBar.hpp"
#include "TopBar/Bookmarks/Bookmarks.hpp"
#include "FileBrowser/FileBrowser.hpp"
#include "FileBrowser/FileBrowserItems/FileBrowserItem.hpp"

#include "ActionMenu/ContextMenu.hpp"

//------------------------------

uint32 constexpr WINDOW_WIDTH_MIN = 260;
uint32 constexpr WINDOW_HEIGHT_MIN = 200;
uint32 constexpr WINDOW_WIDTH_START = 850;
uint32 constexpr WINDOW_HEIGHT_START = 560;

//
// Public
//

AvoExplorer::AvoExplorer(char const* p_initialPath) :
	m_initialPath(p_initialPath)
{
	CoInitialize(0);
	CoCreateInstance(CLSID_WICImagingFactory2, 0, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&m_windowsImagingFactory));

	create("AvoExplorer", WINDOW_WIDTH_START, WINDOW_HEIGHT_START, AvoGUI::WindowStyleFlags::DefaultCustom | AvoGUI::WindowStyleFlags::MaximizeButton);
	waitForFinish();
}
AvoExplorer::~AvoExplorer()
{
	m_windowsImagingFactory->Release();
}

//------------------------------

void AvoExplorer::restartWithElevatedPrivileges()
{
	wchar_t executablePath[MAX_PATH];
	GetModuleFileNameW(0, executablePath, MAX_PATH);
	ShellExecuteW(0, L"runas", executablePath, m_fileBrowser->getPath().c_str(), 0, SW_SHOWNORMAL);
	getWindow()->close();
}

//------------------------------

void AvoExplorer::createContent()
{
	getWindow()->setMinSize(WINDOW_WIDTH_MIN, WINDOW_HEIGHT_MIN);

	//------------------------------

	AvoGUI::DrawingContext* context = getDrawingContext();

	context->setBackgroundColor(Colors::avoExplorerBackground);
	
	setThemeColor("background", Colors::avoExplorerBackground);
	setThemeColor("on background", Colors::avoExplorerOnBackground);
	setThemeColor("shadow", Colors::avoExplorerShadow);
	setThemeColor("selection", Colors::selection);
	setThemeColor("primary", Colors::primary);
	setThemeColor("primary on background", Colors::primaryOnBackground);
	setThemeColor("secondary", Colors::secondary);
	setThemeColor("on secondary", Colors::onSecondary);

	setThemeValue("hover animation speed", 0.4f);
	setThemeValue("position animation speed", 0.1f);

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

	m_titleBar = new TitleBar(this);

	m_fileBrowser = new FileBrowser(this);
	m_topBar = new TopBar(this);

	m_fileBrowser->setWorkingDirectory(m_initialPath);

	//------------------------------

	enableMouseEvents();
}

void AvoExplorer::handleSizeChange()
{
	m_titleBar->setWidth(getWidth());

	m_topBar->setTop(m_titleBar->getBottom());
	m_topBar->setWidth(getWidth());

	m_fileBrowser->setBounds(0.f, m_topBar->getBottom(), getWidth(), getHeight());
}

AvoGUI::WindowBorderArea AvoExplorer::getWindowBorderAreaAtPosition(float p_x, float p_y)
{
	AvoGUI::WindowBorderArea area = Gui::getWindowBorderAreaAtPosition(p_x, p_y);
	if (area == AvoGUI::WindowBorderArea::None && m_titleBar->getIsContaining(p_x, p_y))
	{
		return m_titleBar->getWindowBorderAreaAtPosition(p_x, p_y);
	}
	return area;
}
