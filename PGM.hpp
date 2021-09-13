

#ifndef PGM_HPP
#define PGM_HPP
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <string>
#include <algorithm> 
#include<boost/functional/hash.hpp>
#include <chrono>
using namespace std;


template < typename SEQUENCE > struct seq_hash
{
	std::size_t operator() (const SEQUENCE& seq) const
	{
		std::size_t hash = 0;
		boost::hash_range(hash, seq.begin(), seq.end());
		return hash;
	}
};

template < typename SEQUENCE, typename T >
using unordered_map_sequence = std::unordered_map< SEQUENCE, T, seq_hash<SEQUENCE> >;




//unordered_map<string, unordered_map<unsigned long int, string>> int2strNonJAs; // {composite att name{composite Key as a string,ordinal int}} 
//unordered_map<string, unordered_map<string,unsigned long int>> str2intNonJAs; // {composite att name{composite Key as a string,ordinal int}} 
//      
class clique{
public:
    bool disabled=false;
    vector<string> variableList;  // list of all attributes in this clique
    string conditionedOnVar; // the source of the conditional freq table
    vector<string> otherVars; // other atts in the conditional pot
    string sql;
    vector<vector<string>> rawData;  // from csv files
    unordered_map < int, unordered_map< int, unsigned long long int>> cond_pot_T2;  // x1--> {y1,freq}
    unordered_map<int,unordered_map_sequence <vector<int>,unsigned long long int>> cond_pot_T3; // x1 --> {{y1,y2}, freq}
    unordered_map<int, unsigned long long int> pot_T1; // {x1,freq}

    string pot_type;
    bool potFromDeletion;
    string dbName, user, pass;
    
    void makeConnection(string dbName1, string user1, string pass1); 
    void cliqueQueryDB( string table, vector<string> groupingAtts, vector<vector<string>> equalityAtts, vector<vector<string>> likeAtts);
    
    void cliqueQueryCSV( string tableADD, vector<string> groupingAtts, unordered_map<int, vector<string>> eliminationOrder, unordered_map<int, string> deletionOrder, vector<string> JAs, bool shallIcleanRawData); // read data and calculate all the pots based on the elimination and deletion order
    
    void makePotFromRawPot(string conditionedOn, vector<string> otherAtts); // using struct in values
 
    void clear();
    clique(){
        potFromDeletion=true;
    }
    
};

class gen_clique{
public:
    vector<string> variableList;  // list of all attributes in this clique
    string conditionedOnVar; // the source of the conditional freq table
    vector<string> otherVars; // other atts in the conditional pot
    string type;
    unordered_map <int, vector<pair<int, vector<unsigned long long int>> > >cond_pot_T2;  // x1--> {y1,{bucket, childFac}}
};


class PGM{
public:

    bool shallICleanOldPots; /// to remove or keep the old pots. clearing pots takes time
    unsigned int max_vec_initialization; /// maximim frequency for a specific distinc value in generation
    vector<clique> graph; // includes all the cliques related to a given query
    vector<gen_clique> generativeCliques; // to be used in generation
    vector<string> outputVars;
    unordered_map<int, string> deletionOrder; // for deleting the vars which are not in the output list -- delete one by one
    unordered_map<int, vector<string>> eliminationOrder; // elimination order for output max cliques
    unordered_map< string,int> deletionOrderRev; // reverse map for deleting the vars which are not in the output list -- delete one by one
    unordered_map<string,int> eliminationOrderRev; // reverse map of elimination order for output max cliques
    vector<vector<int>> levelClqs; //is used in generation process. contains clq ids in each level of the tree
    vector<vector<int>> levelParents; // contains the parents of the clqs in each level
    vector<vector<unsigned long long int>> levelFreqs;  // each row contains keys and the freq for all atts in the level
    vector<vector<string>> headers; // for output result
    int levelSize;
    
    
    PGM(int clique_size);
    void quantitative_learning();  // scans the tables the calculates the pots
    vector<int> findInGraph(vector<string> vars); //find the cliques indices that include vars
    vector<int> findInGenGraph(string var);

    void deleteNonOutputVars(); // deletes the non output variables from the graph.
    void eliminateVariables(); // eliminates the vars and prepares the generation tree
    void eliminate(int srcMaxC,int child_factor_id, int extraBucketId);
    int product_T1_T1(vector <int> clqList);
    int sumProductV1(vector <int> clqList, bool summingOut, int singleClqIndx); //(x->y)  * (x->y) ==>  (y) or (x->y)
    int sumProductV2(int clq1, int clq2, int singleClqIndx); // (x->y)  * (x->z) ==> (y->z) if elimination order of y is smaller
    int sumProductV3(int clq1, int clq2, int singleClqIndx); // (x->yw)  * (x->z) ==> (y->wz) if elimination order of y is smaller than elor of z and w
    int sumProductV4(int clq1, int clq2, int singleClqIndx); // (x->yz)  * (x->z) ==> (y->z) if elimination order of y is smaller than elor of z
    int sumProductV5(int clq1, int clq2, int singleClqIndx);  // (x->yw)  * (x->zq) ==> (y->wzq) if elimination order of y is smaller than elor of z,q and w 
    int sumOut(int clq, int singleClqInd);  // gets a single clique and sum out the root
    void sumProductV6(int clq1, int clq2, int singleClqIndx); // (x->yw)  * (x->yz) ==> (y->wz) if elimination order of y is smaller than elor of z and w
    void sumProductV7 (vector<int> clqIndx, bool summingOut, int singleClqIndx);  // (x->yz) * (x->yz) * (x->yz) ==>  (yz) or (x->yz)
    void generateResults(int mode, string out_add);
 void recursive_generation_noFreq(short int level, unsigned long long int parentBucket, vector<int> keys);
    void recursive_generation(short int level, unsigned long long int parentBucket, vector<int> keys); // traverse the graph and generate columnwise
//    void recursive_generation_2(short int level, unsigned long long int parentBucket, vector<int> keys, vector<int> row); // traverse the graph and generate row-wise
    void write2Disk_1(string out_add);  //writes freqs
    void readFromDisk_1(string in_add); // reads freqs
    
    void write2Disk_2(string out_add);  //writes actual data
    void readFromDisk_2(string in_add); // reads actual data
    void generate();
};
int findInVector( vector<string>& vecOfElements,  string& element);
vector<string> vec_substract(vector<string> v1, vector<string> v2);
#endif /* PGM_HPP */

