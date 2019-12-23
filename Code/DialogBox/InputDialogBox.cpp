#include "InputDialogBox.hpp"

//------------------------------

constexpr uint32 INITIAL_WIDTH = 400;
constexpr uint32 INITIAL_HEIGHT = 250;
constexpr float INPUT_PADDING = 3.f * 8.f;

//------------------------------

InputDialogBox::InputDialogBox(AvoGUI::Gui* p_parentGui, char const* p_title, char const* p_message) :
	m_titleTextString(p_title), m_messageTextString(p_message)
{
	create(p_title, INITIAL_WIDTH, INITIAL_HEIGHT, AvoGUI::WindowStyleFlags::CustomBorder);
}

void InputDialogBox::createContent()
{
	enableMouseEvents();
	setThemeColor("background", Colors::dialogBoxBackground);
	setThemeColor("on background", Colors::dialogBoxOnBackground);
	setThemeColor("primary", Colors::primary);
	setThemeColor("primary on background", Colors::primaryOnBackground);

	setThemeValue("text field height", 2.2f);
	setThemeValue("text field font size", 13.f);

	m_titleBar = new TitleBar(this);

	AvoGUI::DrawingContext* context = getDrawingContext();

	m_titleText = context->createText(m_titleTextString, 22.f);
	m_titleText->setTopLeft(30.f, m_titleBar->getBottom() + 20.f);

	m_messageText = context->createText(m_messageTextString, 14.f, AvoGUI::Rectangle<float>(m_titleText->getLeft(), m_titleText->getBottom() + 20.f, getRight() - m_titleText->getLeft(), getBottom() - 50.f));
	m_messageText->setWordWrapping(AvoGUI::WordWrapping::WholeWord);
	m_messageText->setFontWeight((AvoGUI::FontWeight)400);
	m_messageText->setLineHeight(1.1f);
	m_messageText->fitSizeToText();

	m_inputField = new AvoGUI::TextField(this, AvoGUI::TextField::Type::Filled);
	m_inputField->setTop(m_messageText->getBottom() + INPUT_PADDING);
	setKeyboardFocus(m_inputField);

	m_okButton = new AvoGUI::Button(this, Strings::ok, AvoGUI::Button::Emphasis::High);
	m_okButton->setCenterY(m_inputField->getCenterY());
	setHeight(m_inputField->getBottom() + INPUT_PADDING);
}

void InputDialogBox::handleSizeChange()
{
	m_titleBar->setWidth(getWidth());

	m_inputField->setLeft(INPUT_PADDING);
	m_okButton->setRight(getWidth() - INPUT_PADDING);
	m_inputField->setRight(m_okButton->getLeft() - INPUT_PADDING, false);
}
