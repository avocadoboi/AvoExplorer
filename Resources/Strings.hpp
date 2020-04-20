#pragma once

/*
	Strings are ordered lexically
*/

namespace Strings_english
{
	inline std::string const addSuffixes = u8"Add suffixes";
	inline std::string const bookmarks = u8"BOOKMARKS";
	inline std::string const cancel = u8"Cancel";
	inline std::string const directory = u8"Directory";
	inline std::string const directories = u8"Directories";

	inline std::string const directoriesOrFilesAlreadyExistDialogMessage = u8"{0} file with the same names already exist in this directory. \n\nIf you choose Replace, the old items will be moved to the recycle bin. \nIf you choose Add suffixes, a number will be added at the end of the duplicate files.\nIf you choose Skip duplicates, only files with unique names will be added.";
	inline std::string const directoriesOrFilesAlreadyExistDialogTitle = u8"Files already exist";

	inline std::string const fileAlreadyExistsDialogMessage = u8"A file with the same name already exists in this directory. \n\nIf you choose Replace, the old file will be moved to the recycle bin. \nIf you choose Add suffixes, a number will be added at the end of the duplicate file.";
	inline std::string const fileAlreadyExistsDialogTitle = u8"File already exists";

	inline std::string const directoryAlreadyExistsDialogMessage = u8"A directory with the same name already exists in this directory. \n\nIf you choose Replace, the old directory will be moved to the recycle bin. \nIf you choose Add suffixes, a number will be added at the end of the duplicate directory.";
	inline std::string const directoryAlreadyExistsDialogTitle = u8"Directory already exists";

	inline std::string const file = u8"File";
	inline std::string const files = u8"Files";

	inline std::string const newDirectoryDialogMessage = u8"Enter the name of the directory below";
	inline std::string const newDirectoryDialogTitle = u8"Create new directory";

	inline std::string const newDirectoryFailedDialogMessage = u8"Failed to create the directory for an unknown reason.\nMaybe try again!\n\nError message: ";
	inline std::string const newDirectoryFailedDialogTitle = u8"Failed \U0001F633";

	inline std::string const newDirectoryOrFileAlreadyExistsDialogMessage = u8"A directory or file with the same name already exists at the same place. Do you want to replace the old item with a new one?\nThe old item will be moved to the recycle bin.";
	inline std::string const newDirectoryOrFileAlreadyExistsDialogTitle = u8"Path already exists";

	inline std::string const newFileAccessDeniedDialogMessage = u8"The file could not be created in this directory because the process does not have permission to do so.\n\nWould you like to restart AvoExplorer with elevated privileges?";
	inline std::string const newFileAccessDeniedDialogTitle = u8"Access denied! \u26D4";

	inline std::string const newFileDialogMessage = u8"Enter the name of the file below";
	inline std::string const newFileDialogTitle = u8"Create new file";

	inline std::string const newFileFailedDialogMessage = u8"Failed to create the file for an unknown reason.\nMaybe try again!";
	inline std::string const newFileFailedDialogTitle = u8"Failed \U0001F633";

	inline std::string const no = u8"No";
	inline std::string const ok = u8"OK";

	inline std::string const openDirectoryAccessDeniedDialogMessage = u8"This directory could not be opened because the process does not have permission to do so.\n\nWould you like to restart AvoExplorer with elevated privileges?";
	inline std::string const openDirectoryAccessDeniedDialogTitle = u8"Access denied! \u26D4";

	inline std::string const removeBookmark = u8"Remove bookmark";
	inline std::string const replace = u8"Replace";
	inline std::string const restart = u8"Restart";
	inline std::string const skipDuplicates = u8"Skip duplicates";
	inline std::string const thisDirectoryIsEmpty = u8"This directory is empty.";
	inline std::string const yes = u8"Yes";
};

namespace Strings = Strings_english;
