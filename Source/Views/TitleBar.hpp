#pragma once

#include <AvoGUI.hpp>

#include "../Resources/Colors.hpp"
#include "../Resources/ThemeValues.hpp"
#include "../utilities.hpp"

//------------------------------

class TitleBar;

class TitleBarWindowButton :
	public AvoGUI::View
{
public:
	enum class Icon
	{
		Minimize,
		Maximize,
		Restore,
		Close
	};

	static constexpr float BUTTON_ICON_PADDING = 7.f;
	static constexpr float BUTTON_WIDTH_FACTOR = 1.2f;

private:
	TitleBar* m_titleBar;
	
	Icon m_icon;

	AvoGUI::Color m_backgroundColor;
	float m_hoverAnimationTime{ 0.f };
	bool m_isHovering{ false };
	bool m_isEnabled;

public:
	TitleBarWindowButton(TitleBar* p_parent, Icon p_icon, bool p_isCloseButton, bool p_isEnabled = true);

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

	void setIcon(Icon p_icon)
	{
		m_icon = p_icon;
	}

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
						Colors::titleBarBackground, getThemeColor(ThemeColors::background),
						getThemeEasing(ThemeEasings::inOut).easeValue(m_hoverAnimationTime += getThemeValue(ThemeValues::hoverAnimationSpeed))
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
						Colors::titleBarBackground, getThemeColor(ThemeColors::background),
						getThemeEasing(ThemeEasings::inOut).easeValue(m_hoverAnimationTime -= getThemeValue(ThemeValues::hoverAnimationSpeed))
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

		AvoGUI::Color strokeColor = AvoGUI::Color(getThemeColor(ThemeColors::onBackground), m_isEnabled ? 1.f : 0.5f);
		p_context->setColor(strokeColor);

		p_context->moveOrigin((getWidth() - getHeight()) * 0.5f + BUTTON_ICON_PADDING, BUTTON_ICON_PADDING);
		float width = getHeight() - BUTTON_ICON_PADDING * 2.f;
		switch (m_icon)
		{
		case Icon::Minimize:
		{
			p_context->drawLine(0.f, width * 0.5f, width, width * 0.5f);
			break;
		}
		case Icon::Maximize:
		{
			p_context->strokeRectangle(0.f, 0.f, width, width);
			break;
		}
		case Icon::Restore:
		{
			p_context->strokeRectangle(2.f, 0.f, width, width - 2.f);
			p_context->setColor(m_backgroundColor);
			p_context->fillRectangle(0.f, 2.f, width - 2.f, width);
			p_context->setColor(strokeColor);
			p_context->strokeRectangle(0.f, 2.f, width - 2.f, width);
			break;
		}
		case Icon::Close:
		{
			p_context->drawLine(0.f, 0.f, width, width);
			p_context->drawLine(0.f, width, width, 0.f);
			break;
		}
		}
	}
};

//------------------------------

class TitleBar : public AvoGUI::View
{
public:
	static constexpr float HEIGHT = 2.9 * 8.f;

private:
	AvoGUI::Text m_title;
	TitleBarWindowButton* m_minimizeButton{ nullptr };
	TitleBarWindowButton* m_maximizeButton{ nullptr };
	TitleBarWindowButton* m_closeButton{ nullptr };
	bool m_isMaximizeEnabled{ false };

public:
	TitleBar(AvoGUI::Gui* p_parent) :
		View(p_parent, AvoGUI::Rectangle<float>(0, 0, 0, HEIGHT))
	{
		m_isMaximizeEnabled = bool(getWindow()->getStyles() & AvoGUI::WindowStyleFlags::MaximizeButton);

		m_title = getDrawingContext()->createText(getWindow()->getTitle(), 11.f);
		m_title.setFontWeight(AvoGUI::FontWeight::SemiBold);
		m_title.setCharacterSpacing(0.5f);
		m_title.setIsTopTrimmed(true);
		m_title.fitHeightToText();
		m_title.moveY(1.f);
		m_title.setCenterY(HEIGHT * 0.5f);

		if (!p_parent->getParent())
		{
			m_minimizeButton = new TitleBarWindowButton(this, TitleBarWindowButton::Icon::Minimize, false);
			m_minimizeButton->setCenterY(HEIGHT * 0.5f);

			m_maximizeButton = new TitleBarWindowButton(this, TitleBarWindowButton::Icon::Maximize, false, m_isMaximizeEnabled);
			m_maximizeButton->setCenterY(HEIGHT * 0.5f);
		}

		m_closeButton = new TitleBarWindowButton(this, TitleBarWindowButton::Icon::Close, true);
		m_closeButton->setCenterY(HEIGHT * 0.5f);

		getWindow()->windowMaximizeListeners += [this](auto) {
			if (m_maximizeButton)
			{
				m_maximizeButton->setIcon(TitleBarWindowButton::Icon::Restore);
			}
		};
		getWindow()->windowRestoreListeners += [this](auto) {
			if (m_maximizeButton)
			{
				m_maximizeButton->setIcon(TitleBarWindowButton::Icon::Maximize);
			}
		};
	}

	//------------------------------

	void handleSizeChange()
	{
		m_title.setCenterX(getCenterX());

		m_closeButton->setRight(getWidth());
		if (m_maximizeButton)
		{
			m_maximizeButton->setRight(m_closeButton->getLeft());
		}
		if (m_minimizeButton)
		{
			m_minimizeButton->setRight(m_maximizeButton->getLeft());
		}
	}

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
