#include "AvoExplorer.hpp"

#include "TitleBar.hpp"
#include "TopBar.hpp"
#include "Bookmarks.hpp"
#include "FileBrowser.hpp"
#include "FileBrowserItem.hpp"

#include "ContextMenu.hpp"
#include "../Resources/ThemeValues.hpp"

//------------------------------

void AvoExplorer::restartWithElevatedPrivileges()
{
	wchar_t executablePath[MAX_PATH];
	GetModuleFileNameW(0, executablePath, MAX_PATH);
	ShellExecuteW(0, L"runas", executablePath, m_fileBrowser->getPath().c_str(), 0, SW_SHOWNORMAL);
	getWindow()->close();
}

//------------------------------

void AvoExplorer::handleSizeChange()
{
	m_titleBar->setWidth(getWidth());

	m_topBar->setTop(m_titleBar->getBottom());
	m_topBar->setWidth(getWidth());

	m_fileBrowser->setBounds(0.f, m_topBar->getBottom(), getWidth(), getHeight());
}

Avo::WindowBorderArea AvoExplorer::getWindowBorderAreaAtPosition(float p_x, float p_y)
{
	Avo::WindowBorderArea area = Gui::getWindowBorderAreaAtPosition(p_x, p_y);
	if (area == Avo::WindowBorderArea::None && m_titleBar->getIsContaining(p_x, p_y))
	{
		return m_titleBar->getWindowBorderAreaAtPosition(p_x, p_y);
	}
	return area;
}

AvoExplorer::AvoExplorer(Avo::Component* p_parent, char const* p_initialPath) :
	Gui(p_parent),
	m_initialPath(p_initialPath)
{
	create("AvoExplorer", WINDOW_WIDTH_START, WINDOW_HEIGHT_START, Avo::WindowStyleFlags::DefaultCustom);

	getWindow()->setMinSize(WINDOW_WIDTH_MIN, WINDOW_HEIGHT_MIN);

	//------------------------------

	setThemeColor(ThemeColors::background, Colors::avoExplorerBackground);
	setThemeColor(ThemeColors::onBackground, Colors::avoExplorerOnBackground);
	setThemeColor(ThemeColors::shadow, Colors::avoExplorerShadow);
	setThemeColor(ThemeColors::selection, Colors::selection);
	setThemeColor(ThemeColors::primary, Colors::primary);
	setThemeColor(ThemeColors::primaryOnBackground, Colors::primaryOnBackground);
	setThemeColor(ThemeColors::secondary, Colors::secondary);
	setThemeColor(ThemeColors::onSecondary, Colors::onSecondary);

	setThemeValue(ThemeValues::hoverAnimationSpeed, 0.4f);
	setThemeValue(ThemeValues::positionAnimationSpeed, 0.1f);

	//------------------------------

	Avo::TextProperties textProperties = getDrawingContext()->getDefaultTextProperties();
	textProperties.fontFamilyName = "Roboto";
	textProperties.fontSize = 16.f;
	textProperties.fontWeight = Avo::FontWeight::Light;
	textProperties.lineHeight = 1.1f;
	textProperties.characterSpacing = 0.3f;
	getDrawingContext()->setDefaultTextProperties(textProperties);

	//------------------------------

	setId(Ids::avoExplorer);

	//------------------------------

	auto contextMenu = new ContextMenu(this);
	contextMenu->setId(Ids::contextMenu, this);

	//------------------------------

	m_titleBar = new TitleBar(this);

	m_fileBrowser = new FileBrowser(this);
	m_topBar = new TopBar(this);

	m_fileBrowser->setWorkingDirectory(m_initialPath);

	//------------------------------

	enableMouseEvents();
}
