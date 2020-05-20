#pragma once

#include <AvoGUI.hpp>

#include "../Resources/Colors.hpp"
#include "../Resources/ThemeValues.hpp"
#include "../utilities.hpp"

//------------------------------

class TitleBarWindowButton : public Avo::View
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
	void handleMouseUp(Avo::MouseEvent const& p_event) override;

	//------------------------------

	void setIcon(Icon p_icon)
	{
		m_icon = p_icon;
	}

	//------------------------------

private:
	Avo::Animation* m_hoverAnimation = createAnimation(ThemeEasings::inOut, 100, [this](float p_value) {
		m_backgroundColor = Avo::interpolate(Colors::titleBarBackground, getThemeColor(ThemeColors::background), p_value);
		invalidate();
	});
public:
	void handleMouseEnter(Avo::MouseEvent const& p_event) override
	{
		if (m_isEnabled)
		{
			m_hoverAnimation->play(false);
		}
	}
	void handleMouseLeave(Avo::MouseEvent const& p_event) override
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

private:
	Avo::Color m_backgroundColor;
public:
	void draw(Avo::DrawingContext* p_context) override
	{
		p_context->setColor(m_backgroundColor);
		p_context->fillRectangle(getSize());

		Avo::Color strokeColor{ getThemeColor(ThemeColors::onBackground), m_isEnabled ? 1.f : 0.5f };
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
	}

	TitleBarWindowButton(View* p_parent, Icon p_icon, bool p_isCloseButton, bool p_isEnabled = true) :
		View(p_parent),
		m_icon(p_icon),
		m_isEnabled(p_isEnabled)
	{
		setSize(BUTTON_WIDTH_FACTOR * p_parent->getHeight(), p_parent->getHeight());

		setThemeColor(ThemeColors::background, p_isCloseButton ? Colors::titleBarCloseButton : Avo::interpolate(getThemeColor(ThemeColors::background), getThemeColor(ThemeColors::onBackground), 0.3f));
		m_backgroundColor.alpha = 0.f;

		if (p_isEnabled)
		{
			auto ripple = new Avo::Ripple(this, Avo::Color(getThemeColor(ThemeColors::onBackground), p_isCloseButton ? 0.5f : 0.2f));
			ripple->setHasHoverEffect(false);
		}
	}
};

//------------------------------

class TitleBar : public Avo::View
{
public:
	static constexpr float HEIGHT = 2.9 * 8.f;

private:
	Avo::Text m_title;
	TitleBarWindowButton* m_minimizeButton = nullptr;
	TitleBarWindowButton* m_maximizeButton = nullptr;
	TitleBarWindowButton* m_closeButton = nullptr;
	bool m_isMaximizeEnabled = false;

public:
	TitleBar(Avo::Gui* p_parent) :
		View(p_parent, Avo::Rectangle<float>(0, 0, 0, HEIGHT))
	{
		m_isMaximizeEnabled = bool(getWindow()->getStyles() & Avo::WindowStyleFlags::MaximizeButton);

		m_title = getDrawingContext()->createText(getWindow()->getTitle(), 11.f);
		m_title.setFontWeight(Avo::FontWeight::SemiBold);
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

	Avo::WindowBorderArea getWindowBorderAreaAtPosition(float p_x, float p_y)
	{
		return p_y < getHeight() && p_x < (m_minimizeButton ? m_minimizeButton : m_closeButton)->getLeft() ? Avo::WindowBorderArea::Dragging : Avo::WindowBorderArea::None;
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
			if (getGui()->getWindow()->getState() == Avo::WindowState::Maximized)
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

	void draw(Avo::DrawingContext* p_context) override
	{
		p_context->setColor(Colors::titleBarBackground);
		p_context->fillRectangle(getSize());
		p_context->setColor(Colors::titleBarOnBackground);
		p_context->drawText(m_title);
	}
};
