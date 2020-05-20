#include "TitleBar.hpp"

void TitleBarWindowButton::handleMouseUp(Avo::MouseEvent const& p_event) 
{
	if (getSize().getIsContaining(p_event.x, p_event.y) && p_event.mouseButton == Avo::MouseButton::Left)
	{
		getParent<TitleBar>()->handleTitleBarButtonClick(this);
	}
}
