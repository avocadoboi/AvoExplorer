#include "ChoiceDialogBox.hpp"

//------------------------------

constexpr uint32 INITIAL_WIDTH = 400;
constexpr uint32 INITIAL_HEIGHT = 250;

//------------------------------

ChoiceDialogBox::ChoiceDialogBox(AvoGUI::Gui* p_parentGui, char const* p_title, char const* p_text) :
	m_titleTextString(p_title), m_messageTextString(p_text)
{
	create(p_title, INITIAL_WIDTH, INITIAL_HEIGHT, AvoGUI::WindowStyleFlags::CustomBorder, p_parentGui);
}
