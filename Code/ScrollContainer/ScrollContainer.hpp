#pragma once

#include <AvoGUI.hpp>
#include "../../Resources/Colors.hpp"

//------------------------------

class ScrollContainer;

class Scrollbar :
	public AvoGUI::View
{
public:
	enum class Orientation
	{
		Horizontal,
		Vertical
	};

private:
	ScrollContainer* m_scrollContainer;

	Orientation m_orientation;
	bool m_isDragging;

	bool m_isHovering;
	float m_opacityAnimationTime;
	float m_opacity;

public:
	Scrollbar(ScrollContainer* p_parent, Orientation p_orientation);

	//------------------------------

	void handleMouseEnter(AvoGUI::MouseEvent const& p_event) override;
	void handleMouseLeave(AvoGUI::MouseEvent const& p_event) override;
	void handleMouseDown(AvoGUI::MouseEvent const& p_event) override
	{
		m_isDragging = true;
	}
	void handleMouseUp(AvoGUI::MouseEvent const& p_event) override;
	void handleMouseMove(AvoGUI::MouseEvent const& p_event) override;

	//------------------------------

	void updateAnimations();

	void draw(AvoGUI::DrawingContext* p_context)
	{
		p_context->setColor(AvoGUI::Color(Colors::scrollbar, m_opacity));
		p_context->fillRectangle(getSize());
	}
};

//------------------------------

class ScrollContainer :
	public AvoGUI::View,
	public AvoGUI::ViewListener
{
private:
	Scrollbar* m_horizontalScrollbar;
	Scrollbar* m_verticalScrollbar;

	AvoGUI::View* m_content;

	void updateScrollbars()
	{
		m_verticalScrollbar->setIsVisible(m_content->getHeight() > getHeight());
		if (m_content->getHeight() > getHeight())
		{
			m_verticalScrollbar->setHeight(getHeight()*getHeight() / m_content->getHeight());
		}

		m_horizontalScrollbar->setIsVisible(m_content->getWidth() > getWidth());
		if (m_content->getWidth() > getWidth())
		{
			m_horizontalScrollbar->setWidth(getWidth()*getWidth() / m_content->getWidth());
		}
	}

public:
	ScrollContainer(AvoGUI::View* p_parent, AvoGUI::Rectangle<float> const& p_bounds = AvoGUI::Rectangle<float>());

	//------------------------------

	View* getContent()
	{
		return m_content;
	}

	//------------------------------

	void setScrollPosition(AvoGUI::Point<float> const& p_position)
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
			p_position = AvoGUI::max(0.f, AvoGUI::min(m_content->getWidth() - getWidth(), p_position));

			m_content->setLeft(-p_position);
			m_horizontalScrollbar->setLeft(p_position * (getWidth() - m_horizontalScrollbar->getWidth()) / (m_content->getWidth() - getWidth()));
		}
	}
	void setVerticalScrollPosition(float p_position)
	{
		if (m_verticalScrollbar->getIsVisible())
		{
			p_position = AvoGUI::max(0.f, AvoGUI::min(m_content->getHeight() - getHeight(), p_position));

			m_content->setTop(-p_position);
			m_verticalScrollbar->setTop(p_position * (getHeight() - m_verticalScrollbar->getHeight()) / (m_content->getHeight() - getHeight()));
		}
	}

	AvoGUI::Point<float> const& getScrollPosition()
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
		setHorizontalScrollPosition((m_horizontalScrollbar->getLeft() + p_offset) * m_content->getWidth() / getWidth());
	}
	void moveVerticalScrollbar(float p_offset)
	{
		setVerticalScrollPosition((m_verticalScrollbar->getTop() + p_offset) * m_content->getHeight() / getHeight());
	}

	//------------------------------

	void handleMouseScroll(AvoGUI::MouseEvent const& p_event) override;

	//------------------------------

	void handleViewSizeChange(AvoGUI::View* p_view, float p_previousWidth, float p_previousHeight) override
	{
		updateScrollbars();
	}
	void handleSizeChange() override;

	//------------------------------

	void draw(AvoGUI::DrawingContext* p_context) override
	{
		p_context->setColor(getThemeColor("background"));
		p_context->fillRectangle(getSize());
	}
};