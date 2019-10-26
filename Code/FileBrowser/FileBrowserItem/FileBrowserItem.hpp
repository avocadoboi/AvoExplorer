#pragma once

#include "../FileBrowser.hpp"

//------------------------------

class FileBrowserItem :
	public AvoGUI::View
{
private:


public:
	FileBrowserItem(AvoGUI::View* p_parent, AvoGUI::Rectangle<float> const& p_rectangle) :
		View(p_parent, p_rectangle)
	{
		setCornerRadius(6.f);
	}

	void draw(AvoGUI::DrawingContext* p_context) override
	{
		p_context->setColor(Colors::fileBrowserItemBackground);
		p_context->fillRectangle(getSize());
	}
};
