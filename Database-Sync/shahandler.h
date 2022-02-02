#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include <chrono>
#include <ctime>

#include "openssl/sha.h"
#include "openssl/evp.h"
#include "openssl/err.h"

/**
* 
* Handles SHA encryption for use throughout the application. Pulls connection from the OpenSSL library.
* 
* 
*/

class ShaHandler
{
private:
	int m_SHAType = 1;

	static std::string encodesha1(std::string& val, bool raw = false);		// val is the passcode for the hash. if raw false then hex code returned.
	static std::string encodesha256(std::string& val, bool raw = false);	// if raw is true then a char string is returned. 
	static std::string encodesha512(std::string& val, bool raw = false);
public:
	ShaHandler(int shaType);												// initialize with chosen cha type.
	void SetSHAType(int shaType);											// reset sha type.
	std::string Encode(std::string& val, bool raw = false);					// encode and save value to string. 
	std::string hashForPost(std::string& strand, std::string& password, std::string& salt);
	static std::string XorString(std::string& string, std::string codex);	// static function returns the encoded string.
	static std::string unXorString(std::string& string, std::string codex);	// static function returns the unencoded string.		
	~ShaHandler(); // clean up any resources. 
};

