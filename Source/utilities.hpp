#pragma once

#include <AvoGUI.hpp>
#include <Windows.h>
#undef max
#undef min

//------------------------------

namespace ThemeColors = AvoGUI::ThemeColors;
namespace ThemeEasings = AvoGUI::ThemeEasings;
using AvoGUI::Id;

//------------------------------

inline AvoGUI::Image* loadImageFromResource(uint32 p_resourceID, AvoGUI::DrawingContext* p_context)
{
	HRSRC resource = FindResourceW(0, MAKEINTRESOURCEW(p_resourceID), L"IMAGE");
	if (resource)
	{
		return p_context->createImage((uint8*)LockResource(LoadResource(0, resource)), SizeofResource(0, resource));
	}
	return 0;
}

/*
	Returns true if p_a is less than p_b.
*/
inline bool getIsPathStringLessThan(std::wstring const& p_a, std::wstring const& p_b)
{
	return CSTR_LESS_THAN == CompareStringW(LOCALE_SYSTEM_DEFAULT, LINGUISTIC_IGNORECASE | SORT_DIGITSASNUMBERS, p_a.c_str(), p_a.size(), p_b.c_str(), p_b.size());
}
