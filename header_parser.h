#include <iostream>
#include<vector>
#include <string>
#include<map>
#include "json.hpp"

using namespace std;

class request_ {
    public:
        string url, method, version;
        map<string, string> options;
        nlohmann::json body; 
};

request_ parse_header (char *buffer, int size) {
    string request = string(buffer);     
    vector <string> tokens;
    stringstream request_stream(request);
    string intermediate;
    while(getline(request_stream, intermediate, '\n')) {
        tokens.push_back(intermediate);
    }
    request_ req;
    string temp_str;
    vector<string> firstLine;
    stringstream tokenStream(tokens[0]);
    while(getline(tokenStream, temp_str, ' ')) {
        firstLine.push_back(temp_str);
    }
    req.method = firstLine[0];
    req.url = firstLine[1];
    req.version = firstLine[2];
    int bodyIndex = 0;
    for (bodyIndex = 1; bodyIndex < tokens.size() && tokens[bodyIndex][0] != '{'; bodyIndex++) {
        for (int j = 0; j < tokens[bodyIndex].size(); j++) {
            if (tokens[bodyIndex][j] == ':') {
                req.options[tokens[bodyIndex].substr(0, j)] = tokens[bodyIndex].substr(j+1, tokens[bodyIndex].size()-j);
                break;
            }
        }
    }
    // stringstream jsonstring;
    // for (int i=bodyIndex; i < tokens.size(); i++) {
    //     jsonstring << tokens[i];
    // }
    // cout << jsonstring.str() << endl;
    // req.body = nlohmann::json::parse(jsonstring.str());
    return req;
}