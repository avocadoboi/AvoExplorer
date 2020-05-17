#pragma once

#include "FileBrowserItems.hpp"
#include "Bookmarks.hpp"
#include "ContextMenu.hpp"
#include "../Components/IconLoader.hpp"
#include "../Resources/Ids.hpp"
#include "../Resources/Strings.hpp"
#include "../Resources/Colors.hpp"
#include "../Resources/ThemeValues.hpp"
#include "../utilities.hpp"

//------------------------------

/*
	This class is used in both the Bookmarks section and the FileBrowserItems section to represent a directory or file.
*/
class FileBrowserItem : public ContextView
{
public:
	static constexpr float FILE_HEIGHT = 16      * 8.f;
	static constexpr float FILE_NAME_PADDING = 1 * 8.f;
	static constexpr float BOOKMARK_HEIGHT = 4   * 8.f;

	static constexpr float CUT_OPACITY = 0.5f;

private:
	FileBrowserItems* m_fileBrowserItems = nullptr;
	Bookmarks* m_bookmarks = nullptr;

	void handleSizeChange() override
	{
		if (m_isFile && !m_isBookmark)
		{
			m_text_name.setBottomLeft(FILE_NAME_PADDING * 1.1f, getHeight() - FILE_NAME_PADDING);
		}
		else
		{
			m_text_name.setCenterY(getHeight() * 0.5f);
			m_text_name.setLeft(getHeight() - 1.f);
		}
	}
	void handleBoundsChange(AvoGUI::Rectangle<float> const& p_previousBounds) override
	{
		ContextView::handleBoundsChange(p_previousBounds);
	}

	//------------------------------

private:
	bool m_isBookmark;
public:
	void setIsBookmark(bool p_isBookmark)
	{
		m_isBookmark = p_isBookmark;
	}
	bool getIsBookmark()
	{
		return m_isBookmark;
	}

	//------------------------------

private:
	uint32 m_itemIndex = 0u;
public:
	void setItemIndex(uint32 p_index)
	{
		m_itemIndex = p_index;
	}
	void incrementItemIndex()
	{
		m_itemIndex++;
	}
	uint32 getItemIndex()
	{
		return m_itemIndex;
	}

	//------------------------------

private:
	bool m_isSelected{ false };

public:
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

private:
	AvoGUI::Image m_icon;
public:
	void setIcon(AvoGUI::Image const& p_image)
	{
		m_icon = p_image;
		invalidate();
	}
	AvoGUI::Image& getIcon()
	{
		return m_icon;
	}
	bool getHasLoadedIcon()
	{
		return m_icon;
	}

	//------------------------------

private:
	bool m_hasThumbnail = false;
public:
	bool getIsIconThumbnail()
	{
		return m_hasThumbnail;
	}

	//------------------------------

private:
	bool m_isFile = false;
public:
	bool getIsFile()
	{
		return m_isFile;
	}

private:
	std::filesystem::path m_path;
public:
	std::filesystem::path const& getPath()
	{
		return m_path;
	}

private:
	std::string m_name;
public:
	std::string const& getName()
	{
		return m_name;
	}

	//------------------------------

	void handleContextMenuItemChoice(ActionMenuItem* p_item) override
	{
		if (p_item->getAction() == Strings::removeBookmark)
		{
			m_bookmarks->removeBookmark(this);
		}
	}

	//------------------------------

	AvoGUI::DragDropOperation getDragDropOperation(AvoGUI::DragDropEvent const& p_event) override
	{
		if (p_event.modifierKeys & AvoGUI::ModifierKeyFlags::Control)
		{
			return AvoGUI::DragDropOperation::Copy;
		}
		return AvoGUI::DragDropOperation::Move;
	}
	void handleDragDropFinish(AvoGUI::DragDropEvent const& p_event) override
	{
		if (m_isFile)
		{
			m_fileBrowserItems->handleDragDropFinish(p_event);
		}
		else
		{
			if (m_isBookmark)
			{

			}
			else
			{

			}
		}
	}

	//------------------------------

private:
	AvoGUI::Color m_overlayColor{ 0.f, 0.f };

