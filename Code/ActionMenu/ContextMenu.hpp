#pragma once

#include "ActionMenu.hpp"
#include "../AvoExplorer.hpp"

//------------------------------

class ContextMenu;

class ContextView : 
	public AvoGUI::View
{
protected:
	ContextMenu* m_contextMenu;
	std::vector<ActionMenuItemData> m_contextMenuItems;
	float m_contextMenuWidth;
	bool m_isMouseHoveringBackground;

public:
	ContextView(AvoGUI::View* p_parent, AvoGUI::Rectangle<float> const& p_bounds = AvoGUI::Rectangle<float>(0, 0, 0, 0)) :
		View(p_parent, p_bounds), m_contextMenu(0),
		m_contextMenuWidth(150.f), m_isMouseHoveringBackground(false)
	{
		m_contextMenu = ((AvoExplorer*)getGUI())->getContextMenu();
	}

	//------------------------------

	virtual void handleContextMenuItemChoice(ActionMenuItemData const& p_item) { }
	
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

	//------------------------------

	std::vector<ActionMenuItemData> const& getContextMenuItems()
	{
		return m_contextMenuItems;
	}
	float getContextMenuWidth()
	{
		return m_contextMenuWidth;
	}
};

//------------------------------

class ContextMenu :
	public ActionMenu
{
private:
	ContextView* m_currentContextView;

public:
	ContextMenu(AvoGUI::GUI* p_gui) :
		ActionMenu(p_gui),
		m_currentContextView(0)
	{
	}

	void handleActionMenuItemChoice(ActionMenuItem* p_item) override
	{
		ActionMenu::handleActionMenuItemChoice(p_item);

		m_currentContextView->handleContextMenuItemChoice(ActionMenuItemData(p_item->getAction().c_str(), p_item->getShortcut().c_str()));
	}

	void open(ContextView* p_contextView)
	{
		setMenuWidth(p_contextView->getContextMenuWidth());
		setActions(p_contextView->getContextMenuItems());
		m_currentContextView = p_contextView;

		ActionMenu::open(getGUI()->getWindow()->getMousePosition());
	}
};
