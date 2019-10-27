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
	float m_dragAnchorPosition;

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
		m_dragAnchorPosition = m_orientation == Orientation::Vertical ? p_event.y : p_event.x;
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

	void updateScrollbars();

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
	void setHorizontalScrollPosition(float p_position);
	void setVerticalScrollPosition(float p_position);

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

	void moveHorizontalScrollbar(float p_offset);
	void moveVerticalScrollbar(float p_offset);

	//------------------------------

	void handleMouseScroll(AvoGUI::MouseEvent const& p_event) override;

	//------------------------------

	void handleViewSizeChange(AvoGUI::View* p_view, float p_previousWidth, float p_previousHeight) override
	{
		updateScrollbars();
		setVerticalScrollPosition(-m_content->getTop());
		setHorizontalScrollPosition(-m_content->getLeft());
		invalidate();
	}
	void handleSizeChange() override;

	//------------------------------

	void draw(AvoGUI::DrawingContext* p_context) override
	{
		p_context->setColor(getThemeColor("background"));
		p_context->fillRectangle(getSize());
	}
};