cmake_minimum_required(VERSION 3.15)
project(AvoExplorer)

set(CMAKE_CXX_STANDARD 17)

include_directories("../AvoGUI/")

file(GLOB source
	"Source/*/*.hpp"
	"Source/*/*.cpp"
	"Source/*.hpp"
	"Source/*.cpp"
	"../AvoGUI/AvoGUI.hpp"
	"../AvoGUI/AvoGUI.cpp"
)
add_executable(AvoExplorer ${source})