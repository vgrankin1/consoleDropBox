

#include "main.hpp"

#include <iostream>

#include <ctime>
#include <string>
#include <vector>
#include <fstream>

#include <curl/curl.h>


#pragma comment(lib, "libcrypto-1_1-x64.dll.lib")
#pragma comment(lib, "libcurl.dll.a")


int verbose = false;

void printHelp(const std::string &app_name)
{
	std::cout << "Wrong arguments! Usage:\n";
	std::cout << app_name + " put src dest" << "\t\twhere src is a local file" << std::endl;
	std::cout << app_name + " get src dest" << "\t\twhere src is a remote file" << std::endl;
	std::cout << app_name + " ls url" << "\t\t where url is required (" + app_name + " ls \"\")" << std::endl;
	std::cout << app_name + " rm url1 url2 ..." << "\t\t where url is required" << std::endl;
	std::cout << app_name + " mkdir url1 url2 ..." << "\t\t where url is required" << std::endl;
	std::cout << app_name + " mv url1 url2 ... dest" << "\t\t where url is required" << std::endl;
	std::cout << app_name + " cp url1 url2 ... dest" << "\t\t where url is required" << std::endl;

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




int main(int argc, char* argv[])
{
	CURLcode res = CURLE_OK;
	FILE* fi;
	std::string app_name = fileNameFromFull(std::string(argv[0]));
	std::vector<std::string> src_path, dst_path;
	std::string access_token = "";

	tool_action_t tool_action = NO_ACTION;

	for (int i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "-h") == 0)
		{
			printHelp(app_name);
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
			src_path.push_back(argv[i + 1]);
			dst_path.push_back(argv[i + 2]);
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
			src_path.push_back(argv[i + 1]);
			dst_path.push_back(argv[i + 2]);
			tool_action = GET_ACTION;
			i += 2;
		}
		else if (strcmp(argv[i], "ls") == 0)
		{
			if (i + 1 >= argc || argv[i + 1][0] == '-')
				src_path.push_back("");
			else
				src_path.push_back(argv[++i]);
			tool_action = LIST_ACTION;
		}
		else if (strcmp(argv[i], "rm") == 0)
		{
			while (i + 1 < argc && (argv[i + 1][0] == '/' || argv[i + 1][1] == '/'))
				src_path.push_back(argv[++i]);
			if (src_path.size() == 0)
			{
				std::cerr << "rm command requres path\n";
				return 1;
			}
			tool_action = RM_ACTION;
		}
		else if (strcmp(argv[i], "mkdir") == 0)
		{
			while (i + 1 < argc && (argv[i + 1][0] != '-'))//(argv[i + 1][0] == '/' || argv[i + 1][1] == '/'))
				src_path.push_back(argv[++i]);
			if (src_path.size() == 0)
			{
				std::cerr << "mkdir command requres path\n";
				return 1;
			}
			tool_action = MKDIR_ACTION;
		}
		else if (strcmp(argv[i], "mv") == 0)
		{
			while (i + 1 < argc && (argv[i + 1][0] != '-'))
				src_path.push_back(argv[++i]);
			if (src_path.size() == 0)
			{
				std::cerr << "mv command requres files\n";
				return 1;
			}
			tool_action = MV_ACTION;
		}
		else if (strcmp(argv[i], "cp") == 0)
		{
			while (i + 1 < argc && (argv[i + 1][0] != '-'))
				src_path.push_back(argv[++i]);
			if (src_path.size() == 0)
			{
				std::cerr << "cp command requres path\n";
				return 1;
			}
			tool_action = CP_ACTION;
		}
		else if (strcmp(argv[i], "df") == 0)
		{
			tool_action = DF_ACTION;
		}
		else
		{
			printHelp(app_name);
			return 1;
		}
	}
	if (verbose)
		std::cout << curl_version() << "\n";
	
	if (access_token.size() == 0 && argc > 2)//Ask for a token only if there is an action
	{
		std::cout << "Input acces token\n";
		std::getline(std::cin, access_token);
		if (!testToken(access_token))
		{
			std::cerr << "\nBad token\n";
			return 1;
		}
	}

	curl_global_init(CURL_GLOBAL_ALL);
	switch(tool_action)
	{
	case PUT_ACTION:
		if ((fi = fopen(src_path[0].c_str(), "rb")) == 0)
		{
			std::cerr << strerror(errno) << " : " << src_path[0] << std::endl;
			return 1;
		}
		res = upload(fi, access_token, dst_path[0], verbose);
		fclose(fi);
		break;
	case GET_ACTION:
		if ((fi = fopen(dst_path[0].c_str(), "rb")) != 0)
		{
			fclose(fi);
			std::cout << "File: " << dst_path[0] << "\nExists. override? y/n";
			if (!promptYesNo())
				return 0;
		}
		if ((fi = fopen(dst_path[0].c_str(), "wb")) == 0)
		{
			std::cerr << strerror(errno) << " : " << dst_path[0] << std::endl;
			return 1;
		}
		res = download(fi, access_token, src_path[0], verbose);
		fclose(fi);
		break;
	case LIST_ACTION:
		res = list(access_token, src_path[0], verbose);
		break;
	case RM_ACTION:
		res = rm(access_token, src_path, verbose);
		break;
	case MKDIR_ACTION:
		res = mkdir(access_token, src_path, verbose);
		break;
	case MV_ACTION:
		res = mv(access_token, src_path, verbose);
		break;
	case CP_ACTION:
		res = cp(access_token, src_path, verbose);
		break;
	case DF_ACTION:
		res = df(access_token, verbose);
		break;
	default:
		std::cout << "No action\n";
		printHelp(app_name);
		break;
	}
	curl_global_cleanup();
	return res;
}

