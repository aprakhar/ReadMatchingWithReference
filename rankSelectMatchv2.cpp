#include<iostream>
#include<string>
#include<fstream>
#include<vector>
#include<sstream>
#include<algorithm>
#include<stdexcept>
#include<chrono>
#include<thread>
#include<cmath>
#include<mutex>

// Program 5

using std::string; using std::cout; using std::fstream; using std::ios; using std::vector; using std::flush; using std::endl; using std::stringstream;
using std::transform; using std::reverse; using std::invalid_argument; using std::chrono::high_resolution_clock; using std::chrono::duration;
using std::chrono::microseconds; using std::chrono::time_point; using std::chrono::system_clock; using std::thread; using std::ref;
using std::for_each; using std::mutex; using std::unique_lock; 

#define DELTA 3
#define ull unsigned long long
#define ll long long
# define MAGIC_SPLIT_NUMBER 44

ull pos$InLastCol;
string firstColRankFilename = "FirstCol.txt";                   //36B
string chrXFilename = "data/dataset1/chrX.fa";                  //153MB
string readsFilename = "data/dataset1/reads";                   //312MB
string milestoneMapFilename = "chrX_map_milestone.txt";         //467MB
string lastColBitmapFilename = "chrX_last_col_bitmap.txt";      //529MB
string lastColRankFilename = "last_col_rank.txt";               //1.8GB
string readsMatchPositionFilename = "readsMatchPosition.txt";   // (op file) 350MB

void initializeOutputFile(fstream&);
void readFilesAndStoreInMemory(vector<string>&,vector<string>&, vector<vector<ull>>&, vector<int>&, vector<ull>&, vector<ll>&, vector<char>&);
void multiThreadRankSelect(vector<string>&,vector<string>&, vector<vector<ull>>&, vector<int>&, vector<ull>&, fstream&, vector<ll>&, vector<char>&);
void rankSelectThread(ull, ull, ull&, ull&, ull&, vector<string>&, vector<string>&, vector<vector<ull>>&, vector<int>&, vector<ull>&, fstream&, vector<ll>&, vector<char>&, mutex&);
bool splitAndAlign(string, int, vector<ull>&, vector<vector<ull>>&, vector<int>&, vector<ull>& , fstream&, vector<ll>&, vector<char>&, mutex&);
void getFirstCol(vector<ull>&);
void getACGTLastRank(vector<vector<ull>>&);
void getLastColBitMap(vector<int>&);
void getReadsFile(vector<string>&, vector<string>&);
void getMilestoneMapFile(vector<ll>&);
void getChrX(vector<char>&);
string cleanupReadString(string);
string makeReversedComplement(string);
char replaceNWithA(char);
char dnaComplement(char);
bool rankSelectInit(string, vector<ull> &, vector<vector<ull>>&, vector<int>&, vector<ull>&, fstream&, vector<ll>&, mutex&);
bool rankSelect(string&, ll&, ll&, vector<vector<ull>>&, vector<int>&, vector<ull>&);
ll charToBitStr(char);
ll getMileStoneRankFromRankDS(char, ll, vector<vector<ull>>&);
ll getBandInFirstColFromRank(char, ll, vector<ull>&);
void getPosInRef(ll, ll, vector<ull>&, vector<vector<ull>>&, vector<ull>&, vector<int>&, string, fstream&, vector<ll>&, mutex&);
char getLastCharAtRow(int);
void outputToFile(vector<ull>&, string&, fstream&, mutex&, bool, bool, bool);

int main(int argc, char const *argv[]){
    vector<string> reads, revComplReads; vector<ll> milestoneMap; vector<ull> firstCol; vector<vector<ull>> ACGT_last_rank; vector<int> lastColBitMap; vector<char> chrX;
    fstream readsMatchPositionFile;
    
    initializeOutputFile(readsMatchPositionFile);
    readFilesAndStoreInMemory(reads, revComplReads, ACGT_last_rank, lastColBitMap, firstCol, milestoneMap, chrX);
    multiThreadRankSelect(reads, revComplReads, ACGT_last_rank, lastColBitMap, firstCol,readsMatchPositionFile, milestoneMap, chrX);

    if(readsMatchPositionFile.is_open())  readsMatchPositionFile.close();
}

