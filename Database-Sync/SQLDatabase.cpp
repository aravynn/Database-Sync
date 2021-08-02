#include "SQLDatabase.h"

SQLDatabase::SQLDatabase() {
	// load the database connection.

    GetDBLocation();

	int exit = 0;
	exit = sqlite3_open(m_DBLoc.c_str(), &m_DB);

	if (exit) {

		// failed to open Database. Issue with a files permission?
		std::cerr << "Error Opening DB " << sqlite3_errmsg(m_DB) << std::endl;
		return;

	}
	else {

		//std::cout << "Opened Database Successfully! \n";

	}

}

SQLDatabase::~SQLDatabase() {
	// destroy any required connections. 
	sqlite3_close(m_DB);
}


bool SQLDatabase::GetDBLocation()
{
    TCHAR path[MAX_PATH]{ 0 };
    if (::SHGetSpecialFolderPath(NULL, path, CSIDL_MYDOCUMENTS, FALSE)) {
        std::stringstream ss;
        for (char c : path) {
            if (c != '\n' && c != '\r' && c != 0) {
                ss << c;
            }
        }
        m_DBLoc = ss.str() + "\\HoseTracker\\DB\\data.sqlite";
        //std::cout << m_DBLoc << '\n';
    }
    else {
        // get path failed, error handle? 
        return false;
    }

    // get path succeeds.
    return true;
}

DataPair SQLDatabase::Select(std::vector<std::string> columns, std::string table, DataPair filter, int limit, int offset)
{

	if (m_ReturnData.size() > 0) {
		m_ReturnData.resize(0); // clear the data vector.
	}

    std::string REQUEST = "SELECT "; 

    if (columns.size() < 1 || columns.at(0) == "") {
        // no results. 
        REQUEST.append("* ");
    }
    else {
        int count = (int)columns.size();
        for (std::string col : columns) {
            if (--count > 0) {
                REQUEST.append(col + ", ");
            }
            else {
                REQUEST.append(col + " ");
            }
        }
    }

    REQUEST.append("FROM " + table);

    // check for and add any filters.
    if (filter.at(0).first != "") {
        // there is a valid filter value.
        REQUEST.append(" WHERE");
        REQUEST.append(whereString(filter, " AND"));
    }

    // Check if a limit is set, and add it if it.
    if (limit > 0) {
        REQUEST.append(" LIMIT " + std::to_string(limit));
        if (offset > 0) {
            REQUEST.append(" OFFSET " + std::to_string(offset));
        }
    }

    if(m_verbose)
        std::cout << REQUEST << '\n'; // let's see it look OK.
    
    // add the sql statement to be used

    sqlite3_stmt * stmt;
    
    int prep = sqlite3_prepare_v2(m_DB, REQUEST.c_str(), -1, &stmt, 0);

    if (prep != SQLITE_OK) {
        if (m_verbose) {
            std::cerr << "failed to prepare select statement: " << REQUEST << "\n Result: " << prep << '\n';
        }
        return { {(std::string)"",(std::string)""} };
    }
    
    if (!Bind(stmt, filter)) {
        
        if (m_verbose) {
            std::cerr << "ERROR: Failed to bind to statement \n"; 
        }
        return { {(std::string)"",(std::string)""} };
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        // for each row, manage all data. we'll need to have an idea of row count, ideally. 
        for (int i{ 0 }; i < sqlite3_column_count(stmt); ++i) {
            switch (sqlite3_column_type(stmt, i)) {
            case SQLITE_INTEGER:
               // std::cout << sqlite3_column_name(stmt, i) << " -> " << sqlite3_column_int(stmt, i) << '\n';
                m_ReturnData.push_back({ (std::string)sqlite3_column_name(stmt, i),  (IDType)sqlite3_column_int(stmt, i) });
                break;
            case SQLITE_TEXT:
               // std::cout << sqlite3_column_name(stmt, i) << " -> " << sqlite3_column_text(stmt, i) << '\n';
                m_ReturnData.push_back({ (std::string)sqlite3_column_name(stmt, i),  std::string(reinterpret_cast<const char *>(sqlite3_column_text(stmt, i))) });
                break;
            case SQLITE_FLOAT:
               // std::cout << sqlite3_column_name(stmt, i) << " -> " << sqlite3_column_double(stmt, i) << '\n';
                m_ReturnData.push_back({ (std::string)sqlite3_column_name(stmt, i),  (double)sqlite3_column_double(stmt, i) });
                break;
            case SQLITE_NULL:
                //std::cout << "NULL VALUE RETURNED at SQLD 137 \n";
                m_ReturnData.push_back({ (std::string)sqlite3_column_name(stmt, i),  std::string("") });
                break;
            default: 
                //std::cout << "undefined column type:: " << sqlite3_column_type(stmt, i) << ' ' << (std::string)sqlite3_column_name(stmt, i) << ' ' << std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, i))) << '\n';
               
                std::cout << "undefined column type:: " << sqlite3_column_type(stmt, i) << '\n';
                break;
            }
        }
    }
    // complete the function. 
    sqlite3_finalize(stmt);
    
    return m_ReturnData;
}

