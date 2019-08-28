
#include <iostream>
#include <string>
#include <memory>

#include "curl/curl.h"
#include "rapidjson/document.h"


CURLcode invokeUP(const char* endpoint, const curl_slist* headers, std::string& retBuffer, char* errorbuf, const char* buffer, const size_t size, const bool verbose);

CURLcode upload(FILE *fi, const std::string &access_token, const std::string& file_name_url, const bool verbose)
{
	char curlErrorBuffer[CURL_ERROR_SIZE];
	std::string session_id;
	std::string curlBuffer;
	size_t _doffset = 0;
	rapidjson::Document json_d;
	CURLcode res;

	fpos_t fi_size;
	curl_off_t buffer_size = 128 * 1024 * 1024;
	std::unique_ptr<char> buffer(new char[buffer_size]);
	size_t readed_bytes;

	fseek(fi, 0, SEEK_END);
	fgetpos(fi, &fi_size);
	fseek(fi, 0, SEEK_SET);

	
	if ((readed_bytes = fread(buffer.get(), 1, buffer_size, fi)) == 0)
	{
		std::cout << "File is empty\n";
		return CURLE_OK;
	}

	struct curl_slist* headers = NULL;
	headers = curl_slist_append(headers, (std::string("Authorization: Bearer ") + access_token).c_str());
	headers = curl_slist_append(headers, "Dropbox-API-Arg: {\"close\": false}");
	headers = curl_slist_append(headers, "Content-Type: application/octet-stream");

	res = invokeUP("https://content.dropboxapi.com/2/files/upload_session/start", headers, curlBuffer, curlErrorBuffer, buffer.get(), readed_bytes, verbose);
	curl_slist_free_all(headers);
	if (res != CURLE_OK)
	{
		std::cerr << "\n\ncurl_easy_perform() failed: " << curl_easy_strerror(res) << "\n" << curlErrorBuffer;
		return res;
	}
	if (verbose)
		std::cout << curlBuffer << "\n\n";
	json_d.Parse(curlBuffer.c_str());
	if (json_d.IsObject())
	{
		if (json_d.HasMember("error"))
		{
			curl_global_cleanup();
			return CURLE_UPLOAD_FAILED;
		}
		else
		{
			session_id = json_d["session_id"].GetString();
			std::cout << "\nsession_id: " << session_id << "\n";
		}
	}
	curlBuffer.clear();
	_doffset = readed_bytes;

	
	for (; _doffset + buffer_size < fi_size;)
	{
		if ((readed_bytes = fread(buffer.get(), 1, buffer_size, fi)) == 0)
			break;
		headers = NULL;
		headers = curl_slist_append(headers, (std::string("Authorization: Bearer ") + access_token).c_str());
		headers = curl_slist_append(headers, (std::string("Dropbox-API-Arg: {\"cursor\": {\"session_id\":\"") + session_id +
			"\",\"offset\": " + std::to_string(_doffset) + "},\"close\":false}").c_str());
		headers = curl_slist_append(headers, "Content-Type: application/octet-stream");

		res = invokeUP("https://content.dropboxapi.com/2/files/upload_session/append_v2", headers, curlBuffer, curlErrorBuffer, buffer.get(), readed_bytes, verbose);
		curl_slist_free_all(headers);
		if (res != CURLE_OK)
		{
			std::cerr << "\n\ncurl_easy_perform() failed: " << curl_easy_strerror(res) << "\n" << curlErrorBuffer;
			curl_global_cleanup();
			return res;
		}
		if (verbose)
			std::cout << curlBuffer << "\n\n";
		json_d = rapidjson::Document();
		json_d.Parse(curlBuffer.c_str());
		if (json_d.IsObject())
		{
			curl_global_cleanup();
			return CURLE_UPLOAD_FAILED;
		}
		curlBuffer.clear();
		_doffset += readed_bytes;
	}
	

	readed_bytes = fread(buffer.get(), 1, buffer_size, fi);

	headers = NULL;
	headers = curl_slist_append(headers, (std::string("Authorization: Bearer ") + access_token).c_str());
	headers = curl_slist_append(headers, (std::string("Dropbox-API-Arg: {\"cursor\": {\"session_id\":\"") + session_id +
		"\",\"offset\": " + std::to_string(_doffset) + "},\"commit\": {\"path\": \"" + file_name_url + "\","\
		"\"mode\": \"add\",\"autorename\": true,\"mute\": false,\"strict_conflict\": false}}").c_str());
	headers = curl_slist_append(headers, "Content-Type: application/octet-stream");

	res = invokeUP("https://content.dropboxapi.com/2/files/upload_session/finish", headers, curlBuffer, curlErrorBuffer, buffer.get(), readed_bytes, verbose);
	curl_slist_free_all(headers);
	if (res != CURLE_OK)
	{
		std::cerr << "\n\ncurl_easy_perform() failed: " << curl_easy_strerror(res) << "\n" << curlErrorBuffer;
		curl_global_cleanup();
		return res;
	}
	if (verbose)
		std::cout << curlBuffer << "\n\n";
	json_d = rapidjson::Document();
	json_d.Parse(curlBuffer.c_str());
	if (!json_d.IsObject())
	{
		curl_global_cleanup();
		return CURLE_UPLOAD_FAILED;
	}
	if (json_d.HasMember("error"))
	{

	}
	else
	{
		std::string sid = json_d["id"].GetString();
		std::string hash = json_d["content_hash"].GetString();
	}
	curlBuffer.clear();
	return res;
}