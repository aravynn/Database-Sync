#include "NetConnect.h"

NetConnect::NetConnect() : PHPConfig()
{
    // load the connection application and manage the connection. 
    // This will handle all display and other information.


    //for (int i{ 0 }; i < 35; ++i) {
    //    std::cout << statusBar("Sample", i*3, 100);
        //std::this_thread::sleep_for(std::chrono::milliseconds(300));
    //}
    
    
    // initialize the application. Most functions should run primarily here. 
    m_curl = curl_easy_init();

    // Access the DB, initialize connection
    m_DB = new SQLDatabase();

    std::cout << static_cast<unsigned char>(218) << "--------------------------------" << static_cast<unsigned char>(191) << '\n';
    std::cout << '|' <<     " Database Sync For Hose Control " << '|' << '\n';
    std::cout << '|' <<     " Version 2.0.0       01/25/2022 " << '|' << '\n';
    std::cout << '|' <<     " Allow This To Complete Before  " << '|' << '\n';
    std::cout << '|' <<     " Entering or testing Hoses.     " << '|' << '\n';
    std::cout << static_cast<unsigned char>(192) << "--------------------------------" << static_cast<unsigned char>(217) << '\n';

    // connect with php, confirm connection OK
    if (TestConnect()) {
        // do a test transfer from the DB.
        //std::string upload = UploadString();
        //std::cout << upload << "\n\n\n";
        //Transfer(upload);
        //std::cout << "Return Data: " << m_ReturnData << '\n';

        std::cout << "Connection OK \n";


        // request for data transfers and changes to current DB, 10 rows at a time. (id changes, inserts, deletes, updates)
        LoadStatus ret = LoadStatus::OK;

        /*
        std::string downloadReq = json::encode({ {"Download", (IDType)m_LineCount} }, { 1 });

        std::cout << "Download: ";
        while ((int)ret > -1) {
            // Transfer request, though no data included.
            Transfer(downloadReq);
            // apply all changes to DB, wait for no results returned.

           // std::cout << "Download return: " << m_ReturnData << '\n';

            ret = UploadReturn();
            std::cout << ".";
        }
        std::cout << "\n Download OK \n";

        // when server responds with "no more changes", move to next section.

        // create loop to go through all active uploads for transfer.
        ret = LoadStatus::OK;
        */
        
        //std::cout << "Upload: \n";

        int totals = getUploadCount();
        int count{ 0 };

        while ((int)ret > -1) {
            // get ten rows of transfer (updates, inserts, etc)
            // for each 10 rows, get the data to be transferred.
            count = totals - getUploadCount() + 1;
            // concatenate data into JSON file
            std::string uploadReq = UploadString();
            
            //std::cout << "Upload string: " << uploadReq << "\n\n";

            if(uploadReq == ""){
                // there is nothing to upload. 
                ret = LoadStatus::COMPLETE;
                continue;
            }

            // send data to PHP
            Transfer(uploadReq);

           // std::cout << "Return: " << m_ReturnData << "\n\n";

            // get return data and any changes (should only be PK changes) 
            ret = UploadReturn();
            
            // execute any changes as prescribed by the returned changes 
            ClearUploads();

            if (ret == LoadStatus::UPLOAD_ERROR) {
                std::cout << "Upload Failed                                    \n";
                break; // exit the loop, there is an upload error.
            }
            std::cout << statusBar("Upload: ", count, totals);
        }
        std::cout << "Upload OK                                      \n";
    }
    else {

        std::cout << "Connection Failed! Aborting... \n";
    }
    
}

NetConnect::~NetConnect()
{
    // clean up connections and delete objects.

    curl_easy_cleanup(m_curl); // clean up curl. This was a memory leak.
    curl_global_cleanup();
	delete m_DB;
}

