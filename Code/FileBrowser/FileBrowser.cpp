#include "FileBrowser.hpp"

#include "FileBrowserPathEditor/FileBrowserPathEditor.hpp"

//------------------------------

float const FILE_BROWSER_PADDING_HORIZONTAL = 2	* 8.f;
float const FILE_BROWSER_PADDING_TOP = 2		* 8.f;

//------------------------------

FileBrowser::FileBrowser(AvoExplorer* p_parent) :
	View(p_parent), m_avoExplorer(p_parent),
	m_pathEditor(0), m_button_changeView(0), m_button_add(0)
{
	m_pathEditor = new FileBrowserPathEditor(this);
}

void FileBrowser::handleSizeChange()
{
	m_pathEditor->setTopLeft(FILE_BROWSER_PADDING_HORIZONTAL, FILE_BROWSER_PADDING_TOP);
	m_pathEditor->setWidth(getRight() - FILE_BROWSER_PADDING_HORIZONTAL * 2.f);
}
