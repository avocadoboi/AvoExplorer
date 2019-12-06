#include "TitleBar.hpp"

#include "../utilities.hpp"

//------------------------------

float constexpr HEIGHT = 3				* 8.f;
float constexpr BUTTON_ICON_SIZE = 2	* 8.f;
float constexpr BUTTON_WIDTH_FACTOR = 1.2f;

//------------------------------
// class TitleBarWindowButton
//------------------------------

TitleBarWindowButton::TitleBarWindowButton(TitleBar* p_parent, AvoGUI::Image* p_icon, bool p_isCloseButton, bool p_isEnabled) :
	View(p_parent), m_titleBar(p_parent),
	m_icon(p_icon), m_ripple(0),
	m_hoverAnimationTime(0.f), m_isHovering(false), m_isEnabled(p_isEnabled)
{
	setSize(BUTTON_WIDTH_FACTOR * HEIGHT, HEIGHT);
	m_icon->setSize(BUTTON_ICON_SIZE);
	m_icon->setCenter(getSize()*0.5f);
	if (p_isEnabled)
	{
		m_icon->setOpacity(0.9f);
	}
	else
	{
		m_icon->setOpacity(0.5f);
	}

	setThemeColor("background", p_isCloseButton ? Colors::titleBarCloseButton : AvoGUI::Color(getThemeColor("on background"), 0.2f));
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

void TitleBarWindowButton::setIcon(AvoGUI::Image* p_icon)
{
	if (m_icon)
	{
		m_icon->forget();
	}
	m_icon = p_icon;
	m_icon->setSize(BUTTON_ICON_SIZE);
	m_icon->setCenter(getSize()*0.5f);
	m_icon->setOpacity(0.9f);
}

//------------------------------
// class TitleBar
//------------------------------

TitleBar::TitleBar(AvoGUI::GUI* p_parent) :
	View(p_parent, AvoGUI::Rectangle<float>(0, 0, 0, HEIGHT)), 
	m_title(0), m_minimizeButton(0), m_maximizeButton(0), m_closeButton(0), m_isMaximizeEnabled(false)
{
	p_parent->addWindowListener(this);

	m_isMaximizeEnabled = bool(getGUI()->getWindow()->getStyles() & AvoGUI::WindowStyleFlags::ResizeBorder);

	m_title = p_parent->getDrawingContext()->createText(getGUI()->getWindow()->getTitle().c_str(), 11.f);
	m_title->setFontWeight(AvoGUI::FontWeight::SemiBold);
	m_title->setCharacterSpacing(0.5f);
	m_title->setIsTopTrimmed(true);
	m_title->fitHeightToText();
	m_title->moveY(1.f);
	m_title->setCenterY(HEIGHT*0.5f);

	if (!p_parent->getParent())
	{
		m_minimizeButton = new TitleBarWindowButton(this, loadImageFromResource(RESOURCE_ICON_MINIMIZE, getGUI()->getDrawingContext()), false);
		m_minimizeButton->setCenterY(HEIGHT * 0.5f);

		m_maximizeButton = new TitleBarWindowButton(this, loadImageFromResource(RESOURCE_ICON_MAXIMIZE, getGUI()->getDrawingContext()), false, m_isMaximizeEnabled);
		m_maximizeButton->setCenterY(HEIGHT * 0.5f);
	}

	m_closeButton = new TitleBarWindowButton(this, loadImageFromResource(RESOURCE_ICON_CLOSE, getGUI()->getDrawingContext()), true);
	m_closeButton->setCenterY(HEIGHT * 0.5f);
}

//------------------------------

void TitleBar::handleWindowMaximize(AvoGUI::WindowEvent const& p_event)
{
	if (m_maximizeButton)
	{
		m_maximizeButton->setIcon(loadImageFromResource(RESOURCE_ICON_RESTORE, getGUI()->getDrawingContext()));
	}
}
void TitleBar::handleWindowRestore(AvoGUI::WindowEvent const& p_event)
{
	if (m_maximizeButton)
	{
		m_maximizeButton->setIcon(loadImageFromResource(RESOURCE_ICON_MAXIMIZE, getGUI()->getDrawingContext()));
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
