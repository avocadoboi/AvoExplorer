#pragma once

#include "FileBrowserItems.hpp"

//------------------------------

class FileBrowserItem :
	public AvoGUI::View
{
private:
	FileBrowserItems* m_fileBrowserItems;

	AvoGUI::Image* m_icon;

	std::filesystem::path m_path;
	std::filesystem::path m_name;
	AvoGUI::Text* m_text_name;
	bool m_isFile;

	bool m_isSelected;

public:
	FileBrowserItem(FileBrowserItems* p_parent, std::filesystem::path const& p_path, bool p_isFile);
	~FileBrowserItem()
	{
		if (m_icon)
		{
			m_icon->forget();
		}
		if (m_text_name)
		{
			m_text_name->forget();
		}
	}

	//------------------------------

	void handleMouseDown(AvoGUI::MouseEvent const& p_event) override
	{

	}
	void handleMouseDoubleClick(AvoGUI::MouseEvent const& p_event) override
	{
		if (m_isFile)
		{
			ShellExecuteW((HWND)getGUI()->getWindow()->getWindowHandle(), 0, m_path.c_str(), 0, 0, SHOW_OPENWINDOW);
		}
		else
		{
			m_fileBrowserItems->getFileBrowser()->setWorkingDirectory(m_path);
		}
	}

	//------------------------------

	void draw(AvoGUI::DrawingContext* p_context) override
	{
		p_context->setColor(Colors::fileBrowserItemBackground);
		p_context->fillRectangle(getSize());
		p_context->setColor(getThemeColor("on background"));
		p_context->drawText(m_text_name);
		if (m_icon)
		{
			p_context->drawImage(m_icon);
		}
	}
};
