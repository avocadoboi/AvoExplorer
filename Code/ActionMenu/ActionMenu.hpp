#pragma once

#include <AvoGUI.hpp>
#include "../../Resources/Colors.hpp"

//------------------------------

struct ActionMenuItemData
{
	char const* action;
	char const* shortcut;

	ActionMenuItemData() :
		action(""), shortcut("")
	{ }
	ActionMenuItemData(char const* p_action, char const* p_shortcut = "") :
		action(p_action), shortcut(p_shortcut)
	{ }
};

//------------------------------

class ActionMenu;

class ActionMenuItem :
	public AvoGUI::View
{
private:
	ActionMenu* m_parent;
	AvoGUI::Ripple* m_ripple;

	AvoGUI::Text* m_text_action;
	AvoGUI::Text* m_text_shortcut;

public:
	ActionMenuItem(ActionMenu* p_parent, char const* p_action, char const* p_shortcut = "") :
		View((AvoGUI::View*)p_parent), m_parent(p_parent),
		m_text_action(0), m_text_shortcut(0)
	{
		AvoGUI::DrawingContext* context = getGUI()->getDrawingContext();

		m_text_action = context->createText(p_action, getThemeValue("font size"));
		if (*p_shortcut)
		{
			m_text_shortcut = context->createText(p_shortcut, getThemeValue("font size")*0.8f);
			m_text_shortcut->setFontWeight(AvoGUI::FontWeight::Medium);
		}

		m_ripple = new AvoGUI::Ripple(this, AvoGUI::Color(1.f, 0.05f));

		setCursor(AvoGUI::Cursor::Hand);

		enableMouseEvents();
	}
	ActionMenuItem(ActionMenu* p_parent, ActionMenuItemData const& p_itemData) :
		View((AvoGUI::View*)p_parent),
		m_text_action(0), m_text_shortcut(0)
	{
		ActionMenuItem(p_parent, p_itemData.action, p_itemData.shortcut);
	}

	//------------------------------

	void handleSizeChange() override;
	void handleMouseUp(AvoGUI::MouseEvent const& p_event) override;

	void draw(AvoGUI::DrawingContext* p_context) override
	{
		p_context->setOpacity(getThemeValue("opacity"));
		p_context->setColor(getThemeColor("on background"));
		p_context->drawText(m_text_action);
		p_context->setColor(Colors::actionMenuShortcut);
		p_context->drawText(m_text_shortcut);
		p_context->setOpacity(1.f);
	}
};

//------------------------------

class ActionMenu :
	public AvoGUI::View,
	public AvoGUI::KeyboardListener,
	public AvoGUI::GlobalMouseListener
{
private:
	bool m_isOpen;
	float m_openAnimationTime;
	float m_openAnimationValue;

	AvoGUI::Point<float> m_anchor;
	AvoGUI::Rectangle<float> m_targetBounds;

public:
	ActionMenu(AvoGUI::View* p_parent, float p_width = 200.f) :
		View(p_parent, AvoGUI::Rectangle<float>(0.f, 0.f, p_width, 0.f)),
		m_isOpen(false), m_openAnimationTime(0.f), m_openAnimationValue(0.f),
		m_targetBounds(0.f, 0.f, p_width, 0.f)
	{
		setThemeValue("font size", 15.f);
		setThemeColor("background", Colors::actionMenuBackground);
		setThemeValue("opacity", 1.f);

		setCornerRadius(5.f);
		setElevation(8.f);
		setIsVisible(false);

		getGUI()->addGlobalMouseListener(this);

		enableMouseEvents();
	}

	//------------------------------

	void addAction(char const* p_action, char const* p_shortcut = "");
	void addAction(ActionMenuItemData const& p_itemData)
	{
		addAction(p_itemData.action, p_itemData.shortcut);
	}

	//------------------------------

	void open(AvoGUI::Point<float> const& p_anchor)
	{
		open(p_anchor.x, p_anchor.y);
	}
	void open(float p_anchorX, float p_anchorY);

	virtual void handleActionMenuItemChoice() 
	{
		setIsVisible(false);
		invalidate();
	}

	//------------------------------

	void handleGlobalMouseDown(AvoGUI::MouseEvent const& p_event) override
	{
		if (m_openAnimationTime >= 1.f && !getIsContaining(p_event.x, p_event.y))
		{
			m_isOpen = false;
			m_openAnimationTime = 0.f;
			queueAnimationUpdate();
		}
	}
	void handleKeyboardKeyDown(AvoGUI::KeyboardEvent const& p_event) override
	{
	}

	//------------------------------

	void updateAnimations()
	{
		if (m_isOpen)
		{
			if (m_openAnimationTime < 1.f)
			{
				m_openAnimationValue = getThemeEasing("out").easeValue(m_openAnimationTime += 0.1f);
				queueAnimationUpdate();
			}
		}
		else
		{
			if (m_openAnimationTime < 1.f)
			{
				m_openAnimationValue = 1.f - getThemeEasing("out").easeValue(m_openAnimationTime += 0.1f);
				queueAnimationUpdate();
			}
			else
			{
				setIsVisible(false);
			}
		}

		float widthFactor = std::sin(m_openAnimationValue * AvoGUI::HALF_PI);
		float heightFactor = 1.f - std::cos(m_openAnimationValue * AvoGUI::HALF_PI);
		setBounds(
			AvoGUI::interpolate(m_anchor.x, m_targetBounds.left, widthFactor), 
			AvoGUI::interpolate(m_anchor.y, m_targetBounds.top, heightFactor),
			AvoGUI::interpolate(m_anchor.x, m_targetBounds.right, widthFactor),
			AvoGUI::interpolate(m_anchor.y, m_targetBounds.bottom, heightFactor)
		);

		setThemeValue("opacity", AvoGUI::square(m_openAnimationValue));

		invalidate();
	}

	void draw(AvoGUI::DrawingContext* p_context) override
	{
		p_context->setOpacity(m_openAnimationValue);
		p_context->setColor(getThemeColor("background"));
		p_context->fillRectangle(getSize());
		p_context->setOpacity(1.f);
	}
};
