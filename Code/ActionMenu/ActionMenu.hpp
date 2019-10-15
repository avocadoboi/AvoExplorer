#pragma once

#include <AvoGUI.hpp>

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
		View(p_parent), m_parent(p_parent),
		m_text_action(0), m_text_shortcut(0)
	{
		AvoGUI::DrawingContext* context = getGUI()->getDrawingContext();

		m_text_action = context->createText(p_action, getThemeValue("font size"));
		if (*p_shortcut)
		{
			m_text_shortcut = context->createText(p_shortcut, getThemeValue("font size"));
		}

		m_ripple = new AvoGUI::Ripple(this, AvoGUI::Color(1.f, 0.3f));

		enableMouseEvents();
	}
	ActionMenuItem(ActionMenu* p_parent, ActionMenuItemData const& p_itemData) :
		View(p_parent),
		m_text_action(0), m_text_shortcut(0)
	{
		ActionMenuItem(p_parent, p_itemData.action, p_itemData.shortcut);
	}

	//------------------------------

	void handleMouseUp(AvoGUI::MouseEvent const& p_event) override;
};

//------------------------------

class ActionMenu :
	public AvoGUI::View,
	public AvoGUI::KeyboardListener
{
private:
	float m_openAnimationTime;
	float m_openAnimationValue;

	// This is the target animation width and height
	float m_width;
	float m_height;

public:
	ActionMenu(AvoGUI::View* p_parent, float p_width = 200.f) :
		View(p_parent, AvoGUI::Rectangle<float>(0.f, 0.f, p_width, 0.f)),
		m_width(p_width), m_height(0.f)
	{
		setThemeValue("font size", 16.f);
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

	void handleActionMenuItemChoice();

	//------------------------------

	void handleKeyboardKeyDown(AvoGUI::KeyboardEvent const& p_event) override;

	//------------------------------

	void updateAnimations()
	{

	}

	void draw(AvoGUI::DrawingContext* p_context) override
	{

	}
};