bool NetConnect::Transfer(std::string &data)
{
	// manage the data transfer to and from the DB. Should be used for all connections, houses the curl function.
	
    //CURL* curl;
    CURLcode res;
    std::string readBuffer;
    //std::basic_string<unsigned char> readBuffer;
    
    
    if (m_curl) {

       // std::cout << "Curl Parameters: " << m_PHPLocation << ' ' << m_Password << ' ' << m_Salt << '\n';

        curl_easy_setopt(m_curl, CURLOPT_URL, m_PHPLocation.c_str());
        curl_easy_setopt(m_curl, CURLOPT_VERBOSE, 0);
        curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYPEER, 0l); // we won't verify the peer. I know that is a poor decision.

        ShaHandler* sha = new ShaHandler(512);
        
        // thread for transfer. 
        
        //std::cout << data << '\n';

        int threadLength = data.size();
        int maxSize = 7680;

        if (threadLength > maxSize) {
            // 1024 minus to account for username, length to be transferred, plus a decent margin of safety. 
            
            // do a for loop to go through the length of thread. 
            for (int i{ 0 }; i < threadLength; i += maxSize) {
                
                readBuffer = std::string();

                std::string thisdata = data.substr(i, (maxSize > threadLength - i ? threadLength - i : maxSize));



                //std::cout << "ThisData: " << thisdata << "\n\n\n";

                std::string thread = sha->hashForPost(thisdata, m_Password, m_Salt);

                std::string* headersend = new std::string{ 
                    "User=" + m_UserName + 
                    "&ThreadLength=" + std::to_string(threadLength) + 
                    "&thread=" + thread 
                };

                curl_easy_setopt(m_curl, CURLOPT_POSTFIELDS, headersend->c_str());
                curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, WriteCallback);
                curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, &readBuffer);
                res = curl_easy_perform(m_curl);

                // get the size of readbuffer, needs to be below 16K
                //std::cout << "Send size: " << headersend->size() << '\n';

                if (res != CURLE_OK) {
                    fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
                }

                std::stringstream ss;

                for (char c : readBuffer) {
                    if (c >= 0 && c < 128) {
                        ss << c;
                    }
                }

                // for everything but the last transfer, this will be ignored. 
                m_ReturnData = ss.str();

                // always clean up.
                delete headersend;
            }
            

        }
        else {

            std::string thread = sha->hashForPost(data, m_Password, m_Salt);

            std::string* headersend = new std::string{ "User=" + m_UserName + "&thread=" + thread };

            curl_easy_setopt(m_curl, CURLOPT_POSTFIELDS, headersend->c_str());
            curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, &readBuffer);
            res = curl_easy_perform(m_curl);

            // get the size of readbuffer, needs to be below 16K
            //std::cout << "Send size: " << headersend->size() << '\n';

            if (res != CURLE_OK) {
                fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            }

            std::stringstream ss;

            for (char c : readBuffer) {
                if (c >= 0 && c < 128) {
                    ss << c;
                }
            }

            m_ReturnData = ss.str(); 

            // always clean up.
            delete headersend;
           
        }

        delete sha;
    }
    

    return true;
}

size_t NetConnect::WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    // cURL function callback. 

    ((std::string*)userp)->append((char*)contents, size * nmemb);

    return size * nmemb;
}

bool NetConnect::TestConnect(){
    // use Transfer to test that we have connection.
    // return TRUE if there is an OK returned, or FALSE if error.

    DataPair testData;
    testData.push_back(std::pair<std::string, DataPass>({ std::string("TC"), std::string("Test Connection") }));

    std::string data = json::encode(testData, { (size_t)1 });
    Transfer(data);

    if (m_ReturnData == "{\"Return\":\"OK\"}") {
        return true; // connection did work.
    }

    return false;
}


