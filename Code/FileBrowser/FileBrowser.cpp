#include "FileBrowser.hpp"

#include "FileBrowserPathEditor/FileBrowserPathEditor.hpp"
#include "FileBrowserItems/FileBrowserItems.hpp"

//------------------------------

float const FILE_BROWSER_PADDING_HORIZONTAL = 2			* 8.f;
float const FILE_BROWSER_PADDING_TOP = 2				* 8.f;

//------------------------------

//bool getIsPathLessThan(std::filesystem::path const& p_a, std::filesystem::path const& p_b)
//{
//	return p_a < p_b;
//}

//------------------------------

FileBrowser::FileBrowser(AvoExplorer* p_parent) :
	View(p_parent), m_avoExplorer(p_parent),
	m_pathEditor(0), m_button_changeView(0), m_button_add(0)
{
	m_pathEditor = new FileBrowserPathEditor(this);
	ScrollContainer* scrollContainer = new ScrollContainer(this);
	m_items = new FileBrowserItems(scrollContainer);
	scrollContainer->setContentView(m_items);

	setWorkingDirectory("C:/");
}

//------------------------------

void FileBrowser::setWorkingDirectory(std::filesystem::path const& p_path)
{
	m_items->setWorkingDirectory(p_path);
}

//------------------------------

void FileBrowser::handleSizeChange()
{
	m_pathEditor->setTopLeft(FILE_BROWSER_PADDING_HORIZONTAL, FILE_BROWSER_PADDING_TOP);
	m_pathEditor->setWidth(getRight() - FILE_BROWSER_PADDING_HORIZONTAL * 2.f);

	m_items->getParent()->setBounds(0, m_pathEditor->getBottom(), getWidth(), getHeight());
	m_items->updateLayout();
}
