#pragma once

/**
* 
* json controls the creation and conversion of all data to and from a JSON string, 
* for transfer to the server. This will be fairly simple, but must use a few structs
* to transfer the data and maintain final cohesion. This will not manage the actual 
* encryption, which is done by PHP config at send time. 
* 
* static only class.
* 
*/

#include <string>
#include <vector>
#include <sstream>
#include <iostream>

#include "structs.h"


class json
{
private:
	static std::pair<std::string, DataPass> compileData(std::string title, std::string data, DPO type);

public:
	json();
	static std::string encode(DataPair data, std::vector<size_t> rowlen);	//use a full-length array of row lengths to determine each row, so different data sizes can be used.
	static DataPair decode(std::string data);								// decode to datapairs for later use, for return data.
	static std::string ReplaceAll(std::string str, const std::string from, const std::string to);	// replace string in string.
	static std::string CleanString(std::string str);						// clean symbols that would affect content.
};			

