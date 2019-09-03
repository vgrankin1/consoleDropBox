
#include "main.hpp"

#include <iostream>
#include <string>
#include <memory>

#include <curl/curl.h>
#include <rapidjson/document.h>


CURLcode download(FILE* fi, const std::string& access_token, const std::string& file_name_url, const bool verbose)
{
	char curlErrorBuffer[CURL_ERROR_SIZE];
	std::string curlBuffer;
	std::string dropbox_api_result;
	size_t _doffset = 0;
	
	sha256_DB_hash_t hash;
	rapidjson::Document json_d;
	CURLcode res;

	struct curl_slist* headers = NULL;
	headers = curl_slist_append(headers, (std::string("Authorization: Bearer ") + access_token).c_str());
	headers = curl_slist_append(headers, (std::string("Dropbox-API-Arg: {\"path\": \"") + file_name_url + "\"}").c_str());
	headers = curl_slist_append(headers, "Content-Type: application/octet-stream");

	res = invokeDOWN("https://content.dropboxapi.com/2/files/download", headers, curlErrorBuffer, dropbox_api_result, fi, &hash, verbose);
	curl_slist_free_all(headers);
	if (res != CURLE_OK)
		std::cerr << "\n\ncurl_easy_perform() failed: " << curl_easy_strerror(res) << "\n" << curlErrorBuffer;

	json_d.Parse(dropbox_api_result.c_str());
	if (!json_d.IsObject() || json_d.HasMember("error"))
	{
		std::cerr << "\n" << dropbox_api_result << "\n";
		return res;
	}
	
	std::string sid = json_d["id"].GetString();
	std::string content_hash = json_d["content_hash"].GetString();
	std::string local_hash = hash.get();
	if (local_hash.compare(content_hash) == 0)
	{
		if (verbose)
			std::cout << "Local and remote hashes are identical\n";
	}
	else
		std::cout << "[Warning!!!]\nLocal and remote hashes are not identical!!!\n";
	return res;
}

