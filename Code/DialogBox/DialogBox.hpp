#pragma once

#include "../TitleBar/TitleBar.hpp"
#include "../../Resources/Colors.hpp"

//------------------------------

class DialogBox : public AvoGUI::GUI
{
private:
	TitleBar* m_titleBar;

	AvoGUI::Text* m_titleText;
	AvoGUI::Text* m_text;

public:
	enum class Buttons
	{
		YesNo,
		YesNoCancel,
		OkCancel
	};

	DialogBox(AvoGUI::GUI* p_parentGUI, char const* p_title, char const* p_text, Buttons p_buttons = Buttons::YesNo);

	AvoGUI::WindowBorderArea getWindowBorderAreaAtPosition(float p_x, float p_y) override
	{
		return m_titleBar->getWindowBorderAreaAtPosition(p_x, p_y);
	}

};
