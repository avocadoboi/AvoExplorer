#pragma once

#include "../FileBrowser.hpp"

//------------------------------

class FileBrowserPathEditor :
	public AvoGUI::View
{
private:
	FileBrowser* m_fileBrowser;

public:
	FileBrowserPathEditor(FileBrowser* p_parent);

	void draw(AvoGUI::DrawingContext* p_context) override
	{
		p_context->setColor(Colors::fileBrowserPathEditorBackground);
		p_context->fillRectangle(getSize());
		p_context->setColor(Colors::fileBrowserPathEditorBorder);
		p_context->strokeRoundedRectangle(getSize(), getCorners().topLeftSizeX, 3.f);
	}
};
