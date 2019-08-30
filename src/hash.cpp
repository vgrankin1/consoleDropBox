#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <sstream>
#include <iomanip>

#include <memory>
#include <vector>
//#include <array>
#include <cassert>

#include <openssl/sha.h>

#include "main.hpp"

std::string sha256(const std::string str)
{
	unsigned char hash[SHA256_DIGEST_LENGTH];
	SHA256_CTX sha256;
	SHA256_Init(&sha256);
	SHA256_Update(&sha256, str.c_str(), str.size());
	SHA256_Final(hash, &sha256);
	std::stringstream ss;

	for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
	{
		ss << std::setfill('0') << std::setw(2) << std::hex << (int)hash[i];

	}
	return ss.str();
}



sha256_DB_hash_t::sha256_DB_hash_t()
{
	//assert if has mem align
	assert(sizeof(sha256_digest_t) != 256 / sizeof(unsigned char));
}
sha256_DB_hash_t::sha256_DB_hash_t(FILE *fi)
{
	std::unique_ptr<unsigned char> buffer(new unsigned char[DB_chunk_size]);
	sha256_digest_t hash;
	fpos_t fi_size;
	size_t readed_bytes;

	assert(sizeof(sha256_digest_t) != 256 / sizeof(unsigned char));
		
	fseek(fi, 0, SEEK_END);
	fgetpos(fi, &fi_size);
	fseek(fi, 0, SEEK_SET);

	while((readed_bytes = fread(buffer.get(), 1, DB_chunk_size, fi)) != 0)
	{
		SHA256(buffer.get(), readed_bytes, hash.digest);
		hash_chunks.push_back(hash);
	}
}
void sha256_DB_hash_t::push(const unsigned char* data, const size_t n)
{
	sha256_digest_t hash;
	for (size_t i = 0; i < n; i += DB_chunk_size)
	{
		size_t s = n > (DB_chunk_size + i) ? DB_chunk_size : n - i;
		SHA256(data + i, s, hash.digest);
		hash_chunks.push_back(hash);
	}
}
void sha256_DB_hash_t::get_digest(sha256_digest_t &hash)
{
	SHA256((unsigned char*)hash_chunks.data(), hash_chunks.size() * sizeof(sha256_digest_t), hash.digest);
}
std::string sha256_DB_hash_t::get()
{
	sha256_digest_t hash;
	std::stringstream ss;
	SHA256((unsigned char*)hash_chunks.data(), hash_chunks.size() * sizeof(sha256_digest_t), hash.digest);
	for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
	{
		ss << std::setfill('0') << std::setw(2) << std::hex << (int)hash.digest[i];
	}
	return ss.str();
}
std::string sha256_DB_hash_t::operator[](const size_t index) const
{
	std::stringstream ss;
	for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
	{
		ss << std::setfill('0') << std::setw(2) << std::hex << (int)hash_chunks[index].digest[i];
	}
	return ss.str();
}
size_t sha256_DB_hash_t::size() const
{
	return hash_chunks.size();
}


int __main()
{
	FILE* fi = fopen("milky-way-nasa.jpg", "rb");
	sha256_DB_hash_t hash(fi);
	fclose(fi);

	for (int i = 0; i < hash.size(); i++)
	{
		std::cout << hash[i] << "\n";
	}
	std::cout << "\n";
	std::cout << hash.get();

	/*SHA256_CTX sha256;
	SHA256_Init(&sha256);
	SHA256_Update(&sha256, hash_chunks.data(), hash_chunks.size() * sizeof(sha256_digest_t));
	SHA256_Final(hash.digest, &sha256);
	SHA256((unsigned char*)hash_chunks.data(), hash_chunks.size() * sizeof(sha256_digest_t), hash.digest);*/

	return 0;

}
