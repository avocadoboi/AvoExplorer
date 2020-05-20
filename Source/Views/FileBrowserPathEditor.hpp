#pragma once

#include "FileBrowser.hpp"
#include "Bookmarks.hpp"

//------------------------------

class FileBrowserPathEditorDirectoryButton : public Avo::View
{
public:
	static constexpr float PADDING_HORIZONTAL = 1 * 8.f;
	static constexpr float PADDING_VERTICAL = 1.f * 8.f;

private:
	std::filesystem::path m_path;
public:
	void handleMouseUp(Avo::MouseEvent const& p_event) override
	{
		if (p_event.mouseButton == Avo::MouseButton::Left && getSize().getIsContaining(p_event.x, p_event.y))
		{
			getComponentById<FileBrowser>(Ids::fileBrowser)->setWorkingDirectory(m_path);
		}
	}

private:
	Avo::Text m_text;
public:
	void draw(Avo::DrawingContext* p_context) override
	{
		p_context->setColor(getThemeColor(ThemeColors::onBackground));
		p_context->drawText(m_text);
	}

	//------------------------------

	FileBrowserPathEditorDirectoryButton(Avo::View* p_parent, std::filesystem::path const& p_path, std::string const& p_name) :
		View(p_parent), 
		m_path(p_path)
	{
		m_text = getDrawingContext()->createText(p_name, 15.f);
		m_text.setIsTopTrimmed(false);
		m_text.fitSizeToText();

		setSize(m_text.getWidth() + 2.f * PADDING_HORIZONTAL, 15 + 2.f * PADDING_VERTICAL);

		m_text.setCenterX(getCenterX());
		m_text.setTop(PADDING_VERTICAL);

		new Avo::Ripple(this, Avo::Color(getThemeColor(ThemeColors::onBackground), 0.2f));

		setCornerRadius(5.f);
		enableMouseEvents();
		setCursor(Avo::Cursor::Hand);
	}
};

//------------------------------

class FileBrowserPathEditorPath : public Avo::View
{
public:
	static constexpr float DIRECTORY_SEPARATOR_MARGIN = -0.25f * 8.f;
	static constexpr float DIRECTORY_SEPARATOR_SIZE = 2.f      * 8.f;

private:
	Avo::Text m_directorySeparatorIcon{ getDrawingContext()->createText(MaterialIcons::CHEVRON_RIGHT, DIRECTORY_SEPARATOR_SIZE) };
	std::vector<FileBrowserPathEditorDirectoryButton*> m_directoryButtons;
public:
	void setWorkingDirectory(std::filesystem::path const& p_path)
	{
		std::string pathString = p_path.u8string();

		removeAllChildViews();

		m_directoryButtons.clear();

		uint32 directoryStartIndex = 0;
		for (uint32 a = 0; a <= pathString.size(); a++)
		{
			if (a == pathString.size() || pathString[a] == '/' || pathString[a] == '\\')
			{
				FileBrowserPathEditorDirectoryButton* button = new FileBrowserPathEditorDirectoryButton(this, std::filesystem::u8path(pathString.substr(0, a) + '/'), pathString.substr(directoryStartIndex, a - directoryStartIndex));
				button->setCenterY(getHeight() * 0.5f);
				if (m_directoryButtons.size())
				{
					button->setLeft(m_directoryButtons.back()->getRight() + 2.f * DIRECTORY_SEPARATOR_MARGIN + m_directorySeparatorIcon.getWidth());
				}
				else
				{
					button->setLeft(button->getTop());
				}
				m_directoryButtons.push_back(button);

				if (a == pathString.size() - 1)
				{
					break;
				}
				directoryStartIndex = a + 1;
			}
		}
		setWidth(m_directoryButtons.back()->getRight());
	}
	
	void handleSizeChange() override
	{
		m_directorySeparatorIcon.setCenterY(getHeight() * 0.5f);
	}

	void draw(Avo::DrawingContext* p_context) override
	{
		p_context->setColor(Avo::Color(getThemeColor(ThemeColors::onBackground), 0.7f));
		for (uint32 a = 0; a < m_directoryButtons.size() - 1; a++)
		{
			m_directorySeparatorIcon.setLeft(m_directoryButtons[a]->getRight() + DIRECTORY_SEPARATOR_MARGIN);
			p_context->drawText(m_directorySeparatorIcon);
		}
	}

	//------------------------------

