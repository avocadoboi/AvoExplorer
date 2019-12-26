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
	char const* const directoryAlreadyExistsDialogMessage = u8"A directory with the same name already exists. Do you want to replace the old directory with a new one?";
	char const* const directoryAlreadyExistsDialogTitle = u8"Directory already exists";
	char const* const directory = u8"Directory";
	char const* const directories = u8"Directories";
	char const* const fileAlreadyExistsDialogMessage = u8"A file with the same name already exists. Do you want to replace the old file with a new one?";
	char const* const fileAlreadyExistsDialogTitle = u8"File already exists";
	char const* const file = u8"File";
	char const* const files = u8"Files";
	char const* const newDirectoryDialogMessage = u8"Enter the name of the directory below";
	char const* const newDirectoryDialogTitle = u8"Create new directory";
	char const* const newDirectoryFailedDialogMessage = u8"Failed to create the directory for an unknown reason.\nMaybe try again!";
	char const* const newDirectoryFailedDialogTitle = u8"Failed \U0001F633";
	char const* const newFileDialogMessage = u8"Enter the name of the file below";
	char const* const newFileDialogTitle = u8"Create new file";
	char const* const newFileFailedDialogMessage = u8"Failed to create the file for an unknown reason.\nMaybe try again!";
	char const* const newFileFailedDialogTitle = u8"Failed \U0001F633";
	char const* const no = u8"No";
	char const* const ok = u8"OK";
	char const* const replace = u8"Replace";
	char const* const restart = u8"Restart";
	char const* const thisDirectoryIsEmpty = u8"This directory is empty.";
	char const* const yes = u8"Yes";
};

namespace Strings = Strings_english;
