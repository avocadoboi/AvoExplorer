#pragma once

#include <AvoGUI.hpp>
#include "../Resources/Colors.hpp"
#include "../utilities.hpp"
#include "../Resources/ThemeValues.hpp"

#undef min
#undef max

//------------------------------

class Scrollbar :
	public Avo::View
{
public:
	enum class Orientation
	{
		Horizontal,
		Vertical
	};

	static constexpr float SCROLLBAR_OPACITY_NORMAL = 0.3f;
	static constexpr float SCROLLBAR_OPACITY_HOVERING = 0.6f;
	static constexpr float SCROLLBAR_WIDTH = 0.5 * 8.f;

private:
	Orientation m_orientation;

	bool m_isDragging{ false };
	float m_dragAnchorPosition{ 0.f };

public:
	void handleMouseEnter(Avo::MouseEvent const& p_event) override
	{
		m_isHovering = true;
		m_opacityAnimationTime = 0.f;
		queueAnimationUpdate();
	}
	void handleMouseLeave(Avo::MouseEvent const& p_event) override
	{
		if (!m_isDragging)
		{
			m_isHovering = false;
			m_opacityAnimationTime = 1.f;
			queueAnimationUpdate();
		}
	}
	void handleMouseDown(Avo::MouseEvent const& p_event) override
	{
		m_isDragging = true;
		m_dragAnchorPosition = m_orientation == Orientation::Vertical ? p_event.y : p_event.x;
	}
	void handleMouseUp(Avo::MouseEvent const& p_event) override
	{
		m_isDragging = false;
	}
	inline void handleMouseMove(Avo::MouseEvent const& p_event) override;

	//------------------------------

private:
	bool m_isHovering{ false };
	float m_opacityAnimationTime{ 0.f };
	float m_opacity{ SCROLLBAR_OPACITY_NORMAL };

public:
	void updateAnimations()
	{
		if (m_isHovering)
		{
			if (m_opacityAnimationTime < 1.f)
			{
				m_opacity = Avo::interpolate(
					SCROLLBAR_OPACITY_NORMAL, SCROLLBAR_OPACITY_HOVERING,
					getThemeEasing(ThemeEasings::inOut).easeValue(m_opacityAnimationTime += getThemeValue(ThemeValues::hoverAnimationSpeed))
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
				m_opacity = Avo::interpolate(
					SCROLLBAR_OPACITY_NORMAL, SCROLLBAR_OPACITY_HOVERING,
					getThemeEasing(ThemeEasings::inOut).easeValue(m_opacityAnimationTime -= getThemeValue(ThemeValues::hoverAnimationSpeed))
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

	void draw(Avo::DrawingContext* p_context)
	{
		p_context->setColor(Avo::Color(Colors::scrollbar, m_opacity));
		p_context->fillRectangle(getSize());
	}

	//------------------------------

	Scrollbar(Avo::View* p_parent, Orientation p_orientation) :
		Avo::View(p_parent),
		m_orientation(p_orientation)
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
};

//------------------------------

class ScrollContainer :
	public Avo::View
{
public:
	static constexpr float SCROLLBAR_MIN_LENGTH = 2 * 8.f;
	static constexpr float SCROLLBAR_MARGIN = 0.5 * 8.f;
	static constexpr float SCROLL_STEP_SIZE = 4 * 8.f;

private:
	Avo::View* m_content{ new Avo::View(this) };
public:
	void setContentView(Avo::View* p_content)
	{
		if (m_content)
		{
			detachContentListener();
			m_content->setParent(0);
			m_content->forget();
		}

		m_content = p_content;
		attachContentListener();
		m_content->setParent(this);
	}
	View* getContent()
	{
		return m_content;
	}

	//------------------------------

private:
	float m_scrollbarMargin{ SCROLLBAR_MARGIN };
public:
	void setScrollbarMargin(float p_margin)
	{
		m_scrollbarMargin = p_margin;
		handleSizeChange();
	}

	//------------------------------

private:
	Scrollbar* m_horizontalScrollbar{ new Scrollbar(this, Scrollbar::Orientation::Horizontal) };
	Scrollbar* m_verticalScrollbar{ new Scrollbar(this, Scrollbar::Orientation::Vertical) };

	void updateScrollbars()
	{
		m_verticalScrollbar->setIsVisible(m_content->getHeight() > getHeight());
		if (m_content->getHeight() > getHeight())
		{
			m_verticalScrollbar->setHeight(Avo::max((getHeight() - 2.f * m_scrollbarMargin) * getHeight() / m_content->getHeight(), SCROLLBAR_MIN_LENGTH));
		}

		m_horizontalScrollbar->setIsVisible(m_content->getWidth() > getWidth());
		if (m_content->getWidth() > getWidth())
		{
			m_horizontalScrollbar->setWidth(Avo::max((getWidth() - 2.f * m_scrollbarMargin) * getWidth() / m_content->getWidth(), SCROLLBAR_MIN_LENGTH));
		}
	}

public:
	void setScrollPosition(Avo::Point<float> const& p_position)
	{
		setScrollPosition(p_position.x, p_position.y);
	}
	void setScrollPosition(float p_positionX, float p_positionY)
	{
		setHorizontalScrollPosition(p_positionX);
		setVerticalScrollPosition(p_positionY);
	}
	void setHorizontalScrollPosition(float p_position)
	{
		if (m_horizontalScrollbar->getIsVisible())
		{
			p_position = Avo::max(0.f, Avo::min(m_content->getWidth() - getWidth(), p_position));

			m_content->setLeft(-p_position);
			m_horizontalScrollbar->setLeft(m_scrollbarMargin + p_position * (getWidth() - m_horizontalScrollbar->getWidth() - 2.f * m_scrollbarMargin) / (m_content->getWidth() - getWidth()));
		}
		else
		{
			p_position = 0.f;
			m_content->setLeft(0.f);
			m_horizontalScrollbar->setLeft(m_scrollbarMargin);
		}
	}
	void setVerticalScrollPosition(float p_position)
	{
		if (m_verticalScrollbar->getIsVisible())
		{
			p_position = Avo::max(0.f, Avo::min(m_content->getHeight() - getHeight(), p_position));

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

	Avo::Point<float> const& getScrollPosition()
	{
		return -m_content->getTopLeft();
	}
	float getScrollPositionX()
	{
		return -m_content->getLeft();
	}
	float getScrollPositionY()
	{
		return -m_content->getTop();
	}

	//------------------------------

	void moveHorizontalScrollbar(float p_offset)
	{
		setHorizontalScrollPosition((m_horizontalScrollbar->getLeft() - m_scrollbarMargin + p_offset) * (m_content->getWidth() - getWidth()) / (getWidth() - 2.f * m_scrollbarMargin - m_horizontalScrollbar->getWidth()));
	}
	void moveVerticalScrollbar(float p_offset)
	{
		setVerticalScrollPosition((m_verticalScrollbar->getTop() - m_scrollbarMargin + p_offset) * (m_content->getHeight() - getHeight()) / (getHeight() - 2.f * m_scrollbarMargin - m_verticalScrollbar->getHeight()));
	}

	//------------------------------

	void handleMouseScroll(Avo::MouseEvent const& p_event) override
	{
		if (m_verticalScrollbar && m_horizontalScrollbar)
		{
			if (p_event.modifierKeys & Avo::ModifierKeyFlags::Shift)
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

	//------------------------------

private:
	void attachContentListener()
	{
		m_content->sizeChangeListeners += Avo::bind(&ScrollContainer::handleContentSizeChange, this);
	}
	void detachContentListener()
	{
		m_content->sizeChangeListeners -= Avo::bind(&ScrollContainer::handleContentSizeChange, this);
	}

public:
	void handleContentSizeChange(float p_previousWidth, float p_previousHeight)
	{
		updateScrollbars();
		if (p_previousHeight)
		{
			setVerticalScrollPosition(-m_content->getTop()/p_previousHeight * m_content->getHeight());
		}
		else
		{
			setVerticalScrollPosition(0.f);
		}
		if (p_previousWidth)
		{
			setHorizontalScrollPosition(-m_content->getLeft()/p_previousWidth * m_content->getWidth());
		}
		else
		{
			setHorizontalScrollPosition(0.f);
		}
		invalidate();
	}
	void handleSizeChange() override
	{
		m_verticalScrollbar->setRight(getWidth() - m_scrollbarMargin);
		m_horizontalScrollbar->setBottom(getHeight() - m_scrollbarMargin);
		updateScrollbars();
		setVerticalScrollPosition(-m_content->getTop());
		setHorizontalScrollPosition(-m_content->getLeft());
		invalidate();
	}

	void draw(Avo::DrawingContext* p_context) override
	{
		p_context->setColor(getThemeColor(Avo::ThemeColors::background));
		p_context->fillRectangle(getSize());
	}

	//------------------------------

	ScrollContainer(Avo::View* p_parent, Avo::Rectangle<float> const& p_bounds = Avo::Rectangle<float>()) :
		Avo::View(p_parent, p_bounds)
	{
		enableMouseEvents();
		attachContentListener();
	}
};

//------------------------------

void Scrollbar::handleMouseMove(Avo::MouseEvent const& p_event)
{
	if (m_isDragging)
	{
		if (m_orientation == Orientation::Vertical)
		{
			getParent<ScrollContainer>()->moveVerticalScrollbar(p_event.y - m_dragAnchorPosition);
		}
		else
		{
			getParent<ScrollContainer>()->moveHorizontalScrollbar(p_event.x - m_dragAnchorPosition);
		}
		getParent<ScrollContainer>()->invalidate();
	}
}