	FileBrowserPathEditorPath(View* p_parent) :
		View(p_parent)
	{
		enableMouseEvents();

		m_directorySeparatorIcon.setFontFamily(Avo::FONT_FAMILY_MATERIAL_ICONS);
		m_directorySeparatorIcon.setIsTopTrimmed(true);
		m_directorySeparatorIcon.fitSizeToText();
	}
};

//------------------------------

class FileBrowserPathEditor : public Avo::View
{
public:
	static constexpr float HEIGHT = 6 * 8.f;

	static constexpr float PATH_EDITOR_BOOKMARK_ICON_SIZE = 2.75f * 8.f;
	static constexpr float PATH_FADE_GRADIENT_WIDTH = 2.f * 8.f;

private:
	bool m_isBookmark{ false };
	Avo::Button* m_bookmarkButton{ new Avo::Button(this, "", Avo::Button::Emphasis::Low) };

	void updateBookmarkButtonIcon()
	{
		m_bookmarkButton->setString(m_isBookmark ? MaterialIcons::BOOKMARK : MaterialIcons::BOOKMARK_BORDER);
		m_bookmarkButton->setSize(32.f);
		m_bookmarkButton->getText().setFontFamily(Avo::FONT_FAMILY_MATERIAL_ICONS);
		m_bookmarkButton->getText().setFontSize(PATH_EDITOR_BOOKMARK_ICON_SIZE);
		m_bookmarkButton->getText().fitSizeToText();
		m_bookmarkButton->getText().setCenter(m_bookmarkButton->getSize() * 0.5f);
	}

	//------------------------------

	FileBrowserPathEditorPath* m_path{ new FileBrowserPathEditorPath(this) };

public:
	void setWorkingDirectory(std::filesystem::path const& p_path)
	{
		m_isBookmark = getComponentById<Bookmarks>(Ids::bookmarks)->getIsPathBookmarked(p_path);
		updateBookmarkButtonIcon();

		m_path->setWorkingDirectory(p_path);
		updateLayout();
	}

	//------------------------------

	void handleSizeChange() override
	{
		m_bookmarkButton->setRight(getWidth() - m_bookmarkButton->getTop());
		m_bookmarkButton->setCenterY(getHeight() * 0.5f);

		updateLayout();
	}
	void updateLayout()
	{
		float rightPosition = m_bookmarkButton->getLeft() - m_bookmarkButton->getTop();
		if (m_path->getWidth() > rightPosition)
		{
			m_path->setRight(rightPosition);
		}
		else
		{
			m_path->setLeft(0.f);
		}
	}

	//------------------------------

private:
	Avo::LinearGradient m_pathFadeGradient = { getDrawingContext()->createLinearGradient(
		{
			{ getThemeColor(ThemeColors::background), 0.f },
			{ Avo::Color(getThemeColor(ThemeColors::background), 0.f), 1.f }
		},
		0.f, 0.f, PATH_FADE_GRADIENT_WIDTH, 0
	) };
public:
	void draw(Avo::DrawingContext* p_context) override
	{
		p_context->setColor(getThemeColor(ThemeColors::background));
		p_context->fillRectangle(getSize());
	}
	void drawOverlay(Avo::DrawingContext* p_context) override
	{
		if (m_path->getLeft() < 0.f)
		{
			p_context->setGradient(m_pathFadeGradient);
			p_context->fillRectangle(0, m_path->getTop(), PATH_FADE_GRADIENT_WIDTH, m_path->getBottom());
		}

		p_context->setColor(Colors::fileBrowserPathEditorBorder);
		p_context->strokeGeometry(getClipGeometry(), 3.f);
	}

	//------------------------------

	FileBrowserPathEditor(FileBrowser* p_parent) :
		View(p_parent)
	{
		enableMouseEvents();

		setThemeColor(ThemeColors::background, Colors::fileBrowserPathEditorBackground);

		m_path->setHeight(HEIGHT);

		m_bookmarkButton->setThemeColor(ThemeColors::primaryOnBackground, getThemeColor(ThemeColors::onBackground));
		m_bookmarkButton->setCornerRadius(16.f);
		m_bookmarkButton->buttonClickListeners += [this, p_parent](auto) {
			if (m_isBookmark)
			{
				getComponentById<Bookmarks>(Ids::bookmarks)->removeBookmark(p_parent->getPath());
			}
			else
			{
				getComponentById<Bookmarks>(Ids::bookmarks)->addBookmark(p_parent->getPath());
			}

			m_isBookmark = !m_isBookmark;
			updateBookmarkButtonIcon();
		};
		updateBookmarkButtonIcon();

		setCornerRadius(4.f);
		setElevation(3.f);
		setHeight(HEIGHT);
	}
};
