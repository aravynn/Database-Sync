#pragma once

/**
* 
* DBClean: 
* 
* This function controls the database checker, at this point it is a function only for the local SQLite database. It will 
* Check all tables for inconsistencies that might break the main functionality of HoseControl. 
* 
* 
*/

#include <string>
#include <algorithm>

#include "SQLDatabase.h"

enum class Table {
	COMPANIES,
	CONTACTS,
	HOSES,
	HOSETEMPLATES,
	FITTINGTEMPLATES,
	HOSETESTS,
	TESTDATA
};

struct dataLine {
	// pk, testpk, temp, pressure, interval
	int PK{ -1 }, TestPK{ -1 }, Interval{ -1 };
	double Temperature{ -1.0 }, Pressure{ -1.0 };

	dataLine(int pk, int test, double temp, double pres, int inter) : PK{ pk }, TestPK{ test }, Interval{ inter }, Temperature{ temp }, Pressure{ pres } {}
};

class DBClean
{
private: 
	SQLDatabase m_db; // controls all database functions. 

	void printErrors(std::string title, DataPair data);

	void fixDate(DataPair errLine, Table table);
	void fixMismatch(DataPair errLine);
	void fixTestDataLength(DataPair errLine);
public:
	DBClean(); // check the database. 

};

