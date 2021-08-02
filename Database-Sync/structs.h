#pragma once

// stored structs for use in different locations in the application.
#include <vector>
#include <string>
#include <utility>

using IDType = long long int; // long long for maximum data retention.

enum class DPO {
    STRING,
    NUMBER,
    DOUBLE
}; // only 3 options for Datapass, since


struct DataPass {
    std::string str;
    IDType num{ 0 };
    double dbl{ 0 };
    DPO o;
    bool notEqual = false; // manually set after declaration, will force the SQL call to state != as opposed to =
    // quick constructors will reduce calls in main code.
    DataPass(std::string s) : str{ s } { o = DPO::STRING; }
    DataPass(IDType n) : num{ n } { o = DPO::NUMBER; }
    DataPass(double d) : dbl{ d } { o = DPO::DOUBLE; }
    DataPass() {} // default blank constructor.
    DPO GetType() {
        // return the appropriate type.
        return o;
    }
};

enum class ColumnType {
    STRING,
    INTEGER,
    REAL,
    SKIP, // used for skipping explicit adds, in case we need it.
    CHILD,
    IMAGE
};

enum class LoadStatus {
    OK = 0,
    COMPLETE = -1, 
    UNKNOWN_ERROR = -2,
    UPLOAD_ERROR = -3
};

enum class Sync {
    INSERTID,
    UPDATEID,
    DELETEID,
    CHANGEID,
    ERRORID
};



using StrPair = std::vector<std::pair<std::string, std::string>>;
using DataPair = std::vector<std::pair<std::string, DataPass>>;