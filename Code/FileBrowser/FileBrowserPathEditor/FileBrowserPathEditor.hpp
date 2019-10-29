#pragma once

#include "../FileBrowser.hpp"

//------------------------------

class FileBrowserPathEditor;

class FileBrowserPathEditorDirectoryButton :
	public AvoGUI::View
{
private:
	FileBrowserPathEditor* m_pathEditor;

	std::filesystem::path m_path;
	AvoGUI::Text* m_text;
	AvoGUI::Ripple* m_ripple;

public:
	FileBrowserPathEditorDirectoryButton(FileBrowserPathEditor* p_parent, std::filesystem::path const& p_path, std::string const& p_name);
	~FileBrowserPathEditorDirectoryButton()
	{
		m_text->forget();
	}

	//------------------------------

	void handleMouseUp(AvoGUI::MouseEvent const& p_event) override;

	//------------------------------

	void draw(AvoGUI::DrawingContext* p_context) override
	{
		p_context->setColor(getThemeColor("on background"));
		p_context->drawText(m_text);
	}
};

//------------------------------

class FileBrowserPathEditor :
	public AvoGUI::View
{
private:
	FileBrowser* m_fileBrowser;

	AvoGUI::Image* m_directorySeparatorIcon;
	std::vector<FileBrowserPathEditorDirectoryButton*> m_directoryButtons;

public:
	FileBrowserPathEditor(FileBrowser* p_parent);
	~FileBrowserPathEditor()
	{
		m_directorySeparatorIcon->forget();
	}

	//------------------------------

	FileBrowser* getFileBrowser()
	{
		return m_fileBrowser;
	}

	//------------------------------

	void setWorkingDirectory(std::filesystem::path const& p_path);

	//------------------------------

	void draw(AvoGUI::DrawingContext* p_context) override;
};
