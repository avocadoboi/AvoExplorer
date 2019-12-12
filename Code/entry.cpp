#include "AvoExplorer.hpp"

int main(int p_numberOfArguments, char** p_arguments)
{
	new AvoExplorer(p_numberOfArguments > 1 ? p_arguments[1] : "C:");
}