void initializeOutputFile(fstream& readsMatchPositionFile) {
    readsMatchPositionFile.open(readsMatchPositionFilename, ios::out|ios::trunc);
    if(!readsMatchPositionFile.is_open()){
        cout<<"| Unable to open "<<readsMatchPositionFilename<<". Cannot save output. Exiting program.\n";
        exit(EXIT_FAILURE);
    }
    readsMatchPositionFile<<"Exact match(0/1),"<<"Match with variants(0/1),"<<"No. of places string matched,"<<"Read string,"<<"Position 1,"<<"Position 2,"<<"\n";
}

void readFilesAndStoreInMemory(vector<string>& reads, vector<string>& revComplReads, vector<vector<ull>>& ACGT_last_rank, vector<int>& lastColBitMap, vector<ull>& firstCol, vector<ll>& milestoneMap, vector<char>& chrX){
    vector<thread> memInitThreads;
    cout<<"Reading files & storing in memory...\n"<<flush;
    /* 
        Multithread read small files
        Sequential read big files  
    */
    memInitThreads.push_back(thread(getFirstCol, ref(firstCol)));
    memInitThreads.push_back(thread(getReadsFile, ref(reads), ref(revComplReads)));
    memInitThreads.push_back(thread(getMilestoneMapFile, ref(milestoneMap)));
    getACGTLastRank(ACGT_last_rank);
    getLastColBitMap(lastColBitMap);
    getChrX(chrX);
    for_each(memInitThreads.begin(), memInitThreads.end(), [](auto &thr) { thr.join();} ); memInitThreads.clear();
    cout<<"Read into memory completed.\n"<<flush;
}

void multiThreadRankSelect(vector<string>& reads, vector<string>& revComplReads, vector<vector<ull>>& ACGT_last_rank, vector<int>& lastColBitMap, vector<ull>& firstCol, fstream& readsMatchPositionFile, vector<ll>& milestoneMap, vector<char>& chrX) {
    vector<thread> readsMatchingThreads;
    mutex readsMatchOutputMutex, countMatchMutex;
    ull noOfCpuThreadsOnSystem = thread::hardware_concurrency();
    ull start, end, iter = 0, noOfReadsPerThread = static_cast<ull>(ceil((double)reads.size() /  noOfCpuThreadsOnSystem));
    ull sumexactMatch = 0, sumoneError = 0, sumtwoError = 0;
    vector<ull> totalexactMatch(noOfCpuThreadsOnSystem, 0), totaloneError(noOfCpuThreadsOnSystem, 0), totaltwoError(noOfCpuThreadsOnSystem, 0);
    
    cout<<"Processing read strings.\n";
    if ((reads.size()/10000) < noOfCpuThreadsOnSystem) {
        rankSelectThread(0, reads.size(), totalexactMatch[0], totaloneError[0], totaltwoError[0], reads, revComplReads, ACGT_last_rank, lastColBitMap, firstCol, readsMatchPositionFile, milestoneMap, chrX, readsMatchOutputMutex);
        sumexactMatch += totalexactMatch[0]; sumoneError += totaloneError[0]; sumtwoError += totaltwoError[0];
    } else { 
        for (ull i = 0; i < noOfCpuThreadsOnSystem; i++){
            start = iter;
            end = iter + noOfReadsPerThread > reads.size() ? reads.size() : iter + noOfReadsPerThread;
            iter = end;
            readsMatchingThreads.push_back(thread(rankSelectThread, start, end, ref(totalexactMatch[i]), ref(totaloneError[i]), ref(totaltwoError[i]), ref(reads), ref(revComplReads), ref(ACGT_last_rank), ref(lastColBitMap), ref(firstCol),ref(readsMatchPositionFile), ref(milestoneMap), ref(chrX), ref(readsMatchOutputMutex)));
            sumexactMatch += totalexactMatch[i]; sumoneError += totaloneError[i]; sumtwoError += totaltwoError[i];
        }
        for_each(readsMatchingThreads.begin(), readsMatchingThreads.end(), [](auto &thr) { thr.join();} ); readsMatchingThreads.clear();
        for (ull i = 0; i < noOfCpuThreadsOnSystem; i++){
            sumexactMatch += totalexactMatch[i]; sumoneError += totaloneError[i]; sumtwoError += totaltwoError[i];
        }
    }
    
    cout<<"Exact matches: "<<sumexactMatch<<"\n"<<"One error: "<<sumoneError<<"\n"<<"Two erros: "<<sumtwoError<<"\n";
}

