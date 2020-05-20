#pragma once

#include "Worker.hpp"
#include "IconLoader.hpp"
#include "../Views/AvoExplorer.hpp"

/*
	The root component of the application.
*/
class Application : public Avo::Component
{
public:
	Application(char const* p_arguments)
	{
		auto worker = new Worker(this);
		worker->setId(Ids::worker);

		auto iconLoader = new IconLoader(this);
		iconLoader->setId(Ids::iconLoader);

		auto gui = new AvoExplorer(this, p_arguments);
		gui->setId(Ids::avoExplorer);

		gui->run();
	}
};
