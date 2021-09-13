#include "csv_reader.hpp"

pair<bool, int > findInVec( vector<string>& vecOfElements,  string& element)
{
    pair<bool, unsigned int > result;
    // Find given element in vector
    auto it = find(vecOfElements.begin(), vecOfElements.end(), element);
    if (it != vecOfElements.end())
    {
        result.second = (unsigned int) distance(vecOfElements.begin(), it);
        result.first = true;
    }
    else
    {
        result.first = false;
        result.second = -1;
    }
    return result;
}

string trim(const string& str)
{
    size_t first = str.find_first_not_of(' ');
    if (string::npos == first)
    {
        return str;
    }
    size_t last = str.find_last_not_of(' ');
    return str.substr(first, (last - first + 1));
}

vector<int> GetHeadersIndexes(const string& ss, char delimiter, vector<string>& headers)
{
    vector<string> headerline;
    string token;
    istringstream tokenStream(ss);
    while (getline(tokenStream, token, delimiter))
    {
        string s=trim(token);
        if (!s.empty() && s[s.size() - 1] == '\r')
            s=s.substr(0,s.size() - 1);
        headerline.push_back(s);

    }

    vector<int> headerindexes;
    
    if(headers[0]=="*")
        headers=headerline;
    
    for (string s : headers)
    {
        
        std::pair<bool, unsigned int> result = findInVec(headerline, s);
        if (result.first)
        {
            headerindexes.push_back(result.second);
        }
    }

    return headerindexes;

}
void loadRawData(string fileAdd,vector<string> &q_headers, vector<vector<string>> &data, char delimiter)
{
    std::ifstream file(fileAdd);
    if (!file.good())
    {
        cout<<"file not found at: "<<fileAdd<<endl;
        exit(1);
    }
    std::string line = "";

    // Iterate through each line and split the content using delimeter
    string tmpS;int tmpI;
    getline(file, line);
    auto headerIndexes = GetHeadersIndexes(line, delimiter, q_headers);
    // sort pair wise header and indices to scan faster
    for (int h = 0; h < headerIndexes.size()-1;h++)
        for (int j = 0; j < headerIndexes.size()-h-1; j++)
            if (headerIndexes[j] > headerIndexes[j+1])
            {
                tmpS=q_headers[j];
                q_headers[j]=q_headers[j+1];
                q_headers[j+1]=tmpS;
                tmpI=headerIndexes[j];
                headerIndexes[j]=headerIndexes[j+1];
                headerIndexes[j+1]=tmpI;
            }

    std::vector<string> row;
    row.resize(q_headers.size());
    vector<unsigned int> casted;
    casted.resize(row.size());
  
    int lastInx=headerIndexes[headerIndexes.size()-1];
    try {
        while (getline(file, line))
        {
            
                int headerCnt = 0;
                int lineCnt=0;

                std::string::const_iterator start = line.begin();
                std::string::const_iterator end = line.end();
                std::string::const_iterator next = std::find(start, end, delimiter);

                while (next != end) {
                    if(lineCnt==headerIndexes[headerCnt])
                        row[headerCnt++] = string(start, next);
                    
                    if(lastInx==lineCnt)
                            break;
                    start = next + 1;
                    next = find(start, end, delimiter);
                    lineCnt++;
                }

                if (headerCnt<=lastInx && lineCnt==headerIndexes[headerCnt])//last attribute
                    row[headerCnt++] = string(start, next);

//                for (int i = 0; i < row.size(); i++)
//                {
//    //                cout<<i<<":  "<<row[i]<<endl;
//                    casted[i] = stoi(row[i]);
//                }
//                data.push_back(casted);
                data.push_back(row);
        }
    }
    catch (const std::exception& e)
    {
        cout<<"some problems with files."<<endl;
        throw;
    }
    file.close();
}