void rankSelectThread(ull start, ull end, ull& totalexactMatch, ull& totaloneError, ull& totaltwoError, vector<string>& reads, vector<string>& revComplReads, vector<vector<ull>>& ACGT_last_rank, vector<int>& lastColBitMap, vector<ull>& firstCol, fstream& readsMatchPositionFile, vector<ll>& milestoneMap, vector<char>& chrX, mutex& readsMatchOutputMutex ) {
    ull exactMatch = 0, oneError = 0, twoError = 0, readexactMatch = 0, readoneError = 0, readtwoError = 0;
    vector<ull> matchPositions;
    for (ll i = start; i < end; i++){
        if (rankSelectInit(reads[i], matchPositions, ACGT_last_rank, lastColBitMap, firstCol, readsMatchPositionFile, milestoneMap,readsMatchOutputMutex)) readexactMatch++;
        else if (rankSelectInit(revComplReads[i], matchPositions, ACGT_last_rank, lastColBitMap, firstCol, readsMatchPositionFile, milestoneMap,readsMatchOutputMutex)) readexactMatch++;
        else if (splitAndAlign(reads[i], 1, matchPositions, ACGT_last_rank, lastColBitMap, firstCol,readsMatchPositionFile, milestoneMap, chrX, readsMatchOutputMutex)) readoneError++;
        else if (splitAndAlign(reads[i], 2, matchPositions, ACGT_last_rank, lastColBitMap, firstCol,readsMatchPositionFile, milestoneMap, chrX, readsMatchOutputMutex)) readtwoError++;
        else if (splitAndAlign(revComplReads[i], 1, matchPositions, ACGT_last_rank, lastColBitMap, firstCol,readsMatchPositionFile, milestoneMap, chrX, readsMatchOutputMutex)) readoneError++;
        else if (splitAndAlign(revComplReads[i], 2, matchPositions, ACGT_last_rank, lastColBitMap, firstCol,readsMatchPositionFile, milestoneMap, chrX, readsMatchOutputMutex)) readtwoError++;

        // readMatch = splitAndAlign(reads[i], readexactMatch, readoneError, readtwoError, matchPositions, ACGT_last_rank, lastColBitMap, firstCol,readsMatchPositionFile, milestoneMap, chrX, readsMatchOutputMutex);
        // exactMatch += readexactMatch; oneError += readoneError; twoError += readtwoError;
        // readexactMatch = 0; readoneError = 0; readtwoError = 0;
        // revCompReadMatch = splitAndAlign(revComplReads[i],readexactMatch, readoneError, readtwoError, matchPositions, ACGT_last_rank, lastColBitMap, firstCol,readsMatchPositionFile, milestoneMap, chrX, readsMatchOutputMutex);
        
        exactMatch += readexactMatch; oneError += readoneError; twoError += readtwoError;
        outputToFile(matchPositions, reads[i], readsMatchPositionFile, readsMatchOutputMutex, readexactMatch, readoneError, readtwoError);
        readexactMatch = 0; readoneError = 0; readtwoError = 0;
        matchPositions.clear();  
    }
    totalexactMatch = exactMatch;
    totaloneError = oneError;
    totaltwoError = twoError;
}

