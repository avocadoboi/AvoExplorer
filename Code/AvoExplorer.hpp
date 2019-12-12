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
	void removeBookmark(uint32 p_index);
	void removeBookmark(std::filesystem::path const& p_path)
	{
		for (uint32 a = 0; a < m_bookmarkPaths.size(); a++)
		{
			if (m_bookmarkPaths[a] == p_path)
			{
				removeBookmark(a);
				break;
			}
		}
	}
	std::vector<std::filesystem::path> const& getBookmarkPaths()
	{
		return m_bookmarkPaths;
	}
	bool getIsPathBookmarked(std::filesystem::path const& p_path)
	{
		return std::find(m_bookmarkPaths.begin(), m_bookmarkPaths.end(), p_path) != m_bookmarkPaths.end();
	}

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
