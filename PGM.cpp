
#include "PGM.hpp"
#include "csv_reader.hpp"

unordered_map<string, unordered_map<unsigned long int, string>> int2strNonJAs; // {composite att name{composite Key as a string,ordinal int}} 
unordered_map<string, unordered_map<string,unsigned long int>> str2intNonJAs; // {composite att name{composite Key as a string,ordinal int}} 
      
void clique::clear(){
    pot_T1.clear();
    cond_pot_T2.clear();
    cond_pot_T3.clear();
   
}

vector<int> PGM::findInGraph(vector<string> vars){
    vector <int> clqIndx;
    sort(vars.begin(), vars.end());
    for(int i=0;i<graph.size();i++){
        if(!graph[i].disabled){
            auto varList=graph[i].variableList;
            sort(varList.begin(), varList.end());
            bool exists= includes( varList.begin(), varList.end(), vars.begin(), vars.end());
            if (exists)
                clqIndx.push_back(i);
        }
    }
    return clqIndx;
}

vector<int> PGM::findInGenGraph(string var){
    vector <int> clqIndx;
    for(int i=0;i<generativeCliques.size();i++){
        if(generativeCliques[i].conditionedOnVar==var)
            clqIndx.push_back(i);
    }
    return clqIndx;
}




int findInVector( vector<string>& vecOfElements,  string& element)
{
    int indx;
    // Find given element in vector
    auto it = find(vecOfElements.begin(), vecOfElements.end(), element);
    if (it != vecOfElements.end())
        indx = (unsigned int) distance(vecOfElements.begin(), it);
    else
    {
        cout<<"The attribute does not exist in the clique"<<endl;
        exit(1);
    }
    return indx;
}

vector<string> vec_substract(vector<string> v1, vector<string> v2)
{
    sort(v1.begin(),v1.end());
    sort(v2.begin(),v2.end());
    vector<string> v3;
    set_difference(v1.begin(),v1.end(),v2.begin(),v2.end(),back_inserter(v3));
    return v3;
}

void PGM::deleteNonOutputVars(){
    /* 
     * delete the variables one by one according to deletion order
     * find all the values of current variable that exist in all the potentials
     * based on those values make the new cliques (edges)
     * Do not worry if the the edges are already exist, for example, we can have two Phi(A,B) and Phi (A,B)
     * Deleting a max clique (not a variable) is difficult because it is likely the max clique is not a simplicial.
     * create new cliques and delete the old ones
     * 
     * 
     * sumProductV1 (sumOut=true ): (x->y)  * (x->y) * (x->y  ==>  (y)
     * sumProductV1 (sumOut=false): (x->y)  * (x->y) * (x->y  ==>  (x->y)
     * sumProductV2: (x->y)  * (x->z) ==> (y->z) if elimination order of y is smaller
     * sumProductV3: (x->yw)  * (x->z) ==> (y->wz) if elimination order of y is smaller than elor of z and w
     * sumProductV4: (x->yz)  * (x->z) ==> (y->z) if elimination order of y is smaller than elor of z
     * sumProductV5: (x->yw)  * (x->zq) ==> (y->wzq) if elimination order of y is smaller than elor of z,q and w 
     *  
     * 
     * // the followings don not happen when the JAs are single
     * sumProductV6: (x->yw)  * (x->yz) ==> (y->wz) if elimination order of y is smaller than elor of z and w
     * sumProductV7 (sumOut=true ): (x->yz) * (x->yz) * (x->yz) ==>  (yz)          
     * sumProductV7 (sumOut=false): (x->yz) * (x->yz) * (x->yz) ==>  (x->yz)
     */
//    cout<< "Skipped the check whether the query has composite JAs or not"<<endl;
    for (int currIndx=0; currIndx<deletionOrder.size();currIndx++){ // per variable in deletion order
        string currVar= deletionOrder[currIndx];
        auto start = std::chrono::system_clock::now();
    
        auto clqList=findInGraph({currVar});
        vector<int> singleClqIndxV;  // indices of all single cliques in the graph
        
        int singleClqIndx; 
        vector<int> condClqIndx; // indices for all the conditional cliques
        for (auto &c:clqList){
            if(graph[c].pot_type== "T1")
                singleClqIndxV.push_back(c);
            else
                condClqIndx.push_back(c);
        }
        
        if (singleClqIndxV.size()>1)
            singleClqIndx= product_T1_T1(singleClqIndxV); // calculate the product of all single cliques and make a single clique
        else if(singleClqIndxV.size()==1)
            singleClqIndx=singleClqIndxV[0];
        else
            singleClqIndx=-1;
 
        //find the repetitive cliques (x->y)s
        vector<vector<int>> rep (condClqIndx.size());
        for(int i=0;i<condClqIndx.size();i++)
            for(int j=0;j<condClqIndx.size();j++){
                if(i!=j && graph[condClqIndx[i]].pot_type=="T2" && graph[condClqIndx[j]].pot_type=="T2")
                    if(graph[condClqIndx[i]].otherVars[0]==graph[condClqIndx[j]].otherVars[0])
                            rep[i].push_back(condClqIndx[j]);
            }
        unordered_map<int,bool> alreadyAdded;
        for(int i=0; i<rep.size();i++)
            alreadyAdded[i]=false;
        
        for(int i=0; i<rep.size();i++){
            int newInd;
            if(!alreadyAdded[i] && rep[i].size()>1){
                if((rep[i].size()==condClqIndx.size() || (rep[i].size()==condClqIndx.size()-1 && singleClqIndx>-1)))
                    sumProductV1(rep[i], true, singleClqIndx); // there is no other clique then ==> (y)
                else 
                    newInd=sumProductV1(rep[i], false, singleClqIndx); // there is some other cliques then ==> (x->y)
 
                if(singleClqIndx>-1){
                        graph[singleClqIndx].disabled=true;
                        if(shallICleanOldPots)
                            graph[singleClqIndx].clear();
                        singleClqIndx=-1;
                    }
                for(int j=0 ;j<rep[i].size();j++){
                    if(shallICleanOldPots)
                        graph[rep[i][j]].clear();
                    graph[rep[i][j]].disabled=true;
                    alreadyAdded[j]=true;
                }   
                alreadyAdded[i]=true;
            }
        }        

        
        clqList=findInGraph({currVar});
        condClqIndx.clear();
        singleClqIndxV.clear();
        for (auto &c:clqList){
            if(graph[c].pot_type== "T1")
                singleClqIndxV.push_back(c);
            else
                condClqIndx.push_back(c);
        }
        
        
        vector<int> newInd;
        for(int i=0;i<condClqIndx.size();i++)
                for(int j=0;j<condClqIndx.size();j++){
                    if(condClqIndx[i]!=-1 && condClqIndx[j]!=-1 && ((graph[condClqIndx[i]].pot_type=="T2" && graph[condClqIndx[j]].pot_type=="T3") || (graph[condClqIndx[i]].pot_type=="T3" && graph[condClqIndx[j]].pot_type=="T2"))){
                        if(graph[condClqIndx[i]].pot_type=="T2"){
                            auto it = find(graph[condClqIndx[j]].otherVars.begin(), graph[condClqIndx[j]].otherVars.end(), graph[condClqIndx[i]].otherVars[0]);

                            if (it != graph[condClqIndx[j]].otherVars.end())// the single var exists in the second as well
                                // (x->yz)  * (x->z) ==> (y->z) 
                            {
                                singleClqIndx=sumProductV4(condClqIndx[i],condClqIndx[j], singleClqIndx);
                                condClqIndx[i]=-1;
                                condClqIndx[j]=-1;
                                newInd.push_back(graph.size()-1);
                            }
                        }
                        else{
                            auto it = find(graph[condClqIndx[i]].otherVars.begin(), graph[condClqIndx[i]].otherVars.end(), graph[condClqIndx[j]].otherVars[0]);

                            if (it != graph[condClqIndx[i]].otherVars.end())// the single var exists in the second as well
                                // (x->yz)  * (x->z) ==> (y->z) 
                                {
                                singleClqIndx=sumProductV4(condClqIndx[j],condClqIndx[i], singleClqIndx);
                                condClqIndx[i]=-1;
                                condClqIndx[j]=-1;
                                newInd.push_back(graph.size()-1);
                            }
                        }
                        
                        
                    }
                }
               
        for(auto &ind:newInd)
            condClqIndx.push_back(ind);
        
        vector<int> tmp;
        for(auto ind:condClqIndx)
            if(ind!=-1)
                tmp.push_back(ind);
        condClqIndx=tmp;
        
        //now we have only one unique version of each edge
        

        if((singleClqIndx!=-1 && condClqIndx.size()==0) || (singleClqIndx==-1 && condClqIndx.size()==1) || (singleClqIndx!=-1 && condClqIndx.size()==1))
        {
            if(condClqIndx.size()==1)
                sumOut(condClqIndx[0], singleClqIndx);
            else
                sumOut(-1, singleClqIndx);
        }
        else{
            if(condClqIndx.size()>2)
            {
                cout<< "The resulting clique becomes large, so inefficient. Try to keep "<<currVar<<" in the graph."<<endl;
                exit(1);
            }
            for(int i=0;i<condClqIndx.size();i++)
                for(int j=i+1;j<condClqIndx.size();j++){
                    
                        if( graph[condClqIndx[i]].pot_type=="T2" && graph[condClqIndx[j]].pot_type=="T2"){
                            if(graph[condClqIndx[i]].otherVars[0]!=graph[condClqIndx[j]].otherVars[0])
                                singleClqIndx=sumProductV2(condClqIndx[i],condClqIndx[j],singleClqIndx); //(x->y)  * (x->z) ==> (y->z)
                        }
                        else { //(x->yw)  * (x->z) ==> (y->wz)   |   (x->yz)  * (x->z) ==> (y->z)  |     (x->yw)  * (x->zq) ==> (y->wzq)

                            if(graph[condClqIndx[i]].pot_type=="T2" && graph[condClqIndx[j]].pot_type=="T3")
                                singleClqIndx=sumProductV3(condClqIndx[i],condClqIndx[j],singleClqIndx);
                            else if(graph[condClqIndx[j]].pot_type=="T2"&& graph[condClqIndx[i]].pot_type=="T3")
                                singleClqIndx=sumProductV3(condClqIndx[j],condClqIndx[i],singleClqIndx);
                            else if(graph[condClqIndx[i]].pot_type=="T3" && graph[condClqIndx[j]].pot_type=="T3")//(x->yw)  * (x->zq) ==> (y->wzq)
                                singleClqIndx=sumProductV5(condClqIndx[j],condClqIndx[i],singleClqIndx);
                        }
                }
        }
      
       
        for(auto &clq:condClqIndx)
        {
            graph[clq].disabled=true;
            if(shallICleanOldPots)
                graph[clq].clear();
          
        }
        
        
        auto end = std::chrono::system_clock::now();
        std::chrono::duration<double>  elapsed = end - start;
        cout<<currVar<< " deleted in "<<elapsed.count() << "s"<<endl;
    }
}

