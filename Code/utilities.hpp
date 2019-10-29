#pragma once

#include "../Resources/Icons/iconResources.hpp"

#include <AvoGUI.hpp>
#include <Windows.h>

//------------------------------

inline AvoGUI::Image* loadImageFromResource(uint32 p_resourceID, AvoGUI::DrawingContext* p_context)
{
	HRSRC resource = FindResource(0, MAKEINTRESOURCE(p_resourceID), "IMAGE");
	if (resource)
	{
		return p_context->createImage(LockResource(LoadResource(0, resource)), SizeofResource(0, resource));
	}
	return 0;
}
