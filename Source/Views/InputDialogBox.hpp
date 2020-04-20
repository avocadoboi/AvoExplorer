#pragma once

#include "TitleBar.hpp"
#include "../Resources/Colors.hpp"
#include "../Resources/Strings.hpp"
#include "../utilities.hpp"

//------------------------------

class InputDialogBox : public AvoGUI::Gui
{
public:
	static constexpr uint32 INITIAL_WIDTH = 400;
	static constexpr uint32 INITIAL_HEIGHT = 250;
	static constexpr float INPUT_PADDING = 3.f * 8.f;

private:
	TitleBar* m_titleBar{ nullptr };

	AvoGUI::Text* m_titleText{ nullptr };
	AvoGUI::Text* m_messageText{ nullptr };
	std::string m_titleTextString;
	std::string m_messageTextString;

	AvoGUI::TextField* m_inputField{ nullptr };
	AvoGUI::Button* m_okButton{ nullptr };

public:
	void createContent()
	{
		getParent()->getWindow()->disableUserInteraction();
		enableMouseEvents();

		setThemeColor(ThemeColors::background, Colors::dialogBoxBackground);
		setThemeColor(ThemeColors::onBackground, Colors::dialogBoxOnBackground);
		setThemeColor(ThemeColors::primary, Colors::primary);
		setThemeColor(ThemeColors::primaryOnBackground, Colors::primaryOnBackground);

		setThemeValue(ThemeValues::textFieldHeight, 2.2f);
		setThemeValue(ThemeValues::textFieldFontSize, 13.f);

		//------------------------------

		m_titleBar = new TitleBar(this);

		m_titleText = getDrawingContext()->createText(m_titleTextString, 22.f);
		m_titleText->setTopLeft(30.f, m_titleBar->getBottom() + 20.f);

		m_messageText = getDrawingContext()->createText(m_messageTextString, 14.f, AvoGUI::Rectangle<float>(m_titleText->getLeft(), m_titleText->getBottom() + 20.f, getRight() - m_titleText->getLeft(), getBottom() - 50.f));
		m_messageText->setWordWrapping(AvoGUI::WordWrapping::WholeWord);
		m_messageText->setFontWeight((AvoGUI::FontWeight)400);
		m_messageText->setLineHeight(1.1f);
		m_messageText->fitSizeToText();

		//------------------------------

		m_inputField = new AvoGUI::TextField(this, AvoGUI::TextField::Type::Filled);
		m_inputField->setTop(m_messageText->getBottom() + INPUT_PADDING);
		setKeyboardFocus(m_inputField);

		//------------------------------

		m_okButton = new AvoGUI::Button(this, Strings::ok, AvoGUI::Button::Emphasis::High);
		m_okButton->setCenterY(m_inputField->getCenterY());

		//------------------------------

		auto buttonClickListener = [=](auto) {
			getParent()->getWindow()->enableUserInteraction();
			dialogBoxInputListeners(m_inputField->getString());
			getWindow()->close();
		};
		m_inputField->getEditableText()->editableTextEnterListeners += buttonClickListener;
		m_okButton->buttonClickListeners += buttonClickListener;

		//------------------------------

		setHeight(m_inputField->getBottom() + INPUT_PADDING);
	}

	void handleSizeChange() override
	{
		m_titleBar->setWidth(getWidth());

		m_inputField->setLeft(INPUT_PADDING);
		m_okButton->setRight(getWidth() - INPUT_PADDING);
		m_inputField->setRight(m_okButton->getLeft() - INPUT_PADDING, false);
	}

	//------------------------------

	AvoGUI::EventListeners<void(std::string const&)> dialogBoxInputListeners;
	AvoGUI::EventListeners<void()> inputDialogBoxCloseListeners;

	void handleMouseDown(AvoGUI::MouseEvent const& p_event) override
	{
		setKeyboardFocus(nullptr);
	}

	AvoGUI::WindowBorderArea getWindowBorderAreaAtPosition(float p_x, float p_y) override
	{
		return m_titleBar->getWindowBorderAreaAtPosition(p_x, p_y);
	}

	//------------------------------

	void draw(AvoGUI::DrawingContext* p_context) override
	{
		p_context->setColor(getThemeColor(ThemeColors::onBackground));
		p_context->drawText(m_titleText);
		p_context->setColor(AvoGUI::Color(getThemeColor(ThemeColors::onBackground), 0.9));
		p_context->drawText(m_messageText);
	}
	void drawOverlay(AvoGUI::DrawingContext* p_context) override
	{
		p_context->setColor(Colors::dialogBoxOutline);
		p_context->strokeRectangle(getBounds());
	}

	//------------------------------

	InputDialogBox(AvoGUI::Gui* p_parentGui, std::string const& p_title, std::string const& p_message) :
		m_titleTextString(p_title), m_messageTextString(p_message)
	{
		create(p_title, INITIAL_WIDTH, INITIAL_HEIGHT, AvoGUI::WindowStyleFlags::CustomBorder, p_parentGui);
	}
	~InputDialogBox()
	{
		if (!getParent()->getWindow()->getIsUserInteractionEnabled())
		{
			getParent()->getWindow()->enableUserInteraction();
		}
		inputDialogBoxCloseListeners();
	}
};
