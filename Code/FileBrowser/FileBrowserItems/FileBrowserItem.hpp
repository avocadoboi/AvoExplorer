#pragma once

#include "FileBrowserItems.hpp"
#include "../../TopBar/Bookmarks/Bookmarks.hpp"
#include "../../ActionMenu/ContextMenu.hpp"

//------------------------------

/*
	This class is used in both the Bookmarks section and the FileBrowserItems section to represent a directory or file.
*/

class FileBrowserItem :
	public ContextView
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
	AvoGUI::Point<float> m_animationStartPosition;
	AvoGUI::Point<float> m_animationTargetPosition;
	float m_positionAnimationTime;

	float m_hoverAnimationTime;
	float m_hoverAnimationValue;
	bool m_isHovering;
	bool m_isSelected;

protected:
	void updateClipGeometry()
	{
		if (m_isBookmark)
		{
			View::updateClipGeometry();
		}
		else if (m_fileBrowserItems)
		{
			setClipGeometry(m_isFile ? m_fileBrowserItems->getFileGeometry() : m_fileBrowserItems->getDirectoryGeometry());
		}
	}

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

	void handleSizeChange() override;

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
		if (p_event.mouseButton == AvoGUI::MouseButton::Left)
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
		else
		{
			ContextView::handleMouseDown(p_event);
		}
	}
	void handleMouseDoubleClick(AvoGUI::MouseEvent const& p_event) override
	{
		if (p_event.mouseButton == AvoGUI::MouseButton::Left)
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
	}

	//------------------------------

	void setTargetPosition(float p_left, float p_top)
	{
		if (getLeft() || getTop())
		{
			m_animationStartPosition = getTopLeft();
			m_animationTargetPosition.set(p_left, p_top);
			m_positionAnimationTime = 0.f;
			queueAnimationUpdate();
		}
		else
		{
			m_animationStartPosition.set(p_left, p_top);
			m_animationTargetPosition.set(p_left, p_top);
			setTopLeft(p_left, p_top);
			m_positionAnimationTime = 1.f;
		}
	}
	void setTargetPosition(AvoGUI::Point<float> const& p_targetPosition)
	{
		setTargetPosition(p_targetPosition.x, p_targetPosition.y);
	}
	AvoGUI::Point<float> const& getTargetPosition()
	{
		return m_animationTargetPosition;
	}

	//------------------------------

	void updateAnimations()
	{
		if (m_isBookmark)
		{
			if (m_positionAnimationTime < 1.f)
			{
				float animationValue = getThemeEasing("out").easeValue(m_positionAnimationTime += getThemeValue("position animation speed"));
				setTopLeft(AvoGUI::interpolate(m_animationStartPosition, m_animationTargetPosition, animationValue));
				queueAnimationUpdate();
			}
		}
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
