#include "DialogBox.hpp"

//------------------------------

uint32 constexpr INITIAL_WIDTH = 400;
uint32 constexpr INITIAL_HEIGHT = 250;

//------------------------------

DialogBox::DialogBox(AvoGUI::Gui* p_parentGUI, char const* p_title, char const* p_text) :
	m_titleBar(0), m_titleText(0), m_text(0), m_titleTextString(p_title), m_textString(p_text),
	m_listener(0)
{
	create(p_title, INITIAL_WIDTH, INITIAL_HEIGHT, AvoGUI::WindowStyleFlags::CustomBorder, p_parentGUI);
}