	AvoGUI::Animation* m_hoverAnimation = createAnimation(ThemeEasings::inOut, 70, [this](float p_value) {
		m_overlayColor = { getThemeColor(ThemeColors::onBackground), p_value * 0.15f };
		invalidate();
	});
public:
	void handleMouseEnter(AvoGUI::MouseEvent const& p_event) override
	{
		m_hoverAnimation->play(false);
	}
	void handleMouseLeave(AvoGUI::MouseEvent const& p_event) override
	{
		m_hoverAnimation->play(true);
	}
	void handleDragDropEnter(AvoGUI::DragDropEvent const& p_event) override
	{
		m_hoverAnimation->play(false);
	}
	void handleDragDropLeave(AvoGUI::DragDropEvent const& p_event) override
	{
		m_hoverAnimation->play(true);
	}
	void handleMouseDown(AvoGUI::MouseEvent const& p_event) override
	{
		if (p_event.mouseButton == AvoGUI::MouseButton::Left)
		{
			if (m_isBookmark)
			{
				m_positionAnimation->stop();
				m_isDragged = true;
				move(getParent<View>()->getAbsoluteTopLeft());
				setParent(getGui());
				setElevation(-1.f);
			}
			else if (m_isSelected)
			{
				m_isDragged = true;
			}
		}
		else
		{
			ContextView::handleMouseDown(p_event);
		}
	}
	void handleMouseUp(AvoGUI::MouseEvent const& p_event) override
	{
		if (p_event.mouseButton == AvoGUI::MouseButton::Left)
		{
			if (m_isDragged)
			{
				m_isDragged = false;

				if (m_isBookmark)
				{
					m_animationStartPosition = getAbsoluteTopLeft();
					//m_positionAnimationTime = 0.f;
					//queueAnimationUpdate();
					m_positionAnimation->replay();

					m_bookmarks->saveBookmarks();
				}
			}
			else if (!m_isBookmark)
			{
				bool isShiftDown = p_event.modifierKeys & AvoGUI::ModifierKeyFlags::Shift;
				if (p_event.modifierKeys & AvoGUI::ModifierKeyFlags::Control)
				{
					if (isShiftDown)
					{
						m_fileBrowserItems->selectItemsTo(this, true);
					}
					else if (m_isSelected)
					{
						m_fileBrowserItems->removeSelectedItem(this);
					}
					else
					{
						m_fileBrowserItems->addSelectedItem(this);
					}
				}
				else if (isShiftDown)
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
			ContextView::handleMouseUp(p_event);
		}
	}
	void handleMouseDoubleClick(AvoGUI::MouseEvent const& p_event) override
	{
		if (p_event.mouseButton == AvoGUI::MouseButton::Left)
		{
			open();
		}
	}
	void handleMouseMove(AvoGUI::MouseEvent const& p_event) override
	{
		if (m_isDragged)
		{
			if (m_isBookmark)
			{
				move(p_event.movementX, p_event.movementY);
				invalidate();

				m_bookmarks->handleBookmarkDrag(this);
			}
			else if (m_isSelected)
			{
				m_fileBrowserItems->dragSelectedItems();
				m_isDragged = false;
			}
		}
	}

	void open()
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

private:
	AvoGUI::Point<float> m_animationStartPosition;
	AvoGUI::Point<float> m_animationTargetPosition;
	//float m_positionAnimationTime = 0.f;
	bool m_isDragged = false;

	AvoGUI::Animation* m_positionAnimation = createAnimation(ThemeEasings::out, 200, [this](float p_value) {
		if (p_value < 1.f)
		{
			setAbsoluteTopLeft(AvoGUI::interpolate(m_animationStartPosition, m_animationTargetPosition, p_value));
			invalidate();
		}
		else if (getParent() == getGui())
		{
			setParent(m_bookmarks->getBookmarksContainer());
			move(-getParent<View>()->getAbsoluteTopLeft());
		}
	});

public:
	void setTargetPosition(float p_left, float p_top)
	{
		p_left += m_bookmarks->getBookmarksContainer()->getAbsoluteLeft();
		p_top += m_bookmarks->getBookmarksContainer()->getAbsoluteTop();
		if (getLeft() || getTop())
		{
			if (m_animationTargetPosition.x != p_left)
			{
				m_animationTargetPosition.set(p_left, p_top);
				if (!m_isDragged)
				{
					m_animationStartPosition = getAbsoluteTopLeft();
					m_positionAnimation->replay();
				}
			}
		}
		else if (!m_isDragged)
		{
			m_animationStartPosition.set(p_left, p_top);
			m_animationTargetPosition.set(p_left, p_top);
			setAbsoluteTopLeft(p_left, p_top);
			m_positionAnimation->stop();
			//m_positionAnimationTime = 1.f;
		}
	}
	void setTargetPosition(AvoGUI::Point<float> const& p_targetPosition)
	{
		setTargetPosition(p_targetPosition.x, p_targetPosition.y);
	}
	AvoGUI::Point<float> const& getTargetPosition()
	{
		return m_animationTargetPosition - m_bookmarks->getBookmarksContainer()->getAbsoluteTopLeft();
	}

	//------------------------------

	//void updateAnimations()
	//{
	//	if (m_isBookmark && !m_isDragged)
	//	{
	//		if (m_positionAnimationTime < 1.f)
	//		{
	//			float animationValue = getThemeEasing(ThemeEasings::out).easeValue(m_positionAnimationTime += getThemeValue(ThemeValues::positionAnimationSpeed));
	//			setAbsoluteTopLeft(AvoGUI::interpolate(m_animationStartPosition, m_animationTargetPosition, animationValue));
	//			queueAnimationUpdate();
	//		}
	//		else if (getParent() == getGui())
	//		{
	//			setParent(m_bookmarks->getBookmarksContainer());
	//			move(-getParent<View>()->getAbsoluteTopLeft());
	//		}
	//	}
	//	invalidate();
	//}

	//------------------------------

	void setIsCut(bool p_isCut)
	{
		setOpacity(p_isCut ? CUT_OPACITY : 1.f);
		invalidate();
	}

	//------------------------------

protected:
	void updateClipGeometry() override
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

private:
	AvoGUI::Text m_text_name;

public:
	void draw(AvoGUI::DrawingContext* p_context) override
	{
		p_context->setColor(Colors::fileBrowserItemBackground);
		p_context->fillRectangle(getSize());

		if (m_isSelected)
		{
			p_context->setColor(AvoGUI::Color(getThemeColor(ThemeColors::selection)));
			p_context->fillRectangle(getSize());
		}

		auto& gradient = m_fileBrowserItems->getFileNameEndGradient();
		if (!m_isBookmark && m_text_name.getRight() - getWidth() > gradient.getStartPositionX())
		{
			gradient.setOffsetX(getWidth());
			p_context->setGradient(gradient);
		}
		else
		{
			p_context->setColor(getThemeColor(ThemeColors::onBackground));
		}
		p_context->drawText(m_text_name);

		if (m_icon)
		{
			if (m_isFile && !m_isBookmark)
			{
				m_icon.setBounds(0.f, FILE_NAME_PADDING, getWidth(), m_text_name.getTop() - FILE_NAME_PADDING);
				m_icon.setBoundsSizing(AvoGUI::ImageBoundsSizing::Contain);
			}
			else
			{
				m_icon.setSize(getHeight() * 0.65f);
				m_icon.setCenterY(getHeight() * 0.5f);
				m_icon.setLeft(m_icon.getTop());
			}

			p_context->drawImage(m_icon);
		}

		p_context->setColor(m_overlayColor);
		p_context->fillRectangle(getSize());
	}

	//------------------------------

	FileBrowserItem(AvoGUI::View* p_parent, std::filesystem::path const& p_path, bool p_isBookmark) :
		ContextView(p_parent),
		m_isBookmark(p_isBookmark)
	{
		setCornerRadius(6.f);
		enableMouseEvents();
		enableDragDropEvents();

		//------------------------------

		std::wstring pathString = p_path.native();
		for (uint32 a = 0; a < pathString.size(); a++)
		{
			if (pathString[a] == L'/')
			{
				pathString[a] = L'\\';
			}
		}
		m_path = pathString;

		// I got a _Sharing_Violation on some files for some reason, probaly a bug because it doesn't happen when iterating the directory.
		try
		{
			m_isFile = std::filesystem::is_regular_file(m_path);
		}
		catch (std::filesystem::filesystem_error)
		{
			m_isFile = true;
		}

		std::string extension = m_path.extension().u8string();
		m_hasThumbnail =
			m_isFile && (
				extension == u8".jpg" || extension == u8".JPG" ||
				extension == u8".png" || extension == u8".PNG"
				);

		//------------------------------

		if (m_isFile)
		{
			m_name = p_path.filename().u8string();
		}
		else
		{
			if (p_path.root_path() == p_path)
			{
				m_name = p_path.root_name().u8string();
			}
			else if (p_path.has_filename())
			{
				m_name = p_path.filename().u8string();
			}
			else if (p_path.has_parent_path())
			{
				m_name = p_path.parent_path().filename().u8string();
			}
		}

		//------------------------------

		m_fileBrowserItems = getComponentById<FileBrowserItems>(Ids::fileBrowserItems);
		m_bookmarks = getComponentById<Bookmarks>(Ids::bookmarks);

		//------------------------------

		m_text_name = getDrawingContext()->createText(m_name, 11.f);
		m_text_name.setIsTopTrimmed(true);
		m_text_name.fitHeightToText();

		if (m_isBookmark)
		{
			setHeight(BOOKMARK_HEIGHT);
			setWidth(m_text_name.getRight() + 0.5f * (BOOKMARK_HEIGHT - m_text_name.getHeight()));

			getComponentById<IconLoader>(Ids::iconLoader)->requestIconLoadingForItem(this);

			addContextMenuItem(Strings::removeBookmark);
			setContextMenuWidth(170.f);
		}
	}
};
