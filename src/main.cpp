
#define _CRT_SECURE_NO_WARNINGS


#include <iostream>
#include <ctime>
#include <string>
//#include <>

#include "curl/curl.h"


#pragma comment(lib, "libcurl.dll.a")

int verbose = true;

void printHelp()
{
	std::cout << "Wrong arguments! Usage:\n";
	std::cout << "tool.exe acces_token put src_path dst_path" << "\t\twhere src_path is a local file" << std::endl;
	std::cout << "tool.exe acces_token get src_path dst_path" << "\t\twhere src_path is a remote file" << std::endl;
	std::cout << "tool.exe put src_path dst_path" << std::endl;
	std::cout << "tool.exe get src_path dst_path" << std::endl;
	std::cout << "Remote file is always:\n/file\n/folder/file" << std::endl;
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
CURLcode upload(FILE* fi, const std::string& access_token, const std::string& file_name_url, const bool verbose);
CURLcode download(FILE* fi, const std::string& access_token, const std::string& file_name_url, const bool verbose);

//testtool login password put/get src_path dst_path
int main(int argc, char* argv[])
{
	char* src_path, *dst_path;
	std::string access_token = "";
	std::string file_name_url = "/test5.txt";
	bool putFlag = false;
	std::cout << curl_version() << "\n";

	if (argc < 4)
	{
		printHelp();
		return 0;
	}
	if (strlen(argv[1]) == 3)
	{
		if (strcmp(argv[1], "put") == 0)
			putFlag = true;
		else if (strcmp(argv[1], "get") != 0)
		{
			printHelp();
			return 0;
		}
		src_path = argv[2];
		dst_path = argv[3];
	}
	else
	{
		if (strcmp(argv[2], "put") == 0)
			putFlag = true;
		else if (strcmp(argv[2], "get") != 0)
		{
			printHelp();
			return 0;
		}
		access_token = argv[1];
		src_path = argv[3];
		dst_path = argv[4];
	}

	
	CURLcode res;
	FILE* fi;
 
	

	if (putFlag)
	{
		if ((fi = fopen(src_path, "rb")) == 0)
		{
			std::cerr << strerror(errno) << " : " << src_path << std::endl;
			return 1;
		}
		curl_global_init(CURL_GLOBAL_ALL);
		res = upload(fi, access_token, dst_path, verbose);
		curl_global_cleanup();
		fclose(fi);
	}else 
	{
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
		curl_global_init(CURL_GLOBAL_ALL);
		res = download(fi, access_token, src_path, verbose);
		curl_global_cleanup();
		fclose(fi);
	}

	return res;
}

