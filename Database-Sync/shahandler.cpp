#include "shahandler.h"


ShaHandler::ShaHandler(int shaType) : m_SHAType(shaType) {
    // generic call, not sure if needed. Will set a default sha type, that can be changed later. 
    //m_HashValue.reserve(160); // prepare to store up to 160 characters for a full sizes hash. 
}

void ShaHandler::SetSHAType(int shaType) {
    m_SHAType = shaType;
}

ShaHandler::~ShaHandler() {

    //delete m_HashValue; // no longer a new object. we'll declare the class as new. 

}

std::string ShaHandler::Encode(std::string& val, bool raw) {
    // encode using the given sha type expected. Note that RAW returns raw bytes, otherwise HEX
    switch (m_SHAType) {
    case 1:
        return encodesha1(val, raw);
        break;
    case 256:
        return encodesha256(val, raw);
        break;
    case 512:
    default: // default to highest level if it isn't predefined. 
        return encodesha512(val, raw);
        break;
    }
}

std::string ShaHandler::encodesha1(std::string& val, bool raw) {
    // take the val and encode it. 

    unsigned char* q = new unsigned char[val.size() + 1];

    std::copy(val.begin(), val.end(), q);

    q[val.size()] = '\0';

    const unsigned char* c = q;

    unsigned char md[20] = "0";

    unsigned char* chars = SHA1(c, val.size(), md);

    std::stringstream ss;
    for (int i{ 0 }; i < sizeof(md); ++i) {
        if (raw) { 
            ss << md[i];
        }
        else {
            if ((int)md[i] < 16) {
                ss << '0' << std::hex << (int)md[i];
            }
            else {
                ss << std::hex << (int)md[i];
            }
        }
    }

    delete[] q;
    return ss.str();
}
std::string ShaHandler::encodesha256(std::string& val, bool raw) {
    // take the val and encode it. 

    unsigned char* q = new unsigned char[val.size() + 1];

    std::copy(val.begin(), val.end(), q);

    q[val.size()] = '\0';

    const unsigned char* c = q;

    unsigned char md[32] = "0";

    unsigned char* chars = SHA256(c, val.size(), md);

    std::stringstream ss;
    for (int i{ 0 }; i < sizeof(md); ++i) {
        if (raw) { 
            ss << md[i];
        }
        else {
            if ((int)md[i] < 16) {
                ss << '0' << std::hex << (int)md[i];
            }
            else {
                ss << std::hex << (int)md[i];
            }
        }
    }

    delete[] q;
    return ss.str();
  }
std::string ShaHandler::encodesha512(std::string& val, bool raw) {
    // take the val and encode it. 

    unsigned char* q = new unsigned char[val.size() + 1];

    std::copy(val.begin(), val.end(), q);

    q[val.size()] = '\0';

    const unsigned char* c = q;

    unsigned char md[64] = "0";

    unsigned char* chars = SHA512(c, val.size(), md);

    std::stringstream ss;
    for (int i{ 0 }; i < sizeof(md); ++i) {

        if (raw) { 
            ss << md[i];
        }
        else {

            if ((int)md[i] < 16) {
                ss << '0' << std::hex << (int)md[i];
            }
            else {
                ss << std::hex << (int)md[i];
            }
        }
    }

    delete[] q;


    return ss.str();
  
}

