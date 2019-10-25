#pragma once

#include <AvoGUI.hpp>
#include "../../Resources/Colors.hpp"

//------------------------------

class ScrollContainer;

class Scrollbar
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
	AvoGUI::Rectangle<float> bounds;
	bool isEnabled;

	//------------------------------

	Scrollbar(ScrollContainer* p_parent, Orientation p_orientation);

	//------------------------------

	void handleMouseEnter(AvoGUI::MouseEvent const& p_event);
	void handleMouseLeave(AvoGUI::MouseEvent const& p_event);
	void handleMouseDown(AvoGUI::MouseEvent const& p_event)
	{
		m_isDragging = true;
	}
	void handleMouseUp(AvoGUI::MouseEvent const& p_event);
	void handleMouseMove(AvoGUI::MouseEvent const& p_event);

	//------------------------------

	void updateAnimations();

	void draw(AvoGUI::DrawingContext* p_context)
	{
		p_context->setColor(AvoGUI::Color(Colors::scrollbar, m_opacity));
		p_context->fillRectangle(bounds);
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

	void updateScrollbars()
	{
		m_verticalScrollbar->isEnabled = m_contentSize.y > getHeight();
		if (m_contentSize.y > getHeight())
		{
			m_verticalScrollbar->bounds.setHeight(getHeight()*getHeight() / m_contentSize.y);
		}

		m_horizontalScrollbar->isEnabled = m_contentSize.x > getWidth();
		if (m_contentSize.x > getWidth())
		{
			m_horizontalScrollbar->bounds.setWidth(getWidth()*getWidth() / m_contentSize.x);
		}
	}

public:
	ScrollContainer(AvoGUI::View* p_parent, AvoGUI::Rectangle<float> const& p_bounds = AvoGUI::Rectangle<float>());

	//------------------------------

	void handleMouseDown(AvoGUI::MouseEvent const& p_event) override
	{
		if (m_verticalScrollbar->isEnabled && m_verticalScrollbar->bounds.getIsContaining(p_event.x, p_event.y))
		{
			m_verticalScrollbar->handleMouseDown(p_event);
		}
		else if (m_horizontalScrollbar->isEnabled && m_horizontalScrollbar->bounds.getIsContaining(p_event.x, p_event.y))
		{
			m_horizontalScrollbar->handleMouseDown(p_event);
		}
	}
	void handleMouseUp(AvoGUI::MouseEvent const& p_event) override
	{
		if (m_verticalScrollbar->isEnabled)
		{
			m_verticalScrollbar->handleMouseUp(p_event);
		}
		if (m_horizontalScrollbar->isEnabled)
		{
			m_horizontalScrollbar->handleMouseUp(p_event);
		}
	}
	void handleMouseMove(AvoGUI::MouseEvent const& p_event) override
	{
		Scrollbar* scrollbar = m_verticalScrollbar;
		for (uint8 i = 0; i < 3; i++)
		{
			if (scrollbar->isEnabled)
			{
				scrollbar->handleMouseMove(p_event);
				if (scrollbar->bounds.getIsContaining(p_event.x, p_event.y))
				{
					if (!scrollbar->bounds.getIsContaining(p_event.x - p_event.movementX, p_event.y - p_event.movementY))
					{
						scrollbar->handleMouseEnter(p_event);
					}
				}
				else if (scrollbar->bounds.getIsContaining(p_event.x - p_event.movementX, p_event.y - p_event.movementY))
				{
					scrollbar->handleMouseLeave(p_event);
				}
			}

			scrollbar = m_horizontalScrollbar;
		}
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
		if (m_horizontalScrollbar->isEnabled)
		{
			p_position = AvoGUI::max(0.f, AvoGUI::min(m_contentSize.x - getWidth(), p_position));

			for (uint32 a = 0; a < m_children.size(); a++)
			{
				m_children[a]->moveX(m_scrollPosition.x - p_position);
			}
			m_horizontalScrollbar->bounds.setLeft(p_position * (getWidth() - m_horizontalScrollbar->bounds.getWidth()) / (m_contentSize.x - getWidth()));

			m_scrollPosition.x = p_position;
		}
	}
	void setVerticalScrollPosition(float p_position)
	{
		if (m_verticalScrollbar->isEnabled)
		{
			p_position = AvoGUI::max(0.f, AvoGUI::min(m_contentSize.y - getHeight(), p_position));

			for (uint32 a = 0; a < m_children.size(); a++)
			{
				m_children[a]->moveY(m_scrollPosition.y - p_position);
			}
			m_verticalScrollbar->bounds.setTop(p_position * (getHeight() - m_verticalScrollbar->bounds.getHeight()) / (m_contentSize.y - getHeight()));

			m_scrollPosition.y = p_position;
		}
	}

	AvoGUI::Point<float> const& getScrollPosition()
	{
		return m_scrollPosition;
	}
	float getScrollPositionX()
	{
		return m_scrollPosition.x;
	}
	float getScrollPositionY()
	{
		return m_scrollPosition.y;
	}

	//------------------------------

	void setHorizontalScrollbarPosition(float p_position)
	{
		setHorizontalScrollPosition(p_position * m_contentSize.x / getWidth());
	}
	void setVerticalScrollbarPosition(float p_position)
	{
		setVerticalScrollPosition(p_position * m_contentSize.y / getHeight());
	}

	//------------------------------

	void handleMouseScroll(AvoGUI::MouseEvent const& p_event) override
	{
		if (m_verticalScrollbar && m_horizontalScrollbar)
		{
			if (p_event.modifierKeys & AvoGUI::ModifierKeyFlags::Shift)
			{
				setHorizontalScrollPosition(m_scrollPosition.x + p_event.scrollDelta);
			}
			else
			{
				setVerticalScrollPosition(m_scrollPosition.y + p_event.scrollDelta);
			}
		}
		else if (m_horizontalScrollbar)
		{
			setHorizontalScrollPosition(m_scrollPosition.x + p_event.scrollDelta);
		}
		else if (m_verticalScrollbar)
		{
			setVerticalScrollPosition(m_scrollPosition.y + p_event.scrollDelta);
		}
	}

	//------------------------------

	void handleChildAttachment(AvoGUI::View* p_child) override
	{
		p_child->addViewListener(this);
		if (p_child->getRight() > m_contentSize.x)
		{
			m_contentSize.x = p_child->getRight();
		}
		if (p_child->getBottom() > m_contentSize.y)
		{
			m_contentSize.y = p_child->getBottom();
		}
		updateScrollbars();
	}
	void handleChildDetachment(AvoGUI::View* p_child) override
	{
		p_child->removeViewListener(this);
		if (p_child->getRight() == m_contentSize.x)
		{
			m_contentSize.x = calculateContentRight();
		}
		if (p_child->getBottom() == m_contentSize.y)
		{
			m_contentSize.y = calculateContentBottom();
		}
		updateScrollbars();
	}
	void handleViewBoundsChange(AvoGUI::View* p_view, AvoGUI::Rectangle<float> const& p_previousBounds) override
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
		updateScrollbars();
	}
	void handleSizeChange() override;

	//------------------------------

	void updateAnimations() override
	{
		if (m_verticalScrollbar->isEnabled)
		{
			m_verticalScrollbar->updateAnimations();
		}
		if (m_horizontalScrollbar->isEnabled)
		{
			m_horizontalScrollbar->updateAnimations();
		}
	}

	void draw(AvoGUI::DrawingContext* p_context) override
	{
		p_context->setColor(getThemeColor("background"));
		p_context->fillRectangle(getSize());

		if (m_verticalScrollbar->isEnabled)
		{
			m_verticalScrollbar->draw(p_context);
		}
		if (m_horizontalScrollbar->isEnabled)
		{
			m_horizontalScrollbar->draw(p_context);
		}
	}
};