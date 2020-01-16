#pragma once

#include <AvoGUI.hpp>
#include <Windows.h>
#undef max
#undef min

//------------------------------

inline AvoGUI::Image* loadImageFromResource(uint32 p_resourceID, AvoGUI::DrawingContext* p_context)
{
	HRSRC resource = FindResourceW(0, MAKEINTRESOURCEW(p_resourceID), L"IMAGE");
	if (resource)
	{
		return p_context->createImage(LockResource(LoadResource(0, resource)), SizeofResource(0, resource));
	}
	return 0;
}
