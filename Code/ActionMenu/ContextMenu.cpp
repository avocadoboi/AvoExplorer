#include "ContextMenu.hpp"

void ContextView::handleMouseDown(AvoGUI::MouseEvent const& p_event)
{
	if (p_event.mouseButton == AvoGUI::MouseButton::Right && m_isMouseHoveringBackground)
	{
		m_contextMenu->open(this);
	}
}
