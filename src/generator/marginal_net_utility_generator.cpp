#include "marginal_net_utility_generator.h"
#include "../common/constants.h"
#include "../common/util.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <map>
#include <stdlib.h>
#include <sstream>
#include <ctime>
#include <sys/timeb.h>

using namespace std;

void MarginalNetUtilityGenerator::generateTrainTest(char* file, int k){
    
    // train and test file
    char trainFile[200];
    char testFile[200];// 

    ofstream trainOut, testOut;
    
    strcpy(trainFile, file);
    strcpy(testFile, file);
    
    trainOut.open(strcat(trainFile,"-train"));
    testOut.open(strcat(testFile,"-test"));
    
    dataStat(file, k);
       
    //generate training and testing data
    map<string, map<string,int> > Xuit;// purchase count before t
    map<string, map<string,int> > xt;// purchase count at t
    map<string, map<string,int> > purchased;//test set
    
    map<string, int> trainUser;
    map<string, int> trainProduct;
    string last_time = "0";

    trainOut << user_size << SPACE << product_size << SPACE << K << endl;  

    for(int index=0; index<buffer.size(); index++){

        string time = buffer[index].t;
        string userId = buffer[index].u;
        string productId = buffer[index].i;
        
        
        
        if(index < train_size){//generate training data
            
            trainUser[userId] = 1;
            trainProduct[productId] = 1;    
            
            int xuit = 0;
            if(Xuit.find(userId) != Xuit.end() && Xuit[userId].find(productId) != Xuit[userId].end()){
                xuit = Xuit[userId][productId];
            }
            if(xuit != 0){
                
                trainOut << time << SPACE << userId << SPACE << productId << SPACE << xuit << SPACE << "1" << endl;
            }
            xt[userId][productId] = 1;

            if(time != last_time){
               
                int random_scale = 2000;
                map<string,map<string,int> >::iterator itr1;
                map<string,int>::iterator itr2;
                for(itr1=Xuit.begin();itr1!=Xuit.end();itr1++){
                    for(itr2=Xuit[itr1->first].begin();itr2!=Xuit[itr1->first].end();itr2++){
                        timeb timebuffer;
                        ftime(&timebuffer);
                        srand(timebuffer.millitm);

                        int randNum = rand()%random_scale;
                        
                        if(randNum == 0){
                            if(xt.find(itr1->first) == xt.end() || xt[itr1->first].find(itr2->first) == xt[itr1->first].end()){
                                trainOut << last_time << SPACE << itr1->first << SPACE << itr2->first << SPACE << itr2->second << SPACE << "-1" << endl;
                            }
                        }
                    }
                    
                }
                last_time = time;
                for(itr1=xt.begin();itr1!=xt.end();itr1++){
                    for(itr2=itr1->second.begin();itr2!=itr1->second.end();itr2++){
                        if(Xuit.find(itr1->first) != Xuit.end() && Xuit[itr1->first].find(itr2->first) != Xuit[itr1->first].end()){
                            Xuit[itr1->first][itr2->first] += itr2->second;
                        }else{
                            Xuit[itr1->first][itr2->first] = itr2->second;
                        }
                    }
                }
                xt.clear();
            }
        }else{//generate test data
            purchased[userId][productId] = 1;
        } 
    }


    map<string, map<string,int> >::iterator itr1;
    map<string, int>::iterator itr2;
    for(itr1=purchased.begin();itr1!=purchased.end();itr1++){
        
        map<string, int> p;//identify re-purchase 
        int userInTrain = 0;
        int re_purchase = 0;

        if(trainUser.find(itr1->first) != trainUser.end()){
            userInTrain = 1;
        }
        
        string xuit = "";
        for(itr2=Xuit[itr1->first].begin();itr2!=Xuit[itr1->first].end();itr2++){
            xuit += itr2->first + SIMICOLON + Util::itos(itr2->second) + SPACE; 
            p[itr2->first] = 1;//already purchase list
        }

        string purchased_list = "";
        for(itr2=purchased[itr1->first].begin();itr2!=purchased[itr1->first].end();itr2++){
            if(trainProduct.find(itr2->first) != trainProduct.end()){
                purchased_list += itr2->first + SIMICOLON + "1" + SPACE;
            }else{
                purchased_list += itr2->first + SIMICOLON + "0" + SPACE;
            }
            if(p.find(itr2->first) != p.end()){
                re_purchase = 1;
            }
        }
        if(purchased_list.size() != 0){
            testOut << userInTrain <<  re_purchase << TAB << itr1->first << COMMA << xuit << COMMA << purchased_list << endl;
        }
        
    }
    trainOut.close();
    testOut.close();
}

void MarginalNetUtilityGenerator::generate(char* file, int k){
    generateTrainTest(file, k);
}
