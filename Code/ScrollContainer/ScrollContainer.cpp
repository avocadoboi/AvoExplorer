#include "ScrollContainer.hpp"

//------------------------------

float constexpr SCROLLBAR_OPACITY_NORMAL = 0.3f;
float constexpr SCROLLBAR_OPACITY_HOVERING = 0.6f;

float constexpr SCROLLBAR_WIDTH = 0.5	* 8.f;
float constexpr SCROLLBAR_MARGIN = 0.5	* 8.f;

float constexpr SCROLL_STEP_SIZE = 4	* 8.f;

//------------------------------
// class Scrollbar
//------------------------------

Scrollbar::Scrollbar(ScrollContainer* p_parent, Orientation p_orientation) :
	AvoGUI::View(p_parent), m_scrollContainer(p_parent),
	m_orientation(p_orientation), m_isDragging(false), m_dragAnchorPosition(0.f),
	m_isHovering(false), m_opacityAnimationTime(0.f), m_opacity(SCROLLBAR_OPACITY_NORMAL)
{
	setCornerRadius(3.f);

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
		queueAnimationUpdate();
	}
	m_isDragging = false;
}
void Scrollbar::handleMouseMove(AvoGUI::MouseEvent const& p_event)
{
	if (m_isDragging)
	{
		if (m_orientation == Orientation::Vertical)
		{
			m_scrollContainer->moveVerticalScrollbar(p_event.y - m_dragAnchorPosition);
		}
		else
		{
			m_scrollContainer->moveHorizontalScrollbar(p_event.x - m_dragAnchorPosition);
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
			m_opacity = AvoGUI::interpolate(
				SCROLLBAR_OPACITY_NORMAL, SCROLLBAR_OPACITY_HOVERING, 
				m_scrollContainer->getThemeEasing("in out").easeValue(m_opacityAnimationTime += getThemeValue("hover animation speed"))
			);
			queueAnimationUpdate();
		}
		else
		{
			m_opacityAnimationTime = 1.f;
		}
	}
	else
	{
		if (m_opacityAnimationTime > 0.f)
		{
			m_opacity = AvoGUI::interpolate(
				SCROLLBAR_OPACITY_NORMAL, SCROLLBAR_OPACITY_HOVERING, 
				m_scrollContainer->getThemeEasing("in out").easeValue(m_opacityAnimationTime -= getThemeValue("hover animation speed"))
			);
			queueAnimationUpdate();
		}
		else
		{
			m_opacityAnimationTime = 0.f;
		}
	}
	invalidate();
}

//------------------------------
// class ScrollContainer
//------------------------------

//
// Private
//

void ScrollContainer::updateScrollbars()
{
	m_verticalScrollbar->setIsVisible(m_content->getHeight() > getHeight());
	if (m_content->getHeight() > getHeight())
	{
		m_verticalScrollbar->setHeight((getHeight() - 2.f* m_scrollbarMargin) * getHeight() / m_content->getHeight());
	}

	m_horizontalScrollbar->setIsVisible(m_content->getWidth() > getWidth());
	if (m_content->getWidth() > getWidth())
	{
		m_horizontalScrollbar->setWidth((getWidth() - 2.f* m_scrollbarMargin) * getWidth() / m_content->getWidth());
	}
}

//
// Public
//

ScrollContainer::ScrollContainer(AvoGUI::View* p_parent, AvoGUI::Rectangle<float> const& p_bounds) :
	AvoGUI::View(p_parent, p_bounds),
	m_horizontalScrollbar(0), m_verticalScrollbar(0), m_scrollbarMargin(SCROLLBAR_MARGIN),
	m_content(0)
{
	enableMouseEvents();

	m_content = new AvoGUI::View(this);
	m_content->addViewListener(this);

	m_horizontalScrollbar = new Scrollbar(this, Scrollbar::Orientation::Horizontal);
	m_verticalScrollbar = new Scrollbar(this, Scrollbar::Orientation::Vertical);
}

//------------------------------

void ScrollContainer::setScrollbarMargin(float p_margin)
{
	m_scrollbarMargin = p_margin;
	handleSizeChange();
}

//------------------------------

void ScrollContainer::setHorizontalScrollPosition(float p_position)
{
	if (m_horizontalScrollbar->getIsVisible())
	{
		p_position = AvoGUI::max(0.f, AvoGUI::min(m_content->getWidth() - getWidth(), p_position));

		m_content->setLeft(-p_position);
		m_horizontalScrollbar->setLeft(m_scrollbarMargin + p_position * (getWidth() - m_horizontalScrollbar->getWidth() - 2.f* m_scrollbarMargin) / (m_content->getWidth() - getWidth()));
	}
	else
	{
		p_position = 0.f;
		m_content->setLeft(0.f);
		m_horizontalScrollbar->setLeft(m_scrollbarMargin);
	}
}
void ScrollContainer::setVerticalScrollPosition(float p_position)
{
	if (m_verticalScrollbar->getIsVisible())
	{
		p_position = AvoGUI::max(0.f, AvoGUI::min(m_content->getHeight() - getHeight(), p_position));

		m_content->setTop(-p_position);
		m_verticalScrollbar->setTop(m_scrollbarMargin + p_position * (getHeight() - m_verticalScrollbar->getHeight() - 2.f * m_scrollbarMargin) / (m_content->getHeight() - getHeight()));
	}
	else
	{
		p_position = 0.f;
		m_content->setTop(0.f);
		m_verticalScrollbar->setTop(m_scrollbarMargin);
	}
}

//------------------------------

void ScrollContainer::moveHorizontalScrollbar(float p_offset)
{
	setHorizontalScrollPosition((m_horizontalScrollbar->getLeft() - m_scrollbarMargin + p_offset) * m_content->getWidth() / (getWidth() - 2.f * SCROLLBAR_MARGIN));
}
void ScrollContainer::moveVerticalScrollbar(float p_offset)
{
	setVerticalScrollPosition((m_verticalScrollbar->getTop() - m_scrollbarMargin + p_offset) * m_content->getHeight() / (getHeight() - 2.f * SCROLLBAR_MARGIN));
}

//------------------------------

void ScrollContainer::handleMouseScroll(AvoGUI::MouseEvent const& p_event)
{
	if (m_verticalScrollbar && m_horizontalScrollbar)
	{
		if (p_event.modifierKeys & AvoGUI::ModifierKeyFlags::Shift)
		{
			setHorizontalScrollPosition(-m_content->getLeft() - p_event.scrollDelta * SCROLL_STEP_SIZE);
			invalidate();
		}
		else
		{
			setVerticalScrollPosition(-m_content->getTop() - p_event.scrollDelta * SCROLL_STEP_SIZE);
			invalidate();
		}
	}
	else if (m_horizontalScrollbar)
	{
		setHorizontalScrollPosition(-m_content->getLeft() - p_event.scrollDelta * SCROLL_STEP_SIZE);
		invalidate();
	}
	else if (m_verticalScrollbar)
	{
		setVerticalScrollPosition(-m_content->getTop() - p_event.scrollDelta * SCROLL_STEP_SIZE);
		invalidate();
	}
}

void ScrollContainer::handleSizeChange()
{
	m_verticalScrollbar->setRight(getWidth() - m_scrollbarMargin);
	m_horizontalScrollbar->setBottom(getHeight() - m_scrollbarMargin);
	updateScrollbars();
	setVerticalScrollPosition(-m_content->getTop());
	setHorizontalScrollPosition(-m_content->getLeft());
	invalidate();
}