int PGM::product_T1_T1(vector <int> clqList){
    /*
     * multiply the freqs and make a new clique
     * set the pot type as T1
     * delete the old cliques
     * return new clique index
     * 
     */
   
    
    int min=graph[clqList[0]].pot_T1.size();
    int minInd=0;
    for(int ind=1; ind<clqList.size();ind++)
        if(min>graph[clqList[ind]].pot_T1.size())
        {
            min=graph[clqList[ind]].pot_T1.size();
            minInd=ind;
        }

    clique newC;
    newC.pot_type ="T1";
    newC.variableList=graph[clqList[0]].variableList;
    graph.push_back(newC);
    auto newCref = &graph[graph.size()-1];
    auto baseCref=&graph[clqList[minInd]];
    

    
    if(clqList.size()==2){
        auto secondCref=&graph[clqList[abs(minInd-1)]];
        unsigned long long int product;
        for(auto &entry:baseCref->pot_T1){
            
            if(secondCref->pot_T1.find(entry.first)!=secondCref->pot_T1.end()){
                    product = entry.second*secondCref->pot_T1[entry.first];
                    if(product!=0)
                        newCref->pot_T1[entry.first]=product;
                }
        }
    }
    else{
        vector<clique *> refVec;
        for(int i=0;i<clqList.size();i++)
            if(i!=minInd)
                refVec.push_back(&graph[clqList[i]]);
        unsigned long long int product;
        bool exists;
        for(auto &entry:baseCref->pot_T1){
            product=entry.second;
            exists=true;
            for(auto ref:refVec){
                if(ref->pot_T1.find(entry.first)!=ref->pot_T1.end()){
                    product*=ref->pot_T1[entry.first];
                }
                else{
                    exists=false;
                    break;
                }
            }
            if(exists && product!=0)
                newCref->pot_T1[entry.first]=product;
           
        }
    }
    
    
    for(auto ind:clqList){
        graph[ind].disabled=true;
        if(shallICleanOldPots)
            graph[ind].clear();
    }
    return graph.size()-1;
}
int PGM::sumProductV1(vector <int> clqList, bool summingOut, int singleClqIndx){
    /* sum out current node if summingOut is true
     * otherwise make the same conditional pot with product operation    
     * make a new clique and remove all the related cliques
     * single clique is considered of summingout is true
     * change the old clique indices to -1
     * ===========   if summingOut is false, the current clique list should be updated together with the PGM.graph
    */
//    graph.clear();
//    clique c;
//    c.cond_pot_T2["b"]={{"1",10},{"2",20}};
//    c.cond_pot_T2["c"]={{"1",10},{"2",20}};
////    c.cond_pot_T2["d"]={{"1",5},{"2",5}};
//    graph.push_back(c);
//    clique c1;
//    c1.cond_pot_T2["a"]={{"3",1},{"2",3}};
//    c1.cond_pot_T2["b"]={{"2",4}};
//    c1.cond_pot_T2["c"]={{"3",2},{"2",5}};
//    graph.push_back(c1);
//    
//    clique c2;
//    c2.pot_T1={{"1",10},{"2",100}};
//
//    graph.push_back(c2);
    
    if(clqList.size()>2){
        cout<<"Try other elimination order. Not implemented yet!"<<endl;
        exit(1);
    }
    else{
        if(!summingOut){
            clique newC;
            newC.pot_type="T2";
            newC.variableList=graph[clqList[0]].variableList;
            newC.conditionedOnVar=graph[clqList[0]].conditionedOnVar;
            newC.otherVars=graph[clqList[0]].otherVars;
            graph.push_back(newC);
            
            int outCind=graph.size()-1;
            int baseInd=0;
            if(graph[clqList[0]].cond_pot_T2.size()>graph[clqList[1]].cond_pot_T2.size())
                baseInd=1;
            
            auto baseClq=&graph[baseInd];
            auto secondClq=&graph[abs(baseInd-1)];
            auto targetClq=&graph[abs(outCind)];
            for(auto &entry:baseClq->cond_pot_T2)
                if(secondClq->cond_pot_T2.find(entry.first)!=secondClq->cond_pot_T2.end()){
                    auto secpot=&secondClq->cond_pot_T2[entry.first];
                    if((*secpot).size()>entry.second.size()){
                        for(auto &entry2:entry.second)
                            if((*secpot).find(entry2.first)!=(*secpot).end())
                                targetClq->cond_pot_T2[entry.first][entry2.first]=entry2.second*(*secpot)[entry2.first];
                    }
                    else{
                        for(auto &entry2:(*secpot))
                            if(entry.second.find(entry2.first)!=entry.second.end())
                                targetClq->cond_pot_T2[entry.first][entry2.first]=entry2.second*entry.second[entry2.first];
                    }
                }
            
        }
        else{
            //sum out
            if(singleClqIndx<0){  
                clique newC;
                newC.pot_type="T1";
                newC.variableList=graph[clqList[0]].variableList;
                graph.push_back(newC);
                
                int outCind=graph.size()-1;
                int baseInd=0;
                if(graph[clqList[0]].cond_pot_T2.size()>graph[clqList[1]].cond_pot_T2.size())
                    baseInd=1;

                auto baseClq=&graph[baseInd];
                auto secondClq=&graph[abs(baseInd-1)];
                auto targetClq=&graph[abs(outCind)];
                for(auto &entry:baseClq->cond_pot_T2)
                    if(secondClq->cond_pot_T2.find(entry.first)!=secondClq->cond_pot_T2.end()){
                        auto secpot=&secondClq->cond_pot_T2[entry.first];
                        if((*secpot).size()>entry.second.size()){
                            for(auto &entry2:entry.second)
                                if((*secpot).find(entry2.first)!=(*secpot).end())
                                    targetClq->pot_T1[entry2.first]+=entry2.second*(*secpot)[entry2.first];
                        }
                        else{
                            for(auto &entry2:(*secpot))
                                if(entry.second.find(entry2.first)!=entry.second.end())
                                    targetClq->pot_T1[entry2.first]+=entry2.second*entry.second[entry2.first];
                        }
                    }
                
            }
            else{
                //sum out and product with the single clique
                clique newC;
                newC.pot_type="T1";
                newC.variableList=graph[clqList[0]].variableList;
                graph.push_back(newC);
                
                int outCind=graph.size()-1;
                int baseInd=0;
                if(graph[clqList[0]].cond_pot_T2.size()>graph[clqList[1]].cond_pot_T2.size())
                    baseInd=1;

                auto baseClq=&graph[baseInd];
                auto secondClq=&graph[abs(baseInd-1)];
                auto targetClq=&graph[abs(outCind)];
                auto singleClq=&graph[singleClqIndx].pot_T1;
                for(auto &entry:baseClq->cond_pot_T2)
                    if(secondClq->cond_pot_T2.find(entry.first)!=secondClq->cond_pot_T2.end()){
                        auto secpot=&secondClq->cond_pot_T2[entry.first];
                        if((*secpot).size()>entry.second.size()){
                            for(auto &entry2:entry.second)
                                if((*secpot).find(entry2.first)!=(*secpot).end() && (*singleClq).find(entry2.first)!=(*singleClq).end())
                                    targetClq->pot_T1[entry2.first]+=entry2.second*(*secpot)[entry2.first]*(*singleClq)[entry2.first];
                        }
                        else{
                            for(auto &entry2:(*secpot))
                                if(entry.second.find(entry2.first)!=entry.second.end() && (*singleClq).find(entry2.first)!=(*singleClq).end())
                                    targetClq->pot_T1[entry2.first]+=entry2.second*entry.second[entry2.first]*(*singleClq)[entry2.first];
                        }
                    }
            }
        }
    }
    
    return graph.size()-1;
}

int PGM::sumProductV2(int clq1, int clq2, int singleClqIndx){
    
//    deletionOrder={{0,"a"},{1,"b"}};
//    eliminationOrder={{0,{"c"}},{1,{"d"}}};
//    graph.clear();
//    clique c;
//    c.conditionedOnVar="a";
//    c.otherVars={"b"};
//    c.cond_pot_T2["b"]={{"1",10},{"2",20}};
//    c.cond_pot_T2["c"]={{"1",10},{"2",20}};
////    c.cond_pot_T2["d"]={{"1",5},{"2",5}};
//    graph.push_back(c);
//    clique c1;
//    c1.conditionedOnVar="a";
//    c1.otherVars={"c"};
//    c1.cond_pot_T2["a"]={{"mm",1},{"nn",3}};
//    c1.cond_pot_T2["b"]={{"cc",4}};
//    c1.cond_pot_T2["c"]={{"cc",2},{"mm",5}};
//    graph.push_back(c1);
//    
//    clique c2;
//    c2.pot_T1={{"b",10},{"a",100}};
//
//    graph.push_back(c2);
    
    clique newC;
    newC.pot_type="T2";
    graph.push_back(newC);
    int newClqInd=graph.size()-1;
    auto targetClq=&graph[newClqInd].cond_pot_T2;
    
    int firstOrder, secondOrder; // to find the var with earliest elimination or deletion
    
    for(auto &delo: deletionOrder)
        deletionOrderRev[delo.second]=delo.first;
    for(auto &delo: eliminationOrder)
        for(auto &sec:delo.second)
            eliminationOrderRev[sec]=delo.first;
    
    
    if(deletionOrderRev.find(graph[clq1].otherVars[0])!=deletionOrderRev.end()){
        firstOrder=deletionOrderRev[graph[clq1].otherVars[0]];
        
    }
    else{
        firstOrder=eliminationOrderRev[graph[clq1].otherVars[0]]+ deletionOrderRev.size();
        
    }
    
    if(deletionOrderRev.find(graph[clq2].otherVars[0])!=deletionOrderRev.end())
        secondOrder=deletionOrderRev[graph[clq2].otherVars[0]];
    else
        secondOrder=eliminationOrderRev[graph[clq2].otherVars[0]]+ deletionOrderRev.size();
    
    if(firstOrder<secondOrder){
        graph[graph.size()-1].variableList={graph[clq1].otherVars[0],graph[clq2].otherVars[0]};
        graph[graph.size()-1].conditionedOnVar=graph[clq1].otherVars[0];
        graph[graph.size()-1].otherVars.push_back(graph[clq2].otherVars[0]);
    }
    else{
        graph[graph.size()-1].variableList={graph[clq2].otherVars[0],graph[clq1].otherVars[0]};
        graph[graph.size()-1].conditionedOnVar=graph[clq2].otherVars[0];
        graph[graph.size()-1].otherVars.push_back(graph[clq1].otherVars[0]);
    }
    
    
    if(singleClqIndx==-1){ // no product with the single clique
        if(firstOrder<secondOrder){
            
            if (graph[clq1].cond_pot_T2.size()<graph[clq2].cond_pot_T2.size()){
                for(auto &entry:graph[clq1].cond_pot_T2)
                    if(graph[clq2].cond_pot_T2.find(entry.first)!=graph[clq2].cond_pot_T2.end())
                        for(auto &first:entry.second)
                            for(auto &sec:graph[clq2].cond_pot_T2[entry.first]){
                                (*targetClq)[first.first][sec.first]+=first.second*sec.second;
                            }
            }
            else{
                for(auto &entry:graph[clq2].cond_pot_T2)
                    if(graph[clq1].cond_pot_T2.find(entry.first)!=graph[clq1].cond_pot_T2.end())
                        for(auto &first:entry.second)
                            for(auto &sec:graph[clq1].cond_pot_T2[entry.first]){
                                (*targetClq)[sec.first][first.first]+=first.second*sec.second;
                            }
            }
                
        }
        else{ // firstOrder > secondOrder
            if (graph[clq1].cond_pot_T2.size()<graph[clq2].cond_pot_T2.size()){
                for(auto &entry:graph[clq1].cond_pot_T2)
                    if(graph[clq2].cond_pot_T2.find(entry.first)!=graph[clq2].cond_pot_T2.end())
                        for(auto &first:entry.second)
                            for(auto &sec:graph[clq2].cond_pot_T2[entry.first]){
                                (*targetClq)[sec.first][first.first]+=first.second*sec.second;
                            }
            }
            else{
                for(auto &entry:graph[clq2].cond_pot_T2)
                    if(graph[clq1].cond_pot_T2.find(entry.first)!=graph[clq1].cond_pot_T2.end())
                        for(auto &first:entry.second)
                            for(auto &sec:graph[clq1].cond_pot_T2[entry.first]){
                                (*targetClq)[first.first][sec.first]+=first.second*sec.second;
                            }
            }
        }
         
    }
    else{
        //  product with the single clique
        
        auto singleClq=&graph[singleClqIndx].pot_T1;
        
        if(firstOrder<secondOrder){
            if (graph[clq1].cond_pot_T2.size()<graph[clq2].cond_pot_T2.size() && graph[clq1].cond_pot_T2.size()< (*singleClq).size()){
                for(auto &entry:graph[clq1].cond_pot_T2)
                    if(graph[clq2].cond_pot_T2.find(entry.first)!=graph[clq2].cond_pot_T2.end() && (*singleClq).find(entry.first)!=(*singleClq).end())
                        for(auto &first:entry.second)
                            for(auto &sec:graph[clq2].cond_pot_T2[entry.first]){
                                (*targetClq)[first.first][sec.first]+=first.second*sec.second * (*singleClq)[entry.first];
                            }
            }
            else if (graph[clq2].cond_pot_T2.size()<graph[clq1].cond_pot_T2.size() && graph[clq2].cond_pot_T2.size()< (*singleClq).size()){
                for(auto &entry:graph[clq2].cond_pot_T2)
                    if(graph[clq1].cond_pot_T2.find(entry.first)!=graph[clq1].cond_pot_T2.end() && (*singleClq).find(entry.first)!=(*singleClq).end())
                        for(auto &first:entry.second)
                            for(auto &sec:graph[clq1].cond_pot_T2[entry.first]){
                                (*targetClq)[sec.first][first.first]+=first.second*sec.second * (*singleClq)[entry.first];
                            }
            }
            else{
                for(auto &entry:(*singleClq))
                    if(graph[clq1].cond_pot_T2.find(entry.first)!=graph[clq1].cond_pot_T2.end() && graph[clq2].cond_pot_T2.find(entry.first)!=graph[clq2].cond_pot_T2.end())
                        for(auto &first:graph[clq1].cond_pot_T2[entry.first])
                            for(auto &sec:graph[clq2].cond_pot_T2[entry.first]){
                                (*targetClq)[first.first][sec.first]+=first.second*sec.second * (*singleClq)[entry.first];
                            }
            }
                
        }
        else{ // firstOrder > secondOrder
            if (graph[clq1].cond_pot_T2.size()<graph[clq2].cond_pot_T2.size() && graph[clq1].cond_pot_T2.size()< (*singleClq).size()){
                for(auto &entry:graph[clq1].cond_pot_T2)
                    if(graph[clq2].cond_pot_T2.find(entry.first)!=graph[clq2].cond_pot_T2.end() && (*singleClq).find(entry.first)!=(*singleClq).end())
                        for(auto &first:entry.second)
                            for(auto &sec:graph[clq2].cond_pot_T2[entry.first]){
                                (*targetClq)[sec.first][first.first]+=first.second*sec.second * (*singleClq)[entry.first];
                            }
            }
            else if (graph[clq2].cond_pot_T2.size()<graph[clq1].cond_pot_T2.size() && graph[clq2].cond_pot_T2.size()< (*singleClq).size()){
                for(auto &entry:graph[clq2].cond_pot_T2)
                    if(graph[clq1].cond_pot_T2.find(entry.first)!=graph[clq1].cond_pot_T2.end() && (*singleClq).find(entry.first)!=(*singleClq).end())
                        for(auto &first:entry.second)
                            for(auto &sec:graph[clq1].cond_pot_T2[entry.first]){
                                (*targetClq)[first.first][sec.first]+=first.second*sec.second * (*singleClq)[entry.first];
                            }
            }
            else{
                
                for(auto &entry:(*singleClq))
                    if(graph[clq1].cond_pot_T2.find(entry.first)!=graph[clq1].cond_pot_T2.end() && graph[clq2].cond_pot_T2.find(entry.first)!=graph[clq2].cond_pot_T2.end())
                        for(auto &first:graph[clq1].cond_pot_T2[entry.first])
                            for(auto &sec:graph[clq2].cond_pot_T2[entry.first]){
                                (*targetClq)[sec.first][first.first]+=first.second*sec.second * (*singleClq)[entry.first];
                            }
                 
            }
        }
        
        graph[singleClqIndx].disabled=true;
        if(shallICleanOldPots)
            graph[singleClqIndx].clear();
        singleClqIndx=-1;

    }
    return singleClqIndx;
  
}

