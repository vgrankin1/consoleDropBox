
#include <iostream>
#include <string>
#include <vector>

#include "curl/curl.h"


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

CURLcode invokeUP(const char* endpoint, const curl_slist* headers, std::string &retBuffer, char *errorbuf, const char *buffer, const size_t size)
{
	CURL* curl;
	CURLcode res;
	if ((curl = curl_easy_init()) == 0)
	{
		std::cerr << "Failed to initialize curl\n";
		return CURLE_FAILED_INIT;
	}
	curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errorbuf);
	curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);

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
	/*if(buffer->capacity() <= buffer->size() + sz)
		buffer->resize(buffer->capacity() + sz);//!!
	memmove(buffer->data() + buffer->size(), data, sz);*/
	fwrite(data, 1, sz, fi);
	return sz;
}

CURLcode invokeDOWN(const char* endpoint, const curl_slist* headers, char* errorbuf, FILE *fi)
{
	CURL* curl;
	CURLcode res;
	if ((curl = curl_easy_init()) == 0)
	{
		std::cerr << "Failed to initialize curl\n";
		return CURLE_FAILED_INIT;
	}
	curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errorbuf);
	curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);

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
	
	res = curl_easy_perform(curl);
	curl_easy_cleanup(curl);
	
	return res;
}
