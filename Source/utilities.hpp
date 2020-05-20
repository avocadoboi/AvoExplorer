#pragma once

#include <AvoGUI.hpp>
#include <Windows.h>
#undef max
#undef min

//------------------------------

namespace ThemeColors = Avo::ThemeColors;
namespace ThemeEasings = Avo::ThemeEasings;
using Avo::Id;

//------------------------------

constexpr float operator "" grid(long double p_value)
{
	return p_value * 8.f;
}

constexpr float operator "" grid(unsigned long long int p_value)
{
	return p_value * 8.f;
}

//------------------------------

inline Avo::Image loadImageFromResource(uint32 p_resourceID, Avo::DrawingContext* p_context)
{
	HRSRC resource = FindResourceW(0, MAKEINTRESOURCEW(p_resourceID), L"IMAGE");
	if (resource)
	{
		return p_context->createImage((uint8*)LockResource(LoadResource(0, resource)), SizeofResource(0, resource));
	}
	return Avo::Image();
}

/*
	Returns true if p_a is less than p_b.
*/
inline bool getIsPathStringLessThan(std::wstring const& p_a, std::wstring const& p_b)
{
	return CSTR_LESS_THAN == CompareStringW(LOCALE_SYSTEM_DEFAULT, LINGUISTIC_IGNORECASE | SORT_DIGITSASNUMBERS, p_a.c_str(), p_a.size(), p_b.c_str(), p_b.size());
}
