#pragma once

#include "Base.h"
#include "Application.h"

#include <iostream>

int main(int argc, char** argv)
{
	Log::Init();
	LOG_INFO("Start!");


	Application* app = new Application();

	app->Run();

	delete app;
}