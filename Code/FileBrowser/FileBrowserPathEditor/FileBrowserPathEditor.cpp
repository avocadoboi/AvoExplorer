#include "FileBrowserPathEditor.hpp"

//------------------------------

float const FILE_BROWSER_PATH_EDITOR_HEIGHT = 6 * 8.f;

//------------------------------

FileBrowserPathEditor::FileBrowserPathEditor(FileBrowser* p_parent) :
	View(p_parent), m_fileBrowser(p_parent)
{
	setHeight(FILE_BROWSER_PATH_EDITOR_HEIGHT);
	setCornerRadius(4.f);
	setElevation(3.f);
}
