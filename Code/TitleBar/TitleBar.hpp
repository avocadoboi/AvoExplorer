#pragma once

#include <AvoGUI.hpp>

#include "../../Resources/Colors.hpp"

//------------------------------

class TitleBar;

class TitleBarWindowButton :
	public AvoGUI::View
{
private:
	TitleBar* m_titleBar;
	
	AvoGUI::Image* m_icon;
	AvoGUI::Ripple* m_ripple;

	AvoGUI::Color m_backgroundColor;
	float m_hoverAnimationTime;
	bool m_isHovering;
	bool m_isEnabled;

public:
	TitleBarWindowButton(TitleBar* p_parent, AvoGUI::Image* p_icon, bool p_isCloseButton, bool p_isEnabled = true);
	~TitleBarWindowButton()
	{
		m_icon->forget();
	}

	//------------------------------

	void handleMouseEnter(AvoGUI::MouseEvent const& p_event) override
	{
		m_isHovering = true;
		queueAnimationUpdate();
	}
	void handleMouseLeave(AvoGUI::MouseEvent const& p_event) override
	{
		m_isHovering = false;
		queueAnimationUpdate();
	}
	void handleMouseUp(AvoGUI::MouseEvent const& p_event) override;

	//------------------------------

	void setIcon(AvoGUI::Image* p_icon);

	//------------------------------

	void updateAnimations()
	{
		if (m_isEnabled)
		{
			if (m_isHovering)
			{
				if (m_hoverAnimationTime < 1.f)
				{
					m_backgroundColor = AvoGUI::interpolate(
						AvoGUI::Color(0.f, 0.f), getThemeColor("background"),
						getThemeEasing("in out").easeValue(m_hoverAnimationTime += getThemeValue("hover animation speed"))
					);
					queueAnimationUpdate();
					invalidate();
				}
				else
				{
					m_hoverAnimationTime = 1.f;
				}
			}
			else
			{
				if (m_hoverAnimationTime > 0.f)
				{
					m_backgroundColor = AvoGUI::interpolate(
						AvoGUI::Color(0.f, 0.f), getThemeColor("background"),
						getThemeEasing("in out").easeValue(m_hoverAnimationTime -= getThemeValue("hover animation speed"))
					);
					queueAnimationUpdate();
					invalidate();
				}
				else
				{
					m_hoverAnimationTime = 0.f;
				}
			}
		}
	}

	//------------------------------

	void draw(AvoGUI::DrawingContext* p_context) override
	{
		p_context->setColor(m_backgroundColor);
		p_context->fillRectangle(getSize());

		p_context->drawImage(m_icon);
	}
};

//------------------------------

class TitleBar :
	public AvoGUI::View,
	public AvoGUI::WindowListener
{
private:
	AvoGUI::Text* m_title;
	TitleBarWindowButton* m_minimizeButton;
	TitleBarWindowButton* m_maximizeButton;
	TitleBarWindowButton* m_closeButton;
	bool m_isMaximizeEnabled;

public:
	TitleBar(AvoGUI::Gui* p_parent);

	//------------------------------

	void handleWindowMaximize(AvoGUI::WindowEvent const& p_event);
	void handleWindowRestore(AvoGUI::WindowEvent const& p_event);

	//------------------------------

	void handleSizeChange();

	AvoGUI::WindowBorderArea getWindowBorderAreaAtPosition(float p_x, float p_y)
	{
		return p_y < getHeight() && p_x < (m_minimizeButton ? m_minimizeButton : m_closeButton)->getLeft() ? AvoGUI::WindowBorderArea::Dragging : AvoGUI::WindowBorderArea::None;
	}
	void handleTitleBarButtonClick(TitleBarWindowButton* p_button)
	{
		if (p_button == m_closeButton)
		{
			getGui()->getWindow()->close();
		}
		else if (p_button == m_minimizeButton)
		{
			getGui()->getWindow()->minimize();
		}
		else if (m_isMaximizeEnabled)
		{
			if (getGui()->getWindow()->getState() == AvoGUI::WindowState::Maximized)
			{
				getGui()->getWindow()->restore();
			}
			else
			{
				getGui()->getWindow()->maximize();
			}
		}
	}

	//------------------------------

	void draw(AvoGUI::DrawingContext* p_context) override
	{
		p_context->setColor(Colors::titleBarBackground);
		p_context->fillRectangle(getSize());
		p_context->setColor(Colors::titleBarOnBackground);
		p_context->drawText(m_title);
	}
};
