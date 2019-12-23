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
	char const* m_titleTextString;
	char const* m_messageTextString;

	ChoiceDialogBoxListener* m_listener = 0;

	std::vector<AvoGUI::Button*> m_buttons;
	void positionButtons()
	{
		if (m_buttons.size() > 0)
		{
			float padding = 0.5f * (getHeight() - m_messageText->getBottom() - m_buttons[0]->getHeight());
			m_buttons.back()->setBottomRight(getWidth() - padding, getHeight() - padding);
			m_buttons.back()->invalidate();
			for (int32 a = m_buttons.size() - 2; a >= 0; a--)
			{
				m_buttons[a]->setBottomRight(m_buttons[a + 1]->getLeft() - padding, getHeight() - padding);
				m_buttons[a]->invalidate();
			}
		}
	}

public:
	ChoiceDialogBox(AvoGUI::Gui* p_parentGUI, char const* p_title, char const* p_message);
	~ChoiceDialogBox()
	{
		if (m_listener)
		{
			m_listener->handleChoiceDialogBoxClose(this);
		}
	}

	//------------------------------

	void setDialogBoxListener(ChoiceDialogBoxListener* p_listener)
	{
		m_listener = p_listener;
	}
	void handleButtonClick(AvoGUI::Button* p_button)
	{
		if (m_listener)
		{
			m_listener->handleDialogBoxChoice(this, p_button->getString());
			getWindow()->close();
		}
	}

	//------------------------------

	/*
		Added in the order the buttons are shown, from left to right.
	*/
	void addButton(char const* p_text, AvoGUI::Button::Emphasis p_emphasis)
	{
		// Will be called from other threads.
		excludeAnimationThread();
		AvoGUI::Button* button = new AvoGUI::Button(this, p_text, p_emphasis);
		button->addButtonListener(this);
		m_buttons.push_back(button);
		positionButtons();
		includeAnimationThread();
	}

	//------------------------------

	void createContent() override
	{
		enableMouseEvents();
		setThemeColor("background", Colors::dialogBoxBackground);
		setThemeColor("on background", Colors::dialogBoxOnBackground);
		setThemeColor("primary", Colors::primary);
		setThemeColor("primary on background", Colors::primaryOnBackground);

		m_titleBar = new TitleBar(this);

		AvoGUI::DrawingContext* context = getDrawingContext();

		m_titleText = context->createText(m_titleTextString, 22.f);
		m_titleText->setTopLeft(30.f, m_titleBar->getBottom() + 20.f);

		m_messageText = context->createText(m_messageTextString, 14.f, AvoGUI::Rectangle<float>(m_titleText->getLeft(), m_titleText->getBottom() + 20.f, getRight() - m_titleText->getLeft(), getBottom() - 50.f));
		m_messageText->setWordWrapping(AvoGUI::WordWrapping::WholeWord);
		m_messageText->setFontWeight((AvoGUI::FontWeight)400);
		m_messageText->setLineHeight(1.1f);
	}

	void handleSizeChange() override
	{
		m_titleBar->setWidth(getWidth());
		positionButtons();
	}

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
