#pragma once

#include <AvoGUI.hpp>

/*
	These colors are named in a hierarchical way.
	
	For example, avoExplorerOnBackground is used in all children of AvoExplorer and their children
	except the ones that have their own onBackground colors.
*/
namespace Colors
{
	AvoGUI::Color const avoExplorerOnBackground(1.f);
	AvoGUI::Color const avoExplorerBackground(0.1f);
	AvoGUI::Color const avoExplorerShadow(0.f, 0.3f);

	AvoGUI::Color const topBarBackground(0.17f);
	AvoGUI::Color const topBarBookmarksBackground(0.1f);
	AvoGUI::Color const topBarBookmarksBorder(0.3f);

	AvoGUI::Color const actionMenuBackground(0.2f);
	AvoGUI::Color const actionMenuShortcut(0.6f);
};