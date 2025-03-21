#pragma once

#include <openssl/sha.h>

#include <filesystem>
#include <fstream>
#include <array>

inline std::array<uint8_t, SHA_DIGEST_LENGTH> shaFile(const std::filesystem::path& path)
{
	std::array<uint8_t, SHA_DIGEST_LENGTH> hash;
	SHA_CTX sha;
	SHA1_Init(&sha);

	std::ifstream file(path, std::ifstream::binary);

	const std::streamsize buffer_size = 64'000'000;
	char* buffer = new char[buffer_size];

	while (file.read(buffer, buffer_size))
		SHA1_Update(&sha, buffer, file.gcount());

	SHA1_Update(&sha, buffer, file.gcount());

	delete[] buffer;

	SHA1_Final(hash.data(), &sha);

	return hash;
}

inline std::array<uint8_t, SHA256_DIGEST_LENGTH> Sha256(const std::string& str)
{
	std::array<uint8_t, SHA256_DIGEST_LENGTH> hash;
	SHA256_CTX sha256;
	SHA256_Init(&sha256);
	SHA256_Update(&sha256, str.c_str(), str.size());
	SHA256_Final(hash.data(), &sha256);

	return hash;
}