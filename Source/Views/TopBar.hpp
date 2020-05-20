#pragma once

#include "AvoExplorer.hpp"

//------------------------------

class Bookmarks;

class TopBar :
	public Avo::View
{
private:
	AvoExplorer* m_avoExplorer;
	Avo::Text m_title;

	Bookmarks* m_bookmarks{ nullptr };

public:
	TopBar(AvoExplorer* p_parent);

	//------------------------------

	Bookmarks* getBookmarks()
	{
		return m_bookmarks;
	}

	//------------------------------

	void handleSizeChange() override;

	void draw(Avo::DrawingContext* p_context) override
	{
		p_context->setColor(Colors::topBarBackground);
		p_context->fillRectangle(getSize());

		p_context->setColor(Colors::label);
		p_context->drawText(m_title);
	}
};
