
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <curl/curl.h>
//#include <rapidjson/document.h>


static size_t _upWriteFunc(char* data, size_t size, size_t nmemb, std::string* buffer)
{
	size_t result = 0;
	if (buffer != NULL)
	{
		buffer->append(data, size * nmemb);
		result = size * nmemb;
	}
	return result;
}

CURLcode invokeUP(const char* endpoint, const curl_slist* headers, std::string &retBuffer, char *errorbuf, const char *buffer, const size_t size, const bool verbose)
{
	CURL* curl;
	CURLcode res;
	if ((curl = curl_easy_init()) == 0)
	{
		std::cerr << "Failed to initialize curl\n";
		return CURLE_FAILED_INIT;
	}
	curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errorbuf);
	curl_easy_setopt(curl, CURLOPT_VERBOSE, long(verbose));

	res = curl_easy_setopt(curl, CURLOPT_CAINFO, "cacert.pem");
	if (res != CURLE_OK)
	{
		std::cerr << "SSL certificate verification problem: " << errorbuf << "\n";
		std::cout << "Does continued without verification?\nY/n\n";
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
	}
	curl_easy_setopt(curl, CURLOPT_URL, endpoint);
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(curl, CURLOPT_POST, 1L);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &retBuffer);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, _upWriteFunc);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, buffer);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE_LARGE, size);
	res = curl_easy_perform(curl);
	curl_easy_cleanup(curl);
	return res;
}


static size_t _downWriteFunc(char* data, size_t size, size_t nmemb, FILE *fi)
{
	size_t sz = size * nmemb;
	fwrite(data, 1, sz, fi);
	return sz;
}

CURLcode invokeDOWN(const char* endpoint, const curl_slist* headers, char* errorbuf, std::string& dropbox_api_result, FILE *fi, const bool verbose)
{
	CURL* curl;
	CURLcode res;
	if ((curl = curl_easy_init()) == 0)
	{
		std::cerr << "Failed to initialize curl\n";
		return CURLE_FAILED_INIT;
	}
	curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errorbuf);
	curl_easy_setopt(curl, CURLOPT_VERBOSE, long(verbose));

	res = curl_easy_setopt(curl, CURLOPT_CAINFO, "cacert.pem");
	if (res != CURLE_OK)
	{
		std::cerr << "SSL certificate verification problem: " << errorbuf << "\n";
		std::cout << "Does continued without verification?\nY/n\n";
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
	}
	curl_easy_setopt(curl, CURLOPT_URL, endpoint);
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(curl, CURLOPT_POST, 1L);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, fi);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, _downWriteFunc);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, 0);
	curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, _upWriteFunc);//get a dropbox_api_result
	curl_easy_setopt(curl, CURLOPT_HEADERDATA, &dropbox_api_result);

	res = curl_easy_perform(curl);
	curl_easy_cleanup(curl);

	std::string dbstring = "dropbox-api-result:";
	size_t dbpos;
	if ((dbpos = dropbox_api_result.find(dbstring)) != std::string::npos)
	{
		dropbox_api_result = dropbox_api_result.substr(dbpos + dbstring.size());
		dropbox_api_result = dropbox_api_result.substr(0, dropbox_api_result.find('\n'));
	}
	else
		dropbox_api_result = "";
	return res;
}

