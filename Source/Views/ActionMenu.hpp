#pragma once

#include <AvoGUI.hpp>
#include "../Resources/Colors.hpp"
#include "../utilities.hpp"
#include "../Resources/ThemeValues.hpp"

//------------------------------

struct ActionMenuItemData
{
	std::string action;
	std::string shortcut;

	ActionMenuItemData()
	{ }
	ActionMenuItemData(std::string const& p_action, std::string const& p_shortcut = "") :
		action(p_action), shortcut(p_shortcut)
	{ }
};

//------------------------------

class ActionMenu;

class ActionMenuItem :
	public AvoGUI::View
{
public:
	static constexpr float HEIGHT = 4.f        * 8.f;
	static constexpr float LEFT_PADDING = 2.f  * 8.f;
	static constexpr float RIGHT_PADDING = 2.f * 8.f;

private:
	ActionMenu* m_parent;
	AvoGUI::Ripple* m_ripple{ new AvoGUI::Ripple(this, AvoGUI::Color(1.f, 0.3f)) };

	AvoGUI::Text* m_text_action{ nullptr };
	AvoGUI::Text* m_text_shortcut{ nullptr };

public:
	ActionMenuItem(ActionMenu* p_parent, std::string const& p_action, std::string const& p_shortcut = "") :
		View((AvoGUI::View*)p_parent), 
		m_parent(p_parent)
	{
		m_text_action = getDrawingContext()->createText(p_action, getThemeValue(ThemeValues::fontSize));
		if (!p_shortcut.empty())
		{
			m_text_shortcut = getDrawingContext()->createText(p_shortcut, getThemeValue(ThemeValues::fontSize)*0.75f);
			m_text_shortcut->setFontWeight(AvoGUI::FontWeight::Medium);
		}

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
	std::string getShortcut()
	{
		if (m_text_shortcut)
		{
			return m_text_shortcut->getString();
		}
		return "";
	}

	//------------------------------

	void handleSizeChange() override
	{
		m_text_action->setLeft(LEFT_PADDING);
		m_text_action->setCenterY(getHeight() * 0.5f);

		if (m_text_shortcut)
		{
			m_text_shortcut->setRight(getWidth() - RIGHT_PADDING);
			m_text_shortcut->setCenterY(getHeight() * 0.5f);
		}
	}

	void draw(AvoGUI::DrawingContext* p_context) override
	{
		p_context->setOpacity(getThemeValue(ThemeValues::opacity));
		p_context->setColor(getThemeColor(ThemeColors::onBackground));
		p_context->drawText(m_text_action);
		if (m_text_shortcut)
		{
			p_context->setColor(Colors::actionMenuShortcut);
			p_context->drawText(m_text_shortcut);
		}
	}
};

//------------------------------

class ActionMenu :
	public AvoGUI::View
{
public:
	static constexpr float MIN_PARENT_MARGIN = 3.f * 8.f;
	static constexpr float VERTICAL_PADDING = 1.f  * 8.f;
	static constexpr float ANIMATION_SPEED = 0.05f;

private:
	float m_openAnimationTime{ 0.f };
	float m_openAnimationValue{ 0.f };

	AvoGUI::Point<float> m_anchor;
	AvoGUI::Rectangle<float> m_targetBounds;

public:
	ActionMenu(AvoGUI::View* p_parent, float p_width = 160.f) :
		View(p_parent, AvoGUI::Rectangle<float>(0.f, 0.f, p_width, 0.f)),
		m_targetBounds(0.f, 0.f, p_width, 0.f)
	{
		setThemeValue(ThemeValues::fontSize, 14.f);
		setThemeValue(ThemeValues::opacity, 1.f);
		setThemeColor(ThemeColors::background, Colors::actionMenuBackground);

		setCornerRadius(5.f);
		setElevation(8.f);
		setIsVisible(false);
		enableMouseEvents();

		getGui()->mouseDownListeners += [this](AvoGUI::MouseEvent const& p_event) {
			if (m_openAnimationTime >= 1.f && !getIsContainingAbsolute(p_event.x, p_event.y))
			{
				setIsVisible(false);
				invalidate();
			}
		};
	}

	//------------------------------

	AvoGUI::EventListeners<void(ActionMenuItem*)> actionMenuItemChoiceListeners;

	//------------------------------

	void addAction(std::string const& p_action, std::string const& p_shortcut = "")
	{
		ActionMenuItem* actionMenuItem = new ActionMenuItem(this, p_action, p_shortcut);
		if (getNumberOfChildViews() > 1)
		{
			actionMenuItem->setTop(getChildView(getNumberOfChildViews() - 2)->getBottom());
		}
		else
		{
			actionMenuItem->setTop(VERTICAL_PADDING);
		}
		actionMenuItem->setSize(m_targetBounds.getWidth(), ActionMenuItem::HEIGHT);
		actionMenuItem->mouseUpListeners += [this, actionMenuItem](AvoGUI::MouseEvent const& p_event) {
			if (p_event.mouseButton == AvoGUI::MouseButton::Left && actionMenuItem->getSize().getIsContaining(p_event.x, p_event.y))
			{
				actionMenuItemChoiceListeners(actionMenuItem);

				setIsVisible(false);
				invalidate();
			}
		};

		m_targetBounds.setHeight(actionMenuItem->getBottom() + VERTICAL_PADDING);
	}
	void addAction(ActionMenuItemData const& p_itemData)
	{
		addAction(p_itemData.action, p_itemData.shortcut);
	}
	void setActions(std::vector<ActionMenuItemData> const& p_actions)
	{
		removeAllChildViews();
		
		for (uint32 a = 0; a < p_actions.size(); a++)
		{
			addAction(p_actions[a].action, p_actions[a].shortcut);
		}
	}
	void clearActions()
	{
		removeAllChildViews();
	}

	//------------------------------

	void setMenuWidth(float p_width)
	{
		m_targetBounds.setWidth(p_width);

		for (auto child : getChildViews())
		{
			child->setWidth(p_width);
		}
	}

	//------------------------------

	void open(AvoGUI::Point<float> const& p_anchor)
	{
		open(p_anchor.x, p_anchor.y);
	}
	void open(float p_anchorX, float p_anchorY)
	{
		if (p_anchorX + m_targetBounds.getWidth() > getParent<View>()->getWidth() - MIN_PARENT_MARGIN)
		{
			m_targetBounds.setRight(p_anchorX);
			if (p_anchorY + m_targetBounds.getHeight() > getParent<View>()->getHeight() - MIN_PARENT_MARGIN)
			{
				m_targetBounds.setBottom(p_anchorY);
				setCornerRadius(4.f, 4.f, 5.f, 0.f);
			}
			else
			{
				m_targetBounds.setTop(p_anchorY);
				setCornerRadius(5.f, 0.f, 5.f, 5.f);
			}
		}
		else
		{
			m_targetBounds.setLeft(p_anchorX);
			if (p_anchorY + m_targetBounds.getHeight() > getParent<View>()->getHeight() - MIN_PARENT_MARGIN)
			{
				m_targetBounds.setBottom(p_anchorY);
				setCornerRadius(4.f, 4.f, 0.f, 5.f);
			}
			else
			{
				m_targetBounds.setTop(p_anchorY);
				setCornerRadius(0.f, 5.f, 5.f, 5.f);
			}
		}

		m_anchor.set(p_anchorX, p_anchorY);
		setBounds(p_anchorX, p_anchorY, p_anchorX, p_anchorY);

		setIsVisible(true);

		m_openAnimationTime = 0.f;
		queueAnimationUpdate();
	}

	//------------------------------

	void updateAnimations() override
	{
		if (m_openAnimationTime < 1.f)
		{
			m_openAnimationValue = getThemeEasing(ThemeEasings::out).easeValue(m_openAnimationTime += ANIMATION_SPEED);
			queueAnimationUpdate();
		}

		float heightFactor = 1.f - std::cos(m_openAnimationValue * AvoGUI::HALF_PI);
		setBounds(
			m_targetBounds.left,
			AvoGUI::interpolate(m_anchor.y, m_targetBounds.top, heightFactor),
			m_targetBounds.right,
			AvoGUI::interpolate(m_anchor.y, m_targetBounds.bottom, heightFactor)
		);
		for (uint32_t a = 0; a < getNumberOfChildViews(); a++)
		{
			if (a)
			{
				getChildView(a)->setTop(getChildView(a - 1)->getBottom());
			}
			else
			{
				if (m_targetBounds.top != m_anchor.y)
				{
					getChildView(a)->setTop(VERTICAL_PADDING);
				}
				else
				{
					getChildView(a)->setTop(getHeight() - VERTICAL_PADDING - getNumberOfChildViews() * ActionMenuItem::HEIGHT);
				}
			}
		}
		//float itemsTop = m_targetBounds.top - getTop() + ACTION_MENU_VERTICAL_PADDING;
		//if (getChildView(0)->getTop() != itemsTop)
		//{
		//	for (uint32_t a = 0; a < getNumberOfChildViews(); a++)
		//	{
		//		if (a)
		//		{
		//			getChildView(a)->setTop(getChildView(a-1)->getBottom());
		//		}
		//		else
		//		{
		//			getChildView(a)->setTop(itemsTop);
		//		}
		//	}
		//}

		setOpacity(1.f - AvoGUI::square(1.f - m_openAnimationValue));

		invalidate();
	}

	void draw(AvoGUI::DrawingContext* p_context) override
	{
		p_context->setColor(getThemeColor(ThemeColors::background));
		p_context->fillRectangle(getSize());
		p_context->setColor(AvoGUI::Color(getThemeColor(ThemeColors::onBackground), 0.1));
		p_context->strokeRectangle(getSize(), getCorners());
	}
};
