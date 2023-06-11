#pragma once

#include "Log.h"
#include <string>
#include <fstream>

std::string ReadFile(const std::string& filename)
{
	std::ifstream file(filename, std::ios::binary | std::ios::ate);

	ASSERT_IFNOT(file.is_open(), "can not read file!");

	auto size = file.tellg();
	std::string content;
	content.resize(size);
	file.seekg(0);

	file.read(content.data(), content.size());

	return content;
}