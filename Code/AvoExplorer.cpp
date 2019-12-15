#include "AvoExplorer.hpp"

#include "TitleBar/TitleBar.hpp"
#include "TopBar/TopBar.hpp"
#include "TopBar/Bookmarks/Bookmarks.hpp"
#include "FileBrowser/FileBrowser.hpp"

#include "ActionMenu/ContextMenu.hpp"

#include <fstream>

//------------------------------

uint32 constexpr WINDOW_WIDTH_MIN = 260;
uint32 constexpr WINDOW_HEIGHT_MIN = 200;
uint32 constexpr WINDOW_WIDTH_START = 850;
uint32 constexpr WINDOW_HEIGHT_START = 560;

constexpr char const* BOOKMARKS_DATA_PATH = "data";

//
// Private
//

void AvoExplorer::loadBookmarkPaths()
{
	if (std::filesystem::exists(BOOKMARKS_DATA_PATH))
	{
		std::wifstream fileStream(BOOKMARKS_DATA_PATH, std::ios::binary);

		while (!fileStream.eof())
		{
			std::wstring pathString;
			std::getline(fileStream, pathString, (wchar_t)0);
			if (pathString != L"")
			{
				addBookmark(pathString);
			}
		}

		fileStream.close();
	}
}
void AvoExplorer::saveBookmarkPaths()
{
	std::wofstream fileStream(BOOKMARKS_DATA_PATH, std::ios::binary);

	for (uint32 a = 0; a < m_bookmarkPaths.size(); a++)
	{
		fileStream.write(m_bookmarkPaths[a].c_str(), m_bookmarkPaths[a].wstring().size() + 1);
	}

	fileStream.close();
}

//
// Public
//

AvoExplorer::AvoExplorer(char const* p_initialPath) :
	m_titleBar(0), m_topBar(0), m_fileBrowser(0), m_contextMenu(0), 
	m_windowsImagingFactory(0), m_initialPath(p_initialPath)
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

void AvoExplorer::addBookmark(std::filesystem::path const& p_path)
{
	getViewById<Bookmarks>(Ids::bookmarks)->addBookmark(p_path);
	m_bookmarkPaths.push_back(p_path);
	saveBookmarkPaths();
}
void AvoExplorer::removeBookmark(uint32 p_index)
{
	getViewById<Bookmarks>(Ids::bookmarks)->removeBookmark(p_index);
	m_bookmarkPaths.erase(m_bookmarkPaths.begin() + p_index);
	saveBookmarkPaths();
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

	m_bookmarkPaths.reserve(10);
	loadBookmarkPaths();

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