bool SQLDatabase::Insert(std::string table, DataPair inserts)
{
    // create an INSERT statement for the database.
    IDType id = getNextPK(table); // get the next PK, for checking 0 issue.

    if(id == 0 && inserts.at(0).first != "PK"){
        // this is the first item to add.
        int insertlength = inserts.size();

        inserts.resize(insertlength + 1); // add length to inserts.

        for(int i{insertlength}; i > 0; --i){
            // reset the sizes of inserts and adjust to make way for PK
            inserts.at(i) = inserts.at(i - 1);
        }

        // set the PK value
        inserts.at(0) = {"PK", (IDType)0};
    }

    std::string REQUEST = "INSERT INTO " + table + " (";
    std::string VALUES = ") VALUES (";

    // run though all inserts, add to table. During same time, create secondary values string.
    int inSize = (int)inserts.size();
    for (int i{ 0 }; i < inSize; ++i) {
        if (i + 1 == inSize) {
            // at max value
            REQUEST.append(inserts.at(i).first);
            VALUES.append("?"); // just plain ? nowadays.
        }
        else {
            // normal value.
            REQUEST.append(inserts.at(i).first + ", ");
            VALUES.append("?, ");
        }
    }

    // create the final query string
    REQUEST.append(VALUES + ")");

   // std::cout << REQUEST << '\n';

    sqlite3_stmt* stmt;
    int prep = sqlite3_prepare_v2(m_DB, REQUEST.c_str(), -1, &stmt, 0);

    if (prep != SQLITE_OK) {
        std::cerr << "failed to prepare INSERT statement: " << REQUEST << "\n Result: " << prep << '\n';
        return false;
    }

    if (!Bind(stmt, inserts)) {
        std::cerr << "ERROR: Failed to bind to statement \n";
        return false;
    }

    int ret = sqlite3_step(stmt);

    if (ret != SQLITE_DONE) {
        // something went wrong, output.
        if (m_verbose) {
            std::cout << "ERROR: Insert failed. Code: " << ret << '\n';
            std::cout << sqlite3_errmsg(m_DB) << '\n';
            std::cout << "Inserts Data for SQL: \n";
            for (std::pair<std::string, DataPass> f : inserts) {
                std::cout << f.first << " -> ";
                if (f.second.o == DPO::DOUBLE) { std::cout << f.second.dbl << '\n'; }
                if (f.second.o == DPO::STRING) { std::cout << f.second.str << '\n'; }
                if (f.second.o == DPO::NUMBER) { std::cout << f.second.num << '\n'; }
            }
        }
        return false;
    }

    sqlite3_finalize(stmt);

	return true;
}

bool SQLDatabase::Update(std::string table, DataPair updates, DataPair filter)
{
    std::string REQUEST = "UPDATE " + table + " SET";

    if (updates.at(0).first == "") {
        std::cerr << "ERROR: No provided update information for SQL Update \n";
        return false;
    }
    else {
        // there is data for update.
        REQUEST.append(whereString(updates, ", "));
    }

    if (filter.at(0).first == "") {
        std::cerr << "ERROR: No provided filter information for SQL Update \n";
        return false; // we MUST provide filters for update functions.
    }
    else {
        // apply  the filters.
        REQUEST.append(" WHERE");
        REQUEST.append(whereString(filter, " AND"));
    }

    //std::cout << REQUEST << '\n';

    sqlite3_stmt* stmt;
    int prep = sqlite3_prepare_v2(m_DB, REQUEST.c_str(), -1, &stmt, 0);

    if (prep != SQLITE_OK) {
        std::cerr << "failed to prepare UPDATE statement: " << REQUEST << "\n Result: " << prep << '\n';
        return false;
    }

    if (!Bind(stmt, updates)) {
        if (m_verbose) {
            std::cerr << "ERROR: Failed to bind updates to statement \n";
        }
        return false;
    }

    if (!Bind(stmt, filter, (int)updates.size() + 1)) {
        if (m_verbose) {
            std::cerr << "ERROR: Failed to bind inserts to statement \n";
        }
        return false;
    }

    int ret = sqlite3_step(stmt);

    if (ret != SQLITE_DONE) {
        // something went wrong, output.
        if (m_verbose) {
            std::cout << "ERROR: Update failed. Code: " << ret << '\n';
            std::cout << sqlite3_errmsg(m_DB) << '\n';
            std::cout << "Update Data for SQL: \n";
            for (std::pair<std::string, DataPass> f : updates) {
                std::cout << f.first << " -> ";
                if (f.second.o == DPO::DOUBLE) { std::cout << f.second.dbl << '\n'; }
                if (f.second.o == DPO::STRING) { std::cout << f.second.str << '\n'; }
                if (f.second.o == DPO::NUMBER) { std::cout << f.second.num << '\n'; }
            }
            std::cout << "Filter Data for SQL: \n";
            for (std::pair<std::string, DataPass> f : filter) {
                std::cout << f.first << " -> ";
                if (f.second.o == DPO::DOUBLE) { std::cout << f.second.dbl << '\n'; }
                if (f.second.o == DPO::STRING) { std::cout << f.second.str << '\n'; }
                if (f.second.o == DPO::NUMBER) { std::cout << f.second.num << '\n'; }
            }
        }
        return false;
    }

    sqlite3_finalize(stmt);

    return true;
}

