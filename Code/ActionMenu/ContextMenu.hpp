#pragma once

#include "ActionMenu.hpp"
#include "../AvoExplorer.hpp"

//------------------------------

class ContextMenu;

class ContextView : 
	public AvoGUI::View
{
private:
	ContextMenu* m_contextMenu = 0;
	std::vector<ActionMenuItemData> m_contextMenuItems;
	float m_contextMenuWidth;
	bool m_isMouseHoveringBackground;

public:
	ContextView(AvoGUI::View* p_parent, AvoGUI::Rectangle<float> const& p_bounds = AvoGUI::Rectangle<float>(0, 0, 0, 0)) :
		View(p_parent, p_bounds),
		m_contextMenuWidth(150.f), m_isMouseHoveringBackground(false)
	{
		m_contextMenu = getGui<AvoExplorer>()->getContextMenu();
	}

	//------------------------------

	void addContextMenuItem(char const* p_action, char const* p_shortcut = "")
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

	virtual void handleContextMenuItemChoice(std::string const& p_action, std::string const& p_shortcut) { }
	
	//------------------------------

	void handleMouseBackgroundEnter(AvoGUI::MouseEvent const& p_event) override
	{
		View::handleMouseBackgroundEnter(p_event);
		m_isMouseHoveringBackground = true;
	}
	void handleMouseBackgroundLeave(AvoGUI::MouseEvent const& p_event) override
	{
		View::handleMouseBackgroundLeave(p_event);
		m_isMouseHoveringBackground = false;
	}
	void handleMouseDown(AvoGUI::MouseEvent const& p_event) override;
};

//------------------------------

class ContextMenu :
	public ActionMenu
{
private:
	ContextView* m_currentContextView = 0;

public:
	ContextMenu(AvoGUI::Gui* p_gui) :
		ActionMenu(p_gui)
	{
	}

	void handleActionMenuItemChoice(ActionMenuItem* p_item) override
	{
		ActionMenu::handleActionMenuItemChoice(p_item);

		m_currentContextView->handleContextMenuItemChoice(p_item->getAction(), p_item->getShortcut());
	}

	void open(ContextView* p_contextView)
	{
		setMenuWidth(p_contextView->getContextMenuWidth());
		setActions(p_contextView->getContextMenuItems());
		m_currentContextView = p_contextView;

		ActionMenu::open(getGui()->getWindow()->getMousePosition());
	}
};
