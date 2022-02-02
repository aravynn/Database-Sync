// Database-Sync.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

/**
 *
 * Sync a database with the intended location. Will require a new table added to the database, which will 
 * record all information needed to send over data. 
 * 
 * 
 * 
 */
#define NOMINMAX

#include <iostream>
#include <limits>


// includes for DLL's
#include "curl.h"
#include "openssl/sha.h"
#include "openssl/evp.h"
#include "openssl/err.h"
#include "sqlite3.h"

// class includes.
#include "PHPConfig.h"      // sql control class
#include "shahandler.h"     // sha wrapper
#include "SQLDatabase.h"    // database control class.
#include "NetConnect.h"     // Netconnect control 
#include "json.h"
#include "DBClean.h"        // database status checker.

int main()
{
    // get user input for what function to call. only runs once before completion
    std::cout << "HoseControl helper functions. \n Please make a selection: \n"
        << "        1 - Database Sync \n"
        << "        2 - Database Health Check \n";

    int x; 
    std::cin >> x;

    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    // call the appropriate function
    if (x == 1) {
        NetConnect d;
    }
    else if (x == 2) {
        DBClean b;
    }

    system("pause");    // prevent window from automatically closing, at end of execution.
    
    return 0;
}


//PHPConfig* config = new PHPConfig();

//std::cout << "File load complete. \n";

//delete config; // resource cleanup.

// SQLDatabase db;


 //db.PushPK(50, "Companies", 50);

 /*
 std::vector<std::string> columns{ "PK", "TableName" };

 DataPair filter;
 filter.resize(1);
 filter.at(0) = { "TableName", (std::string)"Companies" };

 DataPair data = db.Select(columns, "DataSync", filter, 10, -1);
 */
 /*
 for (std::pair<std::string, DataPass> d : data) {
     std::cout << d.first << ' ';

     if (d.second.o == DPO::STRING)
         std::cout << d.second.str << '\n';
     if(d.second.o == DPO::NUMBER)
         std::cout << d.second.num << '\n';
     if(d.second.o == DPO::DOUBLE)
         std::cout << d.second.dbl << '\n';
 }
 */

 //std::cout << db.getNextPK("Companies") << '\n';

 /*
 DataPair inserts;

 inserts.push_back({"CompanyPK", (IDType)23});
 inserts.push_back({ "Name", (std::string)"Wild Wings" });

 if (!db.Insert("Contacts", inserts)) {
     std::cout << "insert Failed \n";
 }
 */
 /*
 DataPair updates;
 updates.push_back({ "Name", std::string("POOP") });

 DataPair filter;
 filter.push_back({ "Name", (std::string)"Wild Wings" });

 db.Remove("Contacts", updates);
 */
/*

   DataPair data;
   data.push_back({ "One",std::string("Test") });
   data.push_back({ "Two",std::string("Tester") });
   data.push_back({ "Three", (IDType)3 });
   data.push_back({ "Four",(IDType)23422121 });
   data.push_back({ "Five",(double)123.321432 });
   data.push_back({ "Six",(double)900.00 });

   data.push_back({ "One",std::string("Test") });
   data.push_back({ "Two",std::string("Tester") });
   data.push_back({ "Three", (IDType)3 });
   data.push_back({ "Four",(IDType)23422121 });
   data.push_back({ "Five",(double)123.321432 });
   data.push_back({ "Six",(double)900.00 });

   data.push_back({ "One",std::string("Test") });
   data.push_back({ "Two",std::string("Tester") });
   data.push_back({ "Three", (IDType)3 });
   data.push_back({ "Four",(IDType)23422121 });
   data.push_back({ "Five",(double)123.321432 });
   data.push_back({ "Six",(double)900.00 });

   std::string encoded = json::encode(data, 6);

   std::cout << encoded << '\n';

   DataPair data2 = json::decode(encoded);

   //std::cout << data2.at(1).first << data2.at(2).first;

       for (std::pair<std::string, DataPass> d : data) {
           std::cout << d.first << ' ';

           if (d.second.o == DPO::STRING)
               std::cout << d.second.str << '\n';
           if(d.second.o == DPO::NUMBER)
               std::cout << d.second.num << '\n';
           if(d.second.o == DPO::DOUBLE)
               std::cout << d.second.dbl << '\n';
       }
     */