#include <iostream>
#include "PGM.hpp"
#include <chrono>
#include<unistd.h>
#include <sys/resource.h>
#include <math.h>
#include <dirent.h>
using namespace std;
char* getCmdOption(char ** begin, char ** end, const std::string & option)
{
    char ** itr = std::find(begin, end, option);
    if (itr != end && ++itr != end)
    {
        return *itr;
    }
    return 0;
}
bool cmdOptionExists(char** begin, char** end, const std::string& option)
{
    return std::find(begin, end, option) != end;
}
void runJobQuery(string query, string database, string outAdd,int generationMode, unsigned long long int max_vec_initialization, bool shallICleanOldPots, bool shallIcleanRawData);

int main(int argc, char* argv[]) {
  
    
    string outAdd="/media/shani/2TB/output/";
    string fileAdd="/media/shani/2TB/data/lastFM/dupe_int/";
//    string fileAdd="/media/shani/2TB/data/JOB/SF1/numeric/withHeaders/";
    if(cmdOptionExists(argv, argv+argc, "--input"))
        fileAdd = getCmdOption(argv, argv + argc, "--input");
    if(cmdOptionExists(argv, argv+argc, "--output"))
    {
        outAdd = getCmdOption(argv, argv + argc, "--output");
    
        DIR* dir = opendir(getCmdOption(argv, argv + argc, "--output"));
        if (!dir) 
        {
            cout<<"Output Directory does not exist"<<endl;
            exit(1);
        }
    }
    char * que="lastFM_A1";
    if(cmdOptionExists(argv, argv+argc, "--query"))
        que = getCmdOption(argv, argv + argc, "--query");
    
    //mode=0: cal freqs then generate flat res; 
    //mode=1: store freq in disk and read & make flat res.
    // mode=2 make the flat res next store it then load it.
    // mode=3: runs both mode 1 and 2.
    // mode=4: generate the flat res without freq tables and delete the result fo low mem cases
//mode 5, generate the full in mem then generate the freq/store/full
    int generationMode=5;
    if(cmdOptionExists(argv, argv+argc, "--gen_mode"))
        generationMode = std::stoi(getCmdOption(argv, argv + argc, "--gen_mode"));
    
     unsigned long long int max_vec_initialization=200000000;
  
    if(cmdOptionExists(argv, argv+argc, "--max_init"))
        max_vec_initialization = stol(getCmdOption(argv, argv + argc, "--max_init"));
     
     bool shallICleanOldPots=false;
     bool shallIcleanRawData=false;
    //    runJobQuery("16b", "job20", outAdd); 
//    runJobQuery("16b", "job_sf_1", outAdd);
//    runJobQuery("q3","/media/shani/2TB/data/JOB/SF1/numeric/withHeaders/",outAdd);
//    runJobQuery("imdb33","/media/shani/2TB/data/IMDB/SF1/withHeader/",outAdd);
    runJobQuery(que,fileAdd,outAdd,generationMode, max_vec_initialization,shallICleanOldPots,shallIcleanRawData);
//    runJobQuery("tpcds2","/media/shani/2TB/data/TPCDS/10G/int_csv/",outAdd);
    
//    runJobQuery("q15","/media/shani/2TB/data/JOB/SF1/numeric/withHeaders/",outAdd);
//    runJobQuery("q16","/media/shani/2TB/data/JOB/mainData/allTests/20Percent/",outAdd);
    
    return 0;
}

