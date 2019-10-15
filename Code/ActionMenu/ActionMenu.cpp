#include "ActionMenu.hpp"

//------------------------------

float const ACTION_MENU_ITEM_HEIGHT = 4.f			*8.f;
float const ACTION_MENU_ITEM_LEFT_PADDING = 2.f		*8.f;
float const ACTION_MENU_ITEM_RIGHT_PADDING = 2.f	*8.f;

float const ACTION_MENU_MIN_PARENT_MARGIN = 3.f		*8.f;
float const ACTION_MENU_VERTICAL_PADDING = 1.f		*8.f;

//
// Class ActionMenuItem
//

void ActionMenuItem::handleSizeChange() 
{
	m_text_action->setLeft(ACTION_MENU_ITEM_LEFT_PADDING);
	m_text_action->setCenterY(getHeight()*0.5f);

	m_text_shortcut->setRight(getWidth() - ACTION_MENU_ITEM_RIGHT_PADDING);
	m_text_shortcut->setCenterY(getHeight() * 0.5f);
}
void ActionMenuItem::handleMouseUp(AvoGUI::MouseEvent const& p_event)
{
	m_parent->handleActionMenuItemChoice();
}

//
// Class ActionMenu
//

void ActionMenu::addAction(char const* p_action, char const* p_shortcut)
{
	ActionMenuItem* actionMenuItem = new ActionMenuItem(this, p_action, p_shortcut);
	if (getNumberOfChildren() > 1)
	{
		actionMenuItem->setTop(getChild(getNumberOfChildren() - 2)->getBottom());
	}
	else
	{
		actionMenuItem->setTop(ACTION_MENU_VERTICAL_PADDING);
	}
	actionMenuItem->setSize(m_targetBounds.getWidth(), ACTION_MENU_ITEM_HEIGHT);

	m_targetBounds.setHeight(actionMenuItem->getBottom() + ACTION_MENU_VERTICAL_PADDING);
}

void ActionMenu::open(float p_anchorX, float p_anchorY)
{
	if (p_anchorX + m_targetBounds.getWidth() > getParent()->getWidth() - ACTION_MENU_MIN_PARENT_MARGIN)
	{
		m_targetBounds.setRight(p_anchorX);
	}
	else
	{
		m_targetBounds.setLeft(p_anchorX);
	}
	if (p_anchorY + m_targetBounds.getHeight() > getParent()->getHeight() - ACTION_MENU_MIN_PARENT_MARGIN)
	{
		m_targetBounds.setBottom(p_anchorY);
		setCornerRadius(5.f, 0.f, 5.f, 5.f);
	}
	else
	{
		m_targetBounds.setTop(p_anchorY);
		setCornerRadius(5.f, 0.f, 5.f, 5.f);
	}

	m_anchor.set(p_anchorX, p_anchorY);
	setBounds(p_anchorX, p_anchorY, p_anchorX, p_anchorY);

	setIsVisible(true);

	m_isOpen = true;
	m_openAnimationTime = 0.f;
	queueAnimationUpdate();
}
