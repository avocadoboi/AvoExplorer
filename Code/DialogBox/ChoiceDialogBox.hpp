#pragma once

#include "../TitleBar/TitleBar.hpp"
#include "../../Resources/Colors.hpp"
#include "../../Resources/Strings.hpp"

//------------------------------

class ChoiceDialogBox;

class ChoiceDialogBoxListener
{
public:
	virtual void handleDialogBoxChoice(ChoiceDialogBox* p_dialogBox, std::string const& p_choice) {}
	virtual void handleChoiceDialogBoxClose(ChoiceDialogBox* p_dialogBox) {}
};

//------------------------------

class ChoiceDialogBox : 
	public AvoGUI::Gui,
	public AvoGUI::ButtonListener
{
private:
	TitleBar* m_titleBar = 0;

	AvoGUI::Text* m_titleText = 0;
	AvoGUI::Text* m_messageText = 0;
	std::string m_titleTextString;
	std::string m_messageTextString;

	ChoiceDialogBoxListener* m_listener = 0;

	std::vector<AvoGUI::Button*> m_buttons;
	void positionButtons();

	std::vector<std::string> m_dialogArguments;

public:
	ChoiceDialogBox(AvoGUI::Gui* p_parentGUI, char const* p_title, char const* p_message);
	ChoiceDialogBox(AvoGUI::Gui* p_parentGUI, std::string const& p_title, std::string const& p_message);
	~ChoiceDialogBox()
	{
		if (!getParent()->getWindow()->getIsUserInteractionEnabled())
		{
			getParent()->getWindow()->enableUserInteraction();
		}
		if (m_listener)
		{
			m_listener->handleChoiceDialogBoxClose(this);
		}
	}

	//------------------------------

	void setChoiceDialogBoxListener(ChoiceDialogBoxListener* p_listener)
	{
		m_listener = p_listener;
	}
	void handleButtonClick(AvoGUI::Button* p_button)
	{
		getParent()->getWindow()->enableUserInteraction();
		if (m_listener)
		{
			m_listener->handleDialogBoxChoice(this, p_button->getString());
		}
		getWindow()->close();
	}

	//------------------------------

	/*
		Added in the order the buttons are shown, from left to right.
	*/
	void addButton(char const* p_text, AvoGUI::Button::Emphasis p_emphasis);

	void addDialogArgument(std::string const& p_argument)
	{
		m_dialogArguments.push_back(p_argument);
	}
	std::string getDialogArgument(uint32 p_index)
	{
		return m_dialogArguments[p_index];
	}
	uint32 getNumberOfDialogArguments()
	{
		return m_dialogArguments.size();
	}

	//------------------------------

	void createContent() override
	{
		getParent()->getWindow()->disableUserInteraction();

		enableMouseEvents();
		setThemeColor("background", Colors::dialogBoxBackground);
		setThemeColor("on background", Colors::dialogBoxOnBackground);
		setThemeColor("primary", Colors::primary);
		setThemeColor("primary on background", Colors::primaryOnBackground);

		m_titleBar = new TitleBar(this);
		m_titleBar->setWidth(getWidth());

		AvoGUI::DrawingContext* context = getDrawingContext();

		m_titleText = context->createText(m_titleTextString, 22.f);
		m_titleText->setTopLeft(30.f, m_titleBar->getBottom() + 20.f);

		m_messageText = context->createText(
			m_messageTextString, 14.f, 
			AvoGUI::Rectangle<float> 
			{ 
				m_titleText->getLeft(), m_titleText->getBottom() + 20.f, 
				getRight() - m_titleText->getLeft(), getBottom() 
			}
		);
		m_messageText->setWordWrapping(AvoGUI::WordWrapping::WholeWord);
		m_messageText->setFontWeight((AvoGUI::FontWeight)400);
		m_messageText->setLineHeight(1.1f);
		m_messageText->fitHeightToText();
		setHeight(m_messageText->getBottom() + m_messageText->getLeft());
	}

	void handleSizeChange(float p_lastWidth, float p_lastHeight) override;

	//------------------------------

	AvoGUI::WindowBorderArea getWindowBorderAreaAtPosition(float p_x, float p_y) override
	{
		return m_titleBar->getWindowBorderAreaAtPosition(p_x, p_y);
	}

	//------------------------------

	void draw(AvoGUI::DrawingContext* p_context) override
	{
		p_context->setColor(getThemeColor("on background"));
		p_context->drawText(m_titleText);
		p_context->setColor(AvoGUI::Color(getThemeColor("on background"), 0.9));
		p_context->drawText(m_messageText);
	}
	void drawOverlay(AvoGUI::DrawingContext* p_context) override
	{
		p_context->setColor(Colors::dialogBoxOutline);
		p_context->strokeRectangle(getBounds());
	}
};