std::string NetConnect::UploadString()
{
    // take the first 10 lines in the DB, and create a string for them to upload. 
    // for security sake, we won't delete these - they should wait for data return.

    // use m_LineCount to determine number of lines.

    std::string table{ "DataSync" };
    std::vector<std::string> columns{ "TableName", "TablePK", "Type" };
    DataPair filter;
    filter.push_back(std::pair<std::string, DataPass>("", ""));

    // use m_LineCount for LIMIT, no offset. 

    DataPair retData = m_DB->Select(columns, table, filter, m_LineCount);

    if (retData.size() < 1 || retData.at(0).first == "") {
        // there is nothing, abort. 
        return "";
    }

    std::vector<size_t> columnLengths; // column lengths to transfer.
    DataPair finalData; // the final data to compile and tranfer.

    columns.at(2) = "TransferType";

    // once we have the DataSync Lines selected, we need to get the individual content from the Tables specified
    // Once we have all of the data for each line, concetenate both arrays into a single large array for conversion and transfer.

    for (size_t i{ 0 }; i < retData.size(); i += columns.size()) {
        // for each series, use the data to get the data from the DB, and concatenate into a singular large set.
        // how do we do a separation for each of the different database types?

        // push back change data to first 3 columns of the data transfer.
        for (size_t q{ 0 }; q < columns.size(); ++q) {

            retData.at(i + q).first = columns.at(q);

            finalData.push_back(retData.at(i + q));
        }

        // create filter for PK 
        DataPair DataFilter;
        DataFilter.push_back(std::pair<std::string, DataPass>("PK", retData.at(i + 1).second.num));

        // get the column data for this line
        DataPair DataColumns = m_DB->Select({}, retData.at(i).second.str, DataFilter); // get the column at PK.


        // check the tableformat if an image line exists. 
        DataPair imageLine = m_DB->Select({ "ColumnName" }, "TableFormat", { {"TableName", retData.at(i).second.str}, {"ColumnType", (IDType)ColumnType::IMAGE} }, 1);

        // only 1 image is ever on a line.
        std::string imgLineName = "";

        if (imageLine.size() > 0) {
            // an image exists, save the name so we can refer to that later. 
            imgLineName = imageLine.at(0).second.str;
        }



        // move the columns into the final vector for release. 
        for (size_t q{ 0 }; q < DataColumns.size(); ++q) {

            // check if current line is image, if it is, we'll return an image encoded instead. 
            if (imgLineName == DataColumns.at(q).first) {
                // process the image into a hex code data format. return the datapair.
                finalData.push_back(EncodeImage(DataColumns.at(q).first, DataColumns.at(q).second.str));
            }
            else {
                // proceed normally, this is not an image.
                finalData.push_back(DataColumns.at(q));
            }
        }

        // add the final column length to the lengths vector
        columnLengths.push_back(columns.size() + DataColumns.size()); // + RETURN DATA SIZE
    }

    // return the oncoded string.
    return json::encode(finalData, columnLengths);
}

