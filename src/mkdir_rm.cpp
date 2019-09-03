
#include "main.hpp"
#include <iostream>

#include <curl/curl.h>
#include <rapidjson/document.h>


CURLcode mkdir_rm(const char* endpoint, const std::string& access_token, const std::string &data, const bool verbose)
{
	char curlErrorBuffer[CURL_ERROR_SIZE];
	std::string dropbox_api_result;
	rapidjson::Document json_d;
	CURLcode res;

	struct curl_slist* headers = NULL;
	headers = curl_slist_append(headers, (std::string("Authorization: Bearer ") + access_token).c_str());
	headers = curl_slist_append(headers, "Content-Type: application/json");

	res = invokeList(endpoint, headers, data, dropbox_api_result, curlErrorBuffer, verbose);
	curl_slist_free_all(headers);
	if (res != CURLE_OK)
	{
		std::cerr << "\n\ncurl_easy_perform() failed: " << curl_easy_strerror(res) << "\n" << curlErrorBuffer;
		return res;
	}
	if (verbose)
		std::cout << dropbox_api_result << "\n\n";
	json_d.Parse(dropbox_api_result.c_str());
	if (!json_d.IsObject() || json_d.HasMember("error"))
	{
		std::cerr << dropbox_api_result << "\n\n";
		return CURLE_GOT_NOTHING;
	}
	return res;
}

CURLcode mkdir(const std::string& access_token, const std::vector<std::string>& remote_urls, const bool verbose)
{
	std::string data = "{\"paths\": [";
	for (int i = 0; i < remote_urls.size(); i++)
		data += "\"" + remote_urls[i] + "\",";
	data.pop_back();
	data += "],\"autorename\": false,\"force_async\": false}";
	return mkdir_rm("https://api.dropboxapi.com/2/files/create_folder_batch", access_token, data, verbose);
}
CURLcode rm(const std::string& access_token, const std::vector<std::string>& remote_urls, const bool verbose)
{
	std::string data = "{\"entries\": [";
	for (int i = 0; i < remote_urls.size(); i++)
		data += "{\"path\": \"" + remote_urls[i] + "\"},";
	data.pop_back();
	data += "]}";
	return mkdir_rm("https://api.dropboxapi.com/2/files/delete_batch", access_token, data, verbose);
}