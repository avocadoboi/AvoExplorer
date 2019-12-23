#pragma once

/*
	Strings are ordered lexically
*/

namespace Strings_english
{
	char const* const accessDeniedDialogMessage = u8"This directory could not be opened because the process does not have permission to do so.\n\nWould you like to restart AvoExplorer with elevated priveleges?";
	char const* const accessDeniedDialogTitle = u8"Access denied!";
	char const* const bookmarks = u8"BOOKMARKS";
	char const* const cancel = u8"Cancel";
	char const* const directory = u8"Directory";
	char const* const directories = u8"Directories";
	char const* const file = u8"File";
	char const* const files = u8"Files";
	char const* const newDirectoryDialogTitle = u8"Create new directory";
	char const* const newDirectoryDialogMessage = u8"Enter the name of the directory below";
	char const* const newFileDialogTitle = u8"Create new file";
	char const* const newFileDialogMessage = u8"Enter the name of the file below";
	char const* const no = u8"No";
	char const* const ok = u8"OK";
	char const* const restart = u8"Restart";
	char const* const thisDirectoryIsEmpty = u8"This directory is empty.";
	char const* const yes = u8"Yes";
};

namespace Strings = Strings_english;
