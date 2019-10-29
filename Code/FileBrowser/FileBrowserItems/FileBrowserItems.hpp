#pragma once

#include "../FileBrowser.hpp"

//------------------------------

class FileBrowserItem;

class FileBrowserItems :
	public AvoGUI::View
{
private:
	FileBrowser* m_fileBrowser;

	std::vector<FileBrowserItem*> m_fileItems;
	std::vector<FileBrowserItem*> m_directoryItems;
	AvoGUI::Text* m_text_directories;
	AvoGUI::Text* m_text_files;

public:
	FileBrowserItems(ScrollContainer* p_parent, FileBrowser* p_fileBrowser) :
		View(p_parent), m_fileBrowser(p_fileBrowser),
		m_text_directories(0), m_text_files(0)
	{
		m_text_directories = getGUI()->getDrawingContext()->createText(Strings::directories, 16.f);
		m_text_files = getGUI()->getDrawingContext()->createText(Strings::files, 16.f);
	}

	//------------------------------

	FileBrowser* getFileBrowser()
	{
		return m_fileBrowser;
	}

	void setWorkingDirectory(std::filesystem::path const& p_path);

	void updateLayout();

	//------------------------------

	void draw(AvoGUI::DrawingContext* p_context)
	{
		p_context->setColor(Colors::label);
		if (m_directoryItems.size())
		{
			p_context->drawText(m_text_directories);
		}
		if (m_fileItems.size())
		{
			p_context->drawText(m_text_files);
		}
	}
};
