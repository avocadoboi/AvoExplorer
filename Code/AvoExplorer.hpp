#pragma once

#include "../Resources/Colors.hpp"
#include "../Resources/Strings.hpp"

#include <AvoGUI.hpp>
#include <wincodec.h>

//------------------------------

class TopBar;
class FileBrowser;

class ContextMenu;

class AvoExplorer : 
	public AvoGUI::GUI
{
private:
	TopBar* m_topBar;
	FileBrowser* m_fileBrowser;

	ContextMenu* m_contextMenu;

	IWICImagingFactory2* m_windowsImagingFactory;

public:
	AvoExplorer();
	~AvoExplorer();

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
};
