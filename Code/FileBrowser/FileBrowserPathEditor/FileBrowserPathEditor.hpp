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
	FileBrowserPathEditorDirectoryButton(AvoGUI::View* p_parent, FileBrowserPathEditor* p_pathEditor, std::filesystem::path const& p_path, std::string const& p_name);
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

class FileBrowserPathEditorPath :
	public AvoGUI::View
{
private:
	FileBrowserPathEditor* m_pathEditor;

	AvoGUI::Image* m_directorySeparatorIcon;
	std::vector<FileBrowserPathEditorDirectoryButton*> m_directoryButtons;

public:
	FileBrowserPathEditorPath(FileBrowserPathEditor* p_parent);
	~FileBrowserPathEditorPath()
	{
		m_directorySeparatorIcon->forget();
	}

	void setWorkingDirectory(std::filesystem::path const& p_path);
	
	void draw(AvoGUI::DrawingContext* p_context) override;
};

//------------------------------

class FileBrowserPathEditor :
	public AvoGUI::View,
	public AvoGUI::ButtonListener
{
private:
	FileBrowser* m_fileBrowser;
	FileBrowserPathEditorPath* m_path;

	AvoGUI::Image* m_bookmarkIcon_hollow;
	AvoGUI::Image* m_bookmarkIcon_filled;
	AvoGUI::Button* m_bookmarkButton;

public:
	FileBrowserPathEditor(FileBrowser* p_parent);
	~FileBrowserPathEditor()
	{
		m_bookmarkIcon_hollow->forget();
		m_bookmarkIcon_filled->forget();
	}

	void handleSizeChange() override;

	//------------------------------

	FileBrowser* getFileBrowser()
	{
		return m_fileBrowser;
	}

	//------------------------------

	void handleButtonClick(AvoGUI::Button* p_button) override
	{

	}

	void setWorkingDirectory(std::filesystem::path const& p_path);

	//------------------------------

	void draw(AvoGUI::DrawingContext* p_context) override;
};
