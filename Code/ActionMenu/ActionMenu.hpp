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
		AvoGUI::DrawingContext* context = getGui()->getDrawingContext();

		m_text_action = context->createText(p_action, getThemeValue("font size"));
		if (*p_shortcut)
		{
			m_text_shortcut = context->createText(p_shortcut, getThemeValue("font size")*0.75f);
			m_text_shortcut->setFontWeight(AvoGUI::FontWeight::Medium);
		}

		m_ripple = new AvoGUI::Ripple(this, AvoGUI::Color(1.f, 0.3f));

		setCursor(AvoGUI::Cursor::Hand);

		enableMouseEvents();
	}
	ActionMenuItem(ActionMenu* p_parent, ActionMenuItemData const& p_itemData) :
		ActionMenuItem(p_parent, p_itemData.action, p_itemData.shortcut) 
	{
	}

	//------------------------------

	std::string const& getAction()
	{
		return m_text_action->getString();
	}
	std::string const& getShortcut()
	{
		return m_text_shortcut->getString();
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

class ActionMenuListener
{
public:
	virtual void handleActionMenuItemChoice(std::string const& p_action, uint32 p_index) = 0;
};

//------------------------------

class ActionMenu :
	public AvoGUI::View,
	public AvoGUI::KeyboardListener,
	public AvoGUI::GlobalMouseListener
{
private:
	float m_openAnimationTime;
	float m_openAnimationValue;

	AvoGUI::Point<float> m_anchor;
	AvoGUI::Rectangle<float> m_targetBounds;

	std::vector<ActionMenuListener*> m_actionMenuListeners;

public:
	ActionMenu(AvoGUI::View* p_parent, float p_width = 160.f) :
		View(p_parent, AvoGUI::Rectangle<float>(0.f, 0.f, p_width, 0.f)),
		m_openAnimationTime(0.f), m_openAnimationValue(0.f),
		m_targetBounds(0.f, 0.f, p_width, 0.f)
	{
		setThemeValue("font size", 14.f);
		setThemeColor("background", Colors::actionMenuBackground);
		setThemeValue("opacity", 1.f);

		setCornerRadius(5.f);
		setElevation(8.f);
		setIsVisible(false);

		getGui()->addGlobalMouseListener(this);

		enableMouseEvents();
	}

	//------------------------------

	void addActionMenuListener(ActionMenuListener* p_listener)
	{
		m_actionMenuListeners.push_back(p_listener);
	}
	void removeActionMenuListener(ActionMenuListener* p_listener)
	{
		AvoGUI::removeVectorElementWithoutKeepingOrder(m_actionMenuListeners, p_listener);
	}

	//------------------------------

	void addAction(char const* p_action, char const* p_shortcut = "");
	void addAction(ActionMenuItemData const& p_itemData)
	{
		addAction(p_itemData.action, p_itemData.shortcut);
	}
	void setActions(std::vector<ActionMenuItemData> const& p_actions)
	{
		removeAllChildren();
		
		m_children.reserve(p_actions.size());
		for (uint32 a = 0; a < p_actions.size(); a++)
		{
			addAction(p_actions[a].action, p_actions[a].shortcut);
		}
	}
	void clearActions()
	{
		removeAllChildren();
	}

	//------------------------------

	void setMenuWidth(float p_width)
	{
		m_targetBounds.setWidth(p_width);

		for (auto child : m_children)
		{
			child->setWidth(p_width);
		}
	}

	//------------------------------

	void open(AvoGUI::Point<float> const& p_anchor)
	{
		open(p_anchor.x, p_anchor.y);
	}
	void open(float p_anchorX, float p_anchorY);

	virtual void handleActionMenuItemChoice(ActionMenuItem* p_item) 
	{
		for (auto listener : m_actionMenuListeners)
		{
			listener->handleActionMenuItemChoice(p_item->getAction(), p_item->getIndex());
		}

		setIsVisible(false);
		invalidate();
	}

	//------------------------------

	void handleGlobalMouseDown(AvoGUI::MouseEvent const& p_event) override
	{
		if (m_openAnimationTime >= 1.f && !getIsContainingAbsolute(p_event.x, p_event.y))
		{
			setIsVisible(false);
			invalidate();
		}
	}
	void handleKeyboardKeyDown(AvoGUI::KeyboardEvent const& p_event) override
	{
	}

	//------------------------------

	void updateAnimations() override;

	void draw(AvoGUI::DrawingContext* p_context) override
	{
		p_context->setColor(getThemeColor("background"));
		p_context->fillRectangle(getSize());
		p_context->setColor(AvoGUI::Color(getThemeColor("on background"), 0.1));
		p_context->strokeRectangle(getSize(), getCorners());
	}
};
