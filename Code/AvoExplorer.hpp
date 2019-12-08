#pragma once

#include "../Resources/Colors.hpp"
#include "../Resources/Strings.hpp"
#include "../Resources/Ids.hpp"

#include <AvoGUI.hpp>
#include <wincodec.h>
#include <filesystem>

//------------------------------

class TitleBar;
class TopBar;
class FileBrowser;

class ContextMenu;

class AvoExplorer : 
	public AvoGUI::Gui
{
private:
	TitleBar* m_titleBar;
	TopBar* m_topBar;
	FileBrowser* m_fileBrowser;

	ContextMenu* m_contextMenu;

	std::vector<std::filesystem::path> m_bookmarkPaths;
	void loadBookmarkPaths();
	void saveBookmarkPaths();

	IWICImagingFactory2* m_windowsImagingFactory;

	char const* m_initialPath;

public:
	AvoExplorer(char const* p_initialPath);
	~AvoExplorer();

	//------------------------------

	void addBookmark(std::filesystem::path const& p_path);
	std::vector<std::filesystem::path> const& getBookmarkPaths();

	//------------------------------

	ContextMenu* getContextMenu()
	{
		return m_contextMenu;
	}
	IWICImagingFactory2* getWindowsImagingFactory()
	{
		return m_windowsImagingFactory;
	}

	//------------------------------

	void createContent() override;

	void handleSizeChange() override;

	AvoGUI::WindowBorderArea getWindowBorderAreaAtPosition(float p_x, float p_y) override;
};
