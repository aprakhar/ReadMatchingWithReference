#include<iostream>
#include<fstream>
#include<string>
#include<vector>
#include<sstream>
#include<iterator>

// Program 6

using std::cout;

#define ll long long
#define ull unsigned ll
#define MIN_OVERLAP 100

const std::string readsPositionFilename = "readsMatchPosition.txt";

const std::vector<std::pair<ull, ull>> green2Exons = {
    std::make_pair(149330376-1, 149330672-1),
    std::make_pair(149332660-1, 149332828-1),
    std::make_pair(149334296-1, 149334461-1),
    std::make_pair(149336016-1, 149336255-1)
};

const std::vector<std::pair<ull, ull>> green1Exons = {
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

std::vector<long double> green2Count(green2Exons.size(), 0.0L),green1Count(green1Exons.size(), 0.0L), redCount(redExons.size(), 0.0L);

void checkExistenceInRegion(std::vector<std::string>::iterator start, std::vector<std::string>::iterator end);
ll isInAnyGreen2Exon(std::string read, ull startIndex);
ll isInAnyGreen1Exon(std::string read, ull startIndex);
ll isInAnyRedExon(std::string read, ull startIndex);
bool isInExon(ull startIndex, ull endIndex, ull exonStart, ull exonEnd);
void printCounts();
void printPercentage();

int main(int argc, char const *argv[]){
    std::fstream readsPositionFile(readsPositionFilename, std::ios::in);
    std::string readInfo, columnHeaders, subreadInfo;
    getline(readsPositionFile, columnHeaders);
    while(getline(readsPositionFile, readInfo)){
        // 1,0,2,GCTTTTGGGAGAGGAGGCTGTTTCTCTGCATTCTAGGTCATCTCCAGAGAAACTGCTGGTAACGTTTCCTCCCACAGGTGCATTGCTACCAGGTTTCTTCT,131005222,130912139,
        std::stringstream ss(readInfo);
        std::vector<std::string> readInfoVector;
        while(getline(ss, subreadInfo, ',')){
            readInfoVector.push_back(subreadInfo);
        }
        if (readInfoVector.size() > 2 && stoll(readInfoVector[2]) > 0 && stoll(readInfoVector[2]) < 5) {
            checkExistenceInRegion(readInfoVector.begin()+3, readInfoVector.end());
        }
    }
    if(readsPositionFile.is_open())  readsPositionFile.close();
    
    printCounts();
    printPercentage();
}

void checkExistenceInRegion(std::vector<std::string>::iterator start, std::vector<std::string>::iterator end){
    std::string read = *start;
    ll foundInRed, foundInGreen1, foundInGreen2;
    for (std::vector<std::string>::iterator iter = start+1; iter < end; iter++) {
        foundInRed = isInAnyRedExon(read, stoull(*iter));
        foundInGreen1 = isInAnyGreen1Exon(read, stoull(*iter));
        foundInGreen2 = isInAnyGreen2Exon(read, stoull(*iter));
    }
    if (foundInRed > 0 && foundInGreen1 > 0 && foundInGreen2 > 0) {
        green2Count[foundInGreen2]+=(long double)1/3;
        green1Count[foundInGreen1]+=(long double)1/3;
        redCount[foundInRed]+=(long double)1/3;
    }
    if (foundInRed >= 0 && foundInGreen1 >= 0 && foundInGreen2 < 0) {
        green1Count[foundInGreen1]+=(long double)2/3;
        redCount[foundInRed]+=(long double)1/3;
        // green1Count[foundInGreen1]+=(long double)1/2;
        // redCount[foundInRed]+=(long double)1/2;
    }
    if (foundInRed >= 0 && foundInGreen1 < 0 && foundInGreen2 >= 0) {
        green2Count[foundInGreen2]+=(long double)2/3;
        redCount[foundInRed]+=(long double)1/3;
        // green2Count[foundInGreen2]+=(long double)1/2;
        // redCount[foundInRed]+=(long double)1/2;
    }
    if (foundInRed >= 0 && foundInGreen1 < 0 && foundInGreen2 < 0) {
        redCount[foundInRed]+=1;
    }
    if (foundInRed < 0 && foundInGreen1 >= 0 && foundInGreen2 < 0) {
        green1Count[foundInGreen1]+=1;
    }
    if (foundInRed < 0 && foundInGreen1 < 0 && foundInGreen2 >= 0) {
        green2Count[foundInGreen2]+=1;
    }
}

ll isInAnyGreen2Exon(std::string read, ull startIndex) {
    ull endIndex = startIndex + read.length();
    ll foundIndex = -1;
    for (int i = 0; i < green2Exons.size(); i++){
        bool found = isInExon(startIndex, endIndex, green2Exons[i].first, green2Exons[i].second);
        if (found) {
            foundIndex = i;
            break;
        }
    }
    return foundIndex;    
}

ll isInAnyGreen1Exon(std::string read, ull startIndex) {
    ull endIndex = startIndex + read.length();
    ll foundIndex = -1;
    for (int i = 0; i < green1Exons.size(); i++){
        bool found = isInExon(startIndex, endIndex, green1Exons[i].first, green1Exons[i].second);
        if (found) {
            foundIndex = i;
            break;
        }
    }
    return foundIndex;    
}

ll isInAnyRedExon(std::string read, ull startIndex) {
    ull endIndex = startIndex + read.length();
    ll foundIndex = -1;
    for (int i = 0; i < redExons.size(); i++){
        bool found = isInExon(startIndex, endIndex, redExons[i].first, redExons[i].second);
        if (found) {
            foundIndex = i;
            break;
        }
    }
    return foundIndex;    
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

void printCounts() {
    for (unsigned int i = 0; i < redCount.size(); i++){
        cout<<"R "<<i+2<<": "<<redCount[i]<<"\n";
    }
    cout<<"\n";
    for (unsigned int i = 0; i < green1Count.size(); i++){
        cout<<"G1 "<<i+2<<": "<<green1Count[i]<<"\n";
    }
    cout<<"\n";
    for (unsigned int i = 0; i < green2Count.size(); i++){
        cout<<"G2 "<<i+2<<": "<<green2Count[i]<<"\n";
    }
    cout<<"\n";
}

void printPercentage() {
    for (ull i = 0; i < 4; i++){
        cout<<((long double)redCount[i]/(green1Count[i]+green2Count[i]))*100<<"%\t";
    }
    cout<<"\n";
}
