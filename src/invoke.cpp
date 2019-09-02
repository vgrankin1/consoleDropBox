
#include "main.hpp"

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


struct buffer_hash_t
{
	FILE* f;
	std::vector<char> buf;
	sha256_DB_hash_t* phash;
	//size_t wp;


	buffer_hash_t(FILE *fi, sha256_DB_hash_t *ptrhash)
		: f(fi), phash(ptrhash)
	{}
	void append(const char* data, const size_t n)
	{
		size_t size = buf.size();
		unsigned int DB_chunk_size = phash->DB_chunk_size;
		if (buf.capacity() <= size + n)
			buf.reserve(size + DB_chunk_size);
		buf.resize(size + n);
		memcpy(buf.data() + size, data, n);

		if (buf.size() >= DB_chunk_size)
		{
			phash->push((unsigned char*)buf.data(), DB_chunk_size);
			memmove(buf.data(), buf.data() + DB_chunk_size, buf.size() - DB_chunk_size);
			buf.resize(buf.size() - DB_chunk_size);
		}
	}
	void release()//last not pushed block, size that is less DB_chunk_size
	{
		phash->push((unsigned char*)buf.data(), buf.size());
		buf.resize(0);
	}
};
static size_t _downWriteFunc(char* data, size_t size, size_t nmemb, buffer_hash_t *hb)
{
	size_t sz = size * nmemb;
	hb->append(data, sz);
	fwrite(data, 1, sz, hb->f);
	return sz;
}

CURLcode invokeDOWN(const char* endpoint, const curl_slist* headers, char* errorbuf, std::string& dropbox_api_result, FILE *fi, sha256_DB_hash_t* ptrhash, const bool verbose)
{
	CURL* curl;
	CURLcode res;

	buffer_hash_t buffer_hash(fi, ptrhash);

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
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer_hash);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, _downWriteFunc);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, 0);
	curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, _upWriteFunc);//get a dropbox_api_result
	curl_easy_setopt(curl, CURLOPT_HEADERDATA, &dropbox_api_result);

	res = curl_easy_perform(curl);
	curl_easy_cleanup(curl);

	fflush(fi);
	buffer_hash.release();//Called for last block to push last chunk of data and calc last hash

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


CURLcode invokeList(const char* endpoint, const curl_slist* headers, const std::string &data, std::string& retBuffer, char* errorbuf,  const bool verbose)
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
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
	curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE_LARGE, data.size());
	res = curl_easy_perform(curl);
	curl_easy_cleanup(curl);
	return res;
}