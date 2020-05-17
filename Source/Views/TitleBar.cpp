#include "TitleBar.hpp"

void TitleBarWindowButton::handleMouseUp(AvoGUI::MouseEvent const& p_event) 
{
	if (getSize().getIsContaining(p_event.x, p_event.y) && p_event.mouseButton == AvoGUI::MouseButton::Left)
	{
		getParent<TitleBar>()->handleTitleBarButtonClick(this);
	}
}
