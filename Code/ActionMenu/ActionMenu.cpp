#include "ActionMenu.hpp"

//------------------------------

float const ACTION_MENU_MIN_PARENT_MARGIN = 20.f;
float const ACTION_MENU_VERTICAL_PADDING = 10.f;

//
// Class ActionMenuItem
//

void ActionMenuItem::handleMouseUp(AvoGUI::MouseEvent const& p_event)
{
	m_parent->handleActionMenuItemChoice();
}

//
// Class ActionMenu
//

void ActionMenu::addAction(char const* p_action, char const* p_shortcut = "")
{
	ActionMenuItem* actionMenuItem = new ActionMenuItem(this, p_action, p_shortcut);
	if (getNumberOfChildren())
	{
		actionMenuItem->setTop(getChild(getNumberOfChildren() - 1)->getBottom());
	}
}

void ActionMenu::open(float p_anchorX, float p_anchorY)
{
	if (p_anchorX + m_width > getParent()->getWidth())
	{
		setLeft(p_anchorX - m_width);
	}
	else
	{
		setLeft(p_anchorX);
	}

	if (p_anchorY + m_height > getParent()->getHeight())
	{
		setTop(p_anchorY - m_height);
	}
	else
	{
		setTop(p_anchorY);
	}

	queueAnimationUpdate();
}
