#ifndef FILECONTEXT_H
#define FILECONTEXT_H
#include <vector>
#include <string>
#include <filesystem>
#include <fstream>

namespace FileContext
{
	std::vector<char> readFile(const std::string& filename);
}

#endif 
