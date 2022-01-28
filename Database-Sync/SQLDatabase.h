#pragma once

/**
 *
 * SQLDatabase manages the databse connection for all connection controls 
 * ie: the basis of this application. This will call the DB, connect, then 
 * take the data and create the JSON to send to the server. This will not 
 * manage the JSON encryption or decpryption, which is handed by the
 * JSON class.
 * 
 */

#include "sqlite3.h"	// for SQL control
#include "structs.h"	// for DB data struct.

#include <Windows.h>
#include <shlobj.h>
#include <iostream>		// i/o control
#include <string>		// std::string
#include <utility>		// for std::pair
#include <vector>		// std::vector
#include <sstream> // std::Stringstream

class SQLDatabase
{
private: 
	std::string m_DBLoc;	// location of the phyiscal DB location
	sqlite3* m_DB;			// generate a DB object, needs to be destroyed using a special function
	DataPair m_ReturnData;
	bool m_verbose = false;	// set TRUE if you want to see full output. 
	
	bool GetDBLocation();
	std::string whereString(std::vector<std::pair<std::string, DataPass>> filter, std::string separator);
	bool Bind(sqlite3_stmt* stmt, DataPair filter, int start = 1);

public:
	SQLDatabase(); // initialize the DB
	
	DataPair Select(std::vector<std::string> columns, std::string table, DataPair filter, int limit = -1, int offset = -1); // for selecting lines, to be managed by callback. must be datapair now.
	bool Insert(std::string table, DataPair inserts);						// for inserting new lines, at inserts locations.
	bool Update(std::string table, DataPair updates, DataPair filter);		// for updating existing lines
	bool Remove(std::string table, DataPair filter);						// for deleting existing lines.

	bool PushPK(int ID, std::string table, int count = 1);		// push up the PK value for the ID, by COUNT specified.

	DataPair Complex(std::string& command, DataPair filter);

	IDType getNextPK(std::string table);
	
	~SQLDatabase();

};