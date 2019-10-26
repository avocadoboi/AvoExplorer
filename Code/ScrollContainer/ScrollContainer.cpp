#include "ScrollContainer.hpp"

//------------------------------

float const SCROLLBAR_OPACITY_NORMAL = 0.3f;
float const SCROLLBAR_OPACITY_HOVERING = 0.6f;

float const SCROLLBAR_WIDTH = 2		* 8.f;
float const SCROLLBAR_MARGIN = 1	* 8.f;

float const SCROLL_STEP_SIZE = 4	* 8.f;

//------------------------------

Scrollbar::Scrollbar(ScrollContainer* p_parent, Orientation p_orientation) :
	AvoGUI::View(p_parent), m_scrollContainer(p_parent),
	m_orientation(p_orientation), m_isDragging(false), 
	m_isHovering(false), m_opacityAnimationTime(0.f), m_opacity(SCROLLBAR_OPACITY_NORMAL)
{
	if (p_orientation == Orientation::Vertical)
	{
		setWidth(SCROLLBAR_WIDTH);
	}
	else
	{
		setHeight(SCROLLBAR_WIDTH);
	}
	enableMouseEvents();
}

void Scrollbar::handleMouseEnter(AvoGUI::MouseEvent const& p_event)
{
	m_isHovering = true;
	m_opacityAnimationTime = 0.f;
	queueAnimationUpdate();
}
void Scrollbar::handleMouseLeave(AvoGUI::MouseEvent const& p_event)
{
	if (!m_isDragging)
	{
		m_isHovering = false;
		m_opacityAnimationTime = 1.f;
		queueAnimationUpdate();
	}
}
void Scrollbar::handleMouseUp(AvoGUI::MouseEvent const& p_event)
{
	if (m_isDragging && !getIsContaining(p_event.x, p_event.y))
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
			m_scrollContainer->moveVerticalScrollbar(p_event.movementY);
		}
		else
		{
			m_scrollContainer->moveHorizontalScrollbar(p_event.movementX);
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
			queueAnimationUpdate();
		}
	}
	else
	{
		if (m_opacityAnimationTime > 0.f)
		{
			m_opacity = AvoGUI::interpolate(SCROLLBAR_OPACITY_NORMAL, SCROLLBAR_OPACITY_HOVERING, m_scrollContainer->getThemeEasing("in out").easeValue(m_opacityAnimationTime -= 0.1f));
			queueAnimationUpdate();
		}
	}
	invalidate();
}

//
// class ScrollContainer
//

ScrollContainer::ScrollContainer(AvoGUI::View* p_parent, AvoGUI::Rectangle<float> const& p_bounds) :
	AvoGUI::View(p_parent, p_bounds),
	m_horizontalScrollbar(0), m_verticalScrollbar(0), m_content(0)
{
	enableMouseEvents();

	m_content = new AvoGUI::View(this);
	m_content->addViewListener(this);

	m_horizontalScrollbar = new Scrollbar(this, Scrollbar::Orientation::Horizontal);
	m_verticalScrollbar = new Scrollbar(this, Scrollbar::Orientation::Vertical);
}

void ScrollContainer::handleMouseScroll(AvoGUI::MouseEvent const& p_event) 
{
	if (m_verticalScrollbar && m_horizontalScrollbar)
	{
		if (p_event.modifierKeys & AvoGUI::ModifierKeyFlags::Shift)
		{
			setHorizontalScrollPosition(-m_content->getLeft() + p_event.scrollDelta * SCROLL_STEP_SIZE);
			invalidate();
		}
		else
		{
			setVerticalScrollPosition(-m_content->getTop() + p_event.scrollDelta * SCROLL_STEP_SIZE);
			invalidate();
		}
	}
	else if (m_horizontalScrollbar)
	{
		setHorizontalScrollPosition(-m_content->getLeft() + p_event.scrollDelta * SCROLL_STEP_SIZE);
		invalidate();
	}
	else if (m_verticalScrollbar)
	{
		setVerticalScrollPosition(-m_content->getTop() + p_event.scrollDelta * SCROLL_STEP_SIZE);
		invalidate();
	}
}

void ScrollContainer::handleSizeChange()
{
	m_verticalScrollbar->setRight(getWidth() - SCROLLBAR_MARGIN - m_verticalScrollbar->getWidth());
	m_horizontalScrollbar->setBottom(getHeight() - SCROLLBAR_MARGIN - m_horizontalScrollbar->getHeight());
	updateScrollbars();
	invalidate();
}