int PGM::sumProductV3(int clq1, int clq2, int singleClqIndx){  // clq1 has single var x->y, always.
    
    
    
//    deletionOrder={{0,"a"},{1,"d"}};
//    eliminationOrder={{0,{"c"}},{1,{"b"}}};
//    graph.clear();
//    clique c;
//    c.conditionedOnVar="a";
//    c.otherVars={"b"};
//    
//    c.cond_pot_T2["b"]={{"1",10},{"2",20}};
//    c.cond_pot_T2["c"]={{"1",10},{"2",20}};
//   
////    c.cond_pot_T2["d"]={{"1",5},{"2",5}};
//    graph.push_back(c);
//    
//    clique c1;
//    c1.conditionedOnVar="a";
//    c1.otherVars={"c","d"};
//    
//    c1.cond_pot_T3["a"]={{{"m","n"},1},{{"mm","n"},3},{{"mm","nn"},5}};
//    c1.cond_pot_T3["b"]={{{"m","n"},1},{{"mm","n"},3},{{"mm","nn"},5}};
//    c1.cond_pot_T3["c"]={{{"m","n"},1},{{"mmm","n"},3}};
//    graph.push_back(c1);
//    
//    clique c2;
//    c2.pot_T1={{"b",10},{"a",100}};
//
//    graph.push_back(c2);
    

    unordered_map<string,int> elor;
    for(auto &delo: deletionOrder)
        elor[delo.second]=delo.first;
    for(auto &delo: eliminationOrder)
        for(auto &sec:delo.second)
            elor[sec]=delo.first+deletionOrder.size();
    
    
    if(singleClqIndx==-1){ // no single var clique
           
        int keyInd;
        int minOrder;
        minOrder=elor[graph[clq1].otherVars[0]];
        keyInd=-1; // the key is in clq1

        for(int i=0; i<graph[clq2].otherVars.size();i++)
            if(elor[graph[clq2].otherVars[i]]<minOrder){
                minOrder=elor[graph[clq2].otherVars[i]];
                keyInd=i;
            }


        clique newC;
        newC.pot_type="T3";
        if(keyInd==-1){
            newC.conditionedOnVar=graph[clq1].otherVars[0];
            newC.otherVars=graph[clq2].otherVars;
        }
        else{
            newC.otherVars.push_back(graph[clq1].otherVars[0]);
            for(int i=0;i<graph[clq2].otherVars.size();i++){
                if(i==keyInd)
                    newC.conditionedOnVar=graph[clq2].otherVars[i];
                else
                    newC.otherVars.push_back(graph[clq2].otherVars[i]);
            }
        }
        graph.push_back(newC);
        auto targetClq=&graph[graph.size()-1];

        auto baseClq=&graph[clq1].cond_pot_T2;
        auto secondClq=&graph[clq2].cond_pot_T3;
        int key1;
        vector<int> key2;
        for(auto &entry1:(*baseClq)){ 
            if((*secondClq).find(entry1.first)!=(*secondClq).end()){
                for(auto &en1:entry1.second){//cartesian product
                    for(auto &en2:(*secondClq)[entry1.first]){

                        if(keyInd==-1){
                            key1=en1.first;
                            key2=en2.first;
                        }
                        else{
                            key2.clear();
                            key2.push_back(en1.first);
                            for(int i=0;i<en2.first.size();i++){
                                if(i==keyInd)
                                    key1=en2.first[i];
                                else
                                    key2.push_back(en2.first[i]);
                            }
                        }

                        targetClq->cond_pot_T3[key1][key2]+= en1.second * en2.second;
                    }
                }
            }
        } 
    }
    else{//single clique should be considereeed
        int keyInd;
        int minOrder;
        minOrder=elor[graph[clq1].otherVars[0]];
        keyInd=-1; // the key is in clq1

        for(int i=0; i<graph[clq2].otherVars.size();i++)
            if(elor[graph[clq2].otherVars[i]]<minOrder){
                minOrder=elor[graph[clq2].otherVars[i]];
                keyInd=i;
            }


        clique newC;
        newC.pot_type="T3";
        if(keyInd==-1){
            newC.conditionedOnVar=graph[clq1].otherVars[0];
            newC.otherVars=graph[clq2].otherVars;
        }
        else{
            newC.otherVars.push_back(graph[clq1].otherVars[0]);
            for(int i=0;i<graph[clq2].otherVars.size();i++){
                if(i==keyInd)
                    newC.conditionedOnVar=graph[clq2].otherVars[i];
                else
                    newC.otherVars.push_back(graph[clq2].otherVars[i]);
            }
        }
        graph.push_back(newC);
        auto targetClq=&graph[graph.size()-1];
        auto singleClqRef=&graph[singleClqIndx];
        auto baseClq=&graph[clq1].cond_pot_T2;
        auto secondClq=&graph[clq2].cond_pot_T3;
        int key1;
        vector<int> key2;
        for(auto &entry1:(*baseClq)){ 
            if((*secondClq).find(entry1.first)!=(*secondClq).end() && singleClqRef->pot_T1.find(entry1.first)!=singleClqRef->pot_T1.end()){
                for(auto &en1:entry1.second){//cartesian product
                    for(auto &en2:(*secondClq)[entry1.first]){

                        if(keyInd==-1){
                            key1=en1.first;
                            key2=en2.first;
                        }
                        else{
                            key2.clear();
                            key2.push_back(en1.first);
                            for(int i=0;i<en2.first.size();i++){
                                if(i==keyInd)
                                    key1=en2.first[i];
                                else
                                    key2.push_back(en2.first[i]);
                            }
                        }
                        
                        targetClq->cond_pot_T3[key1][key2]+= en1.second * en2.second *singleClqRef->pot_T1[entry1.first];
                    }
                }
            }
        }
        graph[singleClqIndx].disabled=true;
        if(shallICleanOldPots)
            graph[singleClqIndx].clear();
        singleClqIndx=-1;
   
    }
    graph[graph.size()-1].variableList.push_back(graph[graph.size()-1].conditionedOnVar);
    for(auto &var:graph[graph.size()-1].otherVars)
        graph[graph.size()-1].variableList.push_back(var);
    
    return singleClqIndx;
}

int PGM::sumProductV4(int clq1, int clq2, int singleClqIndx){
    // (x->z)  and (x->zy)   ==>  (z->zy)
    // first clique is the clique with single var
    //eliminate the old cliques
//    
//    deletionOrder={{0,"a"},{1,"d"},{2,"e"}};
//    eliminationOrder={{0,{"c"}},{1,{"b"}}};
//    graph.clear();
//    clique c;
//    c.conditionedOnVar="a";
//    c.otherVars={"b"};
//    c.cond_pot_T2["a1"]={{"b1",1},{"b2",5}};
//    c.cond_pot_T2["a2"]={{"b1",10},{"b2",3}};
//   
////    c.cond_pot_T2["d"]={{"1",5},{"2",5}};
//    graph.push_back(c);
//    
//    clique c1;
//    c1.conditionedOnVar="a";
//    c1.otherVars={"b","d"};
//    
//    c1.cond_pot_T3["a3"]={{{"m","n"},1},{{"mm","n"},3},{{"mm","nn"},5}};
//    c1.cond_pot_T3["a1"]={{{"b1","d1"},1},{{"b2","d2"},5}};
//    c1.cond_pot_T3["a2"]={{{"b2","d2"},1}};
//    graph.push_back(c1);
//    
//    clique c2;
//    c2.pot_T1={{"a1",10},{"a",100}};
//
//    graph.push_back(c2);
//    
    clique newC;
    newC.conditionedOnVar=graph[clq1].conditionedOnVar;
    newC.otherVars=graph[clq2].otherVars;
    newC.pot_type="T3";
    graph.push_back(newC);
    
    auto baseClq=&graph[clq1].cond_pot_T2;
    auto secondClq=&graph[clq2].cond_pot_T3;
    auto targetClq=&graph[graph.size()-1].cond_pot_T3;
    
    int sharedVarId;
    for(int i=0; i<graph[clq2].otherVars.size();i++){
        if(graph[clq1].otherVars[0]== graph[clq2].otherVars[i])
            sharedVarId=i;
    }
    
    if(singleClqIndx==-1){
        for(auto &entry1:(*baseClq)){
            if((*secondClq).find(entry1.first)!=(*secondClq).end()){
                for(auto & entry2:(*secondClq)[entry1.first]){
                    if(entry1.second.find(entry2.first[sharedVarId])!=entry1.second.end()){
                        (*targetClq)[entry1.first][entry2.first]= entry2.second * entry1.second[entry2.first[sharedVarId]];
                    }
                        
                }
            }
        }
        
    }
    else{// single var clique exists 
        
        auto singleClq=&graph[singleClqIndx].pot_T1;
        
        for(auto &entry1:(*baseClq)){
            if((*secondClq).find(entry1.first)!=(*secondClq).end() && (*singleClq).find(entry1.first)!=(*singleClq).end()){
                for(auto & entry2:(*secondClq)[entry1.first]){
                    if(entry1.second.find(entry2.first[sharedVarId])!=entry1.second.end()){
                        (*targetClq)[entry1.first][entry2.first]= entry2.second * entry1.second[entry2.first[sharedVarId]] * (*singleClq)[entry1.first];
                    }
                        
                }
            }
        }
        
        graph[singleClqIndx].disabled=true;
        if(shallICleanOldPots)
            graph[singleClqIndx].clear();
        singleClqIndx=-1;
    }
    
    graph[clq1].disabled=true;
    graph[clq2].disabled=true;
    if(shallICleanOldPots){
        graph[clq1].clear();
        graph[clq2].clear();
    }
    
    graph[graph.size()-1].variableList.push_back(graph[graph.size()-1].conditionedOnVar);
    for(auto &var:graph[graph.size()-1].otherVars)
        graph[graph.size()-1].variableList.push_back(var);
    
    return singleClqIndx;
}
int PGM::sumProductV5(int clq1, int clq2, int singleClqIndx){
    
//        
//    deletionOrder={{0,"a"},{1,"d"},{2,"e"}};
//    eliminationOrder={{0,{"c"}},{1,{"b"}}};
//    graph.clear();
//    clique c;
//    c.conditionedOnVar="a";
//    c.otherVars={"b","e"};
//    c.cond_pot_T3["a1"]={{{"b1","e1"},1},{{"b1","e2"},5}};
//    c.cond_pot_T3["a2"]={{{"b1","e2"},1},{{"b2","e1"},3}};
//   
////    c.cond_pot_T2["d"]={{"1",5},{"2",5}};
//    graph.push_back(c);
//    
//    clique c1;
//    c1.conditionedOnVar="a";
//    c1.otherVars={"c","d"};
//    
//    c1.cond_pot_T3["a3"]={{{"m","n"},1},{{"mm","n"},3},{{"mm","nn"},5}};
//    c1.cond_pot_T3["a1"]={{{"c1","d1"},1},{{"c2","d2"},5}};
//    c1.cond_pot_T3["a2"]={{{"c2","d2"},1}};
//    graph.push_back(c1);
//    
//    clique c2;
//    c2.pot_T1={{"a1",10},{"a",100}};
//
//    graph.push_back(c2);
    

    unordered_map<string,int> elor;
    for(auto &delo: deletionOrder)
        elor[delo.second]=delo.first;
    for(auto &delo: eliminationOrder)
        for(auto &sec:delo.second)
            elor[sec]=delo.first+deletionOrder.size();
    
    vector<string> allVars;
    vector<int> allElOrs;
    for(auto &var: graph[clq1].otherVars){
        allVars.push_back(var);
        allElOrs.push_back(elor[var]);
    }
    for(auto &var: graph[clq2].otherVars){
        allVars.push_back(var);
        allElOrs.push_back(elor[var]);
    }
    
    int keyInd=0;
    int minOrder=allElOrs.size();
    for(int i=1;i<allElOrs.size();i++){
        if(allElOrs[i]<minOrder){
            keyInd=i;
            minOrder=allElOrs[i];
        }
            
    }
    
    if(singleClqIndx==-1){ // no single var clique

        clique newC;
        newC.pot_type="T3";
        newC.conditionedOnVar=allVars[keyInd];
        for(int i=0;i<graph[clq1].otherVars.size();i++)
            if(i!=keyInd)
                newC.otherVars.push_back(allVars[i]);
        for(int i=graph[clq1].otherVars.size();i<graph[clq1].otherVars.size()+ graph[clq2].otherVars.size();i++)
            if(i!=keyInd)
                newC.otherVars.push_back(allVars[i]);
        
        graph.push_back(newC);
        auto targetClq=&graph[graph.size()-1];

        auto baseClq=&graph[clq1].cond_pot_T3;
        auto secondClq=&graph[clq2].cond_pot_T3;
        int key1;
        vector<int> key2(allVars.size()-1);
        for(auto &entry1:(*baseClq)){ 
            if((*secondClq).find(entry1.first)!=(*secondClq).end()){
                for(auto &en1:entry1.second){//cartesian product
                    for(auto &en2:(*secondClq)[entry1.first]){
                        int id=0;
                        int id2=0;
                        for(auto &k:en1.first){
                            if(id==keyInd)
                                key1=k;
                            else{
                                key2[id2++]=k;
                            }
                            id++;
                        }

                        for(auto &k:en2.first){
                            if(id==keyInd)
                                key1=k;
                            else{
                                key2[id2++]=k;
                             
                            }
                            id++;
                        }
    
                        
                        targetClq->cond_pot_T3[key1][key2]+= en1.second * en2.second;
                    }
                }
            }
        } 
    }
    else{// single var clique exists 
        
        clique newC;
        newC.pot_type="T3";
        newC.conditionedOnVar=allVars[keyInd];
        for(int i=0;i<graph[clq1].otherVars.size();i++)
            if(i!=keyInd)
                newC.otherVars.push_back(allVars[i]);
        for(int i=graph[clq1].otherVars.size();i<graph[clq1].otherVars.size()+ graph[clq2].otherVars.size();i++)
            if(i!=keyInd)
                newC.otherVars.push_back(allVars[i]);
        
        graph.push_back(newC);
        auto targetClq=&graph[graph.size()-1];

        auto baseClq=&graph[clq1].cond_pot_T3;
        auto secondClq=&graph[clq2].cond_pot_T3;
        auto singleClqRef=&graph[singleClqIndx].pot_T1;
        
        int key1;
        vector<int> key2(allVars.size()-1);
        for(auto &entry1:(*baseClq)){ 
            if((*secondClq).find(entry1.first)!=(*secondClq).end() && (*singleClqRef).find(entry1.first)!= (*singleClqRef).end()){
                for(auto &en1:entry1.second){//cartesian product
                    for(auto &en2:(*secondClq)[entry1.first]){
                        int id=0;
                        int id2=0;
                        for(auto &k:en1.first){
                            if(id==keyInd)
                                key1=k;
                            else{
                                key2[id2++]=k;
                            }
                            id++;
                        }

                        for(auto &k:en2.first){
                            if(id==keyInd)
                                key1=k;
                            else{
                                key2[id2++]=k;
                             
                            }
                            id++;
                        }
    
                        
                        targetClq->cond_pot_T3[key1][key2]+= en1.second * en2.second * (*singleClqRef)[entry1.first];
                    }
                }
            }
        } 
        
        graph[singleClqIndx].disabled=true;
        if(shallICleanOldPots)
            graph[singleClqIndx].clear();
        singleClqIndx=-1;
       
    }
    graph[graph.size()-1].variableList.push_back(graph[graph.size()-1].conditionedOnVar);
    for(auto &var:graph[graph.size()-1].otherVars)
        graph[graph.size()-1].variableList.push_back(var);
    
    return singleClqIndx;
}

