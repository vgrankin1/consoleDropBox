


#include "main.hpp"
#include <iostream>

#include <curl/curl.h>
#include <rapidjson/document.h>




CURLcode mvcp(const char* endpoint, const std::string& access_token, const std::vector<std::string> &remote_urls, const bool verbose)
{
	char curlErrorBuffer[CURL_ERROR_SIZE];
	std::string curlBuffer;

	rapidjson::Document json_d;
	CURLcode res;
	struct curl_slist* headers = NULL;
	headers = curl_slist_append(headers, (std::string("Authorization: Bearer ") + access_token).c_str());
	headers = curl_slist_append(headers, "Content-Type: application/json");
	
	std::string data = "{\"entries\":[";
	if (remote_urls.size() == 2)
		data += "{\"from_path\":\"" + remote_urls[0] + "\",\"to_path\":\"" + remote_urls[1] + "\"}";
	else for (size_t i = 0, last = remote_urls.size() - 1; i < last; i++)
		data += "{\"from_path\":\"" + remote_urls[i] + "\",\"to_path\":\"" + remote_urls[last] + "/" + fileNameFromFull(remote_urls[i]) + "\"},";

	data.pop_back();
	data += "],\"autorename\":false}";
	if (verbose)
		std::cout << "endpoint: " << endpoint << "\ndata: " << data;
	res = invokeList(endpoint, headers, data, curlBuffer, curlErrorBuffer, verbose);
	curl_slist_free_all(headers);
	if (res != CURLE_OK)
	{
		std::cerr << "\n\ncurl_easy_perform() failed: " << curl_easy_strerror(res) << "\n" << curlErrorBuffer;
		return res;
	}
	if (verbose)
		std::cout << curlBuffer << "\n\n";
	json_d.Parse(curlBuffer.c_str());
	if (!json_d.IsObject() || json_d.HasMember("error"))
	{
		std::cerr << curlBuffer << "\n\n";
		return CURLE_GOT_NOTHING;
	}
	curlBuffer.clear();
	return res;
}

CURLcode mv(const std::string& access_token, const std::vector<std::string>& remote_urls, const bool verbose)
{
	return mvcp("https://api.dropboxapi.com/2/files/move_batch_v2", access_token, remote_urls, verbose);
}
CURLcode cp(const std::string& access_token, const std::vector<std::string>& remote_urls, const bool verbose)
{
	return mvcp("https://api.dropboxapi.com/2/files/copy_batch_v2", access_token, remote_urls, verbose);
}