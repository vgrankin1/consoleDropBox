#pragma once

#define _CRT_SECURE_NO_WARNINGS

#include <string>
#include <vector>

#include <curl/curl.h>
#include <openssl/sha.h>

CURLcode invokeUP(const char* endpoint, const curl_slist* headers, std::string& retBuffer, char* errorbuf, const char* buffer, const size_t size, const bool verbose);

CURLcode invokeDOWN(const char* endpoint, const curl_slist* headers, char* errorbuf, std::string& dropbox_api_result, FILE* fi, const bool verbose);

CURLcode upload(FILE* fi, const std::string& access_token, const std::string& file_name_url, const bool verbose);
CURLcode download(FILE* fi, const std::string& access_token, const std::string& file_name_url, const bool verbose);

enum tool_action_t
{
	NO_ACTION = 0, PUT_ACTION, GET_ACTION
};

struct sha256_digest_t
{
	unsigned char digest[SHA256_DIGEST_LENGTH];
};

class sha256_DB_hash_t
{
	//DropBox hash chunksize
	const unsigned int DB_chunk_size = 4 * 1024 * 1024;
	std::vector< sha256_digest_t > hash_chunks;

public:
	sha256_DB_hash_t();
	sha256_DB_hash_t(FILE* fi);
	void push(const unsigned char* data, const size_t n);
	void get_digest(sha256_digest_t& hash);
	std::string get();
	std::string operator[](const size_t index) const;
	size_t size() const;
};