int PGM::sumOut(int clq, int singleClqInd){
    
//    
////        
//    deletionOrder={{0,"a"},{1,"d"},{2,"e"}};
//    eliminationOrder={{0,{"c"}},{1,{"b"}}};
//    graph.clear();
//    clique c;
//    c.pot_type="T2";
//    c.conditionedOnVar="a";
//    c.otherVars={"b"};
//    c.cond_pot_T2["a1"]={{"b1",10},{"b2",5}};
//    c.cond_pot_T2["a2"]={{"b1",4},{"b2",3}};
//   
////    c.cond_pot_T2["d"]={{"1",5},{"2",5}};
//    graph.push_back(c);
//    
//    clique c1;
//    c1.pot_type="T3";
//    c1.conditionedOnVar="a";
//    c1.otherVars={"c","d","e"};
//    
//    c1.cond_pot_T3["a1"]={{{"c1","d1","e1"},10},{{"c2","d2","e2"},5}};
//    c1.cond_pot_T3["a2"]={{{"c2","d2","e2"},2}};
//    graph.push_back(c1);
////    
//    clique c2;
//    c2.pot_type="T1";
//    c2.pot_T1={{"a1",10},{"a2",100}};
//
//    graph.push_back(c2);
//    
    
   
    
    if(clq==-1 ){
        int joinSize=0;
        for(auto &entry:graph[singleClqInd].pot_T1)
            joinSize+=entry.second;
        cout<<"The join size is: "<< joinSize<<endl;
        
        return joinSize;
    }
    else if(graph[clq].pot_type=="T2"){
        clique newC;
        newC.pot_type="T1";
        newC.variableList.push_back(graph[clq].otherVars[0]);
        graph.push_back(newC);
        auto targetClq=&graph[graph.size()-1].pot_T1;
        
        if(singleClqInd==-1){
            for(auto &entry:graph[clq].cond_pot_T2)
                for(auto &entry2:entry.second)
                    (*targetClq)[entry2.first]+=entry2.second;
        }
        else{
            auto singleClq=&graph[singleClqInd].pot_T1;
            for(auto &entry:graph[clq].cond_pot_T2)
                if((*singleClq).find(entry.first)!=(*singleClq).end()){
                    auto auxFreq=(*singleClq)[entry.first];
                    for(auto &entry2:entry.second)
                        (*targetClq)[entry2.first]+=entry2.second*auxFreq;
                }
            graph[singleClqInd].disabled=true;
            if(shallICleanOldPots)
                graph[singleClqInd].clear();
        }
        return graph.size()-1;
    }
    else{
        unordered_map<string,int> elor;
        for(auto &delo: deletionOrder)
            elor[delo.second]=delo.first;
        for(auto &delo: eliminationOrder)
            for(auto &sec:delo.second)
                elor[sec]=delo.first+deletionOrder.size();
           
        int keyInd;
        int minOrder;
        minOrder=elor[graph[clq].otherVars[0]];
        keyInd=0; // the key is in clq1

        for(int i=1; i<graph[clq].otherVars.size();i++)
            if(elor[graph[clq].otherVars[i]]<minOrder){
                minOrder=elor[graph[clq].otherVars[i]];
                keyInd=i;
            }
        if(graph[clq].otherVars.size()==2){
            if(singleClqInd==-1){
                clique newC;
                newC.pot_type="T2";
                newC.variableList=graph[clq].otherVars;
                newC.conditionedOnVar=graph[clq].otherVars[keyInd];
                int otherId=abs(keyInd-1);
                newC.otherVars.push_back(graph[clq].otherVars[otherId]);
                graph.push_back(newC);
                auto targetClq=&graph[graph.size()-1].cond_pot_T2;

                for(auto &entry:graph[clq].cond_pot_T3)
                    for(auto &entry2:entry.second) 
                        (*targetClq)[entry2.first[keyInd]][entry2.first[otherId]]+=entry2.second;
            }
            else{
                
                clique newC;
                newC.pot_type="T2";
                newC.variableList=graph[clq].otherVars;
                newC.conditionedOnVar=graph[clq].otherVars[keyInd];
                int otherId=abs(keyInd-1);
                newC.otherVars.push_back(graph[clq].otherVars[otherId]);
                graph.push_back(newC);
                auto targetClq=&graph[graph.size()-1].cond_pot_T2;
                auto singleClq=&graph[singleClqInd];
                for(auto &entry:graph[clq].cond_pot_T3)
                    if(singleClq->pot_T1.find(entry.first)!=singleClq->pot_T1.end()){
                        auto auxFreq=singleClq->pot_T1[entry.first];
                        for(auto &entry2:entry.second)
                            (*targetClq)[entry2.first[keyInd]][entry2.first[otherId]]+=entry2.second*auxFreq;
                    }
                graph[singleClqInd].disabled=true;
                if(shallICleanOldPots)
                    graph[singleClqInd].clear();   
            }
            return graph.size()-1;        
        }
        else if (graph[clq].otherVars.size()>2){// otherVars size >2  ==> T3
            if(singleClqInd==-1){
                clique newC;
                newC.pot_type="T3";
                newC.variableList=graph[clq].otherVars;
                newC.conditionedOnVar=graph[clq].otherVars[keyInd];
                for(int i=0; i<graph[clq].otherVars.size();i++)
                    if(i!=keyInd)
                        newC.otherVars.push_back(graph[clq].otherVars[i]);
                graph.push_back(newC);
                auto targetClq=&graph[graph.size()-1].cond_pot_T3;
                vector<int> key2(newC.otherVars.size());
                for(auto &entry:graph[clq].cond_pot_T3)
                    for(auto &entry2:entry.second) {
                        int cnt=0;
                        for(int i=0; i<entry2.first.size();i++)
                            if(i!=keyInd)
                                key2[cnt++]=entry2.first[i];
                        
                        (*targetClq)[entry2.first[keyInd]][key2]+=entry2.second;
                    }
            }
            else{ // single clq exists
                clique newC;
                newC.pot_type="T3";
                newC.variableList=graph[clq].otherVars;
                newC.conditionedOnVar=graph[clq].otherVars[keyInd];
                for(int i=0; i<graph[clq].otherVars.size();i++)
                    if(i!=keyInd)
                        newC.otherVars.push_back(graph[clq].otherVars[i]);
                graph.push_back(newC);
                auto targetClq=&graph[graph.size()-1].cond_pot_T3;
                auto singleClq=&graph[singleClqInd];
                vector<int> key2(newC.otherVars.size());
                for(auto &entry:graph[clq].cond_pot_T3)
                    if(singleClq->pot_T1.find(entry.first)!=singleClq->pot_T1.end()){
                        auto auxFreq=singleClq->pot_T1[entry.first];
                        for(auto &entry2:entry.second) {
                            int cnt=0;
                            for(int i=0; i<entry2.first.size();i++)
                                if(i!=keyInd)
                                    key2[cnt++]=entry2.first[i];

                            (*targetClq)[entry2.first[keyInd]][key2]+=entry2.second*auxFreq;
                        }
                    }
                        
            }
        }
    }
    
}

void PGM::sumProductV6(int clq1, int clq2, int singleClqIndx){
    cout<<"The join has composite JAs."<<endl<<"This implementation is not complete yet to support composite join!"<<endl;
    exit(1);
}

void PGM::sumProductV7(vector<int> clqIndx, bool summingOut, int singleClqIndx){
    cout<<"The join has composite JAs."<<endl<<"This implementation is not complete yet to support composite join!"<<endl;
    exit(1);
}

PGM::PGM(int clique_size){
        graph.resize(clique_size);
        
    }

