
#include <iostream>
#include <string>
#include <memory>

#include "curl/curl.h"
#include "rapidjson/document.h"

CURLcode invokeDOWN(const char* endpoint, const curl_slist* headers, char* errorbuf, FILE* fi, const bool verbose);


CURLcode download(FILE* fi, const std::string& access_token, const std::string& file_name_url, const bool verbose)
{
	char curlErrorBuffer[CURL_ERROR_SIZE];
	std::string curlBuffer;
	size_t _doffset = 0;
	rapidjson::Document json_d;
	CURLcode res;

	struct curl_slist* headers = NULL;
	headers = curl_slist_append(headers, (std::string("Authorization: Bearer ") + access_token).c_str());
	headers = curl_slist_append(headers, (std::string("Dropbox-API-Arg: {\"path\": \"") + file_name_url + "\"}").c_str());
	headers = curl_slist_append(headers, "Content-Type: application/octet-stream");

	res = invokeDOWN("https://content.dropboxapi.com/2/files/download", headers, curlErrorBuffer, fi, verbose);
	curl_slist_free_all(headers);
	if (res != CURLE_OK)
		std::cerr << "\n\ncurl_easy_perform() failed: " << curl_easy_strerror(res) << "\n" << curlErrorBuffer;

	return res;
}