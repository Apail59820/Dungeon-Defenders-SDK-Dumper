#pragma once

#include <vector>
#include <string>

void PrintFileHeader(std::ostream& os, const std::vector<std::string>& includes, const bool isHeaderFile);

void PrintFileHeader(std::ostream& os, const bool isHeaderFile);

void PrintFileFooter(std::ostream& os);

void PrintSectionHeader(std::ostream& os, const char* name);

enum class FileContentType
{
	Structs,
	Classes,
	Functions,
	FunctionParameters
};

std::string GenerateFileName(const FileContentType type, const class Package& package);