bool splitAndAlign(string read, int maxMismatch, vector<ull> &matchPositions, vector<vector<ull>>& ACGT_last_rank, vector<int>& lastColBitMap, vector<ull>& firstCol, fstream& readsMatchPositionFile, vector<ll>& milestoneMap, vector<char>& chrX, mutex& readsMatchOutputMutex) {
    ull charsPerSplit, end, start = 0;//, exactMatch = 0, oneError = 0, twoError = 0;
    int noOfSplits = static_cast<ull>(ceil((double)read.length() / MAGIC_SPLIT_NUMBER));
    bool splitMatched = false, matchFound = false;
    charsPerSplit = static_cast<ull>(ceil((double)read.length() / noOfSplits));
    vector<ull> substrMatchPositions;
    end = start + charsPerSplit;
    for (ull i = 0; i < noOfSplits; i++){
        splitMatched = rankSelectInit(read.substr(start, end - start), substrMatchPositions, ACGT_last_rank, lastColBitMap, firstCol,readsMatchPositionFile, milestoneMap, readsMatchOutputMutex);  
        if (splitMatched){
            for (ull j = 0; j < substrMatchPositions.size(); j++){
                int countMismatches = 0;
                ull left = start, right = end, matchIndex = substrMatchPositions[j];
                while (countMismatches <= maxMismatch && right < read.length() && (matchIndex+charsPerSplit < chrX.size())){
                    if (read[right] != chrX[matchIndex+charsPerSplit]) {
                        countMismatches++;
                    }
                    right++;
                    matchIndex++;
                }

                matchIndex = substrMatchPositions[j];
                while (countMismatches <= maxMismatch && left > 0 && matchIndex > 0)
                    if (read[left--] != chrX[matchIndex--]) 
                        countMismatches++;
                if (left == 0 && right == read.length() && countMismatches <= maxMismatch)  {
                    matchPositions.push_back(matchIndex);
                    matchFound = true;
                    // (countMismatches == 0) && exactMatch++;
                    // (countMismatches == 1) && oneError++;
                    // (countMismatches == 2) && twoError++;
                    break;
                } 
            }
            splitMatched = false;
        }
        if (matchFound) break;
        start = end;
        end = start + charsPerSplit > read.length() ? read.length() : start + charsPerSplit;
        substrMatchPositions.clear();
    }
    // readexactMatch = exactMatch; readoneError = oneError; readtwoError = twoError;
    return matchFound;
}

bool rankSelectInit(string read, vector<ull> &matchPositions, vector<vector<ull>>& ACGT_last_rank, vector<int>& lastColBitMap, vector<ull>& firstCol, fstream& readsMatchPositionFile, vector<ll>& milestoneMap, mutex& readsMatchOutputMutex) {
    string readCopy = read;
    ll band_start, band_end;
    ll readStrLen = read.size();  
    char lastChar = read.back();
    bool found = false;

    read.pop_back();

    /*
        A   A   C   C   G   G   T
        0   1   2   3   4   5   6
        firstCol[A] = 2
        firstCol[C] = 2
        firstCol[G] = 2
        firstCol[T] = 1
    */
    ll A_start, C_start, G_start, T_start, A_end, C_end, G_end, T_end;
    A_start = 0; C_start = A_start + firstCol[0]; G_start = C_start + firstCol[1]; T_start = G_start + firstCol[2];
    A_end = firstCol[0] -1; C_end = A_end + firstCol[1]; G_end = C_end + firstCol[2]; T_end = G_end + firstCol[3];

    switch(lastChar) {
        case 'A': band_start = A_start; band_end = A_end; break;
        case 'C': band_start = C_start; band_end = C_end; break;
        case 'G': band_start = G_start; band_end = G_end; break;
        case 'T': band_start = T_start; band_end = T_end; break;
        default: cout<<"Invalid string. Found unknown character: "<<lastChar<<".\n"; exit(EXIT_FAILURE);
    }
    found = rankSelect(read, band_start, band_end, ACGT_last_rank, lastColBitMap, firstCol);

    if (found)
        getPosInRef(band_start, band_end, matchPositions, ACGT_last_rank, firstCol, lastColBitMap, readCopy, readsMatchPositionFile, milestoneMap, readsMatchOutputMutex);
  
    return found;
}

