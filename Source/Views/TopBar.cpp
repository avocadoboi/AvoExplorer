#include "TopBar.hpp"

#include "Bookmarks.hpp"

//------------------------------

float constexpr TOP_BAR_HEIGHT = 11.f							*8.f;
float constexpr TOP_BAR_BOOKMARKS_MARGIN_HORIZONTAL = 2.f		*8.f;
float constexpr TOP_BAR_BOOKMARKS_MARGIN_TOP = 4.f				*8.f;
float constexpr TOP_BAR_BOOKMARKS_MARGIN_BOTTOM = 1.f			*8.f;

//------------------------------

TopBar::TopBar(AvoExplorer* p_parent) :
	View(p_parent), 
	m_avoExplorer(p_parent)
{
	setHeight(TOP_BAR_HEIGHT);

	setCornerRadius(0.f, 0.f, 10.f, 10.f);
	setElevation(5.f);
	setHasShadow(true);

	enableMouseEvents();

	//------------------------------

	m_title = getGui()->getDrawingContext()->createText(Strings::bookmarks, 14.f);
	m_title.setLeft(TOP_BAR_BOOKMARKS_MARGIN_HORIZONTAL + 4.f);
	m_title.setCenterY(TOP_BAR_BOOKMARKS_MARGIN_TOP * 0.5f);
	m_title.setFontWeight(Avo::FontWeight::Light);
	m_title.fitSizeToText();

	m_bookmarks = new Bookmarks(this);
}

void TopBar::handleSizeChange()
{
	if (m_bookmarks)
	{
		m_bookmarks->setBounds(
			TOP_BAR_BOOKMARKS_MARGIN_HORIZONTAL, TOP_BAR_BOOKMARKS_MARGIN_TOP, 
			getWidth() - TOP_BAR_BOOKMARKS_MARGIN_HORIZONTAL, getHeight() - TOP_BAR_BOOKMARKS_MARGIN_BOTTOM
		);
	}
}
