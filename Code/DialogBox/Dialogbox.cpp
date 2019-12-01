#include "DialogBox.hpp"

//------------------------------

uint32 const INITIAL_WIDTH = 600;
uint32 const INITIAL_HEIGHT = 400;

//------------------------------

DialogBox::DialogBox(AvoGUI::GUI* p_parentGUI, char const* p_title, char const* p_text, Buttons p_buttons = Buttons::YesNo)
{
	create(p_title, INITIAL_WIDTH, INITIAL_HEIGHT, AvoGUI::WindowStyleFlags::CustomBorder);
}