bool rankSelect(string& read, ll& band_start, ll& band_end, vector<vector<ull>>& ACGT_last_rank, vector<int>& lastColBitMap, vector<ull>& firstCol){
    if (read.size() == 0) return true;
    if (band_start > band_end) return false; // probably never reached

    char ch = read.back();
    read.pop_back();

    /*
        We have the band_start & band_end.
        The read string's last char is last char to be found in last col,
        i.e., we need the sub-band within this band 
    */
    ll rank_first_ch, rank_last_ch, indexOfNearestMilestoneFromBandStartInBitmap, indexOfNearestMilestoneFromBandStartInRankDS,
        indexOfNearestMilestoneFromBandEndInBitmap, indexOfNearestMilestoneFromBandEndInRankDS, rankOfNearestMilestoneFromStart,
        rankOfNearestMilestoneFromEnd, countOfChFromStartMilestoneToJustBeforeFirstCh = 0, countOfChFromEndMilestoneToLastCh = 0
    ;
    indexOfNearestMilestoneFromBandStartInRankDS = band_start/DELTA; // index of nearest milstone in rank DS (rankDS = rank data structure)
    indexOfNearestMilestoneFromBandEndInRankDS = band_end/DELTA; // index of nearest milstone in rank DS (rankDS = rank data structure)
    indexOfNearestMilestoneFromBandStartInBitmap = band_start - (band_start%DELTA); // index of nearest milstone in bitmap array
    indexOfNearestMilestoneFromBandEndInBitmap = band_end - (band_end%DELTA); // index of nearest milstone in bitmap array
    rankOfNearestMilestoneFromStart = getMileStoneRankFromRankDS(ch, indexOfNearestMilestoneFromBandStartInRankDS, ACGT_last_rank);
    rankOfNearestMilestoneFromEnd = getMileStoneRankFromRankDS(ch, indexOfNearestMilestoneFromBandEndInRankDS, ACGT_last_rank);

    // get number of ch in between indexOfNearestMilestoneFromBandStartInBitmap to the first occurence of ch in the band (indexOfNearestMilestoneFromBandStartInBitmap inclusive only)
    ll iter = indexOfNearestMilestoneFromBandStartInBitmap;
    while(iter < band_end) {
        if (iter >= band_start && lastColBitMap[iter] == charToBitStr(ch)) break;
        if (lastColBitMap[iter] == charToBitStr(ch)) countOfChFromStartMilestoneToJustBeforeFirstCh++;
        iter++;
    }
    iter = indexOfNearestMilestoneFromBandEndInBitmap;
    while(iter <= band_end) {
        if (lastColBitMap[iter] == charToBitStr(ch)) countOfChFromEndMilestoneToLastCh++; // last ch count --if here-- will be removed later
        iter++;
    }

    ll noOfChInBand = (rankOfNearestMilestoneFromEnd + countOfChFromEndMilestoneToLastCh) - (rankOfNearestMilestoneFromStart + countOfChFromStartMilestoneToJustBeforeFirstCh);
    if (noOfChInBand == 0) return false;

    rank_first_ch = rankOfNearestMilestoneFromStart + countOfChFromStartMilestoneToJustBeforeFirstCh;
    rank_last_ch = rankOfNearestMilestoneFromEnd + countOfChFromEndMilestoneToLastCh -1; // to exclude the last ch counted. (reason: see definition of rank)

    /* 
        take the ranks found in last column to first column
        and locate the band start & end for the ranks first 
        & last, respectively
    */
    band_start = getBandInFirstColFromRank(ch, rank_first_ch, firstCol);
    band_end = getBandInFirstColFromRank(ch, rank_last_ch, firstCol);

    return rankSelect(read, band_start, band_end, ACGT_last_rank, lastColBitMap, firstCol);
}

void getFirstCol(vector<ull>& firstCol) {
    fstream firstColRankFile(firstColRankFilename, ios::in);
    string readStr;

    while(getline(firstColRankFile, readStr))
        firstCol.push_back(stoull(readStr));

    if(firstColRankFile.is_open())  firstColRankFile.close();
}

