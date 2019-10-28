#pragma once

#include "FileBrowserItems.hpp"

//------------------------------

class FileBrowserItem :
	public AvoGUI::View
{
private:
	std::filesystem::path m_path;
	std::filesystem::path m_name;
	AvoGUI::Text* m_text_name;
	bool m_isFile;

public:
	FileBrowserItem(AvoGUI::View* p_parent, std::filesystem::path const& p_path, bool p_isFile);

	void draw(AvoGUI::DrawingContext* p_context) override
	{
		p_context->setColor(Colors::fileBrowserItemBackground);
		p_context->fillRectangle(getSize());
		p_context->setColor(getThemeColor("on background"));
		p_context->drawText(m_text_name);
	}
};
