
#include "main.hpp"
#include <iostream>

#include <curl/curl.h>
#include <rapidjson/document.h>


CURLcode df(const std::string& access_token, const bool verbose)
{
	char curlErrorBuffer[CURL_ERROR_SIZE];
	std::string dropbox_api_result;
	rapidjson::Document json_d;
	CURLcode res;

	struct curl_slist* headers = NULL;
	headers = curl_slist_append(headers, (std::string("Authorization: Bearer ") + access_token).c_str());
	headers = curl_slist_append(headers, "Content-Type: application/json");
	std::string data = "null";
	
	res = invokeList("https://api.dropboxapi.com/2/users/get_space_usage", headers, data, dropbox_api_result, curlErrorBuffer, verbose);
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