void getACGTLastRank(vector<vector<ull>>& ACGT_last_rank){
    fstream lastColRankFile(lastColRankFilename, ios::in);
    string readStr;
    while(getline(lastColRankFile, readStr)){
        if(readStr[0] == '$') {
            string position$;
            stringstream ss(readStr);
            while(getline(ss, position$, ':')){}
            pos$InLastCol = stoull(position$);
            continue;
        }
        string rank;
        stringstream ss(readStr);
        vector<ull> rank_row;
        for (int i = 0; i < 4; i++){
            getline(ss, rank, ',');
            rank_row.push_back(stoull(rank));
        }
        ACGT_last_rank.push_back(rank_row);
    }

    if(lastColRankFile.is_open())  lastColRankFile.close();
}

void getLastColBitMap(vector<int>& lastColBitMap) {
    fstream lastColBitmapFile(lastColBitmapFilename, ios::in);
    string readStr;
    while(getline(lastColBitmapFile, readStr))
        lastColBitMap.push_back(stoull(readStr));

    if(lastColBitmapFile.is_open())  lastColBitmapFile.close();
}

void getReadsFile(vector<string>& reads, vector<string>& revComplReads) {
    fstream readsFile(readsFilename, ios::in);
    string read, complementReversed;
    while(getline(readsFile, read)){
        read = cleanupReadString(read);
        complementReversed = makeReversedComplement(read);
        reads.push_back(read);
        revComplReads.push_back(complementReversed);
    }
    if(readsFile.is_open())  readsFile.close();
}

void getMilestoneMapFile(vector<ll>& milestoneMap) {
    fstream milestoneMapFile(milestoneMapFilename, ios::in);
    string read;
    while(getline(milestoneMapFile, read))
        milestoneMap.push_back(stoll(read));

    if(milestoneMapFile.is_open())  milestoneMapFile.close();
}

void getChrX(vector<char>& chrX) {
    fstream chrXFile(chrXFilename, ios::in);
    char ch;
    chrXFile.get(ch);
    string fileMetaData;
    if (ch == '>')
        getline(chrXFile, fileMetaData); 
    else 
        chrX.push_back(ch);
    while(chrXFile.get(ch))
        if (ch == 'A' || ch == 'C' || ch == 'G' || ch == 'T') 
            chrX.push_back(ch);

    if(chrXFile.is_open())
        chrXFile.close();
}

char replaceNWithA(char ch) { return ch == 'N' ? 'A' : ch; } 

string cleanupReadString(string str){
    transform(str.begin(), str.end(), str.begin(), replaceNWithA);
    return str;
}

char dnaComplement(char ch){
    switch (ch){
        case 'A': return 'T';
        case 'C': return 'G';
        case 'G': return 'C';
        case 'T': return 'A';
        default: throw std::invalid_argument("Wrong character "+std::to_string(ch)+" read in dnaComplement.\n");
    }
}

string makeReversedComplement(string str) {
    string reversedComplement(str.length(), 'X');
    transform(str.begin(), str.end(), reversedComplement.begin(), dnaComplement);
    reverse(reversedComplement.begin(), reversedComplement.end());
    return reversedComplement;
}

ll charToBitStr(char ch) {
    switch (ch){
        case 'A': return 1000;
        case 'C': return 100;
        case 'G': return 10;
        case 'T': return 1;
        case '$': return 0;
        default: throw std::invalid_argument("Wrong character "+std::to_string(ch)+" read in charToBitStr.\n");
    }
}

ll getMileStoneRankFromRankDS(char ch, ll index, vector<vector<ull>>& ACGT_last_rank) {
    switch (ch){
        case 'A': return ACGT_last_rank[index][0];
        case 'C': return ACGT_last_rank[index][1];
        case 'G': return ACGT_last_rank[index][2];
        case 'T': return ACGT_last_rank[index][3];
        case '$': return index*DELTA > pos$InLastCol ? 1 : 0;
        default: throw std::invalid_argument("Wrong character "+std::to_string(ch)+" read in getMileStoneRankFromRankDS.\n");
    }
}

