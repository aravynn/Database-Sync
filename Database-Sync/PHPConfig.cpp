#include "PHPConfig.h"

PHPConfig::PHPConfig() {
	// check for the config file and location, and create the file if needed. 

	//std::cout << "Load Path: ";

	if (getDocumentPath()) {
		// we got the path, so we can move to the next step.

		//std::cout << "Check Config File:\n";
		if (!checkConfigFile()) {
			// get the data to create a config file.
			std::cout << "Enter the PHP connection string: \n";
			std::string connectString;
			std::cin >> connectString; 
			std::cin.ignore(65536, '\n');
			std::cin.clear();

			std::cout << "Enter Username: \n";
			std::string userName;
			std::cin >> userName;
			std::cin.ignore(65536, '\n');
			std::cin.clear();
			
			std::cout << "Enter Password: \n";
			std::string password;
			std::cin >> password;
			std::cin.ignore(65536, '\n');
			std::cin.clear();

			std::cout << "Enter Salt: \n";
			std::string salt;
			std::cin >> salt;
			std::cin.ignore(65536, '\n');
			std::cin.clear();

			// create the config file
			createConfigFile(userName, password, salt, connectString);

		}

		//checkConfigFile();
	}
}

PHPConfig::~PHPConfig() {
	// delete any created resources. Will be likely. 
}

bool PHPConfig::getDocumentPath()
{	
	// get the local path of the config file, stored in the DB folder.

	TCHAR path[MAX_PATH]{ 0 };
	if (::SHGetSpecialFolderPath(NULL, path, CSIDL_MYDOCUMENTS, FALSE)) {
		std::stringstream ss;
		for (char c : path) {
			if (c != '\n' && c != '\r' && c != 0) {
				ss << c;
			}
		}
		m_DocumentRoot = ss.str() + "\\HoseTracker\\DB\\";
	//	std::cout << m_DocumentRoot << '\n';
	}
	else {
		// get path failed, error handle? 
		return false;
	}
	
	// get path succeeds.
	return true;
}

bool PHPConfig::checkConfigFile() {
	// check if the config file exists, and load the data if it does.
	std::string filename = m_DocumentRoot + m_CFGFile;
	
	std::ifstream myfile(filename);
	
	//std::cout << "TO DO: decrypt file after encryption \n";

	if (myfile.fail()) return false; // no file exists.

	char c;

	//std::stringstream shaCode;
	std::stringstream ss;
	int stringcount = 0;

	int initCount = 20;

	myfile.seekg(0, std::ios_base::end);  // go to end of file
	int lengthend = myfile.tellg();			
	myfile.seekg(0, std::ios_base::beg);
	int lengthstart = myfile.tellg();


	//while (myfile >> c) {
	for (int i{ 0 }; i < lengthend - lengthstart; ++i){

		myfile >> c;
		
		/* // the sha code is failing, so we'll loop back to this. 
		if (--initCount > 0) {
			shaCode << c;
		}
		else {
			ss << c;
		}*/
		
		// technically correct, but we need to separate the file first into the 2 strings.
		if (c != '|') {
			ss << c;
		}
		else {

			//std::cout << ss.str();
			// assign a variable
			switch (stringcount) {
			case 0:
				m_PHPLocation = ss.str();
				break;
			case 1:
				m_UserName = ss.str();
				break;
			case 2:
				m_Password = ss.str();
			default:
				break;
			}
			// clear the string
			ss.str(std::string());

			// prepare for the next value
			++stringcount;
		}
		
	}

	

	//std::string data = ss.str();
	//std::string codex = shaCode.str();
	//std::string deCrypted = ShaHandler::unXorString(data, codex);
	//std::cout << "Decrypted: " << deCrypted << '\n';

	m_Salt = ss.str();

	myfile.close();

	//std::cout << m_PHPLocation << '\n' << m_UserName << '\n' << m_Password << '\n' << m_Salt << '\n';

	return true; 
}

bool PHPConfig::createConfigFile(std::string& User, std::string& Pass, std::string& Salt, std::string& path) {
	// take the username, password and path and store it in a config file for making the DB connection. 
	// this should apply some sort of reversible encryption to make it a challenge to change/effect if 
	// the file was gotten a hold of. 

	//std::cout << "TO DO: Encrypt config file\n";

	std::string filename = m_DocumentRoot + m_CFGFile;

	std::ofstream myfile(filename);

	ShaHandler sha{ 1 };

	std::string ps = Pass + Salt; // pass is encrypted as salt and password.

	std::string encryptPass = sha.Encode(ps, false); // encode password as hex

	std::string data = path + "|" + User + "|" + encryptPass + "|" + Salt; // password is encoded, salt is RAW

	//data = stringEncrypt(data);

	//std::cout << data << '\n';

	myfile << data;

	myfile.close();

	m_PHPLocation = path;
	m_UserName = User;
	m_Password = Pass;

	return true; 
}

std::string PHPConfig::stringEncrypt(std::string& string) {
	// take the string and apply an encryptor like we've done before, storing a complete encapsulation.

	// use the creation date as the CODEX to encrypt the data. 
	struct tm newtime;
	std::time_t now = time(0);
	localtime_s(&newtime, &now);

	std::string NowDate = std::to_string(newtime.tm_year + 1900) + '-';

	if (newtime.tm_mon < 9) {
		NowDate.append('0' + std::to_string(newtime.tm_mon + 1) + '-' + std::to_string(newtime.tm_mday));
	}
	else {
		NowDate.append(std::to_string(newtime.tm_mon + 1) + '-' + std::to_string(newtime.tm_mday));
	}

	//std::cout << "Date: " << NowDate << '\n';

	ShaHandler sha{ 1 }; // create a sha 1 for the date object, will be used as the encryptor.

	std::string ShaDate = sha.Encode(NowDate, true);

	return ShaDate + ShaHandler::XorString(string, ShaDate); 
}

std::string PHPConfig::stringDecrypt() {

	std::string fileLocation{ m_DocumentRoot + m_CFGFile };

	// use the file creation date as the decyptor. it is stored as a Sha1 value in the start of the string, which will be used to decrypt the rest. 
	// I know this isn't the strongest security, but is only for local file data, to protect the file from tinkering. 
	// FYI - first 20 bytes are the 20 char sha string. 
	// remaining data should be decrypted and sent back, without breaking. 

	std::ifstream myFile{ fileLocation };


	return fileLocation; //-----------------------------------------------------------------------------------------------------!!
}