std::string ShaHandler::hashForPost(std::string& dataStrand, std::string& dataPassword, std::string& dataSalt) {
    // take the existing strand, perform the XOR encryption, and return the value. 
    std::stringstream q;

    // pre encrpyt the password, so it is never used plaintext (sha1) 
    std::string baseEncodePassword = encodesha1(dataPassword);

     // add the salt to the password, then add the date. 
         // get clock time
    struct tm newtime;
    std::time_t now = time(0);
    localtime_s(&newtime, &now);


    std::string NowDate = std::to_string(newtime.tm_year + 1900) + '-';
    /*
    if (newtime.tm_mon < 9) {
        NowDate.append('0' + std::to_string(newtime.tm_mon + 1) + '-' + std::to_string(newtime.tm_mday));
    }
    else {
        NowDate.append(std::to_string(newtime.tm_mon + 1) + '-' + std::to_string(newtime.tm_mday));
    }
    */
    NowDate.append((newtime.tm_mon < 9 ? '0' + std::to_string(newtime.tm_mon + 1) : std::to_string(newtime.tm_mon + 1))
        + '-' + (newtime.tm_mday < 10 ? '0' + std::to_string(newtime.tm_mday) : std::to_string(newtime.tm_mday)));



    //return NowDate;
    q << baseEncodePassword << dataSalt << NowDate;

    //std::cout << q.str() << '\n';

     // sha 1 encrpyt the password again. 

    std::string* Qpass = new std::string{ q.str() };

    std::string* encodedPassword = new std::string{ encodesha1(*Qpass) };

    delete Qpass;
    // using the first 10 characters of the sha1 password, generate a sha512 hash. 
    q.str("");

    for (int i{ 0 }; i < 10; ++i) {

        q << encodedPassword->at(i);
        //   q << static_cast<char>(i);

    }

    std::string codex = q.str();

     // we also need to store the previous sha1 due to the next portion eliminating that. 

    std::string* hash512 = new std::string{ encodesha512(codex, false) };

    //std::cout << *hash512 << "\n\n\n";

      // assemble all elements that need to be in the hashed information. this will go into a new stringstream, then be hashed. 
    std::stringstream* sb = new std::stringstream;

    // this takes the hashes password as well as the data strand and concatenates it for sending. 
    // everything will be done via assumed datasets, (sha1 passwords and plaintext data)
    // data can have external serialization techniques used in future development. 
    *sb << *encodedPassword << dataStrand;

    //std::cout << sb->str() << "\n\n\n";

    //sb->seekg(0, ios_base::end);
    int SBSize = (int)sb->str().size();
    //   sb->seekg(0, ios::beg);

    if (SBSize > hash512->size()) {
        // we have too much data, for now, output an error code. 
        //std::cerr << "Warning: Too much data for single 512 hash \n";

        while (hash512->size() < SBSize) {
            // add to the hash512 using the last 10 characters of the string that were just encoded, so we can create
            // a chain effect of the data string to fully cover all information. 

            codex = sb->str().substr(hash512->size() - 10, 10);

            // add the string to the hash for the next steps. 

            //std::cout << *hash512 << '\n';
            //std::cout << "codex: " << codex << '\n';


            hash512->append(encodesha512(codex, false));

        }

        //std::cout << "Raw Hash: " << *hash512 << "\n\n\n\n";

    }

    // use the 512 hash as a XOR cypher for the entire code. If the XOR cypher isn't long enough, 
        // use the unencoded last 10 characters for the next cypher set. 

    q.str("");

    for (int i{ 0 }; i < sb->str().size(); ++i) {
        //   q->append(static_cast<std::string>(dataStrand.at(i)));
        char x = static_cast<char>(sb->str().at(i)) ^ static_cast<char>(hash512->at(i));

        if ((int)x < 16) {
            q << "0" << std::hex << (int)x;
        }
        else {
            q << std::hex << (int)x;
        }
    }

    delete encodedPassword;
    delete sb;
    delete hash512;
    return q.str();
    // SIMPLIFY THIS CODE, IT CAN USE BELOW SCRIPT. -----------------------------------------------------------------------//
}

std::string ShaHandler::XorString(std::string& string, std::string codex) {
    // take the given values, and encode based on those given. 
    
    // create the initial codex value in a string.
    std::string* hash512 = new std::string{ encodesha512(codex, true) };

    std::cout << " XOR HERE: \n";
    std::cout << "Hash512: " << *hash512 << '\n';
    std::cout << "String: " << string << '\n';
    std::cout << "Codex: " << codex << '\n';

    std::stringstream retval; // final value to return

    while (hash512->size() < string.size()) {
        // during each iteration, take the last 10 characters that would be encrypted by the current hash512 and add a new hash512 value to the end. 
        
        std::string nexcodex = string.substr(hash512->size() - 10, hash512->size());
        
        hash512->append(encodesha512(nexcodex, true));
    }

    // using the now fully lengthed string, XOR the value, and return it.

    int stringSize = (int)string.size();

    for (int i{ 0 }; i < stringSize; ++i) {
        // iterate, character by character.

        char x = static_cast<char>(string.at(i)) ^ static_cast<char>(hash512->at(i));
        
        retval << x;
        
    }
    
    // clean up created resources.
    delete hash512;

    std::cout << "final: " << retval.str() << '\n';

    return retval.str();
}

std::string ShaHandler::unXorString(std::string& string, std::string codex) {

    // first create the initial hash, and decode the first section. 
    std::string hash512 = encodesha512(codex, true);

    std::cout << " UNXOR HERE: \n";
    std::cout << "Hash512: " << hash512 << '\n';
    std::cout << "String: " << string << '\n';
    std::cout << "Codex: " << codex << '\n';

    std::stringstream retval; // final value to return

    // go section by section, and update data as we go. 

    int stringLength = (int)hash512.size();
    const int sl = stringLength; // we'll need a const to check the data time over time. 
    int passes = 0;

    while (passes * sl < string.size()) {
        
        // covert the existing data, using the codex developed.
        for (int i{ 0 }; i < sl; ++i) {

            if(passes*sl + i >= (int)string.size()){
                break; // get out of for loop
            }

            // iterate through, and assign each character. q is to ensure that we don't overflow string.

            int d = i + (passes * sl);

            char x = static_cast<char>(string.at(d)) ^ static_cast<char>(hash512.at(i));
            retval << x;
        }
        
        std::cout << "retval: " << retval.str() << '\n';

        std::string nexcodex = string.substr(retval.str().size() - 10, retval.str().size());
        
        hash512 = encodesha512(nexcodex, true);

        // we completed another pass, iterate.
        stringLength += sl;
        ++passes;

    }
    /*
    while (hash512->size() < string.size()) {
        // during each iteration, take the last 10 characters that would be encrypted by the current hash512 and add a new hash512 value to the end. 

        std::string nexcodex = string.substr(hash512->size() - 10, hash512->size());

        hash512->append(encodesha512(nexcodex, true));
    }

    // using the now fully lengthed string, XOR the value, and return it.

    int stringSize = (int)string.size();

    for (int i{ 0 }; i < stringSize; ++i) {
        // iterate, character by character.

        char x = static_cast<char>(string.at(i)) ^ static_cast<char>(hash512->at(i));

        retval << x;

    }

    */

    return retval.str();
}