#pragma once

#include "../TitleBar/TitleBar.hpp"
#include "../../Resources/Colors.hpp"
#include "../../Resources/Strings.hpp"

//------------------------------

class DialogBoxListener
{
public:
	virtual void handleDialogBoxChoice(std::string const& p_choice) {}
	virtual void handleDialogBoxClose() {}
};

//------------------------------

class DialogBox : 
	public AvoGUI::GUI,
	public AvoGUI::ButtonListener
{
private:
	TitleBar* m_titleBar;

	AvoGUI::Text* m_titleText;
	AvoGUI::Text* m_text;
	char const* m_titleTextString;
	char const* m_textString;

	DialogBoxListener* m_listener;

	std::vector<AvoGUI::Button*> m_buttons;
	void positionButtons()
	{
		if (m_buttons.size() > 0)
		{
			float padding = 0.5f * (getHeight() - m_text->getBottom() - m_buttons[0]->getHeight());
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
	DialogBox(AvoGUI::GUI* p_parentGUI, char const* p_title, char const* p_text);
	~DialogBox()
	{
		if (m_listener)
		{
			m_listener->handleDialogBoxClose();
		}
	}

	//------------------------------

	void setDialogBoxListener(DialogBoxListener* p_listener)
	{
		m_listener = p_listener;
	}
	void handleButtonClick(AvoGUI::Button* p_button)
	{
		if (m_listener)
		{
			m_listener->handleDialogBoxChoice(p_button->getString());
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
		setThemeColor("background", Colors::dialogBoxBackground);
		setThemeColor("on background", Colors::dialogBoxOnBackground);
		setThemeColor("primary", Colors::primary);
		setThemeColor("primary on background", Colors::primaryOnBackground);

		m_titleBar = new TitleBar(this);

		AvoGUI::DrawingContext* context = getDrawingContext();

		m_titleText = context->createText(m_titleTextString, 22.f);
		m_titleText->setTopLeft(30.f, m_titleBar->getBottom() + 20.f);

		m_text = context->createText(m_textString, 14.f, AvoGUI::Rectangle<float>(m_titleText->getLeft(), m_titleText->getBottom() + 20.f, getRight() - m_titleText->getLeft(), getBottom() - 50.f));
		m_text->setWordWrapping(AvoGUI::WordWrapping::WholeWord);
		m_text->setFontWeight((AvoGUI::FontWeight)400);
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
		p_context->drawText(m_text);
	}
	void drawOverlay(AvoGUI::DrawingContext* p_context) override
	{
		p_context->setColor(getThemeColor("on background"));
		p_context->strokeRectangle(getBounds(), 1.f);
	}
};
