#pragma once

#include "FileBrowserItems.hpp"

//------------------------------

class FileBrowserItem :
	public AvoGUI::View
{
private:
	FileBrowserItems* m_fileBrowserItems;

	AvoGUI::Image* m_icon;

	std::filesystem::path m_path;
	std::filesystem::path m_name;
	AvoGUI::Text* m_text_name;
	bool m_isFile;

	float m_hoverAnimationTime;
	float m_hoverAnimationValue;
	bool m_isHovering;

public:
	FileBrowserItem(FileBrowserItems* p_parent, std::filesystem::path const& p_path, bool p_isFile);
	~FileBrowserItem()
	{
		if (m_icon)
		{
			m_icon->forget();
		}
		if (m_text_name)
		{
			m_text_name->forget();
		}
	}

	//------------------------------

	void loadIcon(IImageList2* p_imageList, IThumbnailCache* p_thumbnailCache);
	bool getHasLoadedIcon()
	{
		return m_icon;
	}

	//------------------------------

	void handleMouseEnter(AvoGUI::MouseEvent const& p_event) override
	{
		m_isHovering = true;
		queueAnimationUpdate();
	}
	void handleMouseLeave(AvoGUI::MouseEvent const& p_event) override
	{
		m_isHovering = false;
		queueAnimationUpdate();
	}
	void handleMouseDown(AvoGUI::MouseEvent const& p_event) override
	{
		if (m_fileBrowserItems->getSelectedItem() != this)
		{
			if (m_fileBrowserItems->getSelectedItem())
			{
				m_fileBrowserItems->getSelectedItem()->invalidate();
			}
			m_fileBrowserItems->setSelectedItem(this);
			invalidate();
		}
	}
	void handleMouseDoubleClick(AvoGUI::MouseEvent const& p_event) override
	{
		if (m_isFile)
		{
			ShellExecuteW((HWND)getGUI()->getWindow()->getWindowHandle(), 0, m_path.c_str(), 0, 0, SHOW_OPENWINDOW);
		}
		else
		{
			m_fileBrowserItems->getFileBrowser()->setWorkingDirectory(m_path);
		}
	}

	//------------------------------

	void updateAnimations()
	{
		if (m_isHovering)
		{
			if (m_hoverAnimationTime < 1.f)
			{
				m_hoverAnimationValue = getThemeEasing("in out").easeValue(m_hoverAnimationTime += getThemeValue("hover animation speed"));
				queueAnimationUpdate();
			}
			else
			{
				m_hoverAnimationTime = 1.f;
			}
		}
		else
		{
			if (m_hoverAnimationTime > 0.f)
			{
				m_hoverAnimationValue = getThemeEasing("in out").easeValue(m_hoverAnimationTime -= getThemeValue("hover animation speed"));
				queueAnimationUpdate();
			}
			else
			{
				m_hoverAnimationTime = 0.f;
			}
		}
		invalidate();
	}

	//------------------------------

	void draw(AvoGUI::DrawingContext* p_context) override
	{
		p_context->setColor(Colors::fileBrowserItemBackground);
		p_context->fillRectangle(getSize());
		p_context->setColor(getThemeColor("on background"));
		p_context->drawText(m_text_name);
		if (m_icon)
		{
			p_context->drawImage(m_icon);
		}
		if (m_fileBrowserItems->getSelectedItem() == this)
		{
			p_context->setColor(AvoGUI::Color(getThemeColor("selection")));
		}
		else
		{
			p_context->setColor(AvoGUI::Color(getThemeColor("on background"), m_hoverAnimationValue * 0.2f));
		}
		p_context->fillRectangle(getSize());
	}
};
