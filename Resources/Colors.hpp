#pragma once

#include <AvoGUI.hpp>
using AvoGUI::Color;

/*
	These colors are named in a hierarchical way.
	
	For example, avoExplorerOnBackground is used in all children of AvoExplorer and their children
	except the ones that have their own onBackground colors.
*/
namespace Colors
{
	inline Color const label(0.8f);

	inline Color const selection(0.2f, 1.f, 0.5f, 0.3f);
	inline Color const primary(0.1f, 0.6f, 0.35f);
	inline Color const primaryOnBackground(0.2f, 1.f, 0.5f);
	inline Color const secondary(0.1f, 1.f, 0.4f);
	inline Color const onSecondary(0.f);

	inline Color const avoExplorerBackground(0.12f);
	inline Color const avoExplorerOnBackground(1.f);
	inline Color const avoExplorerShadow(0.f, 0.3f);

	inline Color const titleBarBackground(0.23f);
	inline Color const titleBarOnBackground(0.8f);
	inline Color const titleBarCloseButton(0.8f, 0.f, 0.1f);

	inline Color const topBarBackground(0.17f);
	inline Color const topBarBookmarksBackground(0.12f);
	inline Color const topBarBookmarksBorder(0.3f);

	inline Color const actionMenuBackground(0.1f);
	inline Color const actionMenuShortcut(0.6f);

	inline Color const fileBrowserPathEditorBackground(0.12f);
	inline Color const fileBrowserPathEditorBorder(0.3f);
	inline Color const fileBrowserItemBackground(0.2f);

	inline Color const scrollbar(0.7f);

	inline Color const dialogBoxBackground(0.1f);
	inline Color const dialogBoxOnBackground(1.f);
	inline Color const dialogBoxOutline(0.5f);
};
