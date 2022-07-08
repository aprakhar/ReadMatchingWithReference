#include<iostream>
#include<fstream>
#include<string>
#include<vector>
#include<sstream>
 
using std::cout; using std::string; using std::vector; using std::stringstream; using std::fstream; using std::ios; using std::endl; using std::flush;

#define DELTA 3
#define ull unsigned long long

string firstColFilename = "chrX_map.txt";
// string firstColFilename = "test/testCase1/chrX_map_test.txt";
string mapMilestoneFilename = "chrX_map_milestone.txt";

int main(int argc, char const *argv[]){
    fstream firstColFile(firstColFilename, ios::in);
    fstream mapMilestoneFile(mapMilestoneFilename, ios::out | ios::trunc);

    if(!firstColFile.is_open()){
        cout<<"| Unable to open "<<firstColFilename<<" to read from it. Program terminated.\n";
        return EXIT_FAILURE;
    }
    if(!mapMilestoneFile.is_open()){
        cout<<"| Unable to open "<<mapMilestoneFilename<<" to write into it. Program terminated.\n";
        return EXIT_FAILURE;
    }

    string readIndex;
    ull index, iter = 0;
    vector<ull>milestoneIndices;
    
    cout<<"Parsing..."<<flush;
    while (getline(firstColFile, readIndex)){
        index = stoull(readIndex);
        if (!(iter%DELTA)) milestoneIndices.push_back(index);
        iter++;
    }
    cout<<"\rParsed.\n";
    
    cout<<"Writing to file..."<<flush;
    for (ull i = 0; i < milestoneIndices.size(); i++){
        mapMilestoneFile<<milestoneIndices[i]<<"\n";
    }
    cout<<"\rWritten to file.\n";

    if(firstColFile.is_open())  firstColFile.close();
    if(mapMilestoneFile.is_open())  mapMilestoneFile.close();
}
