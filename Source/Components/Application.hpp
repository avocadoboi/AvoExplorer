#pragma once

#include <AvoGUI.hpp>

#include "Worker.hpp"
#include "IconLoader.hpp"
#include "../Views/AvoExplorer.hpp"

/*
	The root component of the application.
*/
class Application : public AvoGUI::Component
{
public:
	Application(char const* p_arguments)
	{
		auto worker = new Worker(this);
		worker->setId(Ids::worker);

		auto iconLoader = new IconLoader(this);
		iconLoader->setId(Ids::iconLoader);

		AvoExplorer* gui = new AvoExplorer(this, p_arguments);
		gui->setId(Ids::avoExplorer);

		gui->waitForFinish();
	}
};
