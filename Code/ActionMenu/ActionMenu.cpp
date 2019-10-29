#include "ActionMenu.hpp"

//------------------------------

float constexpr ACTION_MENU_ITEM_HEIGHT = 4.f			*8.f;
float constexpr ACTION_MENU_ITEM_LEFT_PADDING = 2.f		*8.f;
float constexpr ACTION_MENU_ITEM_RIGHT_PADDING = 2.f	*8.f;

float constexpr ACTION_MENU_MIN_PARENT_MARGIN = 3.f		*8.f;
float constexpr ACTION_MENU_VERTICAL_PADDING = 1.f		*8.f;

float constexpr ACTION_MENU_ANIMATION_SPEED = 0.1f;

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
	if (p_event.mouseButton == AvoGUI::MouseButton::Left)
	{
		m_parent->handleActionMenuItemChoice(this);
	}
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
		if (p_anchorY + m_targetBounds.getHeight() > getParent()->getHeight() - ACTION_MENU_MIN_PARENT_MARGIN)
		{
			m_targetBounds.setBottom(p_anchorY);
			setCornerRadius(4.f, 4.f, 5.f, 0.f);
		}
		else
		{
			m_targetBounds.setTop(p_anchorY);
			setCornerRadius(5.f, 0.f, 5.f, 5.f);
		}
	}
	else
	{
		m_targetBounds.setLeft(p_anchorX);
		if (p_anchorY + m_targetBounds.getHeight() > getParent()->getHeight() - ACTION_MENU_MIN_PARENT_MARGIN)
		{
			m_targetBounds.setBottom(p_anchorY);
			setCornerRadius(4.f, 4.f, 0.f, 5.f);
		}
		else
		{
			m_targetBounds.setTop(p_anchorY);
			setCornerRadius(0.f, 5.f, 5.f, 5.f);
		}
	}

	m_anchor.set(p_anchorX, p_anchorY);
	setBounds(p_anchorX, p_anchorY, p_anchorX, p_anchorY);

	setIsVisible(true);

	m_openAnimationTime = 0.f;
	queueAnimationUpdate();
}

void ActionMenu::updateAnimations()
{
	if (m_openAnimationTime < 1.f)
	{
		m_openAnimationValue = getThemeEasing("out").easeValue(m_openAnimationTime += ACTION_MENU_ANIMATION_SPEED);
		queueAnimationUpdate();
	}

	float heightFactor = 1.f - std::cos(m_openAnimationValue * AvoGUI::HALF_PI);
	setBounds(
		m_targetBounds.left,
		AvoGUI::interpolate(m_anchor.y, m_targetBounds.top, heightFactor),
		m_targetBounds.right,
		AvoGUI::interpolate(m_anchor.y, m_targetBounds.bottom, heightFactor)
	);
	float itemsTop = m_targetBounds.top - getTop() + ACTION_MENU_VERTICAL_PADDING;
	if (getChild(0)->getTop() != itemsTop)
	{
		for (uint32_t a = 0; a < getNumberOfChildren(); a++)
		{
			if (a)
			{
				getChild(a)->setTop(getChild(a-1)->getBottom());
			}
			else
			{
				getChild(a)->setTop(itemsTop);
			}
		}
	}

	setThemeValue("opacity", AvoGUI::square(AvoGUI::max(m_openAnimationValue * 7.f - 6.f, 0.f)));

	invalidate();
}