void runJobQuery(string query, string database, string outAdd,int generationMode, unsigned long long int max_vec_initialization, bool shallICleanOldPots, bool shallIcleanRawData ){

    double consumed_time=0;
    PGM pgm(0);
    pgm.max_vec_initialization=max_vec_initialization;  // based on the mem size
    pgm.shallICleanOldPots=shallICleanOldPots;  // false makes the code faster
    
    cout<<"*******************    Querying the database, started.    **************** \n"<<endl;
    const clock_t t1 = clock();
    auto start = std::chrono::system_clock::now(); 
    
    if(query=="q16"){
        pgm.graph.resize(8);
        pgm.deletionOrder={{0,"company_id"},{2,"person_id"}, {1,"keyword_id"},{3,{"movie_id"}}};
        pgm.eliminationOrder={{0,{"title"}},{1,{"name"}}}; 
        pgm.outputVars={"name", "title"};
      
        
        clique k; // table keyword
        pgm.graph[0]=k;
        pgm.graph[0].cliqueQueryCSV(database+"keyword.csv", {"keyword_id"}, pgm.eliminationOrder, pgm.deletionOrder,{"keyword_id"},shallIcleanRawData );
        clique mk; // table movie keyword
        pgm.graph[5]=mk;
        pgm.graph[5].cliqueQueryCSV(database+"movie_keyword.csv", {"movie_id","keyword_id"}, pgm.eliminationOrder, pgm.deletionOrder,{"movie_id","keyword_id"} ,shallIcleanRawData);
        
        clique ci;
        pgm.graph[1]=ci;
        pgm.graph[1].cliqueQueryCSV(database+"cast_info.csv", {"movie_id","person_id"},pgm.eliminationOrder, pgm.deletionOrder,{"movie_id","person_id"},shallIcleanRawData);
        
        clique an; // table aka_name
        pgm.graph[2]=an;
        pgm.graph[2].cliqueQueryCSV(database+"aka_name.csv", {"name", "person_id"}, pgm.eliminationOrder, pgm.deletionOrder,{"person_id"},shallIcleanRawData);
        
        clique cn; // table company name
        pgm.graph[3]=cn;
        pgm.graph[3].cliqueQueryCSV(database+"company_name.csv", {"company_id"}, pgm.eliminationOrder, pgm.deletionOrder,{"company_id"},shallIcleanRawData);
        
        clique mc; // table movie_company
        pgm.graph[4]=mc;
        pgm.graph[4].cliqueQueryCSV(database+"movie_companies.csv", {"movie_id","company_id"}, pgm.eliminationOrder, pgm.deletionOrder,{"movie_id","company_id"} ,shallIcleanRawData);
        
       
        clique n; // table name
        pgm.graph[6]=n;
        pgm.graph[6].cliqueQueryCSV(database+"name.csv", {"person_id"}, pgm.eliminationOrder, pgm.deletionOrder,{"person_id"} ,shallIcleanRawData);
        
        clique t; // table title
        pgm.graph[7]=t;
        pgm.graph[7].cliqueQueryCSV(database+"title.csv", {"movie_id", "title"}, pgm.eliminationOrder, pgm.deletionOrder,{"movie_id"},shallIcleanRawData );
        
    }
//    else if (query== "imdb3"){
//        pgm.graph.resize(4);
//        pgm.deletionOrder={};
//        pgm.eliminationOrder={{0,{"primname"}},{1,{"average"}},{2,{"ttype"}},{4,{"nid"}},{3,{"tid"}}}; 
//        pgm.outputVars={"primname", "average", "ttype","nid","tid"};
//      
//        clique p; 
//        pgm.graph[0]=p;
//        pgm.graph[0].cliqueQueryCSV(database+"principals.csv", {"nid","tid"}, pgm.eliminationOrder, pgm.deletionOrder,{"nid","tid"} );
//        clique n; 
//        pgm.graph[1]=n;
//        pgm.graph[1].cliqueQueryCSV(database+"name.csv", {"nid","primname"}, pgm.eliminationOrder, pgm.deletionOrder,{} );
//        clique t; 
//        pgm.graph[2]=t;
//        pgm.graph[2].cliqueQueryCSV(database+"title.csv", {"tid","ttype"}, pgm.eliminationOrder, pgm.deletionOrder,{} );
//        
//        clique r; 
//        pgm.graph[3]=r;
//        pgm.graph[3].cliqueQueryCSV(database+"ratings.csv", {"tid","average"}, pgm.eliminationOrder, pgm.deletionOrder,{} );
//
//    }
//     else if (query== "imdb33"){
//        pgm.graph.resize(4);
//        pgm.deletionOrder={{0,"ordr|category|job|characters"}};
//        pgm.eliminationOrder={{0,{"primname|birthyear|deathyea"}},{1,{"average|numvotes"}},{2,{"ttype|primtitle|origtitle|adult|syear|eyear|runtime"}},{3,{"tid"}},{4,{"nid"}}}; 
//        pgm.outputVars={"primname|birthyear|deathyea", "average|numvotes", "ttype|primtitle|origtitle|adult|syear|eyear|runtime","nid","tid"};
//      
//        clique n; 
//        pgm.graph[0]=n;
//        pgm.graph[0].cliqueQueryCSV(database+"name.csv", {"*"}, pgm.eliminationOrder, pgm.deletionOrder,{"nid"},shallIcleanRawData );
//        clique t; 
//        pgm.graph[2]=t;
//        pgm.graph[2].cliqueQueryCSV(database+"title.csv", {"*"}, pgm.eliminationOrder, pgm.deletionOrder,{"tid"},shallIcleanRawData );
//        
//        clique p; 
//        pgm.graph[1]=p;
//        pgm.graph[1].cliqueQueryCSV(database+"principals.csv", {"*"}, pgm.eliminationOrder, pgm.deletionOrder,{"nid","tid"}, shallIcleanRawData );
//        
//        clique r; 
//        pgm.graph[3]=r;
//        pgm.graph[3].cliqueQueryCSV(database+"ratings.csv", {"*"}, pgm.eliminationOrder, pgm.deletionOrder,{"tid"},shallIcleanRawData );
//
//    }
    else if(query=="tpcds2"){
        pgm.graph.resize(4);
        pgm.deletionOrder={{2,"item_sk"} ,{0,"ticket_number"},{1,"reason_sk"} };
        pgm.eliminationOrder={{0,{"i_brand"}},{1,{"reason_desc"}} }; 
        pgm.outputVars={"i_brand" ,"reason_desc"};
       
        
        clique k; // table keyword
        pgm.graph[0]=k;
        pgm.graph[0].cliqueQueryCSV(database+"item.csv", {"item_sk","i_brand"}, pgm.eliminationOrder, pgm.deletionOrder,{"item_sk"},shallIcleanRawData );

        clique cn; // table company name
        pgm.graph[1]=cn;
        pgm.graph[1].cliqueQueryCSV(database+"reason.csv", {"reason_sk","reason_desc"}, pgm.eliminationOrder, pgm.deletionOrder,{"reason_sk"},shallIcleanRawData);
        
        clique mc; // table movie_company
        pgm.graph[2]=mc;
        pgm.graph[2].cliqueQueryCSV(database+"store_sales.csv", {"item_sk","ticket_number"}, pgm.eliminationOrder, pgm.deletionOrder,{"item_sk","ticket_number"},shallIcleanRawData );
        
        clique m1c; // table movie_company
        pgm.graph[3]=m1c;
        pgm.graph[3].cliqueQueryCSV(database+"store_returns.csv", {"item_sk","reason_sk","ticket_number"}, pgm.eliminationOrder, pgm.deletionOrder,{"item_sk","reason_sk","ticket_number"},shallIcleanRawData );
        

    }
    else if(query=="tpcds1"){
        pgm.graph.resize(4);
        pgm.deletionOrder={{0,"order_number"} ,{1,"ship_mode_sk"} };
        pgm.eliminationOrder={{2,{"i_current_price"}},{0,{"sm_carrier"}},{1,{"item_sk"}}  }; 
        pgm.outputVars={"i_current_price" ,"item_sk", "sm_carrier"};
       
        
        clique k; // table keyword
        pgm.graph[0]=k;
        pgm.graph[0].cliqueQueryCSV(database+"item.csv", {"item_sk","i_current_price"}, pgm.eliminationOrder, pgm.deletionOrder,{"item_sk"},shallIcleanRawData );

        clique cn; // table company name
        pgm.graph[1]=cn;
        pgm.graph[1].cliqueQueryCSV(database+"ship_mode.csv", {"ship_mode_sk","sm_carrier"}, pgm.eliminationOrder, pgm.deletionOrder,{"ship_mode_sk"},shallIcleanRawData);
        
        clique mc; // table movie_company
        pgm.graph[2]=mc;
        pgm.graph[2].cliqueQueryCSV(database+"web_sales.csv", {"item_sk","order_number","ship_mode_sk"}, pgm.eliminationOrder, pgm.deletionOrder,{"item_sk","order_number","ship_mode_sk"},shallIcleanRawData );
        
        clique m1c; // table movie_company
        pgm.graph[3]=m1c;
        pgm.graph[3].cliqueQueryCSV(database+"web_returns.csv", {"item_sk","order_number",}, pgm.eliminationOrder, pgm.deletionOrder,{"item_sk","order_number"},shallIcleanRawData );
        

    }
    else if(query=="lastFM_A1"){
        pgm.graph.resize(3);
        pgm.deletionOrder={};
        pgm.eliminationOrder={{1,{"userID"}},{2,{"userID1"}} ,{0,{"weight"}} ,{3,{"weight1"}}  }; 
        pgm.outputVars={"userID" ,"userID1","weight","weight1"};
       
        
        clique k; // table keyword
        pgm.graph[0]=k;
        pgm.graph[0].cliqueQueryCSV(database+"ua1.csv", {"userID1","weight1"}, pgm.eliminationOrder, pgm.deletionOrder,{"userID1"},shallIcleanRawData );

        clique cn; // table company name
        pgm.graph[1]=cn;
        pgm.graph[1].cliqueQueryCSV(database+"ua.csv", {"userID","weight"}, pgm.eliminationOrder, pgm.deletionOrder,{"userID"},shallIcleanRawData);
        
        clique mc; // table movie_company
        pgm.graph[2]=mc;
        pgm.graph[2].cliqueQueryCSV(database+"uf.csv", {"userID","userID1"}, pgm.eliminationOrder, pgm.deletionOrder,{"userID","userID1"},shallIcleanRawData );
        

    }
    else if(query=="lastFM_A2"){
        pgm.graph.resize(4);
        pgm.deletionOrder={{0,"userID1"}};
        pgm.eliminationOrder={{2,{"userID"}},{3,{"userID2"}} ,{0,{"weight"}},{1,{"weight2"}}   }; 
        pgm.outputVars={"userID" ,"userID2","weight","weight2"};
       
        
        clique k; // table keyword
        pgm.graph[0]=k;
        pgm.graph[0].cliqueQueryCSV(database+"ua2.csv", {"userID2","weight2"}, pgm.eliminationOrder, pgm.deletionOrder,{"userID2"},shallIcleanRawData );

        clique cn; // table company name
        pgm.graph[1]=cn;
        pgm.graph[1].cliqueQueryCSV(database+"ua.csv", {"userID","weight"}, pgm.eliminationOrder, pgm.deletionOrder,{"userID"},shallIcleanRawData);
        
        clique mc; // table movie_company
        pgm.graph[2]=mc;
        pgm.graph[2].cliqueQueryCSV(database+"uf.csv", {"userID","userID1"}, pgm.eliminationOrder, pgm.deletionOrder,{"userID","userID1"},shallIcleanRawData );

        clique uf1; // table movie_company
        pgm.graph[3]=uf1;
        pgm.graph[3].cliqueQueryCSV(database+"uf1.csv", {"userID1","userID2"}, pgm.eliminationOrder, pgm.deletionOrder,{"userID1","userID2"},shallIcleanRawData );
       
    }
    else if(query=="L111"){
        pgm.graph.resize(5);
        pgm.deletionOrder={{0,"userID1"},{1,"userID2"}};
        pgm.eliminationOrder={{2,{"userID"}},{3,{"userID3"}} ,{0,{"weight"}},{1,{"weight3"}}   }; 
        pgm.outputVars={"userID" ,"userID3","weight","weight3"};
       
        
        clique k; // table keyword
        pgm.graph[0]=k;
        pgm.graph[0].cliqueQueryCSV(database+"ua3.csv", {"userID3","weight3"}, pgm.eliminationOrder, pgm.deletionOrder,{"userID3"},shallIcleanRawData );

        clique cn; // table company name
        pgm.graph[1]=cn;
        pgm.graph[1].cliqueQueryCSV(database+"ua.csv", {"userID","weight"}, pgm.eliminationOrder, pgm.deletionOrder,{"userID"},shallIcleanRawData);
        
        clique mc; // table movie_company
        pgm.graph[2]=mc;
        pgm.graph[2].cliqueQueryCSV(database+"uf.csv", {"userID","userID1"}, pgm.eliminationOrder, pgm.deletionOrder,{"userID","userID1"},shallIcleanRawData );

        clique uf1; // table movie_company
        pgm.graph[3]=uf1;
        pgm.graph[3].cliqueQueryCSV(database+"uf1.csv", {"userID1","userID2"}, pgm.eliminationOrder, pgm.deletionOrder,{"userID1","userID2"},shallIcleanRawData );
       
        clique uf2; // table movie_company
        pgm.graph[4]=uf2;
        pgm.graph[4].cliqueQueryCSV(database+"uf2.csv", {"userID2","userID3"}, pgm.eliminationOrder, pgm.deletionOrder,{"userID2","userID3"},shallIcleanRawData );
    }
    else if(query=="L2"){
        pgm.graph.resize(5);
        pgm.deletionOrder={};
        pgm.eliminationOrder={{4,{"userID"}},{2,{"userID1"}} ,{5,{"weight"}},{1,{"weight1"}} ,{3,{"year"}},{0,{"year1"}}  }; 
        pgm.outputVars={"userID" ,"userID1","weight","weight1", "year", "year1"};
       
        
        clique k; // table keyword
        pgm.graph[0]=k;
        pgm.graph[0].cliqueQueryCSV(database+"ua1.csv", {"userID1","weight1"}, pgm.eliminationOrder, pgm.deletionOrder,{"userID1"},shallIcleanRawData );

        clique cn; // table company name
        pgm.graph[1]=cn;
        pgm.graph[1].cliqueQueryCSV(database+"ua.csv", {"userID","weight"}, pgm.eliminationOrder, pgm.deletionOrder,{"userID"},shallIcleanRawData);
        
        clique mc; // table movie_company
        pgm.graph[2]=mc;
        pgm.graph[2].cliqueQueryCSV(database+"uf.csv", {"userID","userID1"}, pgm.eliminationOrder, pgm.deletionOrder,{"userID","userID1"},shallIcleanRawData );

        clique ut; // table movie_company
        pgm.graph[3]=ut;
        pgm.graph[3].cliqueQueryCSV(database+"ut.csv", {"userID","year"}, pgm.eliminationOrder, pgm.deletionOrder,{"userID"},shallIcleanRawData );
       
        clique ut1; // table movie_company
        pgm.graph[4]=ut1;
        pgm.graph[4].cliqueQueryCSV(database+"ut1.csv", {"userID1","year1"}, pgm.eliminationOrder, pgm.deletionOrder,{"userID1"},shallIcleanRawData );
    }
    else if(query=="L22"){
        pgm.graph.resize(3);
        pgm.deletionOrder={};
        pgm.eliminationOrder={{3,{"userID"}},{2,{"userID1"}},{1,{"artistID1"}},{0,{"artistID"}}  }; 
        pgm.outputVars={"userID" ,"userID1","artistID","artistID1"};
       
        
        clique mc; // table movie_company
        pgm.graph[0]=mc;
        pgm.graph[0].cliqueQueryCSV(database+"uf.csv", {"userID","userID1"}, pgm.eliminationOrder, pgm.deletionOrder,{"userID","userID1"},shallIcleanRawData );

        clique ut; // table movie_company
        pgm.graph[1]=ut;
        pgm.graph[1].cliqueQueryCSV(database+"ut.csv", {"userID","artistID"}, pgm.eliminationOrder, pgm.deletionOrder,{"userID"},shallIcleanRawData );
       
        clique ut1; // table movie_company
        pgm.graph[2]=ut1;
        pgm.graph[2].cliqueQueryCSV(database+"ut1.csv", {"userID1","artistID1"}, pgm.eliminationOrder, pgm.deletionOrder,{"userID1"},shallIcleanRawData );
    }
    else if(query=="L2222"){
        pgm.graph.resize(5);
        pgm.deletionOrder={{0,"userID1"},{1,"userID2"}};
        pgm.eliminationOrder={{3,{"userID"}},{2,{"userID3"}},{1,{"artistID3"}},{0,{"artistID"}}  }; 
        pgm.outputVars={"userID" ,"userID3","artistID","artistID3"};
       
        clique mc; // table movie_company
        pgm.graph[0]=mc;
        pgm.graph[0].cliqueQueryCSV(database+"uf.csv", {"userID","userID1"}, pgm.eliminationOrder, pgm.deletionOrder,{"userID","userID1"},shallIcleanRawData );
        clique uf1; // table movie_company
        pgm.graph[3]=uf1;
        pgm.graph[3].cliqueQueryCSV(database+"uf1.csv", {"userID1","userID2"}, pgm.eliminationOrder, pgm.deletionOrder,{"userID1","userID2"},shallIcleanRawData );
       clique uf2; // table movie_company
        pgm.graph[4]=uf2;
        pgm.graph[4].cliqueQueryCSV(database+"uf2.csv", {"userID2","userID3"}, pgm.eliminationOrder, pgm.deletionOrder,{"userID3","userID2"},shallIcleanRawData );
        clique ut; // table movie_company
        pgm.graph[1]=ut;
        pgm.graph[1].cliqueQueryCSV(database+"ut.csv", {"userID","artistID"}, pgm.eliminationOrder, pgm.deletionOrder,{"userID"},shallIcleanRawData );
       
        clique ut1; // table movie_company
        pgm.graph[2]=ut1;
        pgm.graph[2].cliqueQueryCSV(database+"ut3.csv", {"userID3","artistID3"}, pgm.eliminationOrder, pgm.deletionOrder,{"userID3"},shallIcleanRawData );
    }
    
    else if(query=="lastFM_B"){
        pgm.graph.resize(4);
        pgm.deletionOrder={{0,"userID1"}};
        pgm.eliminationOrder={{3,{"userID"}},{2,{"userID2"}},{1,{"artistID2"}},{0,{"artistID"}}  }; 
        pgm.outputVars={"userID" ,"userID2","artistID","artistID2"};
       
        clique mc; // table movie_company
        pgm.graph[0]=mc;
        pgm.graph[0].cliqueQueryCSV(database+"uf.csv", {"userID","userID1"}, pgm.eliminationOrder, pgm.deletionOrder,{"userID","userID1"},shallIcleanRawData );
        clique uf1; // table movie_company
        pgm.graph[3]=uf1;
        pgm.graph[3].cliqueQueryCSV(database+"uf1.csv", {"userID1","userID2"}, pgm.eliminationOrder, pgm.deletionOrder,{"userID1","userID2"},shallIcleanRawData );
       
        clique ut; // table movie_company
        pgm.graph[1]=ut;
        pgm.graph[1].cliqueQueryCSV(database+"ut.csv", {"userID","artistID"}, pgm.eliminationOrder, pgm.deletionOrder,{"userID"},shallIcleanRawData );
       
        clique ut1; // table movie_company
        pgm.graph[2]=ut1;
        pgm.graph[2].cliqueQueryCSV(database+"ut2.csv", {"userID2","artistID2"}, pgm.eliminationOrder, pgm.deletionOrder,{"userID2"},shallIcleanRawData );
    }
    else if(query=="q2"){
        pgm.graph.resize(5);
        pgm.deletionOrder={{2,"movie_id"},{0,"keyword_id"},{1,"company_id"}};
        pgm.eliminationOrder={{0,{"title"}} }; 
        pgm.outputVars={ "title"};
       
        
        clique k; // table keyword
        pgm.graph[0]=k;
        pgm.graph[0].cliqueQueryCSV(database+"keyword.csv", {"keyword_id"}, pgm.eliminationOrder, pgm.deletionOrder,{"keyword_id"},shallIcleanRawData );

        clique cn; // table company name
        pgm.graph[1]=cn;
        pgm.graph[1].cliqueQueryCSV(database+"company_name.csv", {"company_id"}, pgm.eliminationOrder, pgm.deletionOrder,{"company_id"},shallIcleanRawData);
        
        clique mc; // table movie_company
        pgm.graph[2]=mc;
        pgm.graph[2].cliqueQueryCSV(database+"movie_companies.csv", {"movie_id","company_id"}, pgm.eliminationOrder, pgm.deletionOrder,{"movie_id","company_id"},shallIcleanRawData );
        
        clique mk; // table movie keyword
        pgm.graph[3]=mk;
        pgm.graph[3].cliqueQueryCSV(database+"movie_keyword.csv", {"movie_id","keyword_id"}, pgm.eliminationOrder, pgm.deletionOrder,{"movie_id","keyword_id"},shallIcleanRawData);
        
        clique t; // table title
        pgm.graph[4]=t;
        pgm.graph[4].cliqueQueryCSV(database+"title.csv", {"movie_id", "title"}, pgm.eliminationOrder, pgm.deletionOrder,{"movie_id"},shallIcleanRawData );
        
    }
    else if(query=="q6"){
        pgm.graph.resize(5);
        pgm.deletionOrder={{1,"person_id"}, {0,"keyword_id"}};
        pgm.eliminationOrder={{0,{"name"}},{1,{"keyword"}},{2,{"title"}},{3,{"movie_id"}}}; 
        pgm.outputVars={ "title", "name", "keyword"};
        
        
        clique k; // table keyword
        pgm.graph[0]=k;
        pgm.graph[0].cliqueQueryCSV(database+"keyword.csv", {"keyword_id", "keyword"}, pgm.eliminationOrder, pgm.deletionOrder,{"keyword_id"},shallIcleanRawData );

        clique ci;
        pgm.graph[1]=ci;
        pgm.graph[1].cliqueQueryCSV(database+"cast_info.csv", {"movie_id","person_id"},pgm.eliminationOrder, pgm.deletionOrder,{"movie_id","person_id"},shallIcleanRawData);
        
        clique mc; // table movie_company
        pgm.graph[2]=mc;
        pgm.graph[2].cliqueQueryCSV(database+"name.csv", {"name","person_id"}, pgm.eliminationOrder, pgm.deletionOrder,{"person_id"},shallIcleanRawData );
        
        clique mk; // table movie keyword
        pgm.graph[3]=mk;
        pgm.graph[3].cliqueQueryCSV(database+"movie_keyword.csv", {"movie_id","keyword_id"}, pgm.eliminationOrder, pgm.deletionOrder,{"movie_id","keyword_id"},shallIcleanRawData );
        
        clique t; // table title
        pgm.graph[4]=t;
        pgm.graph[4].cliqueQueryCSV(database+"title.csv", {"movie_id", "title"}, pgm.eliminationOrder, pgm.deletionOrder,{"movie_id"},shallIcleanRawData );
        
    }
    else if(query=="q11"){
        pgm.graph.resize(8);
        pgm.deletionOrder={{0,"keyword_id"},{3,"company_id"},{1,"link_id"},{2,"company_type_id"}};
        pgm.eliminationOrder={{3,{"title"}}, {1,{"name"}}, {0,{"link"}}, {2,{"movie_id"}}}; 
        pgm.outputVars={ "title","name", "link"};
       
        
        clique ct; 
        pgm.graph[0]=ct;
        pgm.graph[0].cliqueQueryCSV(database+"company_type.csv", {"company_type_id"}, pgm.eliminationOrder, pgm.deletionOrder,{"company_type_id"},shallIcleanRawData );

        clique k; 
        pgm.graph[1]=k;
        pgm.graph[1].cliqueQueryCSV(database+"keyword.csv", {"keyword_id"}, pgm.eliminationOrder, pgm.deletionOrder,{"keyword_id"},shallIcleanRawData );
        
        clique cn; 
        pgm.graph[2]=cn;
        pgm.graph[2].cliqueQueryCSV(database+"company_name.csv", {"company_id", "name"}, pgm.eliminationOrder, pgm.deletionOrder,{"company_id"},shallIcleanRawData);
        
        clique mc; 
        pgm.graph[3]=mc;
        pgm.graph[3].cliqueQueryCSV(database+"movie_companies.csv", {"movie_id","company_id", "company_type_id"}, pgm.eliminationOrder, pgm.deletionOrder,{"movie_id","company_id", "company_type_id"} ,shallIcleanRawData);
        
        clique mk; 
        pgm.graph[4]=mk;
        pgm.graph[4].cliqueQueryCSV(database+"movie_keyword.csv", {"movie_id","keyword_id"}, pgm.eliminationOrder, pgm.deletionOrder,{"movie_id","keyword_id"} ,shallIcleanRawData);
        
        clique t; 
        pgm.graph[5]=t;
        pgm.graph[5].cliqueQueryCSV(database+"title.csv", {"movie_id", "title"}, pgm.eliminationOrder, pgm.deletionOrder,{"movie_id"},shallIcleanRawData );
        
        clique lt; 
        pgm.graph[6]=lt;
        pgm.graph[6].cliqueQueryCSV(database+"link_type.csv", {"link_id", "link"}, pgm.eliminationOrder, pgm.deletionOrder,{"link_id"},shallIcleanRawData );
        
        clique ml; 
        pgm.graph[7]=ml;
        pgm.graph[7].cliqueQueryCSV(database+"movie_link.csv", {"movie_id","link_id"}, pgm.eliminationOrder, pgm.deletionOrder,{"movie_id"},shallIcleanRawData );
    }
    else if(query=="q12"){
        pgm.graph.resize(8);
        pgm.deletionOrder={{0,"info_type_id1"},{1,"info_type_id"},{2,"company_id"},{3,"company_type_id"}};
        pgm.eliminationOrder={{3,{"title"}}, {1,{"name"}}, {0,{"info1"}}, {2,{"movie_id"}}}; 
        pgm.outputVars={ "title","name", "info1"};
       
        
        clique ct; 
        pgm.graph[0]=ct;
        pgm.graph[0].cliqueQueryCSV(database+"company_type.csv", {"company_type_id"}, pgm.eliminationOrder, pgm.deletionOrder,{"company_type_id"},shallIcleanRawData );

         clique it; 
        pgm.graph[2]=it;
        pgm.graph[2].cliqueQueryCSV(database+"info_type.csv", {"info_type_id"}, pgm.eliminationOrder, pgm.deletionOrder,{"info_type_id"} ,shallIcleanRawData);

        clique k; 
        pgm.graph[1]=k;
        pgm.graph[1].cliqueQueryCSV(database+"info_type1.csv", {"info_type_id1"}, pgm.eliminationOrder, pgm.deletionOrder,{"info_type_id1"} ,shallIcleanRawData);
        
        clique mc; 
        pgm.graph[3]=mc;
        pgm.graph[3].cliqueQueryCSV(database+"movie_companies.csv", {"movie_id","company_id", "company_type_id"}, pgm.eliminationOrder, pgm.deletionOrder,{"movie_id","company_id", "company_type_id"} ,shallIcleanRawData);
        
        clique mi; 
        pgm.graph[4]=mi;
        pgm.graph[4].cliqueQueryCSV(database+"movie_info_idx.csv", {"movie_id","info_type_id1", "info1"}, pgm.eliminationOrder, pgm.deletionOrder,{"movie_id","info_type_id1"},shallIcleanRawData );
        
        clique t; 
        pgm.graph[5]=t;
        pgm.graph[5].cliqueQueryCSV(database+"title.csv", {"movie_id", "title"}, pgm.eliminationOrder, pgm.deletionOrder,{"movie_id"},shallIcleanRawData );
        
        clique cn; 
        pgm.graph[6]=cn;
        pgm.graph[6].cliqueQueryCSV(database+"company_name.csv", {"company_id", "name"}, pgm.eliminationOrder, pgm.deletionOrder,{"company_id"},shallIcleanRawData);
        
        clique mii; 
        pgm.graph[7]=mii;
        pgm.graph[7].cliqueQueryCSV(database+"movie_info.csv", {"movie_id", "info_type_id"}, pgm.eliminationOrder, pgm.deletionOrder,{"movie_id", "info_type_id"},shallIcleanRawData);
        
        
    }
    
    else if(query=="JOB_D"){
        pgm.graph.resize(9);
        pgm.deletionOrder={{2,"info_type_id1"},{1,"info_type_id"},{0,"keyword_id"},{3,"person_id"}};
        pgm.eliminationOrder={{4,{"title"}}, {1,{"name"}}, {0,{"info1"}}, {3,{"movie_id"}}, {2,{"info"}}}; 
        pgm.outputVars={ "title","name", "info1", "info"};
       

        clique it; 
        pgm.graph[0]=it;
        pgm.graph[0].cliqueQueryCSV(database+"info_type.csv", {"info_type_id"}, pgm.eliminationOrder, pgm.deletionOrder,{"info_type_id"} ,shallIcleanRawData);

        clique ke; 
        pgm.graph[1]=ke;
        pgm.graph[1].cliqueQueryCSV(database+"info_type1.csv", {"info_type_id1"}, pgm.eliminationOrder, pgm.deletionOrder,{"info_type_id1"} ,shallIcleanRawData);
        
        clique k; // table keyword
        pgm.graph[2]=k;
        pgm.graph[2].cliqueQueryCSV(database+"keyword.csv", {"keyword_id"}, pgm.eliminationOrder, pgm.deletionOrder,{"keyword_id"},shallIcleanRawData );
        
        clique mk; // table movie keyword
        pgm.graph[3]=mk;
        pgm.graph[3].cliqueQueryCSV(database+"movie_keyword.csv", {"movie_id","keyword_id"}, pgm.eliminationOrder, pgm.deletionOrder,{"movie_id","keyword_id"} ,shallIcleanRawData);
        
         
        clique mi; 
        pgm.graph[4]=mi;
        pgm.graph[4].cliqueQueryCSV(database+"movie_info_idx.csv", {"movie_id","info_type_id1", "info1"}, pgm.eliminationOrder, pgm.deletionOrder,{"movie_id","info_type_id1"},shallIcleanRawData );
        
        clique t; 
        pgm.graph[5]=t;
        pgm.graph[5].cliqueQueryCSV(database+"title.csv", {"movie_id", "title"}, pgm.eliminationOrder, pgm.deletionOrder,{"movie_id"},shallIcleanRawData );
        
        clique cn; 
        pgm.graph[6]=cn;
        pgm.graph[6].cliqueQueryCSV(database+"company_name.csv", {"company_id", "name"}, pgm.eliminationOrder, pgm.deletionOrder,{"company_id"},shallIcleanRawData);
        
        clique mii; 
        pgm.graph[7]=mii;
        pgm.graph[7].cliqueQueryCSV(database+"movie_info.csv", {"movie_id", "info_type_id", "info"}, pgm.eliminationOrder, pgm.deletionOrder,{"movie_id", "info_type_id"},shallIcleanRawData);
        
        clique ci;
        pgm.graph[8]=ci;
        pgm.graph[8].cliqueQueryCSV(database+"cast_info.csv", {"movie_id","person_id"},pgm.eliminationOrder, pgm.deletionOrder,{"movie_id","person_id"},shallIcleanRawData);
        
        
    }
//    
    else if(query=="JOB_C"){
        pgm.graph.resize(7);
        pgm.deletionOrder={{2,"info_type_id"},{1,"info_type_id1"},{0,"person_id"}};
        pgm.eliminationOrder={{1,{"title"}}, {3,{"info"}}, {0,{"info1"}}, {2,{"movie_id"}}}; 
        pgm.outputVars={ "info","info1", "title"};  //imdb_id is before gender in the table
       
        clique lt; 
        pgm.graph[0]=lt;
        pgm.graph[0].cliqueQueryCSV(database+"name.csv", {"person_id"}, pgm.eliminationOrder, pgm.deletionOrder,{"person_id"} ,shallIcleanRawData);
        
        clique ct; 
        pgm.graph[6]=ct;
        pgm.graph[6].cliqueQueryCSV(database+"info_type.csv", {"info_type_id"}, pgm.eliminationOrder, pgm.deletionOrder,{"info_type_id"} ,shallIcleanRawData);

        clique k; 
        pgm.graph[1]=k;
        pgm.graph[1].cliqueQueryCSV(database+"info_type1.csv", {"info_type_id1"}, pgm.eliminationOrder, pgm.deletionOrder,{"info_type_id1"} ,shallIcleanRawData);
        
        clique cn; 
        pgm.graph[2]=cn;
        pgm.graph[2].cliqueQueryCSV(database+"movie_info.csv", {"movie_id", "info_type_id", "info"}, pgm.eliminationOrder, pgm.deletionOrder,{"movie_id", "info_type_id"},shallIcleanRawData);
        
        clique mc; 
        pgm.graph[3]=mc;
        pgm.graph[3].cliqueQueryCSV(database+"movie_info_idx.csv", {"movie_id","info_type_id1", "info1"}, pgm.eliminationOrder, pgm.deletionOrder,{"movie_id","info_type_id1"},shallIcleanRawData );
        
        clique mk; 
        pgm.graph[4]=mk;
        pgm.graph[4].cliqueQueryCSV(database+"cast_info.csv", {"movie_id","person_id"}, pgm.eliminationOrder, pgm.deletionOrder,{"movie_id","person_id"},shallIcleanRawData );
        
        clique t; 
        pgm.graph[5]=t;
        pgm.graph[5].cliqueQueryCSV(database+"title.csv", {"movie_id", "title"}, pgm.eliminationOrder, pgm.deletionOrder,{"movie_id"} ,shallIcleanRawData);
       
    }
    else if(query=="q1"){
        pgm.graph.resize(5);
        pgm.deletionOrder={{0,"info_type_id1"},{1,"company_type_id"},{2,"movie_id"}};
        pgm.eliminationOrder={{1,{"note"}}, {0,{"title|production_year"}}}; 
        pgm.outputVars={ "note","production_year", "title"};  //imdb_id is before gender in the table
       
        clique lt; 
        pgm.graph[0]=lt;
        pgm.graph[0].cliqueQueryCSV(database+"company_type.csv", {"company_type_id"}, pgm.eliminationOrder, pgm.deletionOrder,{"company_type_id"} ,shallIcleanRawData);
        
        clique ct; 
        pgm.graph[1]=ct;
        pgm.graph[1].cliqueQueryCSV(database+"info_type1.csv", {"info_type_id1"}, pgm.eliminationOrder, pgm.deletionOrder,{"info_type_id1"} ,shallIcleanRawData);

        clique k; 
        pgm.graph[2]=k;
        pgm.graph[2].cliqueQueryCSV(database+"movie_companies.csv", {"movie_id","company_type_id", "note"}, pgm.eliminationOrder, pgm.deletionOrder,{"movie_id","company_type_id"} ,shallIcleanRawData);
        
        clique mc; 
        pgm.graph[3]=mc;
        pgm.graph[3].cliqueQueryCSV(database+"movie_info_idx.csv", {"movie_id","info_type_id1"}, pgm.eliminationOrder, pgm.deletionOrder,{"movie_id","info_type_id1"},shallIcleanRawData );
        
        clique t; 
        pgm.graph[4]=t;
        pgm.graph[4].cliqueQueryCSV(database+"title.csv", {"movie_id", "title", "production_year"}, pgm.eliminationOrder, pgm.deletionOrder,{"movie_id"} ,shallIcleanRawData);
       
    }
    else if(query=="JOB_A"){
        pgm.graph.resize(4);
        pgm.deletionOrder={{1,"keyword_id"},{0,"movie_id"}};
        pgm.eliminationOrder={{0,{"title"}}}; 
        pgm.outputVars={  "title"};  //imdb_id is before gender in the table
       
        clique lt; 
        pgm.graph[0]=lt;
        pgm.graph[0].cliqueQueryCSV(database+"movie_info.csv", {"movie_id"}, pgm.eliminationOrder, pgm.deletionOrder,{"movie_id"} ,shallIcleanRawData);
        
        clique ct; 
        pgm.graph[1]=ct;
        pgm.graph[1].cliqueQueryCSV(database+"movie_keyword.csv", {"movie_id","keyword_id"}, pgm.eliminationOrder, pgm.deletionOrder,{"movie_id","keyword_id"} ,shallIcleanRawData);

        clique mc; 
        pgm.graph[2]=mc;
        pgm.graph[2].cliqueQueryCSV(database+"keyword.csv", {"keyword_id"}, pgm.eliminationOrder, pgm.deletionOrder,{"keyword_id"},shallIcleanRawData );
        
        clique t; 
        pgm.graph[3]=t;
        pgm.graph[3].cliqueQueryCSV(database+"title.csv", {"movie_id", "title"}, pgm.eliminationOrder, pgm.deletionOrder,{"movie_id"} ,shallIcleanRawData);
       
    }
    else if(query=="JOB_B"){
        pgm.graph.resize(9);
        pgm.deletionOrder={{1,"info_type_id"},{3,"company_id"}, {0,"keyword_id"},{4,{"movie_id"}},{2,{"company_type_id"}}};
        pgm.eliminationOrder={{0,{"title"}},{1,{"info"}}}; 
        pgm.outputVars={"info", "title"};
      
        
        clique k; // table keyword
        pgm.graph[0]=k;
        pgm.graph[0].cliqueQueryCSV(database+"keyword.csv", {"keyword_id"}, pgm.eliminationOrder, pgm.deletionOrder,{"keyword_id"},shallIcleanRawData );

        clique ci;
        pgm.graph[1]=ci;
        pgm.graph[1].cliqueQueryCSV(database+"aka_title.csv", {"movie_id"},pgm.eliminationOrder, pgm.deletionOrder,{"movie_id"},shallIcleanRawData);
        
        clique an; // table aka_name
        pgm.graph[2]=an;
        pgm.graph[2].cliqueQueryCSV(database+"company_type.csv", {"company_type_id"}, pgm.eliminationOrder, pgm.deletionOrder,{"company_type_id"},shallIcleanRawData);
        
        clique cn; // table company name
        pgm.graph[3]=cn;
        pgm.graph[3].cliqueQueryCSV(database+"company_name.csv", {"company_id"}, pgm.eliminationOrder, pgm.deletionOrder,{"company_id"},shallIcleanRawData);
        
        clique mc; // table movie_company
        pgm.graph[4]=mc;
        pgm.graph[4].cliqueQueryCSV(database+"movie_companies.csv", {"movie_id","company_id","company_type_id"}, pgm.eliminationOrder, pgm.deletionOrder,{"movie_id","company_id","company_type_id"} ,shallIcleanRawData);
        
        clique mk; // table movie keyword
        pgm.graph[5]=mk;
        pgm.graph[5].cliqueQueryCSV(database+"movie_keyword.csv", {"movie_id","keyword_id"}, pgm.eliminationOrder, pgm.deletionOrder,{"movie_id","keyword_id"} ,shallIcleanRawData);
        
        clique n; // table name
        pgm.graph[6]=n;
        pgm.graph[6].cliqueQueryCSV(database+"movie_info.csv", {"info_type_id","movie_id","info"}, pgm.eliminationOrder, pgm.deletionOrder,{"info_type_id","movie_id"} ,shallIcleanRawData);
        
        clique t; // table title
        pgm.graph[7]=t;
        pgm.graph[7].cliqueQueryCSV(database+"title.csv", {"movie_id", "title"}, pgm.eliminationOrder, pgm.deletionOrder,{"movie_id"},shallIcleanRawData );
        clique nq; // table name
        pgm.graph[8]=nq;
        pgm.graph[8].cliqueQueryCSV(database+"info_type.csv", {"info_type_id"}, pgm.eliminationOrder, pgm.deletionOrder,{"info_type_id"} ,shallIcleanRawData);
        
    }
    auto end = std::chrono::system_clock::now();

    std::chrono::duration<double> elapsed = end - start;
    std::cout << "\nElapsed wall time for csv query: " << elapsed.count() << "s";
    consumed_time+=elapsed.count();
//   
//    unsigned int microsecond = 1000000;
//    usleep(3 * microsecond);
    cout<<"\n *******************    Deleting non-output variables, started.    ****************"<<endl;
    start = std::chrono::system_clock::now();
    pgm.deleteNonOutputVars();
    end = std::chrono::system_clock::now();
    elapsed = end - start;
    std::cout << "\nElapsed wall time for Deleting non-output variables: " << elapsed.count() << "s";
    consumed_time+=elapsed.count();
//    
    struct rusage r_usage;
//    getrusage(RUSAGE_SELF,&r_usage);
//    cout<<"\nPeak Memory usage is: "<<r_usage.ru_maxrss/1024<<"MB"<<endl;
//    
    cout<<"\n *******************    Eliminating the output variables, started.    ****************"<<endl;
    start = std::chrono::system_clock::now();
    pgm.eliminateVariables();
    end = std::chrono::system_clock::now();
    elapsed = end - start;
    std::cout << "\nElapsed wall time for Eliminating the output variables: " << elapsed.count() << "s";
    consumed_time+=elapsed.count();

    
//    getrusage(RUSAGE_SELF,&r_usage);
//    cout<<"\nPeak Memory usage is: "<<r_usage.ru_maxrss/1024<<"MB"<<endl;
//    

    cout<<"\n*******************    Generation, started.    ****************"<<endl;
    start = std::chrono::system_clock::now();
    pgm.generateResults(generationMode, outAdd);
    end = std::chrono::system_clock::now();
    elapsed = end - start;
    std::cout << "\nElapsed wall time for all generations: " << elapsed.count() << "s";
    consumed_time+=elapsed.count();
//    getrusage(RUSAGE_SELF,&r_usage);
//    cout<<"\nPeak Memory usage is: "<<r_usage.ru_maxrss/1024<<"MB"<<endl;
    cout<< "\n Overall the consumed time is: "<<  consumed_time<<endl;


}