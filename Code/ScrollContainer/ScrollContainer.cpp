#include "ScrollContainer.hpp"

//------------------------------

float const SCROLLBAR_OPACITY_NORMAL = 0.3f;
float const SCROLLBAR_OPACITY_HOVERING = 0.6f;

float const SCROLLBAR_WIDTH = 1		* 8.f;
float const SCROLLBAR_MARGIN = 1	* 8.f;

//------------------------------

Scrollbar::Scrollbar(ScrollContainer* p_parent, Orientation p_orientation) :
	m_scrollContainer(p_parent),
	m_orientation(Orientation::Vertical), m_isDragging(false), 
	m_isHovering(false), m_opacityAnimationTime(0.f), m_opacity(SCROLLBAR_OPACITY_NORMAL),
	isEnabled(false)
{
	if (p_orientation == Orientation::Vertical)
	{
		bounds.setWidth(SCROLLBAR_WIDTH);
	}
	else
	{
		bounds.setHeight(SCROLLBAR_WIDTH);
	}
}

void Scrollbar::handleMouseEnter(AvoGUI::MouseEvent const& p_event)
{
	m_isHovering = true;
	m_opacityAnimationTime = 0.f;
	m_scrollContainer->queueAnimationUpdate();
}
void Scrollbar::handleMouseLeave(AvoGUI::MouseEvent const& p_event)
{
	if (!m_isDragging)
	{
		m_isHovering = false;
		m_opacityAnimationTime = 1.f;
		m_scrollContainer->queueAnimationUpdate();
	}
}
void Scrollbar::handleMouseUp(AvoGUI::MouseEvent const& p_event)
{
	if (m_isDragging && !bounds.getIsContaining(p_event.x, p_event.y))
	{
		m_isHovering = false;
		m_opacityAnimationTime = 1.f;
		m_scrollContainer->queueAnimationUpdate();
	}
	m_isDragging = false;
}
void Scrollbar::handleMouseMove(AvoGUI::MouseEvent const& p_event)
{
	if (m_isDragging)
	{
		if (m_orientation == Orientation::Vertical)
		{
			m_scrollContainer->setVerticalScrollbarPosition(bounds.top + p_event.movementY);
		}
		else
		{
			m_scrollContainer->setHorizontalScrollbarPosition(bounds.left + p_event.movementX);
		}
		m_scrollContainer->invalidate();
	}
}

void Scrollbar::updateAnimations()
{
	if (m_isHovering)
	{
		if (m_opacityAnimationTime < 1.f)
		{
			m_opacity = AvoGUI::interpolate(SCROLLBAR_OPACITY_NORMAL, SCROLLBAR_OPACITY_HOVERING, m_scrollContainer->getThemeEasing("in out").easeValue(m_opacityAnimationTime += 0.1f));
			m_scrollContainer->queueAnimationUpdate();
			m_scrollContainer->getGUI()->invalidateRectangle(bounds.createMovedCopy(m_scrollContainer->getAbsoluteTopLeft()));
		}
	}
	else
	{
		if (m_opacityAnimationTime > 0.f)
		{
			m_opacity = AvoGUI::interpolate(SCROLLBAR_OPACITY_NORMAL, SCROLLBAR_OPACITY_HOVERING, m_scrollContainer->getThemeEasing("in out").easeValue(m_opacityAnimationTime -= 0.1f));
			m_scrollContainer->queueAnimationUpdate();
			m_scrollContainer->getGUI()->invalidateRectangle(bounds.createMovedCopy(m_scrollContainer->getAbsoluteTopLeft()));
		}
	}
}

//
// class ScrollContainer
//

ScrollContainer::ScrollContainer(AvoGUI::View* p_parent, AvoGUI::Rectangle<float> const& p_bounds) :
	AvoGUI::View(p_parent, p_bounds)
{
	enableMouseEvents();

	m_horizontalScrollbar = new Scrollbar(this, Scrollbar::Orientation::Horizontal);
	m_verticalScrollbar = new Scrollbar(this, Scrollbar::Orientation::Vertical);
}

void ScrollContainer::handleSizeChange()
{
	m_verticalScrollbar->bounds.setRight(getWidth() - SCROLLBAR_MARGIN - m_verticalScrollbar->bounds.getWidth());
	m_horizontalScrollbar->bounds.setBottom(getHeight() - SCROLLBAR_MARGIN - m_horizontalScrollbar->bounds.getHeight());
	updateScrollbars();
	invalidate();
}
