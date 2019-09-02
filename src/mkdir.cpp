


#include "main.hpp"
#include <iostream>

#include <curl/curl.h>
#include <rapidjson/document.h>


//remote_urls urls divided by lines
CURLcode mkdir(const std::string& access_token, const std::string& remote_urls, const bool verbose)
{
	char curlErrorBuffer[CURL_ERROR_SIZE];

	std::string curlBuffer;

	rapidjson::Document json_d;
	CURLcode res;

	struct curl_slist* headers = NULL;
	headers = curl_slist_append(headers, (std::string("Authorization: Bearer ") + access_token).c_str());
	headers = curl_slist_append(headers, "Content-Type: application/json");

	std::string data = "{\"paths\": [";
	for (std::string s = remote_urls; ; )
	{
		size_t b = s.rfind('\n');
		if (b == std::string::npos)
		{
			if (s.size())
				data += "\"" + s + "\"";
			break;
		}
		data += "\"" + s.substr(b + 1, std::string::npos) + "\",";
		s.erase(b, std::string::npos);
	}
	data += "],\"autorename\": false,\"force_async\": false}";
	std::cout << data;
	res = invokeList("https://api.dropboxapi.com/2/files/create_folder_batch", headers, data, curlBuffer, curlErrorBuffer, verbose);
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