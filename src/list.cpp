
#include "main.hpp"
#include <iostream>

#include <curl/curl.h>
#include <rapidjson/document.h>



struct file_info_t
{
	std::string tag;
	std::string name;
	std::string path_display;
	std::string id;
	std::string client_modified;
	std::string server_modified;
	size_t size;
	bool is_downloadable;
	std::string content_hash;
	file_info_t()
		: size(), is_downloadable()
	{}
};


CURLcode list(const std::string& access_token, const std::string& remote_url, const bool verbose)
{
	char curlErrorBuffer[CURL_ERROR_SIZE];
	std::string dropbox_api_result;
	rapidjson::Document json_d;
	CURLcode res;

	struct curl_slist* headers = NULL;
	headers = curl_slist_append(headers, (std::string("Authorization: Bearer ") + access_token).c_str());
	headers = curl_slist_append(headers, "Content-Type: application/json");
	
	std::string data =
		"{\"path\":\"" + remote_url + "\"," \
		"\"recursive\" : false,"\
		"\"include_media_info\" : false,"
		"\"include_deleted\" : false,"
		"\"include_has_explicit_shared_members\" : false,"
		"\"include_mounted_folders\" : true,"
		"\"include_non_downloadable_files\" : true}";

	res = invokeList("https://api.dropboxapi.com/2/files/list_folder", headers, data, dropbox_api_result, curlErrorBuffer, verbose);
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
	dropbox_api_result.clear();

	const rapidjson::Value &entries = json_d["entries"];
	std::string cursor = json_d["cursor"].GetString();
	bool hasmore = json_d["has_more"].GetBool();

	assert(entries.IsArray());
	for (unsigned i = 0; i < entries.Size(); i++)
	{
		const rapidjson::Value& obj = entries[i];
		if (!obj.IsObject())
		{

			break;
		}
		file_info_t info;
		info.tag			= obj[".tag"].GetString();
		info.name			= obj["name"].GetString();
		info.path_display = obj["path_display"].GetString();
		info.id = obj["id"].GetString();
		if (info.tag.compare("folder") != 0)
		{
			info.client_modified = obj["client_modified"].GetString();
			info.server_modified = obj["server_modified"].GetString();
			info.size = obj["size"].GetUint64();
			info.is_downloadable = obj["is_downloadable"].GetBool();
			info.content_hash = obj["content_hash"].GetString();
		}
		std::cout << info.tag << " " << info.client_modified << " " << info.size << "\t\t" << info.name << "\n";
	}
	
	return res;
}