void PGM::eliminateVariables(){
    
    for(int maxClq=0; maxClq<eliminationOrder.size();maxClq++){
        auto currVarset=eliminationOrder[maxClq];
        cout<< "The max clique number "<<maxClq<< " is being eliminated."<<endl;
        // find the related cliques
        auto clqList=findInGraph(currVarset);

        vector<int> singleClqDeletion;
        vector<int> singleClqElimination;
        vector <int> condClqIndx;
        int singleClqIndxDel;
        int singleClqIndxEli;
        for (auto &c:clqList){
            if(graph[c].pot_type== "T1" && graph[c].potFromDeletion==true)
                singleClqDeletion.push_back(c);
            else if(graph[c].pot_type== "T1" && graph[c].potFromDeletion==false)
                singleClqElimination.push_back(c);
            else
                condClqIndx.push_back(c);
        }
        
        if (singleClqDeletion.size()>1){
            singleClqIndxDel= product_T1_T1(singleClqDeletion); 
            graph[singleClqIndxDel].potFromDeletion=true;
        }
        else if(singleClqDeletion.size()==0)
            singleClqIndxDel=-1;
        else if(singleClqDeletion.size()==1)
            singleClqIndxDel=singleClqDeletion[0];
        
        if (singleClqElimination.size()>1){
            singleClqIndxEli= product_T1_T1(singleClqElimination);
            graph[singleClqIndxEli].potFromDeletion=false;
        }
        else if(singleClqElimination.size()==0)
            singleClqIndxEli=-1;
        else if(singleClqElimination.size()==1)
            singleClqIndxEli=singleClqElimination[0];
        
        
        //there should be just one or zero max clique in condClqIndx because in a JT every node has a single parent
        
       
        
        if(condClqIndx.size()>1){
            cout<< "something went wrong"<<endl;
            exit(1);
        }
        
        if(condClqIndx.size()>0 && graph[condClqIndx[0]].pot_type=="T3"){
            cout<<"not implemented yet."<<endl;
            exit(1);
        }
        else if(condClqIndx.size()>0 && graph[condClqIndx[0]].pot_type=="T2"){
            eliminate(condClqIndx[0],singleClqIndxEli,singleClqIndxDel);
        }
        else if(condClqIndx.size()==0 )
            eliminate(-1,singleClqIndxEli,singleClqIndxDel);
        else{
            cout<< "somthing went wrong"<<endl;
            exit(1);
        }
    }

}
void PGM:: eliminate(int srcMaxC,int child_factor_id, int extraBucketId){
   
//    deletionOrder={{0,"a"},{1,"d"},{2,"e"}};
//    eliminationOrder={{0,{"c"}},{1,{"b"}}};
//    graph.clear();
//    clique c;
//    c.pot_type="T2";
//    c.conditionedOnVar="a";
//    c.otherVars={"b"};
//    c.cond_pot_T2["a1"]={{"b1",10},{"b2",5}};
//    c.cond_pot_T2["a2"]={{"b1",4},{"b2",3}};
//    graph.push_back(c);
////    
//////    
//    clique c1;
//    c1.pot_type="T1";
//    c1.pot_T1={{"a1",5},{"a3",50}};
//    graph.push_back(c1);
//    clique c2;
//    c2.pot_type="T1";
//    c2.pot_T1={{"a1",10},{"a3",100}};
////
//    graph.push_back(c2);
//   
    if(srcMaxC==-1 &&  child_factor_id>-1 && extraBucketId==-1){
        unsigned long long joinSize=0;
        for(auto &entry:graph[child_factor_id].pot_T1)
            joinSize+=entry.second;
        cout<<"The join size is: "<< joinSize<<endl;
        return;
    }
    if(srcMaxC==-1 &&  child_factor_id>-1 && extraBucketId>-1){
        unsigned long long joinSize=0;
        auto ref=&graph[extraBucketId].pot_T1;
        for(auto &entry:graph[child_factor_id].pot_T1)
            if((*ref).find(entry.first)!=(*ref).end())
                joinSize+=entry.second * (*ref)[entry.first];
        cout<<"The join size is: "<< joinSize<<endl;
        return;
    }
    else if(srcMaxC==-1 &&  child_factor_id==-1 && extraBucketId>-1){
        unsigned long long joinSize=0;
        for(auto &entry:graph[extraBucketId].pot_T1)
            joinSize+=entry.second;
        cout<<"The join size is: "<< joinSize<<endl;
        return;
    }
    // if srcMax>-1  the following code will run
    
    if(child_factor_id==-1 && extraBucketId ==-1){ // no factor clique from children and no extra bucket from deletion
        
        gen_clique newG;
        newG.conditionedOnVar=graph[srcMaxC].otherVars[0];
        newG.otherVars.push_back(graph[srcMaxC].conditionedOnVar);
        newG.type="T2";
        generativeCliques.push_back(newG);
        auto target=&generativeCliques[generativeCliques.size()-1].cond_pot_T2;
        
        clique newC;
        newC.variableList={graph[srcMaxC].otherVars[0]};
        newC.pot_type="T1";
        newC.potFromDeletion=false;
        graph.push_back(newC);
        
        auto baseRef=&graph[srcMaxC].cond_pot_T2;
        auto fac_ref=&graph[graph.size()-1].pot_T1;
        for(auto &entry: (*baseRef)) // x->y
            for(auto &entry2:entry.second ){
                (*target)[entry2.first].push_back({entry.first,{entry2.second,1}});
                (*fac_ref)[entry2.first]+=entry2.second;
            }
//        cout<<""<<endl;
    }
    
    else if(child_factor_id>-1 && extraBucketId ==-1){// factor clique exists, but no extra bucket
        gen_clique newG;
        newG.conditionedOnVar=graph[srcMaxC].otherVars[0];
        newG.otherVars.push_back(graph[srcMaxC].conditionedOnVar);
        newG.type="T2";
        generativeCliques.push_back(newG);
        auto target=&generativeCliques[generativeCliques.size()-1].cond_pot_T2;
        
        
        clique newC;
        newC.variableList={graph[srcMaxC].otherVars[0]};
        newC.pot_type="T1";
        newC.potFromDeletion=false;
        graph.push_back(newC);
      
        
        auto baseRef=&graph[srcMaxC].cond_pot_T2;
        auto fac_ref=&graph[graph.size()-1].pot_T1;
        auto childFac=&graph[child_factor_id].pot_T1;
        
        for(auto &entry1:(*childFac))
            if((*baseRef).find(entry1.first)!=(*baseRef).end())
                for(auto &entry2:(*baseRef)[entry1.first]){
                    (*target)[entry2.first].push_back({entry1.first,{entry2.second,entry1.second}});
                    (*fac_ref)[entry2.first]+=entry2.second*entry1.second;
                }
                    
        graph[child_factor_id].disabled=true;
        if(shallICleanOldPots)
            graph[child_factor_id].clear();
    }
    
    else if(child_factor_id==-1 && extraBucketId >-1){// this is a leaf, so the extra bucket can be considered as frerqs
        child_factor_id=extraBucketId;
        gen_clique newG;
        newG.conditionedOnVar=graph[srcMaxC].otherVars[0];
        newG.otherVars.push_back(graph[srcMaxC].conditionedOnVar);
        newG.type="T2";
        generativeCliques.push_back(newG);
        auto target=&generativeCliques[generativeCliques.size()-1].cond_pot_T2;
        
        
        clique newC;
        newC.variableList={graph[srcMaxC].otherVars[0]};
        newC.pot_type="T1";
        newC.potFromDeletion=false;
        graph.push_back(newC);
      
        
        auto baseRef=&graph[srcMaxC].cond_pot_T2;
        auto fac_ref=&graph[graph.size()-1].pot_T1;
        auto childFac=&graph[child_factor_id].pot_T1;
        
        for(auto &entry1:(*childFac))
            if((*baseRef).find(entry1.first)!=(*baseRef).end())
                for(auto &entry2:(*baseRef)[entry1.first]){
                    (*target)[entry2.first].push_back({entry1.first,{entry2.second,entry1.second}});
                    (*fac_ref)[entry2.first]+=entry2.second*entry1.second;
                }
                    
        graph[child_factor_id].disabled=true;
        if(shallICleanOldPots)
            graph[child_factor_id].clear();
    }
    
    else if(child_factor_id>-1 && extraBucketId >-1){// both exists. extra bucket should be multiplied to the bucket of the cond clq

        gen_clique newG;
        newG.conditionedOnVar=graph[srcMaxC].otherVars[0];
        newG.otherVars.push_back(graph[srcMaxC].conditionedOnVar);
        newG.type="T2";
        generativeCliques.push_back(newG);
        auto target=&generativeCliques[generativeCliques.size()-1].cond_pot_T2;
        
        
        clique newC;
        newC.variableList={graph[srcMaxC].otherVars[0]};
        newC.pot_type="T1";
        newC.potFromDeletion=false;
        graph.push_back(newC);
        
        
        auto baseRef=&graph[srcMaxC].cond_pot_T2;
        auto fac_ref=&graph[graph.size()-1].pot_T1;
        auto childFac=&graph[child_factor_id].pot_T1;
        auto extraDel=&graph[extraBucketId].pot_T1;
        
        for(auto &entry1:(*childFac))
            if((*baseRef).find(entry1.first)!=(*baseRef).end() && (*extraDel).find(entry1.first)!=(*extraDel).end())
                for(auto &entry2:(*baseRef)[entry1.first]){
                    (*target)[entry2.first].push_back({entry1.first,{entry2.second*(*extraDel)[entry1.first],entry1.second}});
                    (*fac_ref)[entry2.first]+=entry2.second*entry1.second * (*extraDel)[entry1.first];
                }
                    
        graph[child_factor_id].disabled=true;
        if(shallICleanOldPots)
             graph[child_factor_id].clear();
        graph[extraBucketId].disabled=true;
        if(shallICleanOldPots)
            graph[extraBucketId].clear();
    }
    
    
    
    graph[srcMaxC].disabled=true;
    if(shallICleanOldPots)
        graph[srcMaxC].clear();
}


void PGM::generateResults(int mode, string out_add){

    // mode=0 =>  just generate the result in mem
    // mode=1 =>  store the freqs and read and make the result
    // mode =2 => make the result and store it and read it
    // both the modes 1 and 2
    
//    int root= generativeCliques.size()-1;
    levelClqs.push_back({-1});  // the root is the las clique in the main graph
    levelParents.push_back({-1});
    
    vector<int> newLevelClq;
    vector<int> newLevelParent;
    vector<int> newLevelExtraBucket;
    int cnt=0; 
    do{
        newLevelClq.clear();
        newLevelParent.clear();
        int pa_id=0;
        for(auto id:levelClqs[cnt]){
            vector<int> clqs;
            if(id==-1)
                clqs=findInGenGraph(graph[graph.size()-1].variableList[0]);
            else
                clqs=findInGenGraph(generativeCliques[id].otherVars[0]);
            
            for(auto idd:clqs)
                if(generativeCliques[idd].type!="T1") // not the root again
                {
                    newLevelClq.push_back(idd);
                    newLevelParent.push_back(pa_id); // id and idd are the index of the cliques in generativeCliques (-1, if it is in main Graph)
                        
                }
            pa_id++;
        }
        if(newLevelClq.size()>0){
            levelClqs.push_back(newLevelClq);
            levelParents.push_back(newLevelParent);
          
            cnt++;
        }
        
    }
    while (newLevelClq.size()>0);
    levelSize=levelClqs.size()-1;
    //start generation
    levelFreqs.resize(levelSize+1);
    headers.resize(levelSize+1);
    headers[0].push_back(graph[graph.size()-1].variableList[0]);
    for(int i=1; i<levelClqs.size();i++)
        for(auto &id:levelClqs[i])
            headers[i].push_back(generativeCliques[id].otherVars[0]);

    
    
    if (mode==5){
        auto start1 = std::chrono::system_clock::now();
         recursive_generation_noFreq(0, 1, {}); // generate the data directly with no freq table production
        auto end1 = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed = end1 - start1;
        std::cout << "Elapsed wall time for flat join in memory: " << elapsed.count() << "s"<<endl;
        levelFreqs.clear();
        levelFreqs.resize(levelSize+1);
        auto start11 = std::chrono::system_clock::now();
        recursive_generation(0, 1, {});
        auto end11 = std::chrono::system_clock::now();
        elapsed = end11 - start11;
        std::cout << "Elapsed wall time to calculate the summary : " << elapsed.count() << "s"<<endl;
        
        auto start2 = std::chrono::system_clock::now(); 
        write2Disk_1(out_add);
        auto end2 = std::chrono::system_clock::now();
         elapsed = end2 - start2;
        std::cout << "Elapsed wall time for writing freqs: " << elapsed.count() << "s"<<endl;
        elapsed= end2-start11;
        std::cout << "Elapsed wall time for cal summ and write in disk: " << elapsed.count() << "s"<<endl;
        
        auto start3 = std::chrono::system_clock::now();
        readFromDisk_1(out_add);
        auto end3 = std::chrono::system_clock::now();
        elapsed = end3 - start3;
        std::cout << "Elapsed wall time for reading the freqs and de-summarization: " << elapsed.count() << "s"<<endl;
    }
    else{
        auto start1 = std::chrono::system_clock::now();
        if(mode<4){
            recursive_generation(0, 1, {});
            auto end1 = std::chrono::system_clock::now();
            std::chrono::duration<double> elapsed = end1 - start1;
            std::cout << "Elapsed wall time to calculate the summary : " << elapsed.count() << "s"<<endl;
        }
        else if (mode==4){
            recursive_generation_noFreq(0, 1, {}); // generate the data directly with no freq table production
            auto end1 = std::chrono::system_clock::now();
            std::chrono::duration<double> elapsed = end1 - start1;
            std::cout << "Elapsed wall time for flat join in memory: " << elapsed.count() << "s"<<endl;
        }


        if(mode==0){
            auto start1 = std::chrono::system_clock::now();
            generate();
            auto end1 = std::chrono::system_clock::now();
            std::chrono::duration<double> elapsed = end1 - start1;
            std::cout << "Elapsed wall time for generating data: " << elapsed.count() << "s"<<endl;

        }
        if(mode==1){
            auto start1 = std::chrono::system_clock::now(); 
            write2Disk_1(out_add);
            auto end1 = std::chrono::system_clock::now();
            std::chrono::duration<double> elapsed = end1 - start1;
            std::cout << "Elapsed wall time for writing freqs: " << elapsed.count() << "s"<<endl;

            auto start2 = std::chrono::system_clock::now();
            readFromDisk_1(out_add);
            auto end2 = std::chrono::system_clock::now();
            elapsed = end2 - start2;
            std::cout << "Elapsed wall time for reading the freqs and de-summarization: " << elapsed.count() << "s"<<endl;

        }

        if(mode==2){
            auto start1 = std::chrono::system_clock::now();
            write2Disk_2(out_add);
            auto end1 = std::chrono::system_clock::now();
            auto elapsed = end1 - start1;
            std::cout << "Elapsed wall time for writing data: " << elapsed.count() << "s"<<endl;

            auto start2 = std::chrono::system_clock::now();
            readFromDisk_2(out_add);
            auto end2 = std::chrono::system_clock::now();
            elapsed = end2 - start2;
            std::cout << "Elapsed wall time for reading the data: " << elapsed.count() << "s"<<endl;

        }
        if(mode==3){
            auto start1 = std::chrono::system_clock::now(); 
            write2Disk_1(out_add);
            auto end1 = std::chrono::system_clock::now();
            std::chrono::duration<double> elapsed = end1 - start1;
            std::cout << "Elapsed wall time for writing freqs: " << elapsed.count() << "s"<<endl;

            auto start2 = std::chrono::system_clock::now();
            readFromDisk_1(out_add);
            auto end2 = std::chrono::system_clock::now();
            elapsed = end2 - start2;
            std::cout << "Elapsed wall time for reading the freqs and desummarization: " << elapsed.count() << "s"<<endl;

            start1 = std::chrono::system_clock::now();
            write2Disk_2(out_add);
            end1 = std::chrono::system_clock::now();
            elapsed = end1 - start1;
            std::cout << "Elapsed wall time for writing data: " << elapsed.count() << "s"<<endl;

            start2 = std::chrono::system_clock::now();
            readFromDisk_2(out_add);
            end2 = std::chrono::system_clock::now();
            elapsed = end2 - start2;
            std::cout << "Elapsed wall time for reading the data: " << elapsed.count() << "s"<<endl;
        }
    }
    
       
}

