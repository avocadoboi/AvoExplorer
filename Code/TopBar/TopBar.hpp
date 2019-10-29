#pragma once

#include "..//AvoExplorer.hpp"

//------------------------------

class Bookmarks;

class TopBar :
	public AvoGUI::View
{
private:
	AvoExplorer* m_avoExplorer;
	AvoGUI::Text* m_title;

	Bookmarks* m_bookmarks;

public:
	TopBar(AvoExplorer* p_parent);
	~TopBar()
	{
		m_title->forget();
	}

	//------------------------------

	void handleSizeChange() override;

	void draw(AvoGUI::DrawingContext* p_context) override
	{
		p_context->setColor(Colors::topBarBackground);
		p_context->fillRectangle(getSize());

		p_context->setColor(Colors::label);
		p_context->drawText(m_title);
	}
};
