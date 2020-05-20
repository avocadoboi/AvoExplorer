#pragma once

#include "ActionMenu.hpp"
#include "AvoExplorer.hpp"

//------------------------------

class ContextMenu;

class ContextView : public Avo::View
{
private:
	std::vector<ActionMenuItemData> m_contextMenuItems;
	float m_contextMenuWidth{ 150.f };
	bool m_isMouseHoveringBackground{ false };

public:
	void addContextMenuItem(std::string const& p_action, std::string const& p_shortcut = "")
	{
		m_contextMenuItems.push_back({ p_action, p_shortcut });
	}
	std::vector<ActionMenuItemData> const& getContextMenuItems()
	{
		return m_contextMenuItems;
	}

	void setContextMenuWidth(float p_width)
	{
		m_contextMenuWidth = p_width;
	}
	float getContextMenuWidth()
	{
		return m_contextMenuWidth;
	}

	//------------------------------
	
	virtual void handleContextMenuItemChoice(ActionMenuItem*) { }

	//------------------------------

	void handleMouseBackgroundEnter(Avo::MouseEvent const& p_event) override
	{
		View::handleMouseBackgroundEnter(p_event);
		m_isMouseHoveringBackground = true;
	}
	void handleMouseBackgroundLeave(Avo::MouseEvent const& p_event) override
	{
		View::handleMouseBackgroundLeave(p_event);
		m_isMouseHoveringBackground = false;
	}
	inline void handleMouseDown(Avo::MouseEvent const& p_event) override;

	inline ContextView(Avo::View* p_parent, Avo::Rectangle<float> const& p_bounds = Avo::Rectangle<float>(0, 0, 0, 0)) :
		View(p_parent, p_bounds)
	{
	}
};

//------------------------------

class ContextMenu : public ActionMenu
{
private:
	ContextView* m_currentContextView{ nullptr };

public:
	ContextMenu(Avo::Gui* p_gui) :
		ActionMenu(p_gui)
	{
		actionMenuItemChoiceListeners += [this](auto item) {
			if (m_currentContextView)
			{
				m_currentContextView->handleContextMenuItemChoice(item);
			}
		};
	}

	void open(ContextView* p_contextView)
	{
		setMenuWidth(p_contextView->getContextMenuWidth());
		setActions(p_contextView->getContextMenuItems());
		m_currentContextView = p_contextView;

		ActionMenu::open(getWindow()->getMousePosition());
	}
};

//------------------------------

void ContextView::handleMouseDown(Avo::MouseEvent const& p_event)
{
	if (p_event.mouseButton == Avo::MouseButton::Right && m_isMouseHoveringBackground)
	{
		getComponentById<ContextMenu>(Ids::contextMenu)->open(this);
	}
}
