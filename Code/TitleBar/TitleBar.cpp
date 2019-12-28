#include "TitleBar.hpp"

#include "../utilities.hpp"

//------------------------------

float constexpr HEIGHT = 2.9 * 8.f;

float constexpr BUTTON_ICON_PADDING = 7.f;
float constexpr BUTTON_WIDTH_FACTOR = 1.2f;

//------------------------------
// class TitleBarWindowButton
//------------------------------

TitleBarWindowButton::TitleBarWindowButton(TitleBar* p_parent, Icon p_icon, bool p_isCloseButton, bool p_isEnabled) :
	View(p_parent), m_titleBar(p_parent),
	m_icon(p_icon), m_ripple(0),
	m_hoverAnimationTime(0.f), m_isHovering(false), m_isEnabled(p_isEnabled)
{
	setSize(BUTTON_WIDTH_FACTOR * HEIGHT, HEIGHT);

	setThemeColor("background", p_isCloseButton ? Colors::titleBarCloseButton : AvoGUI::interpolate(getThemeColor("background"), getThemeColor("on background"), 0.3f));
	m_backgroundColor.alpha = 0.f;

	if (p_isEnabled)
	{
		m_ripple = new AvoGUI::Ripple(this, AvoGUI::Color(getThemeColor("on background"), p_isCloseButton ? 0.5f : 0.2f));
		m_ripple->setHasHoverEffect(false);
	}

	enableMouseEvents();
}

void TitleBarWindowButton::handleMouseUp(AvoGUI::MouseEvent const& p_event) 
{
	if (m_bounds.createCopyAtOrigin().getIsContaining(p_event.x, p_event.y))
	{
		m_titleBar->handleTitleBarButtonClick(this);
	}
}

//------------------------------

void TitleBarWindowButton::setIcon(Icon p_icon)
{
	m_icon = p_icon;
}

//------------------------------

void TitleBarWindowButton::draw(AvoGUI::DrawingContext* p_context)
{
	p_context->setColor(m_backgroundColor);
	p_context->fillRectangle(getSize());

	AvoGUI::Color strokeColor = AvoGUI::Color(getThemeColor("on background"), m_isEnabled ? 1.f : 0.5f);
	p_context->setColor(strokeColor);

	p_context->moveOrigin((getWidth() - getHeight()) * 0.5f + BUTTON_ICON_PADDING, BUTTON_ICON_PADDING);
	float width = getHeight() - BUTTON_ICON_PADDING*2.f;
	switch (m_icon)
	{
	case Icon::Minimize:
	{
		p_context->drawLine(0.f, width * 0.5f, width, width * 0.5f);
		break;
	}
	case Icon::Maximize:
	{
		p_context->strokeRectangle(0.f, 0.f, width, width);
		break;
	}
	case Icon::Restore:
	{
		p_context->strokeRectangle(2.f, 0.f, width, width - 2.f);
		p_context->setColor(m_backgroundColor);
		p_context->fillRectangle(0.f, 2.f, width - 2.f, width);
		p_context->setColor(strokeColor);
		p_context->strokeRectangle(0.f, 2.f, width - 2.f, width);
		break;
	}
	case Icon::Close:
	{
		p_context->drawLine(0.f, 0.f, width, width);
		p_context->drawLine(0.f, width, width, 0.f);
		break;
	}
	}
}

//------------------------------
// class TitleBar
//------------------------------

TitleBar::TitleBar(AvoGUI::Gui* p_parent) :
	View(p_parent, AvoGUI::Rectangle<float>(0, 0, 0, HEIGHT)), 
	m_title(0), m_minimizeButton(0), m_maximizeButton(0), m_closeButton(0), m_isMaximizeEnabled(false)
{
	p_parent->addWindowListener(this);

	m_isMaximizeEnabled = bool(getGui()->getWindow()->getStyles() & AvoGUI::WindowStyleFlags::MaximizeButton);

	m_title = p_parent->getDrawingContext()->createText(getGui()->getWindow()->getTitle().c_str(), 11.f);
	m_title->setFontWeight(AvoGUI::FontWeight::SemiBold);
	m_title->setCharacterSpacing(0.5f);
	m_title->setIsTopTrimmed(true);
	m_title->fitHeightToText();
	m_title->moveY(1.f);
	m_title->setCenterY(HEIGHT*0.5f);

	if (!p_parent->getParent())
	{
		m_minimizeButton = new TitleBarWindowButton(this, TitleBarWindowButton::Icon::Minimize, false);
		m_minimizeButton->setCenterY(HEIGHT * 0.5f);

		m_maximizeButton = new TitleBarWindowButton(this, TitleBarWindowButton::Icon::Maximize, false, m_isMaximizeEnabled);
		m_maximizeButton->setCenterY(HEIGHT * 0.5f);
	}

	m_closeButton = new TitleBarWindowButton(this, TitleBarWindowButton::Icon::Close, true);
	m_closeButton->setCenterY(HEIGHT * 0.5f);
}

//------------------------------

void TitleBar::handleWindowMaximize(AvoGUI::WindowEvent const& p_event)
{
	if (m_maximizeButton)
	{
		m_maximizeButton->setIcon(TitleBarWindowButton::Icon::Restore);
	}
}
void TitleBar::handleWindowRestore(AvoGUI::WindowEvent const& p_event)
{
	if (m_maximizeButton)
	{
		m_maximizeButton->setIcon(TitleBarWindowButton::Icon::Maximize);
	}
}

void TitleBar::handleSizeChange()
{
	m_title->setCenterX(getCenterX());

	m_closeButton->setRight(getWidth());
	if (m_maximizeButton)
	{
		m_maximizeButton->setRight(m_closeButton->getLeft());
	}
	if (m_minimizeButton)
	{
		m_minimizeButton->setRight(m_maximizeButton->getLeft());
	}
}
