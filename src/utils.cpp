

#include <string>
#include <vector>



std::string fileNameFromFull(const std::string& full)
{
	std::string res;
	size_t b = full.rfind('/');
	if (b == std::string::npos)
		if((b = full.rfind('\\')) == std::string::npos)
			return "";
	return full.substr(b + 1, std::string::npos);
}
