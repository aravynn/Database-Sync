#pragma once

#include "PHPConfig.h"   // for PHP control information
#include "SQLDatabase.h" // for SQL Database connection
#include "shahandler.h"	 // for sha, encryption functionality
#include "json.h"
#include "structs.h"	 // for error codes.

#include "curl.h"
#include "openssl/sha.h"
#include "openssl/evp.h"
#include "openssl/err.h"

#include <iostream>		// i/o control
#include <ios>
#include <string>		// std::string
#include <utility>		// for std::pair
#include <vector>		// std::vector
#include <sstream> // std::Stringstream
#include <iomanip>		// for std::setfill, std::setw
#include <climits>		// for SCHAR_MAX

class NetConnect : public PHPConfig
{
private:
	SQLDatabase *m_DB;	// database for local connection.

	CURL* m_curl;

	std::string m_ReturnData;	// returned data from callback function.
	int m_LineCount = 3;		// can be overridden if we want to send more than 10 lines at a time. 

	static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);

	bool TestConnect();		// test the connection agains the server. 

	LoadStatus UploadReturn();	// return from upload, manage any changes to the DB. 
	void ClearUploads();	// clear the uploads, only use if upload return is OK.

	std::pair<std::string, DataPass> EncodeImage(std::string columnName, const std::string& filePath);
	std::string decodeImage(std::string data, std::string name);

	bool checkChar(char c);
	std::string cleanName(std::string& name);
	std::string getFileType(std::string& fileData);

public:
	NetConnect();
	~NetConnect();

	bool Transfer(std::string &data);	// handle all curl connections and data transfer. returns true if no issues. data moves to m_ReturnData

	std::string UploadString();	// create string to send for upload.
	
};

