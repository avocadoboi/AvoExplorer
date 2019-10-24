#include "ScrollContainer.hpp"

//------------------------------

float const SCROLLBAR_OPACITY_NORMAL = 0.3f;
float const SCROLLBAR_OPACITY_HOVERING = 0.6f;

float const SCROLLBAR_WIDTH = 1 * 8.f;

//------------------------------

Scrollbar::Scrollbar(ScrollContainer* p_parent, Orientation p_orientation) :
	View(p_parent), m_isDragging(false), m_opacity(SCROLLBAR_OPACITY_NORMAL)
{
	setCornerRadius(3.f);

	setCursor(AvoGUI::Cursor::Hand);
}

void Scrollbar::updateAnimations()
{
	if (m_isHovering)
	{
		if (m_opacityAnimationTime < 1.f)
		{
			m_opacity = AvoGUI::interpolate(SCROLLBAR_OPACITY_NORMAL, SCROLLBAR_OPACITY_HOVERING, getThemeEasing("in out").easeValue(m_opacityAnimationTime += 0.1f));
		}
	}
	else
	{
		if (m_opacityAnimationTime > 0.f)
		{
			m_opacity = AvoGUI::interpolate(SCROLLBAR_OPACITY_NORMAL, SCROLLBAR_OPACITY_HOVERING, getThemeEasing("in out").easeValue(m_opacityAnimationTime -= 0.1f));
		}
	}
}

//------------------------------

ScrollContainer::ScrollContainer(AvoGUI::View* p_parent, AvoGUI::Rectangle<float> const& p_bounds) :
	AvoGUI::View(p_parent, p_bounds)
{
	enableMouseEvents();
}
