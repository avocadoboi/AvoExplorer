#include "ChoiceDialogBox.hpp"

//------------------------------

constexpr uint32 INITIAL_WIDTH = 400;
constexpr uint32 INITIAL_HEIGHT = 250;

constexpr float BUTTON_MARGIN = 2 * 8.f;

//
// Private
//

void ChoiceDialogBox::positionButtons()
{
	if (m_buttons.size() > 0)
	{
		if (m_buttons.size() == 1)
		{
			m_buttons[0]->setCenter(getCenter());
			m_buttons[0]->setBottom(getHeight() - BUTTON_MARGIN);
		}
		else
		{
			m_buttons.back()->setBottomRight(getWidth() - BUTTON_MARGIN, getHeight() - BUTTON_MARGIN);
			for (int32 a = m_buttons.size() - 2; a >= 0; a--)
			{
				m_buttons[a]->setBottomRight(m_buttons[a + 1]->getLeft() - BUTTON_MARGIN, getHeight() - BUTTON_MARGIN);
			}
		}
	}
}

//
// Public
//

ChoiceDialogBox::ChoiceDialogBox(AvoGUI::Gui* p_parentGui, char const* p_title, char const* p_text) :
	m_titleTextString(p_title), m_messageTextString(p_text)
{
	create(p_title, INITIAL_WIDTH, INITIAL_HEIGHT, AvoGUI::WindowStyleFlags::CustomBorder, p_parentGui);
}

void ChoiceDialogBox::addButton(char const* p_text, AvoGUI::Button::Emphasis p_emphasis)
{
	// Will be called from other threads.
	excludeAnimationThread();
	AvoGUI::Button* button = new AvoGUI::Button(this, p_text, p_emphasis);
	button->addButtonListener(this);
	m_buttons.push_back(button);
	setHeight(m_messageText->getBottom() + m_messageText->getLeft() + button->getHeight() + BUTTON_MARGIN);
	positionButtons();
	includeAnimationThread();
}
