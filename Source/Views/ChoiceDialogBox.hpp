#pragma once

#include "TitleBar.hpp"
#include "../Resources/Colors.hpp"
#include "../Resources/Strings.hpp"
#include "../utilities.hpp"

//------------------------------

class ChoiceDialogBox : public AvoGUI::Gui
{
public:
	static constexpr uint32 INITIAL_WIDTH = 400;
	static constexpr uint32 INITIAL_HEIGHT = 250;
	static constexpr float BUTTON_MARGIN = 2 * 8.f;

public:
	AvoGUI::EventListeners<void(std::string const&)> dialogBoxChoiceListeners;
	AvoGUI::EventListeners<void()> choiceDialogBoxCloseListeners;

	//------------------------------

private:
	std::vector<AvoGUI::Button*> m_buttons;
	void positionButtons()
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
public:
	/*
		Added in the order the buttons are shown, from left to right.
	*/
	void addButton(std::string const& p_text, AvoGUI::Button::Emphasis p_emphasis)
	{
		// Will be called from other threads.
		excludeAnimationThread();
		AvoGUI::Button* button = new AvoGUI::Button(this, p_text, p_emphasis);
		button->buttonClickListeners += [this, button](AvoGUI::Button*) {
			getParent()->getWindow()->enableUserInteraction();
			dialogBoxChoiceListeners(button->getString());
			getWindow()->close();
		};
		m_buttons.push_back(button);
		positionButtons();
		if (m_buttons.size() == 1)
		{
			setHeight(m_messageText.getBottom() + m_messageText.getLeft() + button->getHeight() + BUTTON_MARGIN);
		}
		includeAnimationThread();
	}

	//------------------------------

private:
	std::vector<std::string> m_dialogArguments;
public:
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

private:
	TitleBar* m_titleBar{ nullptr };

	AvoGUI::Text m_titleText;
	AvoGUI::Text m_messageText;
	std::string m_titleTextString;
	std::string m_messageTextString;

public:
	void handleSizeChange(float p_lastWidth, float p_lastHeight) override
	{
		if (p_lastWidth != getWidth())
		{
			m_titleBar->setWidth(getWidth());
			m_messageText.setRight(getWidth() - m_titleText.getLeft(), false);
			m_messageText.fitHeightToText();
			setHeight(m_messageText.getBottom() + m_messageText.getLeft() + (m_buttons.size() ? m_buttons[0]->getHeight() + BUTTON_MARGIN : 0.f));
			positionButtons();
			invalidate();
		}
	}

	//------------------------------

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

	ChoiceDialogBox(AvoGUI::Gui* p_parentGui, std::string const& p_title, std::string const& p_message) :
		m_titleTextString(p_title), m_messageTextString(p_message)
	{
		create(p_title, INITIAL_WIDTH, INITIAL_HEIGHT, AvoGUI::WindowStyleFlags::CustomBorder, p_parentGui);

		getParent()->getWindow()->disableUserInteraction();

		enableMouseEvents();
		setThemeColor(ThemeColors::background, Colors::dialogBoxBackground);
		setThemeColor(ThemeColors::onBackground, Colors::dialogBoxOnBackground);
		setThemeColor(ThemeColors::primary, Colors::primary);
		setThemeColor(ThemeColors::primaryOnBackground, Colors::primaryOnBackground);

		m_titleBar = new TitleBar(this);
		m_titleBar->setWidth(getWidth());

		m_titleText = getDrawingContext()->createText(m_titleTextString, 22.f);
		m_titleText.setTopLeft(30.f, m_titleBar->getBottom() + 20.f);

		m_messageText = getDrawingContext()->createText(
			m_messageTextString, 14.f,
			{
				m_titleText.getLeft(), m_titleText.getBottom() + 20.f,
				getRight() - m_titleText.getLeft(), getBottom()
			}
		);
		m_messageText.setWordWrapping(AvoGUI::WordWrapping::WholeWord);
		m_messageText.setFontWeight((AvoGUI::FontWeight)400);
		m_messageText.setLineHeight(1.1f);
		m_messageText.fitHeightToText();
	}
	~ChoiceDialogBox()
	{
		if (!getParent()->getWindow()->getIsUserInteractionEnabled())
		{
			getParent()->getWindow()->enableUserInteraction();
		}
		choiceDialogBoxCloseListeners();
	}
};
