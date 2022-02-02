#pragma once

#include <Windows.h>
#include <iostream>
#include <fstream>
#include <shlobj.h>
#include <limits>
#include <string>
#include <sstream>
#include <ios>

#include "shahandler.h"	// sha controller function

/**
* 
* PHP Config manages the php connection for the database and 
* handles the configuration file for automated connection.
* 
*/

class PHPConfig
{
protected:
	std::string m_DocumentRoot;				// document root, used by DBLocation and file location
	std::string m_DBName{"data.sqlite"};	// DB Location for sqlite
	std::string m_CFGFile{"config.ara"};	// Config file.
	std::string m_PHPLocation;				// PHP address, used to connect to the remote server address.
	std::string m_UserName;					// PHP username, as retrieved from the server
	std::string m_Password;					// PHP password, as retrieved from the server
	std::string m_Salt;						// PHP salt

	bool getDocumentPath();					// return the document path for use in getting the save file.
	bool checkConfigFile();					// check if the config file is created
	bool createConfigFile(std::string& User, std::string& Pass, std::string& Salt, std::string& path);

	std::string stringEncrypt(std::string& string); // encrypt a string using a chain approach for temporary storage.

	std::string stringDecrypt(); // decrypter will always use the default file.

public:
	PHPConfig();
	~PHPConfig();


};

