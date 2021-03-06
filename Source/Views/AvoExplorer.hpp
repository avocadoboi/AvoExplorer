#pragma once

#include "../Resources/Colors.hpp"
#include "../Resources/Strings.hpp"
#include "../Resources/Ids.hpp"

#include <AvoGUI.hpp>
#include <filesystem>

//------------------------------

class TitleBar;
class TopBar;
class FileBrowser;

class AvoExplorer : public Avo::Gui
{
public:
	static constexpr uint32 WINDOW_WIDTH_MIN = 260;
	static constexpr uint32 WINDOW_HEIGHT_MIN = 200;
	static constexpr uint32 WINDOW_WIDTH_START = 850;
	static constexpr uint32 WINDOW_HEIGHT_START = 560;

private:
	TitleBar* m_titleBar{ nullptr };
	TopBar* m_topBar{ nullptr };
	FileBrowser* m_fileBrowser{ nullptr };
	char const* m_initialPath;

public:
	void handleSizeChange() override;

	Avo::WindowBorderArea getWindowBorderAreaAtPosition(float p_x, float p_y) override;

	void restartWithElevatedPrivileges();

	//------------------------------

	AvoExplorer(Avo::Component* p_parent, char const* p_initialPath);
};
