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

	void handleMouseEnter(AvoGUI::MouseEvent const& p_event) override
	{
		m_isHovering = true;
		m_opacityAnimationTime = 0.f;
		queueAnimationUpdate();
	}
	void handleMouseLeave(AvoGUI::MouseEvent const& p_event) override
	{
		m_isHovering = false;
		m_opacityAnimationTime = 1.f;
	}
	void handleMouseDown(AvoGUI::MouseEvent const& p_event) override
	{
		m_isDragging = true;
	}
	void handleMouseUp(AvoGUI::MouseEvent const& p_event) override;
	void handleMouseMove(AvoGUI::MouseEvent const& p_event) override;

	//------------------------------

	void updateAnimations() override;

	void draw(AvoGUI::DrawingContext* p_context) override
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
	AvoGUI::Point<float> m_contentSize;
	AvoGUI::Point<float> m_scrollPosition;

	Scrollbar* m_horizontalScrollbar;
	Scrollbar* m_verticalScrollbar;

public:
	ScrollContainer(AvoGUI::View* p_parent, AvoGUI::Rectangle<float> const& p_bounds = AvoGUI::Rectangle<float>());

	//------------------------------

	void handleMouseScroll(AvoGUI::MouseEvent const& p_event) override;

	//------------------------------

	void handleChildAttachment(AvoGUI::View* p_child) override
	{
		if (p_child != m_horizontalScrollbar && p_child != m_verticalScrollbar)
		{
			if (p_child->getRight() > m_contentSize.x)
			{
				m_contentSize.x = p_child->getRight();
			}
			if (p_child->getBottom() > m_contentSize.y)
			{
				m_contentSize.y = p_child->getBottom();
			}
		}
	}
	void handleChildDetachment(AvoGUI::View* p_child) override
	{
		if (p_child != m_horizontalScrollbar && p_child != m_verticalScrollbar)
		{
			if (p_child->getRight() == m_contentSize.x)
			{
				m_contentSize.x = calculateContentRight();
			}
			if (p_child->getBottom() == m_contentSize.y)
			{
				m_contentSize.y = calculateContentBottom();
			}
		}
	}
	void handleViewBoundsChange(AvoGUI::View* p_view, AvoGUI::Rectangle<float> const& p_previousBounds) override
	{
		if (p_view == m_horizontalScrollbar || p_view == m_verticalScrollbar)
		{

		}
		else
		{
			if (p_view->getRight() > m_contentSize.x)
			{
				m_contentSize.x = p_view->getRight();
			}
			else if (p_previousBounds.right == m_contentSize.x && p_view->getRight() < m_contentSize.x)
			{
				m_contentSize.x = calculateContentRight();
			}
			if (p_view->getBottom() > m_contentSize.y)
			{
				m_contentSize.y = p_view->getBottom();
			}
			else if (p_previousBounds.bottom == m_contentSize.y && p_view->getBottom() < m_contentSize.y)
			{
				m_contentSize.y = calculateContentBottom();
			}
		}
	}
	void handleSizeChange() override
	{
		invalidate();
	}

	void drawOverlay(AvoGUI::DrawingContext* p_context)
	{
		if (m_contentSize.y > getHeight())
		{
		}
	}
};