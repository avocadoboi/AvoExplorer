#pragma once

#include "TitleBar.hpp"
#include "../Resources/Colors.hpp"
#include "../Resources/Strings.hpp"
#include "../utilities.hpp"

//------------------------------

class InputDialogBox : public Avo::Gui
{
public:
	static constexpr uint32 INITIAL_WIDTH = 400;
	static constexpr uint32 INITIAL_HEIGHT = 250;
	static constexpr float INPUT_PADDING = 3.f * 8.f;

private:
	TitleBar* m_titleBar{ nullptr };

	Avo::Text m_titleText;
	Avo::Text m_messageText;
	std::string m_titleTextString;
	std::string m_messageTextString;

	Avo::TextField* m_inputField{ nullptr };
	Avo::Button* m_okButton{ nullptr };

public:
	void handleSizeChange() override
	{
		m_titleBar->setWidth(getWidth());

		m_inputField->setLeft(INPUT_PADDING);
		m_okButton->setRight(getWidth() - INPUT_PADDING);
		m_inputField->setRight(m_okButton->getLeft() - INPUT_PADDING, false);
	}

	//------------------------------

	Avo::EventListeners<void(std::string const&)> dialogBoxInputListeners;
	Avo::EventListeners<void()> inputDialogBoxCloseListeners;

	void handleMouseDown(Avo::MouseEvent const& p_event) override
	{
		setKeyboardFocus(nullptr);
	}

	Avo::WindowBorderArea getWindowBorderAreaAtPosition(float p_x, float p_y) override
	{
		return m_titleBar->getWindowBorderAreaAtPosition(p_x, p_y);
	}

	//------------------------------

	void draw(Avo::DrawingContext* p_context) override
	{
		p_context->setColor(getThemeColor(ThemeColors::onBackground));
		p_context->drawText(m_titleText);
		p_context->setColor(Avo::Color(getThemeColor(ThemeColors::onBackground), 0.9));
		p_context->drawText(m_messageText);
	}
	void drawOverlay(Avo::DrawingContext* p_context) override
	{
		p_context->setColor(Colors::dialogBoxOutline);
		p_context->strokeRectangle(getBounds());
	}

	//------------------------------

	InputDialogBox(Avo::Gui* p_parentGui, std::string const& p_title, std::string const& p_message) :
		m_titleTextString(p_title), m_messageTextString(p_message)
	{
		create(p_title, INITIAL_WIDTH, INITIAL_HEIGHT, Avo::WindowStyleFlags::CustomBorder, p_parentGui);

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
		m_titleText.setTopLeft(30.f, m_titleBar->getBottom() + 20.f);

		m_messageText = getDrawingContext()->createText(m_messageTextString, 14.f, Avo::Rectangle<float>(m_titleText.getLeft(), m_titleText.getBottom() + 20.f, getRight() - m_titleText.getLeft(), getBottom() - 50.f));
		m_messageText.setWordWrapping(Avo::WordWrapping::WholeWord);
		m_messageText.setFontWeight((Avo::FontWeight)400);
		m_messageText.setLineHeight(1.1f);
		m_messageText.fitSizeToText();

		//------------------------------

		m_inputField = new Avo::TextField(this, Avo::TextField::Type::Filled);
		m_inputField->setTop(m_messageText.getBottom() + INPUT_PADDING);
		setKeyboardFocus(m_inputField);

		//------------------------------

		m_okButton = new Avo::Button(this, Strings::ok, Avo::Button::Emphasis::High);
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
	~InputDialogBox()
	{
		if (!getParent()->getWindow()->getIsUserInteractionEnabled())
		{
			getParent()->getWindow()->enableUserInteraction();
		}
		inputDialogBoxCloseListeners();
	}
};
