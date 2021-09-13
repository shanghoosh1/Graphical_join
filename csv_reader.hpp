
#ifndef csv_reader_hpp
#define csv_reader_hpp

#include <unordered_map>
#include<map>
#include <chrono>
#include <iostream>
#include <fstream>
#include <vector>
#include <iterator>
#include <string>
#include <algorithm>
#include <sstream>
using namespace std;
vector<int> GetHeadersIndexes(const string& ss, char delimiter, vector<string>& headers);
pair<bool, int > findInVec( vector<string>& vecOfElements,  string& element);
string trim(const string& str);
void loadRawData(string fileAdd,vector<string> &q_headers, vector<vector<string>> &data, char delimiter);
#endif /* csv_reader_hpp */