ll getBandInFirstColFromRank(char ch, ll rank, vector<ull>& firstCol) {
    /*
        rank = 1, ch = C;
        A   A   C   C   G   G   T
        0   1   2   3   4   5   6
    */
    switch (ch){
        case 'A': return rank;
        case 'C': return firstCol[0] + rank;
        case 'G': return firstCol[0] + firstCol[1] + rank;
        case 'T': return firstCol[0] + firstCol[1] + firstCol[2] + rank;
        case '$': return firstCol[0] + firstCol[1] + firstCol[2] + firstCol[3] + rank;
        default: throw std::invalid_argument("Wrong character "+std::to_string(ch)+" read in getBandInFirstColFromRank.\n");
    }
}

void getPosInRef(ll band_start, ll band_end, vector<ull> &matchPositions, vector<vector<ull>>& ACGT_last_rank, vector<ull>& firstCol, vector<int>& lastColBitMap, string read, fstream& readsMatchPositionFile, vector<ll>& milestoneMap, mutex& readsMatchOutputMutex) {
    ll startIndexInRef;
    ull count_jumps = 0;

    for (ll band_iter = band_start; band_iter <= band_end; band_iter++){
        ll band = band_iter;
        while(band%DELTA != 0){
            count_jumps++;
            char lastChInBand = getLastCharAtRow(lastColBitMap[band]);
            ll rank_ch, indexOfNearestMilestoneFromBandStartInBitmap, indexOfNearestMilestoneFromBandStartInRankDS, countOfChFromStartMilestoneToJustBeforeFirstCh = 0;

            indexOfNearestMilestoneFromBandStartInRankDS = band/DELTA; // index of nearest milstone in rank DS (rankDS = rank data structure)
            indexOfNearestMilestoneFromBandStartInBitmap = band - (band%DELTA); // index of nearest milstone in bitmap array
            for (ll i = indexOfNearestMilestoneFromBandStartInBitmap; i < band; i++)
                if(lastColBitMap[i] == charToBitStr(lastChInBand)) countOfChFromStartMilestoneToJustBeforeFirstCh++; // counts until just before band char (see definition of rank).
            rank_ch = getMileStoneRankFromRankDS(lastChInBand, indexOfNearestMilestoneFromBandStartInRankDS, ACGT_last_rank) + countOfChFromStartMilestoneToJustBeforeFirstCh;
            band = getBandInFirstColFromRank(lastChInBand, rank_ch, firstCol);
        }
        startIndexInRef = milestoneMap[(ll)(band/DELTA)]+count_jumps;
        matchPositions.push_back(startIndexInRef);
        count_jumps = 0;
    }
}

char getLastCharAtRow(int chAsInt) {
    switch(chAsInt) {
        case 1000: return 'A';
        case 100: return 'C';
        case 10: return 'G';
        case 1: return 'T';
        case 0: return '$';
        default: throw std::invalid_argument("Wrong character value "+std::to_string(chAsInt)+" read in getLastCharAtRow.\n");
    };
}

void outputToFile(vector<ull>& matchPositions, string& read, fstream& readsMatchPositionFile, mutex& readsMatchOutputMutex, bool readexactMatch, bool readoneError, bool readtwoError){
/*
    csv formatted output blueprint
    exact    variant      str      matchCount    matchPos1    matchPos2...
    1            0                     ACGT         4           1234            5678...
    0            1                     GGGGT        1           34567           
    0            0                     TTCA         0
    ...  
*/
    unique_lock<mutex> ul(readsMatchOutputMutex);
    readsMatchPositionFile<<readexactMatch<<","<<(readoneError||readtwoError)<<","<<matchPositions.size()<<","<<read<<",";
    for (ull j = 0; j < matchPositions.size(); j++)
        readsMatchPositionFile<<matchPositions[j]<<",";
    readsMatchPositionFile<<"\n"<<flush;
    ul.unlock();
}