void PGM::recursive_generation(short int level, unsigned long long int parentBucket, vector<int> keys){
    
    
    if(level==0){ // we are at the root which comes from the main graph
        auto clqList=findInGraph(graph[graph.size()-1].variableList);

        int singleClqIndxDel=-1;
        int singleClqIndxEli=-1;

        for (auto &c:clqList){
            if(graph[c].pot_type== "T1" && graph[c].potFromDeletion==true)
                singleClqIndxDel=c;
            else if(graph[c].pot_type== "T1" && graph[c].potFromDeletion==false)
                singleClqIndxEli=c;
                
            
        }
        
        if(singleClqIndxDel==-1 && singleClqIndxEli>-1)
            for(auto &entry:graph[graph.size()-1].pot_T1){
                //cout<<entry.second<< " bucket of the  "<<entry.first<<endl;
                levelFreqs[0].push_back(entry.first);
                levelFreqs[0].push_back(entry.second);
                
                if(level<levelSize)
                    recursive_generation(1, 1, {entry.first});
            }
        else if(singleClqIndxDel>-1 && singleClqIndxEli>-1){
            auto buc=&graph[singleClqIndxDel].pot_T1;
            for(auto &entry:graph[graph.size()-1].pot_T1){
                if( (*buc).find(entry.first)!=(*buc).end() ){
                    //cout<<entry.second * (*buc)[entry.first]<< " bucket of the  "<<entry.first<<endl;
                    levelFreqs[0].push_back(entry.first);
                    levelFreqs[0].push_back(entry.second * (*buc)[entry.first]);
                    if(level<levelSize)
                        recursive_generation(1, (*buc)[entry.first], {entry.first});
                }
            }
        }
        else if(singleClqIndxDel>-1 && singleClqIndxEli==-1)
            for(auto &entry:graph[singleClqIndxDel].pot_T1){
                //cout<<entry.second<< " bucket of the  "<<entry.first<<endl;
                levelFreqs[0].push_back(entry.first);
                levelFreqs[0].push_back(entry.second);
                
                if(level<levelSize)
                    recursive_generation(1, 1, {entry.first});
            }
    }

    else{ // we are in intermediate levels
  
        if(levelClqs[level].size()==1 ){//we have one parent 
            
            for(auto &entry: generativeCliques[levelClqs[level][0]].cond_pot_T2[keys[levelParents[level][0]]]){
                //cout<<entry.second[0] *entry.second[1] * parentBucket << " of the  "<<entry.first<<endl;
                levelFreqs[level].push_back(entry.first);
                levelFreqs[level].push_back(entry.second[0] *entry.second[1] * parentBucket);
                if(level<levelSize)
                    recursive_generation(level+1,entry.second[0] * parentBucket, {entry.first});
            }
        }

        else if(levelClqs[level].size()==2){
            for(auto &entry1: generativeCliques[levelClqs[level][0]].cond_pot_T2[keys[levelParents[level][0]]])
                for(auto &entry2: generativeCliques[levelClqs[level][1]].cond_pot_T2[keys[levelParents[level][1]]]){
                    unsigned long long int newBucket= entry1.second[0]*entry2.second[0] * parentBucket;
                    //cout<< entry1.first <<" and "<< entry2.first<< " , with size of:"<< newBucket * entry1.second[1]*  entry2.second[1]<<endl;
                    levelFreqs[level].push_back(entry1.first);
                    levelFreqs[level].push_back(entry2.first);
                    levelFreqs[level].push_back(newBucket * entry1.second[1]*  entry2.second[1]);
                    if(level<levelSize)
                        recursive_generation(level+1,newBucket, {entry1.first,entry2.first});
                }
        }
        else if(levelClqs[level].size()==3){
            for(auto &entry1: generativeCliques[levelClqs[level][0]].cond_pot_T2[keys[levelParents[level][0]]])
                for(auto &entry2: generativeCliques[levelClqs[level][1]].cond_pot_T2[keys[levelParents[level][1]]])
                    for(auto &entry3: generativeCliques[levelClqs[level][2]].cond_pot_T2[keys[levelParents[level][2]]]){
                        unsigned long long int newBucket=entry1.second[0]*entry2.second[0]* entry3.second[0] * parentBucket;
                        //cout<< entry1.first <<" and "<< entry2.first<<" and "<< entry3.first<< " , with size of:"<< newBucket * entry1.second[1]*  entry2.second[1]*  entry3.second[1]<<endl;
                        levelFreqs[level].push_back(entry1.first);
                        levelFreqs[level].push_back(entry2.first);
                        levelFreqs[level].push_back(entry3.first);
                        levelFreqs[level].push_back(newBucket * entry1.second[1]*  entry2.second[1]*  entry3.second[1]);
                        if(level<levelSize)
                            recursive_generation(level+1,newBucket, {entry1.first,entry2.first, entry3.first});
                    }
        }
        else if(levelClqs[level].size()==4){
            for(auto &entry1: generativeCliques[levelClqs[level][0]].cond_pot_T2[keys[levelParents[level][0]]])
                for(auto &entry2: generativeCliques[levelClqs[level][1]].cond_pot_T2[keys[levelParents[level][1]]])
                    for(auto &entry3: generativeCliques[levelClqs[level][2]].cond_pot_T2[keys[levelParents[level][2]]])
                        for(auto &entry4: generativeCliques[levelClqs[level][3]].cond_pot_T2[keys[levelParents[level][3]]]){
                            unsigned long long int newBucket=entry1.second[0]*entry2.second[0]* entry3.second[0] * entry4.second[0] * parentBucket;
                           // cout<< entry1.first <<" and "<< entry2.first<<" and "<< entry3.first<< " , with size of:"<< newBucket * entry1.second[1]*  entry2.second[1]*  entry3.second[1] * entry4.second[1]<<endl;
                            levelFreqs[level].push_back(entry1.first);
                            levelFreqs[level].push_back(entry2.first);
                            levelFreqs[level].push_back(entry3.first);
                            levelFreqs[level].push_back(entry4.first);
                            levelFreqs[level].push_back( newBucket * entry1.second[1]*  entry2.second[1]*  entry3.second[1] * entry4.second[1]);
                            if(level<levelSize)
                                recursive_generation(level+1,newBucket, {entry1.first,entry2.first, entry3.first, entry4.first});
                    }
        }
    }
}


void PGM::write2Disk_1(string out_add)
{

    
    for (int i=0; i<levelFreqs.size();i++)
    {
        bool write=true;
        for(auto hed:headers[i]){
            auto it = find(outputVars.begin(), outputVars.end(), hed);
            if (it == outputVars.end())
                write=false;
        }
        if(write){    
            string fileName=""+out_add;
            for(auto hed:headers[i])
                fileName+="_"+hed;
            fileName+="_att.txt";
            ofstream output_file(fileName);

            ostream_iterator<long long int> output_iterator(output_file, "|");
            copy(levelFreqs[i].begin(), levelFreqs[i].end(), output_iterator);
            output_file.close();
        }
    }
    
}

void PGM::write2Disk_2(string out_add){

    for (int i=0; i<levelFreqs.size();i++){
        bool write=true;
        for(auto hed:headers[i]){
            auto it = find(outputVars.begin(), outputVars.end(), hed);
            if (it == outputVars.end())
                write=false;
        }
        if(write){ 
            if(headers[i].size()==1){
                string fileName=""+out_add;
                fileName+="_"+headers[i][0]+"_data.txt";
                ofstream output_file(fileName);
                ostream_iterator<long long int> output_iterator(output_file, "\n");
                for(int j=0; j<levelFreqs[i].size();j+=2){
                    vector<unsigned long long int> tmp;
                    tmp.insert(tmp.end(),levelFreqs[i][j+1], levelFreqs[i][j]);
                    copy(tmp.begin(), tmp.end(), output_iterator);
                }
                output_file.close();
            }
        
            if(headers[i].size()>1){
                int h_s=headers[i].size();
                for(int  hed=0; hed<h_s;hed++){
                    string fileName=""+out_add;
                    fileName+="_"+headers[i][hed]+"_data.txt";
                    ofstream output_file(fileName);
                    ostream_iterator<unsigned long long int> output_iterator(output_file, "|");

                    for(int j=hed; j<levelFreqs[i].size();j+=h_s+1){

                        vector<unsigned long long int> tmp;
                        tmp.insert(tmp.end(),levelFreqs[i][j+h_s-hed], levelFreqs[i][j]);
                   
                        copy(tmp.begin(), tmp.end(), output_iterator);
                        output_file<<"\n";
                    }
                    output_file.close();
                }

            }
        }
    }
      
    
}

void PGM::readFromDisk_1(string in_add){ // use summarization


        
    std::string line = "";
    for (auto atts:headers){
        bool read=true;
        for(auto hed:atts){
            auto it = find(outputVars.begin(), outputVars.end(), hed);
            if (it == outputVars.end())
                read=false;
        }
        if(read){
            if(atts.size()==1){
                string add=in_add+"_" + atts[0]+"_att.txt";
                std::ifstream file(add);
                if (!file.good())
                    continue;

                getline(file, line);
                string::const_iterator start = line.begin();
                string::const_iterator end = line.end();
                string::const_iterator next = std::find(start, end, '|');
                int key;
                unsigned long long int value;
                while (next != end)
                {
                    key = stoi(string(start, next));
                    start = next + 1;
                    next = find(start, end, '|');
                    value=stoll(string(start, next));
                    start = next + 1;
                    next = find(start, end, '|');

                    
                    if(value<max_vec_initialization){
                        vector <int> tmp;
                        tmp.assign(value, key);
                    }
                    else{
                        for(int i=0;i<value / max_vec_initialization;i++){
                            vector <int> tmp;
                            tmp.assign(max_vec_initialization, key);
                            
                        }
                        vector <int> tmp;
                        tmp.assign(value % max_vec_initialization, key); // remaining
                    }

                }
                cout<< "Att "<<atts[0]<< " loaded."<<endl;
            }
            else if(atts.size()==2){
                string add=in_add+"_" + atts[0]+"_"+ atts[1]+"_att.txt";
                std::ifstream file(add);
                if (!file.good())
                    continue;

                getline(file, line);
                string::const_iterator start = line.begin();
                string::const_iterator end = line.end();
                string::const_iterator next = std::find(start, end, '|');
                int key1, key2;
                unsigned long long int value;
                while (next != end)
                {
                    key1 = stoi(string(start, next));
                    start = next + 1;
                    next = find(start, end, '|');

                    key2 = stoi(string(start, next));
                    start = next + 1;
                    next = find(start, end, '|');

                    value=stoll(string(start, next));
                    start = next + 1;
                    next = find(start, end, '|');

                    
                    if(value<max_vec_initialization){
//                        vector<vector<int>> tmp;
//                        tmp.assign(value, {key1,key2});
                        vector<int> tmp;
                        tmp.assign(value, key1);
                        tmp.assign(value, key2);
                    }
                    else{
                        for(int i=0;i<value / max_vec_initialization;i++){
//                            vector<vector<int>> tmp;
//                            tmp.assign(max_vec_initialization, {key1,key2});
                            vector<int> tmp;
                            tmp.assign(max_vec_initialization, key1);
                            tmp.assign(max_vec_initialization, key2);
                           
                        }
//                        vector<vector<int>> tmp;
//                        tmp.assign(value % max_vec_initialization, {key1,key2}); // remaining
                        vector<int> tmp;
                        tmp.assign(value % max_vec_initialization, key1); // remaining
                        tmp.assign(value % max_vec_initialization, key2); // remaining
                    }
                   

                }
                cout<< "Att "<<atts[0]<<", and " <<atts[1]<< " loaded."<<endl;
            }
            else if(atts.size()==3){
                string add=in_add+"_" + atts[0]+"_"+ atts[1]+"_"+ atts[2]+"_att.txt";
                std::ifstream file(add);
                if (!file.good())
                    continue;

                getline(file, line);
                string::const_iterator start = line.begin();
                string::const_iterator end = line.end();
                string::const_iterator next = std::find(start, end, '|');
                int key1, key2, key3;
                unsigned long long int value;
                while (next != end)
                {
                    key1 = stoi(string(start, next));
                    start = next + 1;
                    next = find(start, end, '|');

                    key2 = stoi(string(start, next));
                    start = next + 1;
                    next = find(start, end, '|');

                    key3 = stoi(string(start, next));
                    start = next + 1;
                    next = find(start, end, '|');

                    value=stoll(string(start, next));
                    start = next + 1;
                    next = find(start, end, '|');

                    vector<vector<int>> tmp;
                    if(value<max_vec_initialization){
//                        vector<vector<int>> tmp;
//                        tmp.assign(value, {key1,key2,key3});
                        vector<int> tmp;
                        tmp.assign(value, key1);
                        tmp.assign(value, key2);
                        tmp.assign(value, key3);
                    }
                    else{
                        for(int i=0;i<value / max_vec_initialization;i++){
//                            vector<vector<int>> tmp;
//                            tmp.assign(max_vec_initialization, {key1,key2,key3});
                            vector<int> tmp;
                            tmp.assign(max_vec_initialization, key1);
                            tmp.assign(max_vec_initialization, key2);
                            tmp.assign(max_vec_initialization, key3);
                            
                           
                        }
//                        vector<vector<int>> tmp;
//                        tmp.assign(value % max_vec_initialization, {key1,key2,key3}); // remaining
                        vector<int> tmp;
                        tmp.assign(value % max_vec_initialization, key1);
                        tmp.assign(value % max_vec_initialization, key2);
                        tmp.assign(value % max_vec_initialization, key3);
                      
                    }

                }
                cout<< "Att "<<atts[0]<<", and " <<atts[1]<<", and " <<atts[2]<< " loaded."<<endl;
            }
            else if(atts.size()==4){
                string add=in_add+"_" + atts[0]+"_"+ atts[1]+"_"+ atts[2]+"_"+ atts[3]+"_att.txt";
                std::ifstream file(add);
                if (!file.good())
                    continue;

                getline(file, line);
                string::const_iterator start = line.begin();
                string::const_iterator end = line.end();
                string::const_iterator next = std::find(start, end, '|');
                int key1, key2, key3,key4;
                unsigned long long int value;
                while (next != end)
                {
                    key1 = stoi(string(start, next));
                    start = next + 1;
                    next = find(start, end, '|');

                    key2 = stoi(string(start, next));
                    start = next + 1;
                    next = find(start, end, '|');

                    key3 = stoi(string(start, next));
                    start = next + 1;
                    next = find(start, end, '|');

                    key4 = stoi(string(start, next));
                    start = next + 1;
                    next = find(start, end, '|');

                    value=stoll(string(start, next));
                    start = next + 1;
                    next = find(start, end, '|');

                    vector<vector<int>> tmp;
                
                    if(value<max_vec_initialization){
//                        vector<vector<int>> tmp;
//                        tmp.assign(value, {key1,key2,key3,key4});
                        vector<int> tmp;
                        tmp.assign(value, key1);
                        tmp.assign(value, key2);
                        tmp.assign(value, key3);
                        tmp.assign(value, key4);
                    }
                    else{
                        for(int i=0;i<value / max_vec_initialization;i++){
//                            vector<vector<int>> tmp;
//                            tmp.assign(max_vec_initialization, {key1,key2,key3,key4});
                            vector<int> tmp;
                            tmp.assign(max_vec_initialization, key1);
                            tmp.assign(max_vec_initialization, key2);
                            tmp.assign(max_vec_initialization, key3);
                            tmp.assign(max_vec_initialization, key4);
                         
                        }
//                        vector<vector<int>> tmp;
//                        tmp.assign(value % max_vec_initialization, {key1,key2,key3,key4}); // remaining
                        vector<int> tmp;
                         tmp.assign(value % max_vec_initialization, key1);
                        tmp.assign(value % max_vec_initialization, key2);
                        tmp.assign(value % max_vec_initialization, key3);
                        tmp.assign(value % max_vec_initialization, key4);
                    }

                }
                cout<< "Att "<<atts[0]<<", and " <<atts[1]<<", and " <<atts[2]<<", and " <<atts[3]<< " loaded."<<endl;
            }

        }
    } 
            
    
}