LoadStatus NetConnect::UploadReturn()
{
    // check the updates, and make sure everything is OK,
    // then, manage any changes returned by the server. 
    // return 0 if everything is OK. return -x for various errors.
   
    //std::cout << m_ReturnData << '\n';

    if (m_ReturnData == "{\"Return\":\"OK\"}") {

        // in this case, we can assume everything worked, and to move to next step
        return LoadStatus::OK;
    }

    if (m_ReturnData == "{\"Return\":\"Complete\"}") {

        // in this case, we can assume that no additional downloads are required.
        return LoadStatus::COMPLETE;
    }


    // This prevents the remaining code, as we'll give an OK value, even if data remains.
    return LoadStatus::OK;


    // decode the return into an array. 
    DataPair data = json::decode(m_ReturnData);

    std::vector<size_t> rowWidth;
    bool first = true;
    size_t count = 0;

    // go through all data, and create row lengths for the next return steps. 
    for (std::pair<std::string, DataPass> d : data) {

        if (d.first == "TableName") {
            if (first) {
                first = false;
            }
            else {
                rowWidth.push_back(count);
                //std::cout << "row width count: " << count << '\n';
                count = 0;
            }
        }
        ++count;

    }
    rowWidth.push_back(count);
    //std::cout << "row width count: " << count << '\n';
    rowWidth.push_back(100); // push back an additional value to prevent later crashes. 

    // for each line, we'll need to check what it is for, then use the data appropriately.
    // use the first row type (always the same) of "TableName" to determine row widths. 

    DataPair errorLines;
    std::vector<size_t> errLineCount;
    size_t rowCount{ 0 };

    for (size_t i{ 0 }; i < data.size() && rowCount < rowWidth.size(); i += rowWidth.at(rowCount - 1)) {
        // for each set, check (2) for type, and perform action accordingly.

        //std::cout << data.at(i + 2).first << ' ' << data.at(i + 2).second.num << '|' << data.at(i + 2).second.str << '|' << data.at(i + 2).second.dbl << '\n';

        // check each file for images, if one exists, handle the file save accordingly. 

       // std::cout << "@ 372 - Check PK for name \n";
        std::string name{ std::to_string(data.at(i + 1).second.num) };

        for (size_t q{ i }; q < i + rowWidth.at(rowCount); ++q) {
            // for each element, check if it is Image, if it is save the file and update the contents of the datapass.
            if (data.at(q).first == "Name") {
                name = data.at(q).second.str;
              //  std::cout << "@ 379 - Found name: " << name << " \n";
            }

            if (data.at(q).first == "Image") {
             //   std::cout << "@ 383 - Image Found, start Decode. DATA: " << data.at(q).second.str << " \n";
                
                std::string filepath = decodeImage(data.at(q).second.str, name);
                
             //   std::cout << "@ 385 - Decode Complete \n";
                
                data.at(q).second.str = filepath;
            }
        }

       // std::cout << "@ 388 - Proceed with standard data management \n";

        switch (data.at(i + 2).second.num) {
        case (IDType)Sync::INSERTID:
            // encapsulate the new data and add it to the appropriate location. 
            {
            //std::cout << "Check insert \n";
                // check the current PK is not already inhabited by a different item. 
                DataPair checks = m_DB->Select({ "PK" }, data.at(i).second.str, { {"PK", data.at(i + 1).second.num} }, 1);
                
                //std::cout << "Push PK\n";

                // if it is, push back by one to make space. 
                if (checks.size() > 0) {
                    // it got sometihng. 
                    m_DB->PushPK(data.at(i + 1).second.num, data.at(i).second.str); // default second value
                }

                //std::cout << "start Insert\n";

                // then, proceed to insert as per normal. 
                DataPair transferData;
                for (size_t q{ 3 }; q < rowWidth.at(rowCount); ++q) {
                    // transfer the information to a dedicated array
                    transferData.push_back(data.at(i + q));
                }
                m_DB->Insert(data.at(i).second.str, transferData);
            }
            break;
        case (IDType)Sync::UPDATEID:
            // update existing row, using given data. 
            {
            //std::cout << "start Update Number: " << i << "\n";
                DataPair transferData;
                for (size_t q{ 3 }; q < rowWidth.at(rowCount); ++q) {
                    // transfer the information to a dedicated array
                    //std::cout << "counts: " << (i + q) << '\n';

                    transferData.push_back(data.at(i + q));
                    //std::cout << "Transfer Col: " << data.at(i + q).first << "->" << data.at(i + q).second.num << '|'
                    //    << data.at(i + q).second.str << '|' << data.at(i + q).second.dbl << '\n';
                }
               // std::cout << "Update SQL: \n";

                m_DB->Update(data.at(i).second.str, transferData, { {"PK", (IDType)data.at(i + 1).second.num} });
            }
            break;
        case (IDType)Sync::DELETEID:
            // Remove a line, will this ever be used? 
            m_DB->Remove(data.at(i).second.str, { {"PK", (IDType)data.at(i + 1).second.num} });
            break;
        case (IDType)Sync::CHANGEID:
            // push up ID's by value specified. 
            if (data.at(i + 3).second.num > 0) {
                // only apply this if the difference is a positive number. 
                m_DB->PushPK(data.at(i + 1).second.num, data.at(i).second.str, data.at(i + 3).second.num);
            }
            break;
        case (IDType)Sync::ERRORID:
            // Push the ID lines for the next step.
            errorLines.push_back(data.at(i));
            errorLines.push_back(data.at(i + 1));
            errLineCount.push_back(2);
            break;
        default:
            // do nothing.
            break;
        }
        ++rowCount;
    }

    // after all updates are completed, convert array of errors to json array to pass back for 
    // clear function to manage, and skip those rows for re-attempt.
    
    if (errorLines.size() > 0) {
        // theres lines to send
        m_ReturnData = json::encode(errorLines, errLineCount);
        
        if (m_ReturnData.size() >= m_LineCount) {
            // if there is this many issues, we need to abort, something is bigtime wrong. 
            return LoadStatus::UPLOAD_ERROR;
        }
    }
    else {
        // we go nothing. return OK 
        m_ReturnData = "{\"Return\":\"OK\"}";
    }

    return LoadStatus::OK; // return OK if everything at this point is good. 
}

