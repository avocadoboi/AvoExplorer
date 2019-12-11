#pragma once

#include "FileBrowserItems.hpp"
#include "../../TopBar/Bookmarks/Bookmarks.hpp"

//------------------------------

/*
	This class is used in both the Bookmarks section and the FileBrowserItems section to represent a directory or file.
*/

class FileBrowserItem :
	public AvoGUI::View
{
private:
	FileBrowserItems* m_fileBrowserItems;
	Bookmarks* m_bookmarks;

	AvoGUI::Image* m_icon;

	std::filesystem::path m_path;
	std::filesystem::path m_name;
	AvoGUI::Text* m_text_name;
	bool m_isFile;
	bool m_hasThumbnail;
	bool m_isBookmark;

	float m_hoverAnimationTime;
	float m_hoverAnimationValue;
	bool m_isHovering;
	bool m_isSelected;

public:
	FileBrowserItem(AvoGUI::View* p_parent, std::filesystem::path const& p_path, bool p_isBookmark);
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

	void setIsBookmark(bool p_isBookmark)
	{
		m_isBookmark = p_isBookmark;
	}
	bool getIsBookmark()
	{
		return m_isBookmark;
	}

	//------------------------------

	void select()
	{
		m_isSelected = true;
		invalidate();
	}
	void deselect()
	{
		m_isSelected = false;
		invalidate();
	}
	bool getIsSelected()
	{
		return m_isSelected;
	}

	//------------------------------

	void setIcon(AvoGUI::Image* p_image);
	bool getHasLoadedIcon()
	{
		return m_icon;
	}
	bool getIsIconThumbnail()
	{
		return m_hasThumbnail;
	}
	bool getIsFile()
	{
		return m_isFile;
	}
	std::filesystem::path const& getPath()
	{
		return m_path;
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
		if (!m_isBookmark)
		{
			if (p_event.modifierKeys & AvoGUI::ModifierKeyFlags::Ctrl)
			{
				if (m_isSelected)
				{
					m_fileBrowserItems->removeSelectedItem(this);
				}
				else
				{
					m_fileBrowserItems->addSelectedItem(this);
				}
			}
			else if (p_event.modifierKeys & AvoGUI::ModifierKeyFlags::Shift)
			{
				m_fileBrowserItems->selectItemsTo(this);
			}
			else
			{
				m_fileBrowserItems->setSelectedItem(this);
			}
		}
	}
	void handleMouseDoubleClick(AvoGUI::MouseEvent const& p_event) override
	{
		if (m_isFile)
		{
			ShellExecuteW((HWND)getGui()->getWindow()->getNativeHandle(), 0, m_path.c_str(), 0, m_path.parent_path().c_str(), SHOW_OPENWINDOW);
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

	void draw(AvoGUI::DrawingContext* p_context) override;
};