void PGM::readFromDisk_2(string in_add){ // use summarization

        
    std::string line = "";
    for (auto atts:headers)
        for(auto att:atts){
            bool read=true;
            auto it = find(outputVars.begin(), outputVars.end(), att);
            if (it == outputVars.end())
                read=false;
            if(read){
            
                string add=in_add+"_" + att+"_data.txt";
                 std::ifstream file(add);
                 if (!file.good())
                     continue;

                 while (getline(file, line)){
                    string::const_iterator start = line.begin();
                    string::const_iterator end = line.end();
                    string::const_iterator next = std::find(start, end, '|');
                    vector<int> tmp;
                    int key;

                    while (next != end)
                    {
                        key = stoi(string(start, next));
                        start = next + 1;
                        next = find(start, end, '|');
                        tmp={key};

                    }
                 }
                 cout<< "Att "<<att<< " loaded."<<endl;
            }
        }
}
void PGM::generate(){

    for (int i=0; i<levelFreqs.size();i++){
        bool gen=true;
        bool alterData=false;
        for(auto hed:headers[i]){
            auto it = find(outputVars.begin(), outputVars.end(), hed);
            if (it == outputVars.end())
                gen=false;
            
        }

        if(gen ){ 
            if(headers[i].size()==1){
   
                for(int j=0; j<levelFreqs[i].size();j+=2){
                    vector<int> tmp;
                    
                    unsigned long long int value=levelFreqs[i][j+1];
                    int key=levelFreqs[i][j];
                    if(value<max_vec_initialization){
                        vector <int> tmp;
                        tmp.assign(value, key);
                    }
                    else{
                        for(int i=0;i<value / max_vec_initialization;i++){
                            vector <int> tmp;
                            tmp.assign(max_vec_initialization, key);
                            
                        }
                        vector <int> tmp;
                        tmp.assign(value % max_vec_initialization, key); // remaining
                    }
             
                }
            
            }
        
            if(headers[i].size()>1){
                int h_s=headers[i].size();
                for(int  hed=0; hed<h_s;hed++){
   
                    for(int j=hed; j<levelFreqs[i].size();j+=h_s+1){

//                        vector<string> tmp;
//                        tmp.insert(tmp.end(),stoll(levelFreqs[i][j+h_s-hed]), levelFreqs[i][j]);
                      
                        unsigned long long int value=levelFreqs[i][j+h_s-hed];
                        int key=levelFreqs[i][j];
                        if(value<max_vec_initialization){
                            vector <int> tmp;
                            tmp.assign(value, key);
                        }
                        else{
                            for(int i=0;i<value / max_vec_initialization;i++){
                                vector <int> tmp;
                                tmp.assign(max_vec_initialization, key);

                            }
                            vector <int> tmp;
                            tmp.assign(value % max_vec_initialization, key); // remaining
                        }
                    } 
                }
            }
        }

    } 
}

void clique::cliqueQueryCSV( string tableADD, vector<string> groupingAtts, unordered_map<int, vector<string>> eliminationOrder, unordered_map<int, string> deletionOrder, vector<string> JAs, bool shallIcleanRawData){
    
    if(eliminationOrder.size()==0){
        cout<<"Set the elimination and deletion order before using csv query."<<endl;
        exit(1);
    }
    string nonJA;
    variableList=groupingAtts;
    loadRawData(tableADD,variableList, rawData, '|');
    // merge all the non JAs together
    bool alterData=false;
    vector<vector<string>> rawMaxData;
    if(variableList.size()-JAs.size()>1){ //more than one non JA exists
        auto start = std::chrono::system_clock::now();
        alterData=true;
        vector <int> nonJAsInd;
        vector <int> JAsInd;
        vector<string> newJAs; // to make correct order of JAs based on variableList
        
        int ind=0;
        for(auto &att:variableList){
            auto it = find(JAs.begin(), JAs.end(), att);
            if (it == JAs.end()){
                nonJAsInd.push_back(ind);
                nonJA+=att +"|";
            }
            else{
                JAsInd.push_back(ind);
                newJAs.push_back(att);
            }
            ind++;
        }
        
        nonJA=nonJA.substr(0,nonJA.size()-1);
        
        auto int2str=&int2strNonJAs[nonJA];
        auto str2int=&str2intNonJAs[nonJA];
        unsigned long int new_index=0;
        
        variableList=newJAs;
        variableList.push_back(nonJA);
        rawMaxData.resize( rawData.size() , vector<string> (variableList.size()));
        int rowId=0;
        for(auto &row:rawData){
            int col_id=0;
            for(auto &id:JAsInd)
                rawMaxData[rowId][col_id++]=row[id];
            string star;
            for(auto &id:nonJAsInd)
                star+=row[id]+"|";
            if((*str2int).find(star)!=(*str2int).end())
                rawMaxData[rowId][col_id]=to_string((*str2int)[star]);
            else{
                (*str2int)[star]=new_index;
                (*int2str)[new_index]=star;
                rawMaxData[rowId][col_id]=to_string(new_index);
                new_index++;
            }
            rowId++;
        }
        auto end = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed = end - start;
        std::cout << "\nElapsed wall time for merging nonJAs (it may not be included in processing time): " << elapsed.count() << "s"<<endl;
    }

    
    if(variableList.size()==1){
        pot_type="T1";
        conditionedOnVar=variableList[0];
        if(!alterData)
            for (auto &row : rawData)
                pot_T1[stoi(row[0])] += 1; 
        else
            for (auto &row : rawMaxData)
                pot_T1[stoi(row[0])] += 1;
    }
    else{
        
        unordered_map<string,int> elor;
        for(auto &delo: deletionOrder)
            elor[delo.second]=delo.first;
        for(auto &delo: eliminationOrder)
            for(auto &sec:delo.second)
                elor[sec]=delo.first+deletionOrder.size();
        
        if(variableList.size()==2){
            pot_type="T2";
          
            if(elor[variableList[0]]<elor[variableList[1]]){
                conditionedOnVar=variableList[0];
                otherVars.push_back(variableList[1]);
                if(!alterData){
                    for (auto & row : rawData)
                         cond_pot_T2[stoi(row[0])][stoi(row[1])] += 1;
                }
                else{
                    for (auto & row : rawMaxData)
                         cond_pot_T2[stoi(row[0])][stoi(row[1])] += 1;
                }
            }
            else if(elor[variableList[0]]>elor[variableList[1]]){
                conditionedOnVar=variableList[1];
                otherVars.push_back(variableList[0]);
                if(!alterData){
                    for (auto & row : rawData)
                        cond_pot_T2[stoi(row[1])][stoi(row[0])] += 1;
                }
                else{
                    for (auto & row : rawMaxData)
                        cond_pot_T2[stoi(row[1])][stoi(row[0])] += 1;
                }
            }     
        }
        else if(variableList.size()>2){
            pot_type="T3";
            int min= elor[variableList[0]];
            int min_id=0;
            for(int i=0; i< variableList.size();i++)
                if(elor[variableList[i]]<min){
                    min=elor[variableList[i]];
                    min_id=i;
                }
            conditionedOnVar=variableList[min_id];
            for(int i=0; i<variableList.size();i++)
                if(i!=min_id)    
                    otherVars.push_back(variableList[i]);
            
            int varSize=variableList.size();
            int keyIndx=findInVec(variableList, conditionedOnVar).second;
            vector<int> otherIndx;
            for (auto &att: otherVars)
                otherIndx.push_back(findInVec(variableList, att).second);
            if(!alterData)
                for (auto & row : rawData) {
                    int key=stoi(row[keyIndx]);
                    vector<int> tmp;
                    for(int i=0; i< otherVars.size(); i++)
                        tmp.push_back( stoi(row[otherIndx[i]]));
                    cond_pot_T3[key][tmp]+=1;

                }
            else
                for (auto & row : rawMaxData) {
                    int key=stoi(row[keyIndx]);
                    vector<int> tmp;
                    for(int i=0; i< otherVars.size(); i++)
                        tmp.push_back( stoi(row[otherIndx[i]]));
                    cond_pot_T3[key][tmp]+=1;

                }
        }
    }
    
    if(shallIcleanRawData){
        rawData.clear();
        rawMaxData.clear();
        str2intNonJAs[nonJA].clear();
    }
    if(!alterData)
        cout<< "Done with table "<<tableADD<<", size= "<<rawData.size()<<endl;
    else
        cout<< "Done with table "<<tableADD<<", size= "<<rawMaxData.size()<<endl;
}

