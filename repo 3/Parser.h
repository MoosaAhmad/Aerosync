#pragma once
//#include"Vector.h"
#include<string>
#include<vector>

class Parser {
public:
    static vector<string>slice(const string& data, char delimiter) {
        vector<string>res;
        if (data.empty())return res;

        int len = data.length();
        int prev = 0;
        for (int i = 0;i < len;++i) {
            if (data[i] == delimiter) {
                res.push_back(data.substr(prev, i - prev));
                prev = i + 1;
            }
        }

        if (data.back() != delimiter)
            res.push_back(data.substr(prev));
        return res;
    }
};