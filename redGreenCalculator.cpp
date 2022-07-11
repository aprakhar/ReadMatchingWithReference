#include<iostream>
#include<fstream>
#include<string>
#include<vector>
#include<sstream>
#include<iterator>

// Program 6

using std::cout;

#define ull unsigned long long
#define MIN_OVERLAP 10

const std::string readsPositionFilename = "readsMatchPosition.txt";

const std::vector<std::pair<ull, ull>> greenExons = {
    std::make_pair(149293258-1, 149293554-1),
    std::make_pair(149295542-1, 149295710-1),
    std::make_pair(149297178-1, 149297343-1),
    std::make_pair(149298898-1, 149299137-1)
};

const std::vector<std::pair<ull, ull>> redExons = {
    std::make_pair(149256127-1, 149256423-1),
    std::make_pair(149258412-1, 149258580-1),
    std::make_pair(149260048-1, 149260213-1),
    std::make_pair(149261768-1, 149262007-1)
};

std::vector<ull> greenCount(greenExons.size(), 0), redCount(redExons.size(), 0);

void checkExistenceInRegion(std::vector<std::string>::iterator start, std::vector<std::string>::iterator end);
bool isInAnyGreenExon(std::string read, ull startIndex);
bool isInAnyRedExon(std::string read, ull startIndex);
bool isInExon(ull startIndex, ull endIndex, ull exonStart, ull exonEnd);


int main(int argc, char const *argv[]){
    std::fstream readsPositionFile(readsPositionFilename, std::ios::in);
    std::string readInfo, columnHeaders, subreadInfo;
    getline(readsPositionFile, columnHeaders);
    while(getline(readsPositionFile, readInfo)){
        // 1,1,2,GCTTTTGGGAGAGGAGGCTGTTTCTCTGCATTCTAGGTCATCTCCAGAGAAACTGCTGGTAACGTTTCCTCCCACAGGTGCATTGCTACCAGGTTTCTTCT,131005222,130912139,
        std::stringstream ss(readInfo);
        std::vector<std::string> readInfoVector;
        while(getline(ss, subreadInfo, ',')){
            readInfoVector.push_back(subreadInfo);
        }
        if (readInfoVector.size() > 2 && stoll(readInfoVector[2]) > 0) {
            checkExistenceInRegion(readInfoVector.begin()+3, readInfoVector.end());
        }
    }
    if(readsPositionFile.is_open())  readsPositionFile.close();

    
    for (unsigned int i = 0; i < redCount.size(); i++){
        cout<<"R "<<i+2<<": "<<redCount[i]<<"\n";
    }
    cout<<"\n";
    for (unsigned int i = 0; i < greenCount.size(); i++){
        cout<<"G "<<i+2<<": "<<greenCount[i]<<"\n";
    }
    cout<<"\n";
}

void checkExistenceInRegion(std::vector<std::string>::iterator start, std::vector<std::string>::iterator end){
    std::string read = *start;
    for (std::vector<std::string>::iterator iter = start+1; iter < end; iter++) {
        isInAnyRedExon(read, stoull(*iter));
        isInAnyGreenExon(read, stoull(*iter));
    }
}

bool isInAnyGreenExon(std::string read, ull startIndex) {
    ull endIndex = startIndex + read.length();
    bool found = false;
    for (int i = 0; i < 4; i++){
        found = isInExon(startIndex, endIndex, greenExons[i].first, greenExons[i].second);
        if (found) {
            greenCount[i]++;
            break;
        }
    }
    return found;    
}

bool isInAnyRedExon(std::string read, ull startIndex) {
    ull endIndex = startIndex + read.length();
    bool found = false;
    for (int i = 0; i < 4; i++){
        found = isInExon(startIndex, endIndex, redExons[i].first, redExons[i].second);
        if (found) {
            redCount[i]++;
            break;
        }
    }
    return found;    
}

bool isInExon(ull startIndex, ull endIndex, ull exonStart, ull exonEnd) {
    bool inExon = true;
    if (startIndex <= exonStart && endIndex < exonStart + MIN_OVERLAP) {
        inExon = false;
    } 
    if (endIndex >= exonEnd && startIndex > exonEnd - MIN_OVERLAP) {
        inExon = false;
    }
    return inExon;
}
