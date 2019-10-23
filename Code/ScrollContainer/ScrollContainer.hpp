#pragma once

#include <AvoGUI.hpp>

//------------------------------

class ScrollContainer :
	public AvoGUI::View,
	public AvoGUI::ViewListener
{
private:
	AvoGUI::Point<float> m_contentSize;
	AvoGUI::Point<float> m_scrollPosition;

	AvoGUI::Rectangle<float> m_scrollbarBounds;
	float m_scrollbarOpacity;

public:
	ScrollContainer(AvoGUI::View* p_parent, AvoGUI::Rectangle<float> const& p_bounds = AvoGUI::Rectangle<float>());

	//------------------------------

	void handleChildAttachment(View* p_child) override
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
	void handleChildDetachment(View* p_child) override
	{
		if (p_child->getRight() == m_contentSize.x)
		{
			m_contentSize.x = 0.f;
			for (auto child : getChildren())
			{
				if (child->getRight() > m_contentSize.x)
				{
					m_contentSize.x = child->getRight();
				}
			}
		}
		if (p_child->getBottom() == m_contentSize.y)
		{
			m_contentSize.y = 0.f;
			for (auto child : getChildren())
			{
				if (child->getRight() > m_contentSize.y)
				{
					m_contentSize.y = child->getRight();
				}
			}
		}
	}

	void drawOverlay(AvoGUI::DrawingContext* p_context)
	{
		if (m_contentSize.y > getHeight())
		{
		}
	}
};