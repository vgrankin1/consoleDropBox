

#include "main.hpp"

#include <iostream>

#include <ctime>
#include <string>
#include <fstream>

#include <curl/curl.h>


#pragma comment(lib, "libcrypto-1_1-x64.dll.lib")
#pragma comment(lib, "libcurl.dll.a")


int verbose = false;

void printHelp()
{
	std::cout << "Wrong arguments! Usage:\n";
	std::cout << "tool.exe put src_path dst_path" << "\t\twhere src_path is a local file" << std::endl;
	std::cout << "tool.exe get src_path dst_path" << "\t\twhere src_path is a remote file" << std::endl;
	std::cout << "\nPossible arguments:\n";
	std::cout << "  -h \t this message" << std::endl;
	std::cout << "  -v \t verbose" << std::endl;
	std::cout << "  -a token \t where token is your DropBox access token" << std::endl;
	std::cout << "  -k file \t where file name that contains access token" << std::endl;
	std::cout << "Remote file is unix style always:\n  /file\n  /folder/file" << std::endl;
}

bool promptYesNo()
{
	for (; ;)
	{
		int symb = getc(stdin);
		if (symb == 'y' || symb == 'Y')
			return true;
		if (symb == 'n' || symb == 'N')
			return false;
	}
}


bool testToken(std::string &token)
{
	return token.size() == 64;
}


CURLcode list(const std::string& access_token, const std::string& remote_url, const bool verbose);

int main(int argc, char* argv[])
{
	CURLcode res;
	FILE* fi;
	const char* src_path, *dst_path;
	src_path = dst_path = "";
	std::string access_token = "";

	tool_action_t tool_action = NO_ACTION;

	for (int i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "-h") == 0)
		{
			printHelp();
			return 0;
		}else if (strcmp(argv[i], "-v") == 0)
			verbose = true;
		else if (strcmp(argv[i], "-a") == 0)//token in arguments
		{
			if (i + 1 >= argc)
			{
				std::cerr << "No token in arguments\n";
				return 1;
			}
			access_token = argv[i + 1];
			if (!testToken(access_token))
			{
				std::cerr << "Token file: " << argv[i + 1] << "\nBad token\n";
				return 1;
			}
			i++;
		}
		else if (strcmp(argv[i], "-k") == 0)//token file arg
		{
			if (i + 1 >= argc)
			{
				std::cerr << "No token file in arguments\n";
				return 1;
			}
			std::ifstream fkey(argv[i + 1], std::ios::in);
			if (!fkey.is_open())
			{
				std::cerr << strerror(errno) << " : " << argv[i + 1];
				return 1;
			}
			std::getline(fkey, access_token);
			if (!testToken( access_token) )
			{
				std::cerr << "Token file:" << argv[i + 1] << "\nBad token\n";
				return 1;
			}
			i++;
		}
		else if (strcmp(argv[i], "put") == 0)
		{
			if (i + 2 >= argc)
			{
				std::cerr << "Wrong arguments for put command\n";
				return 1;
			}
			src_path = argv[i + 1];
			dst_path = argv[i + 2];
			tool_action = PUT_ACTION;
			i += 2;
		}
		else if (strcmp(argv[i], "get") == 0)
		{
			if (i + 2 >= argc)
			{
				std::cerr << "Wrong arguments for get command\n";
				return 1;
			}
			src_path = argv[i + 1];
			dst_path = argv[i + 2];
			tool_action = GET_ACTION;
			i += 2;
		}
		else if (strcmp(argv[i], "ls") == 0)
		{
			if (i + 1 >= argc)
			{
				std::cerr << "Wrong arguments for list command\n";
				return 1;
			}
			src_path = argv[i + 1];
			tool_action = LIST_ACTION;
			i += 1;
		}
		else
		{
			printHelp();
			return 1;
		}
	}
	if (access_token.size() == 0)
	{
		std::cout << "Input acces token\n";
		std::getline(std::cin, access_token);
		if (!testToken(access_token))
		{
			std::cerr << "\nBad token\n";
			return 1;
		}
	}
	if (verbose)
		std::cout << curl_version() << "\n";
	
	curl_global_init(CURL_GLOBAL_ALL);
	switch(tool_action)
	{
	case PUT_ACTION:
		if ((fi = fopen(src_path, "rb")) == 0)
		{
			std::cerr << strerror(errno) << " : " << src_path << std::endl;
			return 1;
		}
		res = upload(fi, access_token, dst_path, verbose);
		fclose(fi);
		break;
	case GET_ACTION:
		if ((fi = fopen(dst_path, "rb")) != 0)
		{
			fclose(fi);
			std::cout << "File: " << dst_path << "\nExists. override? y/n";
			if (!promptYesNo())
				return 0;
		}
		if ((fi = fopen(dst_path, "wb")) == 0)
		{
			std::cerr << strerror(errno) << " : " << dst_path << std::endl;
			return 1;
		}
		res = download(fi, access_token, src_path, verbose);
		fclose(fi);
		break;
	case LIST_ACTION:
		res = list(access_token, src_path, verbose);
		break;
	default:
		std::cout << "No action\n";
		printHelp();
		break;
	}
	curl_global_cleanup();
	return res;
}

