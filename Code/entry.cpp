#include "AvoExplorer.hpp"

//------------------------------

// Remove console window for release
#ifndef _DEBUG
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#endif

int main(int p_numberOfArguments, char** p_arguments)
{
	new AvoExplorer(p_numberOfArguments > 1 ? p_arguments[1] : "C:");
}

