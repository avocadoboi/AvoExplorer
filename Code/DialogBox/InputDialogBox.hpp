#pragma once

#include "../TitleBar/TitleBar.hpp"
#include "../../Resources/Colors.hpp"
#include "../../Resources/Strings.hpp"

//------------------------------

class InputDialogBox;

class InputDialogBoxListener
{
public:
	virtual void handleDialogBoxInput(InputDialogBox* p_dialogBox, std::string const& p_choice) {}
	virtual void handleInputDialogBoxClose(InputDialogBox* p_dialogBox) {}
};

//------------------------------

class InputDialogBox :
	public AvoGUI::Gui
{
private:
	TitleBar* m_titleBar = 0;

	AvoGUI::Text* m_titleText = 0;
	AvoGUI::Text* m_messageText = 0;
	char const* m_titleTextString;
	char const* m_messageTextString;

	AvoGUI::TextField* m_inputField = 0;
	AvoGUI::Button* m_okButton = 0;

public:
	InputDialogBox(AvoGUI::Gui* p_parentGui, char const* p_title, char const* p_message);

	void createContent();

	void handleSizeChange();

	//------------------------------

	void handleMouseDown(AvoGUI::MouseEvent const& p_event) override
	{
		setKeyboardFocus(0);
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
