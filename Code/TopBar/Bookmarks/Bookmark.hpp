#pragma once

#include "Bookmarks.hpp"

class Bookmark :
	public AvoGUI::View
{
public:
	Bookmark(Bookmarks* p_bookmarks) :
		View(p_bookmarks)
	{

	}
};