bool SQLDatabase::Remove(std::string table, DataPair filter)
{

    std::string REQUEST = "DELETE FROM " + table + " WHERE";

    // check filter size
    if (filter.size() < 1) {
        
        if (m_verbose) {
            std::cerr << "ERROR: No filters for remove statement. \n";
        }
        return false;
    }

    // load filter onto the string.
    REQUEST.append(whereString(filter, " AND"));

    //std::cout << REQUEST << '\n';


    sqlite3_stmt* stmt;
    int prep = sqlite3_prepare_v2(m_DB, REQUEST.c_str(), -1, &stmt, 0);

    if (prep != SQLITE_OK) {
        if (m_verbose) {
            std::cerr << "failed to prepare DELETE statement: " << REQUEST << "\n Result: " << prep << '\n';
        }
        return false;
    }

    if (!Bind(stmt, filter)) {
        if (m_verbose) {
            std::cerr << "ERROR: Failed to bind to statement \n";
        }
        return false;
    }

    int ret = sqlite3_step(stmt);

    if (ret != SQLITE_DONE) {
        // something went wrong, output.
        if (m_verbose) {
            std::cout << "ERROR: Remove failed. Code: " << ret << '\n';
            std::cout << sqlite3_errmsg(m_DB) << '\n';
            std::cout << "Filter Data for SQL: \n";
            for (std::pair<std::string, DataPass> f : filter) {
                std::cout << f.first << " -> ";
                if (f.second.o == DPO::DOUBLE) { std::cout << f.second.dbl << '\n'; }
                if (f.second.o == DPO::STRING) { std::cout << f.second.str << '\n'; }
                if (f.second.o == DPO::NUMBER) { std::cout << f.second.num << '\n'; }
            }
        }
        return false;
    }

    sqlite3_finalize(stmt);

	return true;
}

bool SQLDatabase::PushPK(int ID, std::string table, int count)
{
    // pusk all PK's above the given number UP by one, to make a row for an incoming insert. 
    // this function will also need to manage every table that is related, as per the CHILD values given in the TableFormat table. 

    //std::cout << 1 << '\n';

    // step 1 - get the appropriate child lines (if any) from table format. 
    DataPair filter;
    filter.push_back({ "TableName", table });
    filter.push_back({ "ColumnType", (IDType)ColumnType::CHILD });
    DataPair childLines = Select({"ColumnID", "ColumnName" }, "TableFormat", filter); // NOTE: these need to get the column name again still. ColumnName is the TableName in reality.

    DataPair childColumns; // will store the column names that also need updating, if applicable.

    //std::cout << 2 << '\n';



    for (size_t i{ 0 }; i < childLines.size(); i+=2) {
        // for every row returned from the childLines, we'll need to go through and manage the data, and determine all of the 
        // child elements that will need to be updated. 

        DataPair filter;
        filter.push_back( { "TableName", childLines.at(i + 1).second.str } );
        filter.push_back( { "ColumnID", childLines.at(i).second.num } );
        filter.push_back({ "ColumnType", (IDType)ColumnType::CHILD });

        filter.at(2).second.notEqual = true;

        DataPair results = Select({ "ColumnName" }, "TableFormat", filter);

        for (std::pair<std::string, DataPass> r : results) {
            childColumns.push_back({ childLines.at(i + 1).second.str, r.second});
            //std::cout << childLines.at(i + 1).second.str << " -> " << r.second.str << '\n';
        }

    }

    //std::cout << 3 << '\n';


    
    // step 2 - get the highest number existing PK for the table
    IDType maxPK = getNextPK(table) - 1; // -1, since we aren't effecting the "new" line.

    //std::cout << 4 << '\n';



    // step 3, go row by row from highest to lowest in DB, updating the PK up by one, on all tables and child tables.
    for (IDType i{ maxPK }; i >= ID; --i) {
        // descend through, and update as we go. Inclusive to ID specified.

        Update(table, { {"PK", (IDType)(i + count)} }, { {"PK", (IDType)(i)} }); // change ID to i to make work.

        if (childColumns.size() > 0) {
            // iterate through the potential child columns and fix those too.- -------------------------------------------!!!

            for (std::pair<std::string, DataPass> c : childColumns) {

                // each child, do the update for the child column value
                Update(c.first, { {c.second.str, (IDType)(i + count)} }, { {c.second.str, (IDType)(i)} }); // change ID to i to make work.
            }

        }

        Update("DataSync", { {"TablePK", (IDType)(i + count)} }, { {"TablePK", (IDType)(i)}, {"TableName", table} }); // change datasync information, for future information may be subject to change.

    }

    //std::cout << 5 << '\n';




    return false;
}

