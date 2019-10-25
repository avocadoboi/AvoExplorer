#include "FileBrowser.hpp"

#include "FileBrowserPathEditor/FileBrowserPathEditor.hpp"
#include "FileBrowserItem/FileBrowserItem.hpp"

//------------------------------

float const FILE_BROWSER_PADDING_HORIZONTAL = 2	* 8.f;
float const FILE_BROWSER_PADDING_TOP = 2		* 8.f;

//------------------------------

FileBrowser::FileBrowser(AvoExplorer* p_parent) :
	View(p_parent), m_avoExplorer(p_parent),
	m_pathEditor(0), m_button_changeView(0), m_button_add(0)
{
	m_pathEditor = new FileBrowserPathEditor(this);

	m_itemsContainer = new ScrollContainer(this);
	for (uint32 a = 0; a < 20; a++)
	{
		FileBrowserItem* item = new FileBrowserItem(m_itemsContainer, AvoGUI::Rectangle<float>(0.f, 0.f, 300.f, 200.f));
	}
}

void FileBrowser::handleSizeChange()
{
	m_pathEditor->setTopLeft(FILE_BROWSER_PADDING_HORIZONTAL, FILE_BROWSER_PADDING_TOP);
	m_pathEditor->setWidth(getRight() - FILE_BROWSER_PADDING_HORIZONTAL * 2.f);

	m_itemsContainer->setBounds(0, m_pathEditor->getBottom(), getWidth(), getHeight());

	FileBrowserItem* lastItem = 0;
	for (FileBrowserItem* item : (std::vector<FileBrowserItem*> const&)m_itemsContainer->getChildren())
	{
		if (lastItem)
		{
			if (lastItem->getRight() + 30.f + item->getWidth() > m_itemsContainer->getWidth() - 20.f)
			{
				item->setTopLeft(20.f, lastItem->getBottom() + 30.f);
			}
			else
			{
				item->setTopLeft(lastItem->getRight() + 30.f, lastItem->getTop());
			}
		}
		else
		{
			item->setTopLeft(20.f, 20.f);
		}
		lastItem = item;
	}
}
