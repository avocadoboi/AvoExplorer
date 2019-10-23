#include "ScrollContainer.hpp"

//------------------------------

float const SCROLLBAR_OPACITY_NORMAL = 0.3f;
float const SCROLLBAR_OPACITY_HOVERING = 0.6f;

//------------------------------

ScrollContainer::ScrollContainer(AvoGUI::View* p_parent, AvoGUI::Rectangle<float> const& p_bounds) :
	AvoGUI::View(p_parent, p_bounds),
	m_scrollbarOpacity()
{
	enableMouseEvents();
}
