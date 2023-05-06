#pragma once

#include "Application.h"

#include <iostream>

int main(int argc, char** argv)
{
	std::cout << "Start VulkanDemo!" << std::endl;	

	Application* app = new Application();

	app->Run();

	delete app;
}