//void PGM::recursive_generation_2(short int level, unsigned long long int parentBucket, vector<string> keys, vector<string> row){
//    
//    
//    if(level==0){ // we are at the root which comes from the main graph
//        auto clqList=findInGraph(graph[graph.size()-1].variableList);
//
//        int singleClqIndxDel=-1;
//        int singleClqIndxEli=-1;
//
//        for (auto &c:clqList){
//            if(graph[c].pot_type== "T1" && graph[c].potFromDeletion==true)
//                singleClqIndxDel=c;
//            else if(graph[c].pot_type== "T1" && graph[c].potFromDeletion==false)
//                singleClqIndxEli=c;
//                
//            
//        }
//        
//        if(singleClqIndxDel==-1 && singleClqIndxEli>-1)
//            for(auto &entry:graph[graph.size()-1].pot_T1){
//                //cout<<entry.second<< " bucket of the  "<<entry.first<<endl;
////                levelFreqs[0].push_back(entry.first);
////                levelFreqs[0].push_back(to_string(entry.second));
//                
//                if(level<levelSize)
//                    recursive_generation_2(1, 1, {entry.first},{entry.first});
//                else{
//                    vector<string> tmp;
//                    tmp.insert(tmp.end(),entry.second, entry.first );
//                }
//            }
//        else if(singleClqIndxDel>-1 && singleClqIndxEli>-1){
//            auto buc=&graph[singleClqIndxDel].pot_T1;
//            for(auto &entry:graph[graph.size()-1].pot_T1){
//                if( (*buc).find(entry.first)!=(*buc).end() ){
//                    //cout<<entry.second * (*buc)[entry.first]<< " bucket of the  "<<entry.first<<endl;
////                    levelFreqs[0].push_back(entry.first);
////                    levelFreqs[0].push_back(to_string(entry.second * (*buc)[entry.first]));
//                    if(level<levelSize)
//                        recursive_generation_2(1, (*buc)[entry.first], {entry.first},{entry.first});
//                        else{
//                            vector<string> tmp;
//                            tmp.insert(tmp.end(),entry.second * (*buc)[entry.first], entry.first );
//                        }
//                }
//            }
//        }
//        else if(singleClqIndxDel>-1 && singleClqIndxEli==-1)
//            for(auto &entry:graph[singleClqIndxDel].pot_T1){
//                //cout<<entry.second<< " bucket of the  "<<entry.first<<endl;
////                levelFreqs[0].push_back(entry.first);
////                levelFreqs[0].push_back(to_string(entry.second));
////                
//                if(level<levelSize)
//                    recursive_generation_2(1, 1, {entry.first},{entry.first});
//                else{
//                    vector<string> tmp;
//                    tmp.insert(tmp.end(),entry.second, entry.first );
//                }    
//            }
//    }
//
//    else{ // we are in intermediate levels
//  
//        if(levelClqs[level].size()==1 ){//we have one parent 
//            
//            for(auto &entry: generativeCliques[levelClqs[level][0]].cond_pot_T2[keys[levelParents[level][0]]]){
//                //cout<<entry.second[0] *entry.second[1] * parentBucket << " of the  "<<entry.first<<endl;
////                levelFreqs[level].push_back(entry.first);
////                levelFreqs[level].push_back(to_string(entry.second[0] *entry.second[1] * parentBucket));
//                row.push_back(entry.first);
//                if(level<levelSize)
//                    recursive_generation_2(level+1,entry.second[0] * parentBucket, {entry.first},row);
//                else{
//                    vector<vector<string>> tmp;
//                    tmp.insert(tmp.end(),entry.second[0] *entry.second[1] * parentBucket, row );
//                }
//            }
//        }
//
//        else if(levelClqs[level].size()==2){
//            for(auto &entry1: generativeCliques[levelClqs[level][0]].cond_pot_T2[keys[levelParents[level][0]]])
//                for(auto &entry2: generativeCliques[levelClqs[level][1]].cond_pot_T2[keys[levelParents[level][1]]]){
//                    unsigned long long int newBucket= entry1.second[0]*entry2.second[0] * parentBucket;
//                    //cout<< entry1.first <<" and "<< entry2.first<< " , with size of:"<< newBucket * entry1.second[1]*  entry2.second[1]<<endl;
////                    levelFreqs[level].push_back(entry1.first);
////                    levelFreqs[level].push_back(entry2.first);
////                    levelFreqs[level].push_back(to_string(newBucket * entry1.second[1]*  entry2.second[1]));
//                    row.push_back(entry1.first);row.push_back(entry2.first);
//                    if(level<levelSize)
//                        recursive_generation_2(level+1,newBucket, {entry1.first,entry2.first},row);
//                    else{
//                        vector<vector<string>> tmp;
//                        tmp.insert(tmp.end(),newBucket * entry1.second[1]*  entry2.second[1], row );
//                    }
//                }
//        }
//        else if(levelClqs[level].size()==3){
//            for(auto &entry1: generativeCliques[levelClqs[level][0]].cond_pot_T2[keys[levelParents[level][0]]])
//                for(auto &entry2: generativeCliques[levelClqs[level][1]].cond_pot_T2[keys[levelParents[level][1]]])
//                    for(auto &entry3: generativeCliques[levelClqs[level][2]].cond_pot_T2[keys[levelParents[level][2]]]){
//                        unsigned long long int newBucket=entry1.second[0]*entry2.second[0]* entry3.second[0] * parentBucket;
//                        //cout<< entry1.first <<" and "<< entry2.first<<" and "<< entry3.first<< " , with size of:"<< newBucket * entry1.second[1]*  entry2.second[1]*  entry3.second[1]<<endl;
////                        levelFreqs[level].push_back(entry1.first);
////                        levelFreqs[level].push_back(entry2.first);
////                        levelFreqs[level].push_back(entry3.first);
////                        levelFreqs[level].push_back(to_string(newBucket * entry1.second[1]*  entry2.second[1]*  entry3.second[1]));
//                        row.push_back(entry1.first);row.push_back(entry2.first);row.push_back(entry3.first);
//                        if(level<levelSize)
//                            recursive_generation_2(level+1,newBucket, {entry1.first,entry2.first, entry3.first},row);
//                        else{
//                            vector<vector<string>> tmp;
//                            tmp.insert(tmp.end(),newBucket * entry1.second[1]*  entry2.second[1]*  entry3.second[1], row );
//                        }
//                    }
//        }
//        else if(levelClqs[level].size()==4){
//            for(auto &entry1: generativeCliques[levelClqs[level][0]].cond_pot_T2[keys[levelParents[level][0]]])
//                for(auto &entry2: generativeCliques[levelClqs[level][1]].cond_pot_T2[keys[levelParents[level][1]]])
//                    for(auto &entry3: generativeCliques[levelClqs[level][2]].cond_pot_T2[keys[levelParents[level][2]]])
//                        for(auto &entry4: generativeCliques[levelClqs[level][3]].cond_pot_T2[keys[levelParents[level][3]]]){
//                            unsigned long long int newBucket=entry1.second[0]*entry2.second[0]* entry3.second[0] * entry4.second[0] * parentBucket;
//                           // cout<< entry1.first <<" and "<< entry2.first<<" and "<< entry3.first<< " , with size of:"<< newBucket * entry1.second[1]*  entry2.second[1]*  entry3.second[1] * entry4.second[1]<<endl;
//                            row.push_back(entry1.first);
//                            row.push_back(entry2.first);
//                            row.push_back(entry3.first);
//                            row.push_back(entry4.first);
////                            levelFreqs[level].push_back(to_string( newBucket * entry1.second[1]*  entry2.second[1]*  entry3.second[1] * entry4.second[1]));
//                            if(level<levelSize)
//                                recursive_generation_2(level+1,newBucket, {entry1.first,entry2.first, entry3.first, entry4.first},row);
//                             else{
//                                vector<vector<string>> tmp;
//                                tmp.insert(tmp.end(),newBucket * entry1.second[1]*  entry2.second[1]*  entry3.second[1] * entry4.second[1], row );
//                            }   
//                    }
//        }
//    }
//}

void PGM::recursive_generation_noFreq(short int level, unsigned long long int parentBucket, vector<int> keys){
    
    
    if(level==0){ // we are at the root which comes from the main graph
        auto clqList=findInGraph(graph[graph.size()-1].variableList);

        int singleClqIndxDel=-1;
        int singleClqIndxEli=-1;

        for (auto &c:clqList){
            if(graph[c].pot_type== "T1" && graph[c].potFromDeletion==true)
                singleClqIndxDel=c;
            else if(graph[c].pot_type== "T1" && graph[c].potFromDeletion==false)
                singleClqIndxEli=c;
                
            
        }
        
        if(singleClqIndxDel==-1 && singleClqIndxEli>-1)
            for(auto &entry:graph[graph.size()-1].pot_T1){
                //cout<<entry.second<< " bucket of the  "<<entry.first<<endl;
                
                if(entry.second<max_vec_initialization){
                      levelFreqs[0].assign(entry.second,entry.first ); //delete the oldests to avoid mem issues  
                }
                else{
                    for(int i=0;i<entry.second / max_vec_initialization;i++)
                        levelFreqs[0].assign(max_vec_initialization,entry.first );
                    levelFreqs[0].assign(entry.second % max_vec_initialization,entry.first );
                }
                if(level<levelSize)
                    recursive_generation_noFreq(1, 1, {entry.first});
            }
        else if(singleClqIndxDel>-1 && singleClqIndxEli>-1){
            auto buc=&graph[singleClqIndxDel].pot_T1;
            for(auto &entry:graph[graph.size()-1].pot_T1){
                if( (*buc).find(entry.first)!=(*buc).end() ){
                    //cout<<entry.second * (*buc)[entry.first]<< " bucket of the  "<<entry.first<<endl;
            
                    unsigned long long int value=entry.second * (*buc)[entry.first];
                    
                    if(value<max_vec_initialization){
                            levelFreqs[0].assign(value,entry.first ); //delete the oldests to avoid mem issues  
                    }
                    else{
                          for(int i=0;i<value / max_vec_initialization;i++)
                              levelFreqs[0].assign(max_vec_initialization,entry.first );
                          levelFreqs[0].assign(value% max_vec_initialization,entry.first );
                    }
                    if(level<levelSize)
                        recursive_generation_noFreq(1, (*buc)[entry.first], {entry.first});
                }
            }
        }
        else if(singleClqIndxDel>-1 && singleClqIndxEli==-1)
            for(auto &entry:graph[singleClqIndxDel].pot_T1){
                //cout<<entry.second<< " bucket of the  "<<entry.first<<endl;
                if(entry.second<max_vec_initialization){
                      levelFreqs[0].assign(entry.second,entry.first ); //delete the oldests to avoid mem issues  
                }
                else{
                    for(int i=0;i<entry.second / max_vec_initialization;i++)
                        levelFreqs[0].assign(max_vec_initialization,entry.first );
                    levelFreqs[0].assign(entry.second % max_vec_initialization,entry.first );
                }
                if(level<levelSize)
                    recursive_generation_noFreq(1, 1, {entry.first});
            }
    }

    else{ // we are in intermediate levels
  
        if(levelClqs[level].size()==1 ){//we have one parent 
            
            for(auto &entry: generativeCliques[levelClqs[level][0]].cond_pot_T2[keys[levelParents[level][0]]]){
                //cout<<entry.second[0] *entry.second[1] * parentBucket << " of the  "<<entry.first<<endl;
                unsigned long long int value=entry.second[0] *entry.second[1] * parentBucket;
                    
                if(value<max_vec_initialization){
                        levelFreqs[level].assign(value,entry.first ); //delete the oldests to avoid mem issues  
                }
                else{
                      for(int i=0;i<value / max_vec_initialization;i++)
                          levelFreqs[level].assign(max_vec_initialization,entry.first );
                      levelFreqs[level].assign(value% max_vec_initialization,entry.first );
                }
                if(level<levelSize)
                    recursive_generation_noFreq(level+1,entry.second[0] * parentBucket, {entry.first});
            }
        }

        else if(levelClqs[level].size()==2){
            for(auto &entry1: generativeCliques[levelClqs[level][0]].cond_pot_T2[keys[levelParents[level][0]]])
                for(auto &entry2: generativeCliques[levelClqs[level][1]].cond_pot_T2[keys[levelParents[level][1]]]){
                    unsigned long long int newBucket= entry1.second[0]*entry2.second[0] * parentBucket;
                    //cout<< entry1.first <<" and "<< entry2.first<< " , with size of:"<< newBucket * entry1.second[1]*  entry2.second[1]<<endl;
  
                    unsigned long long int value=newBucket * entry1.second[1]*  entry2.second[1];
                    
                    if(value<max_vec_initialization){
                            levelFreqs[level].assign(value,entry1.first ); //delete the oldests to avoid mem issues
                            levelFreqs[level].assign(value,entry2.first );
                    }
                    else{
                          for(int i=0;i<value / max_vec_initialization;i++){
                              levelFreqs[level].assign(max_vec_initialization,entry1.first );
                              levelFreqs[level].assign(max_vec_initialization,entry2.first );
                          }
                          levelFreqs[level].assign(value% max_vec_initialization,entry1.first );
                          levelFreqs[level].assign(value% max_vec_initialization,entry2.first );
                    }
                    if(level<levelSize)
                        recursive_generation_noFreq(level+1,newBucket, {entry1.first,entry2.first});
                }
        }
        else if(levelClqs[level].size()==3){
            for(auto &entry1: generativeCliques[levelClqs[level][0]].cond_pot_T2[keys[levelParents[level][0]]])
                for(auto &entry2: generativeCliques[levelClqs[level][1]].cond_pot_T2[keys[levelParents[level][1]]])
                    for(auto &entry3: generativeCliques[levelClqs[level][2]].cond_pot_T2[keys[levelParents[level][2]]]){
                        unsigned long long int newBucket=entry1.second[0]*entry2.second[0]* entry3.second[0] * parentBucket;
                        //cout<< entry1.first <<" and "<< entry2.first<<" and "<< entry3.first<< " , with size of:"<< newBucket * entry1.second[1]*  entry2.second[1]*  entry3.second[1]<<endl;
             
                        unsigned long long int value=newBucket * entry1.second[1]*  entry2.second[1]*  entry3.second[1];

                        if(value<max_vec_initialization){
                                levelFreqs[level].assign(value,entry1.first ); //delete the oldests to avoid mem issues
                                levelFreqs[level].assign(value,entry2.first );
                                levelFreqs[level].assign(value,entry3.first );
                        }
                        else{
                              for(int i=0;i<value / max_vec_initialization;i++){
                                  levelFreqs[level].assign(max_vec_initialization,entry1.first );
                                  levelFreqs[level].assign(max_vec_initialization,entry2.first );
                                  levelFreqs[level].assign(max_vec_initialization,entry3.first );
                              }
                              levelFreqs[level].assign(value% max_vec_initialization,entry1.first );
                              levelFreqs[level].assign(value% max_vec_initialization,entry2.first );
                              levelFreqs[level].assign(value% max_vec_initialization,entry3.first );
                        }
                          
                          if(level<levelSize)
                            recursive_generation_noFreq(level+1,newBucket, {entry1.first,entry2.first, entry3.first});
                    }
        }
        else if(levelClqs[level].size()==4){
            for(auto &entry1: generativeCliques[levelClqs[level][0]].cond_pot_T2[keys[levelParents[level][0]]])
                for(auto &entry2: generativeCliques[levelClqs[level][1]].cond_pot_T2[keys[levelParents[level][1]]])
                    for(auto &entry3: generativeCliques[levelClqs[level][2]].cond_pot_T2[keys[levelParents[level][2]]])
                        for(auto &entry4: generativeCliques[levelClqs[level][3]].cond_pot_T2[keys[levelParents[level][3]]]){
                            unsigned long long int newBucket=entry1.second[0]*entry2.second[0]* entry3.second[0] * entry4.second[0] * parentBucket;
                           // cout<< entry1.first <<" and "<< entry2.first<<" and "<< entry3.first<< " , with size of:"<< newBucket * entry1.second[1]*  entry2.second[1]*  entry3.second[1] * entry4.second[1]<<endl;
                           
                             unsigned long long int value=newBucket * entry1.second[1]*  entry2.second[1]*  entry3.second[1] * entry4.second[1];

                            if(value<max_vec_initialization){
                                    levelFreqs[level].assign(value,entry1.first ); //delete the oldests to avoid mem issues
                                    levelFreqs[level].assign(value,entry2.first );
                                    levelFreqs[level].assign(value,entry3.first );
                                    levelFreqs[level].assign(value,entry4.first );
                            }
                            else{
                                  for(int i=0;i<value / max_vec_initialization;i++){
                                      levelFreqs[level].assign(max_vec_initialization,entry1.first );
                                      levelFreqs[level].assign(max_vec_initialization,entry2.first );
                                      levelFreqs[level].assign(max_vec_initialization,entry3.first );
                                      levelFreqs[level].assign(max_vec_initialization,entry4.first );
                                  }
                                  levelFreqs[level].assign(value% max_vec_initialization,entry1.first );
                                  levelFreqs[level].assign(value% max_vec_initialization,entry2.first );
                                  levelFreqs[level].assign(value% max_vec_initialization,entry3.first );
                                  levelFreqs[level].assign(value% max_vec_initialization,entry4.first );
                            }
                             
                             if(level<levelSize)
                                recursive_generation_noFreq(level+1,newBucket, {entry1.first,entry2.first, entry3.first, entry4.first});
                    }
        }
    }
}