void NetConnect::ClearUploads() 
{
    // delete the upload lines in the DB, and move onto the next lines. 

    // get the lines we used previously, and compare them to the data gotten by m_return 

    std::string table{ "DataSync" };
    std::vector<std::string> columns{ "PK" };
    DataPair filter;
    filter.push_back(std::pair<std::string, DataPass>("", ""));

    // use m_LineCount for LIMIT, no offset. 
    DataPair retData = m_DB->Select(columns, table, filter, m_LineCount);

    //std::cout << "CLEAR: " << m_ReturnData << '\n';

    if (m_ReturnData != "{\"Return\":\"OK\"}") {
        // we'll need to set only the values given as removed. 
        
        

        DataPair removePairs = json::decode(m_ReturnData);
        std::vector<IDType> Haystack;

        for (std::pair<std::string, DataPass> d : removePairs) {
            Haystack.push_back(d.second.num);
        }
        
        for (size_t i{ 0 }; i < retData.size(); ++i) {
            if (std::find(Haystack.begin(), Haystack.end(), retData.at(i).second.num) == Haystack.end()) {
                m_DB->Remove("DataSync", { {"PK", retData.at(i).second.num} });
            }
        }
        
    }
    else {

        //std::cout << "Remove All \n";
        // we can delete everything here. 
        for (size_t i{ 0 }; i < retData.size(); ++i) {
            m_DB->Remove("DataSync", { {"PK", retData.at(i).second.num} });
        }

    }
}

std::pair<std::string, DataPass> NetConnect::EncodeImage(std::string columnName, const std::string &filePath) {
   
    // take the encoded string, decode it, then save it to a file.
    //std::cout << "TESTING FILE SAVE TO DESKTOP: \n";
    //std::cout << "Location: C:/Users/aravy/Desktop/imagetest.jpg \n";

        std::stringstream testOutput;

        //std::ofstream makeFile("C:/Users/aravy/Desktop/imagetest.jpg", std::ios::binary);

        std::ifstream myfile(filePath, std::ios::binary);
        myfile.seekg(0, std::ios_base::beg);

        std::stringstream tester;
        std::stringstream testerHex;
        tester << myfile.rdbuf();

        for(unsigned char c : tester.str()){
            
            //testerHex << std::hex << std::setfill('0') << std::setw(2) << c;
            if ((int)c < 16) {
                testerHex << "0" << std::hex << (int)c;
            }
            else {
                testerHex << std::hex << (int)c;
            }
        }

        std::string fileHexed = testerHex.str();
      /*
        
        std::string origStr = tester.str();
        
        for (size_t i{ 0 }, q{ 0 }; i < fileHexed.size(); i += 2, ++q) {
           // for every 2, enter the char data
           // q is original position
           int a, b;
           if (fileHexed.at(i) <= '9' && fileHexed.at(i) >= '0') {
               a = (int)fileHexed.at(i) - 48;
           } else if (fileHexed.at(i) <= 'F' && fileHexed.at(i) >= 'A') {
               a = (int)fileHexed.at(i) - 55;
           }
           else if (fileHexed.at(i) <= 'f' && fileHexed.at(i) >= 'a') {
               a = (int)fileHexed.at(i) - 87;
           }
           else {
               std::cout << "Unable to determine the value for : " << fileHexed.at(i) << " Character is not hex \n";
           }

           if (fileHexed.at(i + 1) <= '9' && fileHexed.at(i + 1) >= '+') {
               b = (int)fileHexed.at(i + 1) - 48;
           }
           else if (fileHexed.at(i + 1) <= 'F' && fileHexed.at(i + 1) >= 'A') {
               b = (int)fileHexed.at(i + 1) - 55;
           }
           else if (fileHexed.at(i + 1) <= 'f' && fileHexed.at(i + 1) >= 'a') {
               b = (int)fileHexed.at(i + 1) - 87;
           }
           else {
               std::cout << "Unable to determine the value for : " << fileHexed.at(i + 1) << " Character is not hex \n";
           }

           int xcc = (a * 16) + b;

           // manual forced conversion ensures proper typing.
           char c = (char)(xcc > SCHAR_MAX ? xcc - 256 : xcc);

           testOutput << c;
           
           if (q < 200) { // only do 100 characters. 
               if (c != origStr.at(q)) {
                   std::cout << "String Error: " << fileHexed.at(i) << fileHexed.at(i + 1) << ' ' << c << '(' << (int)c << ':' << 'a' << a << " b" << b << ' ' << xcc << ") is not equal to " << origStr.at(q) << '(' << (int)origStr.at(q) << ")\n";
               }
               else {
                 //  std::cout << "String OK: " << fileHexed.at(i) << fileHexed.at(i + 1) << ' ' << c << '(' << (int)c << ':' << xcc << ") is equal to " << origStr.at(q) << '(' << (int)origStr.at(q) << ")\n";
               }
           }
           
       }

       // FILE NOT SAVING CORRECTLY STILL, BUT THIS IS A GOOD START, or something. =--------------------------------------------!!

        makeFile << testOutput.str(); //tester.str(); tester works, as standard stringstream. 

        */

        // close the file, we're done. 
        myfile.close();
        //makeFile.close();

    return std::pair<std::string, DataPass>(columnName, fileHexed);
}

