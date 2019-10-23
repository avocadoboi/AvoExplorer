#pragma once

#include "Bookmarks.hpp"
#include "../ActionMenu/ContextMenu.hpp"

class Bookmark :
	public ContextView
{
public:
	Bookmark(Bookmarks* p_bookmarks) :
		ContextView(p_bookmarks)
	{

	}
};