#include "TitleBar.hpp"

#include "../utilities.hpp"

//------------------------------
// class TitleBarWindowButton
//------------------------------

TitleBarWindowButton::TitleBarWindowButton(TitleBar* p_parent, Icon p_icon, bool p_isCloseButton, bool p_isEnabled) :
	View(p_parent), 
	m_titleBar(p_parent),
	m_icon(p_icon), 
	m_isEnabled(p_isEnabled)
{
	setSize(BUTTON_WIDTH_FACTOR * p_parent->getHeight(), p_parent->getHeight());

	setThemeColor(ThemeColors::background, p_isCloseButton ? Colors::titleBarCloseButton : AvoGUI::interpolate(getThemeColor(ThemeColors::background), getThemeColor(ThemeColors::onBackground), 0.3f));
	m_backgroundColor.alpha = 0.f;

	if (p_isEnabled)
	{
		m_ripple = new AvoGUI::Ripple(this, AvoGUI::Color(getThemeColor(ThemeColors::onBackground), p_isCloseButton ? 0.5f : 0.2f));
		m_ripple->setHasHoverEffect(false);
	}

	enableMouseEvents();
}

void TitleBarWindowButton::handleMouseUp(AvoGUI::MouseEvent const& p_event) 
{
	if (getSize().getIsContaining(p_event.x, p_event.y))
	{
		m_titleBar->handleTitleBarButtonClick(this);
	}
}