std::string NetConnect::decodeImage(std::string data, std::string name) {
    // download the file to the appropriate location, and return the location string for the DB.
    
    // this is the path that will be returned.
    std::string filePath;

    // determine the filetype 
    std::string filetype = getFileType(data);

  //  std::cout << "@ 627 - Got file extension: " << filetype << " \n";

    // get the path from the system.
    TCHAR path[MAX_PATH]{ 0 };
    if (::SHGetSpecialFolderPath(NULL, path, CSIDL_MYDOCUMENTS, FALSE)) {
        std::stringstream ss;
        for (char c : path) {
            if (c != '\n' && c != '\r' && c != 0) {
                ss << c;
            }
        }

        // set the final filePath.
        filePath = ss.str() + "\\HoseTracker\\Images\\MEPBrothers\\Download_" + cleanName(name) + filetype;

        bool newFile{ true };
        int step{ 0 };

     //   std::cout << "@ 645 - Created file path (initial): " << filePath << " \n";

        while (newFile) {
            // check for the file, if exists, add a _n
            std::ifstream fileTest(filePath);
            
            if (!fileTest.good()) {
                // this file  does not exist, use this name.
                //::cout << "@ 655 File Does Not Exist \n"

                newFile = false;
            }
            else {
                ++step;
                // update the filepath for the next check.
                filePath = ss.str() + "\\HoseTracker\\Images\\MEPBrothers\\Download_" + cleanName(name) + "_" + std::to_string(step) + filetype;     
            }

            // we only need to check the file, not the contents.
            fileTest.close();
        }


     //   std::cout << "@ 666 - Created file path (final): " << filePath << " \n";
    }

    // decode the data
    std::ofstream makeFile(filePath, std::ios::binary);
    std::stringstream testOutput;

    //std::cout << "@ 673 - Start data conversion to final bit string \n";

    // convert the hex to a bit string
    for (size_t i{ 0 }, q{ 0 }; i < data.size(); i += 2, ++q) {
         // for every 2, enter the char data
         // q is original position
         int a, b;
         if (data.at(i) <= '9' && data.at(i) >= '0') {
             a = (int)data.at(i) - 48;
         } else if (data.at(i) <= 'F' && data.at(i) >= 'A') {
             a = (int)data.at(i) - 55;
         }
         else if (data.at(i) <= 'f' && data.at(i) >= 'a') {
             a = (int)data.at(i) - 87;
         }
         else {
             std::cout << "Unable to determine the value for : " << data.at(i) << " Character is not hex \n";
         }

         if (data.at(i + 1) <= '9' && data.at(i + 1) >= '+') {
             b = (int)data.at(i + 1) - 48;
         }
         else if (data.at(i + 1) <= 'F' && data.at(i + 1) >= 'A') {
             b = (int)data.at(i + 1) - 55;
         }
         else if (data.at(i + 1) <= 'f' && data.at(i + 1) >= 'a') {
             b = (int)data.at(i + 1) - 87;
         }
         else {
             std::cout << "Unable to determine the value for : " << data.at(i + 1) << " Character is not hex \n";
         }

         int xcc = (a * 16) + b;

         // manual forced conversion ensures proper typing.
         char c = (char)(xcc > SCHAR_MAX ? xcc - 256 : xcc);

         testOutput << c;

     }

   // std::cout << "@ 714 - Output final file \n";

    // output to file
     makeFile << testOutput.str(); 
     makeFile.close();

     //std::cout << "@ 720 - Return the File Path \n";

     // return the path.
     return filePath;

}

