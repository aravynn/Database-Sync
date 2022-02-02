#include "json.h"

json::json() {
	// doing nothin'
}

std::string json::encode(DataPair data, std::vector<size_t> rowlen) {

	// Encode data to a json format. The information must be granted a length to create array values for later passing. 

	size_t dataSize = data.size();
	size_t rowSize = rowlen.size();
	
	rowlen.push_back(100); // add an extra value. this prevents an overflow issue.

	std::stringstream ss;

	ss << "[";
	
	size_t rCount{0};

	for (size_t i{ 0 }; i < dataSize && rCount < rowSize; i += rowlen.at(rCount-1)) {

		//std::cout << i << '-' << rowSize << '-' << rowlen.at(rCount) << ' ';

		// for each row, we'll iterate through and create a string for return

		if (i != 0)
			ss << ",{";
		else
			ss << "{";
		
		for (size_t q{ 0 }; q < rowlen.at(rCount); ++q) {
			// for each element, set up as "key":"value" or "key":value, depending on type. 
			// we'll need an image solution later as well. 
			
			if (q != 0) { ss << ","; }

			ss << "\"" << CleanString(data.at(i + q).first) << "\":";
			
			if (data.at(i + q).second.o == DPO::STRING) {
				ss << "\"" << CleanString(data.at(i + q).second.str) << "\"";
			} 
			if (data.at(i + q).second.o == DPO::NUMBER) {
				ss << std::to_string(data.at(i + q).second.num);
			}
			if (data.at(i + q).second.o == DPO::DOUBLE) {
				ss << std::to_string(data.at(i + q).second.dbl);
			}
		}
		
		ss << "}";
		
		//std::cout << ss.str() << "\n\n\n";

		++rCount;
	}

	ss << "]";



	return ss.str();


}

DataPair json::decode(std::string data) {

	// decode JSON string back to Datapair values for parsing in later fnctions.

	size_t dataLength = data.size();
	DataPair retData;
	bool rowOpen = false;		// set to TRUE after a {, indicating a new row.
	bool stringOpen = false;	// set to TRUE after a ", indicating a new string variable.
	bool rowNameOpen = true;	// set to true by default, off ater a : is hit.
	bool esc = false;
	DPO dataType = DPO::NUMBER;		// default assume number, unless STRING defined.

	std::stringstream ss;		// store data in here for transfer to next variable. 

	std::string colName;

	//std::cout << "dataLength: " << dataLength << '\n';


	for (size_t i{ 0 }; i < dataLength; ++i) {
		// for each item, go through and convert into the data stream we expect. 

		if (esc) {
			ss << data.at(i);
			esc = false;
			continue;
		}

		if (stringOpen && data.at(i) == '\\') {
			esc = true;
			continue;
		}

		if (stringOpen && data.at(i) != '"') {
			ss << data.at(i);
			continue;
		}


		switch (data.at(i))
		{
		case  '"':
			// toggle stringOpen
			if (!stringOpen) {
				dataType = DPO::STRING;
				stringOpen = true;
			}
			else {
				stringOpen = false;
			}
			break;

		case '.':
			// this is a decimal for the numerical value, keep, and set to DECIMAL;
			dataType = DPO::DOUBLE;
			ss << data.at(i);
			break;

		case '{': 
			rowOpen = true;
			rowNameOpen = true; 
			dataType = DPO::NUMBER;
			break;

		case '}': 
			rowOpen = false;
			retData.push_back(compileData(colName, ss.str(), dataType));
			dataType = DPO::NUMBER;
			colName = "";
			ss.str(std::string());
			break;

		case ',': 
			if (rowOpen) {
				// only do this in this case.
				retData.push_back(compileData(colName, ss.str(), dataType));
				dataType = DPO::NUMBER;
				colName = "";
				ss.str(std::string());
			}
			break;

		case ':': 
			rowNameOpen = false;
			colName = ss.str();
			dataType = DPO::NUMBER;
			ss.str(std::string());
			break;

		case '[':
		case ']':
			// do nothing, can be ignored - we're only 2 arrays deep.
			break;

		default: 
			// anything not covered is DATA, treat as such.
			ss << data.at(i);
		}
	}

	return retData;
}

std::pair<std::string, DataPass> json::compileData(std::string title, std::string data, DPO type)
{

	//convert given data to a datapass.

	//std::cout << "JSON - Compile variable: " << title; 

	std::pair<std::string, DataPass> retData;

	retData.first = title;

	if (type == DPO::STRING) {
	//	std::cout << " String VALUE: " << data << '\n';
		retData.second = data;
	}
	if (type == DPO::NUMBER) {
	//	std::cout << " Number VALUE: " << data << '\n';
		retData.second = (IDType)std::stol(data);
	}
	if (type == DPO::DOUBLE) {
	//	std::cout << " Double VALUE: " << data << '\n';
		retData.second = (double)std::stod(data);
	}
	

	return retData;
}

std::string json::ReplaceAll(std::string str, const std::string from, const std::string to) {
	// replace all instances of a string with a different string.
	
	size_t start_pos{ 0 };
	while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length();

	}
	return str;
}

std::string json::CleanString(std::string str) {
	// replace all strings in array with ''. I don't know if we'll even need ReplaceAll

	str = ReplaceAll(str, "\n", "");
	str = ReplaceAll(str, "\r", "");
	str = ReplaceAll(str, "\\n", "");
	str = ReplaceAll(str, "\\r", "");
	str = ReplaceAll(str, "\"", "\\\"");
	str = ReplaceAll(str, std::string(1, unsigned char(248)), "");
	
	std::stringstream ss;

	for (char c : str) {
		if (c >= 0 && c < 128) {
			ss << c;
		}
	}

	return ss.str();
}