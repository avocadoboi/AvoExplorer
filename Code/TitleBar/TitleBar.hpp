#pragma once

#include "../AvoExplorer.hpp"

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

public:
	TitleBarWindowButton(TitleBar* p_parent, AvoGUI::Image* p_icon, bool p_isCloseButton);
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

	void updateAnimations()
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
	public AvoGUI::View
{
private:
	AvoExplorer* m_avoExplorer;

	AvoGUI::Text* m_title;
	TitleBarWindowButton* m_minimizeButton;
	TitleBarWindowButton* m_maximizeButton;
	TitleBarWindowButton* m_closeButton;

public:
	TitleBar(AvoExplorer* p_parent);

	//------------------------------

	void handleSizeChange();

	AvoGUI::WindowBorderArea getWindowBorderAreaAtPosition(float p_x, float p_y)
	{
		return p_x < m_minimizeButton->getLeft() ? AvoGUI::WindowBorderArea::Dragging : AvoGUI::WindowBorderArea::None;
	}
	void handleWindowButtonClick(TitleBarWindowButton* p_button)
	{
		if (p_button == m_closeButton)
		{
			getGUI()->getWindow()->close();
		}
		else if (p_button == m_minimizeButton)
		{
			getGUI()->getWindow()->minimize();
		}
		else
		{
			if (getGUI()->getWindow()->getState() == AvoGUI::WindowState::Maximized)
			{
				getGUI()->getWindow()->restore();
			}
			else
			{
				getGUI()->getWindow()->maximize();
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