bool NetConnect::checkChar(char c) {
    // check if character is valid or not, return true if it is.
    int d = (int)c;

    if (d > 47 && d < 58) {
        return true;
    }
    if (d > 64 && d < 91) {
        return true;
    }
    if (d > 96 && d < 123) {
        return true;
    }
    return false;
}

std::string NetConnect::cleanName(std::string& name) {
    // clean a string and make it acceptable for use as a directory name
    // limit string length to 20 characters, regardless of string length.
    // cannot use: /\?%*:|"<>. also space
    // will limit to ascii 48-57 or 65-90 or 97-122
    std::stringstream ss;
    int counter = 20;

    for (int i{ 0 }; i < name.length(); ++i) {
        // iterate though, and clear all non 0-9a-zA-z, feed into ss.
       // if(base.at(i) == )
        char q = name.at(i);

        if (checkChar(q)) {
            ss << name.at(i);
            --counter;
        }
        if (counter == 0)
            break;
    }
    return ss.str();
}

std::string NetConnect::getFileType(std::string& fileData) {
    // get the file type of the uploaded file, and return the type or a ERR if the file is invalid.

    std::string png = "89504e470d0a1a0a"; // allegedly the string starter for the png. 8 bits as hex.
    std::string jpg = "ffd8ffd9"; // opener and closer of jpg.
    std::string gif87a = "4749463837613b"; //gif87a; the ; is the last bit of the file.
    std::string gif89a = "4749463839613b"; //gif89a;
    std::string bmp = "424d"; // the first 2 bits.

    std::stringstream ss;
    
   // std::cout << "@ 778 - Start Checking Type \n";

    if (fileData.size() < 1) {
        // we have an issue, return nothing.
       // std::cout << "ERROR @ 783 - file too short. DATA: " << fileData << " \n";
        return ".unknown";
    }
    
    // 8 bit buffer, PNG
    for (int i{ 0 }; i < 16; ++i) { ss << fileData.at(i); }
    if (ss.str() == png) { return ".png"; }

    ss.clear();
    ss.str(std::string());

    //std::cout << "@ 788 - Type not PNG \n";

    // For simplicity, 2 bytes only.
    ss << fileData.at(0) << fileData.at(1) << fileData.at(2) << fileData.at(3);
    ss << fileData.at(fileData.size() - 4) << fileData.at(fileData.size() - 3) << fileData.at(fileData.size() - 2 ) << fileData.at(fileData.size() - 1 );

   // std::cout << "@ 799 Checking jpeg. Data: " << ss.str() << " \n";

    if (ss.str() == jpg) { return ".jpg"; }

    ss.clear();
    ss.str(std::string());

    // 7 bit buffer, gif, first 6 and lst bit.
    for (int i{ 0 }; i < 12; ++i) { ss << fileData.at(i); }
    ss << fileData.at(fileData.size() - 2) << fileData.at(fileData.size() - 1);
    if (ss.str() == gif87a || ss.str() == gif89a) { return ".gif"; }

    ss.clear();
    ss.str(std::string());

    //2 bit buffer, first 2 bits.
    for (int i{ 0 }; i < 4; ++i) { ss << fileData.at(i); }
    if (ss.str() == bmp) { return ".bmp"; }

    // if nothing matched, then it is not a valid image.
    return ".unknown";
}

std::string NetConnect::statusBar(std::string title, int current, int total)
{
    // generate a styled status bar for easier progress reading.

    std::stringstream ss;
    // create a viewable string
    ss << title << " [";

    // we'll display 20 blocks to denote percent complete. 
    // calculate the current percent completed.
    double percent = (double)current / (double)total;

    int fillblocks = (int)(percent * 40);
    
    for (int i{ 0 }; i < 40; i++) {
        ss << (i < fillblocks ? '#' : '_');
    }
    
    percent *= 100; // get a true percentage.

    ss << "] " << current << '/' << total;

    return ss.str() + (current >= total ? '\n' : '\r');
}

int NetConnect::getUploadCount()
{
    std::vector<std::string> columns{ "count(PK)" };

    DataPair filter;
    filter.push_back({ std::string(), std::string() });
    
    DataPair res = m_DB->Select(columns, "DataSync", filter);

    return res.at(0).second.num;
}