IDType SQLDatabase::getNextPK(std::string table) {
    // get the next PK to be entered into a particular table
    std::string REQUEST = "SELECT count(PK) FROM " + table;

    sqlite3_stmt* stmt;
    int prep = sqlite3_prepare_v2(m_DB, REQUEST.c_str(), -1, &stmt, 0);

    if (prep != SQLITE_OK) {
        if (m_verbose) {
            std::cerr << "failed to prepare pk statement: " << REQUEST << "\n Result: " << prep << '\n';
        }
        return -1;
    }

    int ret = -1;
    
    if(sqlite3_step(stmt) == SQLITE_ROW) {
        // for each row, manage all data. we'll need to have an idea of row count, ideally. 
        ret = sqlite3_column_int(stmt, 0);
                
    }
    // complete the function. 
    sqlite3_finalize(stmt);

    return ret; 
}

std::string SQLDatabase::whereString(std::vector<std::pair<std::string, DataPass>> filter, std::string separator) {
    // take the filter variable, and filter it into a QString object for return..
    if (filter.size() < 1 || filter.at(0).first != "") {
        // there is a filter, we'll iterate through and add them. we'll need to do this a second time for binding.
        std::string REQUEST = ""; // NULL value, so it can be used for multiple functions.
        int count = (int)filter.size();
        for (std::pair<std::string, DataPass>& f : filter) { // DataPass second value is unused here.

            std::string equal = "=";

            if (f.second.notEqual) {
                equal = "!=";
            }

            if (--count > 0) {
                REQUEST.append(" " + f.first + " " + equal + " ?" + separator); // " :" + f.first + separator); // there are more
            }
            else {
                REQUEST.append(" " + f.first + " " + equal + " ?"); // " :" + f.first);
            }
        }
        return REQUEST;
    }
    // if no filters exist, bypass and return a NULL value.
    return "";
}

bool SQLDatabase::Bind(sqlite3_stmt* stmt, DataPair filter, int start) {
    // bind vectors to the chosen statement. 
    if (filter.size() < 1 || filter.at(0).first == "") {
        return true; // nothing to bind. 
    } 

    int q = start; // starts at 1 not 0
    // use existing structure to transfer data. 
    for (std::pair<std::string, DataPass>& f : filter) {
        
        int binder{ 9000 };

        if (f.second.o == DPO::NUMBER) { // bind the NUMBER value.
         //   std::cout << "Binding int For: " << f.first << " Value: " << f.second.num << '\n';
            binder = sqlite3_bind_int(stmt, q, f.second.num);
        }
        else if (f.second.o == DPO::DOUBLE) { // bind the DOUBLE value.
         //   std::cout << "Binding double For: " << f.first << " Value: " << f.second.dbl << '\n';
            binder = sqlite3_bind_double(stmt, q, f.second.dbl);
        }
        else if (f.second.o == DPO::STRING) { // bind the STRING value.
        //    std::cout << "Binding string For: " << f.first << " Value: " << f.second.str << '\n';
            binder = sqlite3_bind_text(stmt, q, f.second.str.c_str(), f.second.str.size(), SQLITE_TRANSIENT);
            
        }

        ++q;

        if (binder != SQLITE_OK) {
            if (m_verbose) {
                std::cout << "Bind result: " << binder << '\n';
            }
            return false;
        }
        else {
          //  std::cout << "Binding " << filter.size() << " results OK Val: " << binder << "\n";
        }
    }

    return true;
}
