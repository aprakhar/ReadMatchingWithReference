#include<iostream>
#include<fstream>
#include<string>
#include<vector>
#include<utility>
#include<sstream>

using std::cout;using std::fstream;using std::ios;using std::string;using std::vector;using std::stringstream;using std::flush;using std::endl;using std::pair;using std::make_pair;

#define ull unsigned long long
#define ll long long
#define DELTA 3


void getFirstCol(vector<ull> &);
void getACGTLastRank(vector<vector<ull>> &);
void getLastColBitMap(vector<int> &);
void rankSelect(string, const vector<ull>&, const vector<vector<ull>>&, const vector<int>&);
bool rankSelectRecursion(string, ll, ll, const vector<ull>&, const vector<vector<ull>>&, const vector<int>&);
ll findNewBandStart(const char, const ll, const vector<ull>&);
ll findNewBandEnd(const char ch, ll, const vector<ull>&);
ll getRankFromACGT(ll , char , const vector<vector<ull>>& );
ll findFirstOccurenceInLastCol(const char , const vector<int>& , ll , ll );
ll findLastOccurenceInLastCol(const char, const vector<int>& , ll , ll);
int getNumericFormat(char );

vector<pair<ull, ull>> range;

int main(int argc, char const *argv[])
{
    // fstream readsFile("reads", ios::in);    
    fstream readsFile("reads_test", ios::in);
    string readStr;

    cout<<"Reading from files and storing in memory... "<<flush;

    vector<ull> firstCol;
    vector<vector<ull>> ACGT_last_rank;
    vector<int> lastColBitMap;


    getFirstCol(firstCol);
    getACGTLastRank(ACGT_last_rank);
    getLastColBitMap(lastColBitMap);

    cout<<"\rRead into memory completed. "<<flush;
    
    while(getline(readsFile, readStr)){
        rankSelect(readStr, firstCol, ACGT_last_rank, lastColBitMap);
    }
    for (ull i = 0; i < range.size(); i++){
        cout<<"Final start: "<<range[i].first<<", Final end: "<<range[i].second<<".\n";
    }
    
    cout<<endl;
}

void rankSelect(string read, const vector<ull>& firstCol, const vector<vector<ull>>& ACGT_last_rank, const vector<int>& lastColBitMap){
    if(read.length() == 0) return;

    char ch = read.back();
    ll start = -1, end = -1;

    switch(ch) {
        // this will store select for first column, easier coz 1st col is lexicographically sorted
        case 'A': start = 0; end = firstCol[0] -1; break;
        case 'C': start = firstCol[0]; end = start + firstCol[1] -1; break;
        case 'G': start = firstCol[0]+firstCol[1]; end = start + firstCol[2] -1; break;
        case 'T': start = firstCol[0]+firstCol[1]+firstCol[2]; end = start + firstCol[3] -1; break;
        default: cout<<"Invalid read string (read character: "<<ch<<")\n"; return;
    }

    read.pop_back();

    bool found = false;

    cout<<endl;
    
    if(read.length()) found = rankSelectRecursion(read, start, end, firstCol, ACGT_last_rank, lastColBitMap);

    cout<<"\nfound "<<found<<'\n';
}

bool rankSelectRecursion(string read,ll start_band, ll end_band, const vector<ull>& firstCol, const vector<vector<ull>>& ACGT_last_rank, const vector<int>& lastColBitMap) {
    char ch = read.back();
    // cout<<"str_read:"<<read<<". Band start:"<<start_band<<","<<", end:"<<end_band<<".\n"<<flush;

    ll start_milestone = start_band - start_band%DELTA < 0, 
        start_rank_of_ch, 
        end_rank_of_ch
    ;
    ll nearest_rank_milestone = (ll)start_band/DELTA;

    ll counter = 0;

    // get the no of ch seen from milestone index to just before start_band index
    for (ll i = start_milestone; i < start_band; i++){
        if(lastColBitMap[i] == getNumericFormat(ch)) counter++;
    }

    // start_rank_of_ch = getRankFromACGT(start_milestone, ch, ACGT_last_rank) + counter;
    start_rank_of_ch = getRankFromACGT(nearest_rank_milestone, ch, ACGT_last_rank) + counter;

    // check if there exists ch in our band last col
    // in the process of searching for endrankofch
    counter = 0;
    for (ll i = start_band; i <= end_band; i++){
        if(lastColBitMap[i] == getNumericFormat(ch)) counter++;
    }

    if (counter == 0) return false;
    counter--; // to remove the last counted ch, according to definition of rank.

    end_rank_of_ch = start_rank_of_ch + counter;

    // cout<<"\tStart rank="<<start_rank_of_ch<<", end rank="<<end_rank_of_ch<<".\n";
    // take these ranks and goto first col

    ll new_start_band = findNewBandStart(ch, start_rank_of_ch, firstCol),
        new_end_band = findNewBandEnd(ch, end_rank_of_ch, firstCol);

    if(new_start_band == -1 || new_end_band == -1) return false;
    
    read.pop_back();

    if(read.size() == 0){
        range.push_back(make_pair(new_start_band, new_end_band));
        return true;
    }
    return rankSelectRecursion(read, new_start_band, new_end_band, firstCol, ACGT_last_rank, lastColBitMap);
    

    // ll new_start = findFirstOccurenceInLastCol(ch, lastColBitMap, start, end), new_end = findLastOccurenceInLastCol(ch, lastColBitMap, start, end);

    // if (new_start == -1 || new_end == -1) return false;


    // get the rank of this ch in the last column ACGT_last_rank

}

ll findNewBandStart(const char ch, const ll start_rank, const vector<ull>& firstCol) {
    switch (ch){
        case 'A': return start_rank;
        case 'C': return start_rank +  firstCol[0];
        case 'G': return start_rank + firstCol[0] + firstCol[1];
        case 'T': return start_rank+ firstCol[0] + firstCol[1] + firstCol[2];
    }
    return -1;
}

ll findNewBandEnd(const char ch, ll end_rank, const vector<ull>& firstCol) {
    switch (ch){
        case 'A': return end_rank;
        case 'C': return end_rank +  firstCol[0];
        case 'G': return end_rank + firstCol[0] + firstCol[1];
        case 'T': return end_rank+ firstCol[0] + firstCol[1] + firstCol[2];
    }
    return -1;
}

ll getRankFromACGT(ll index, char ch, const vector<vector<ull>>& ACGT_last_rank) {
    vector<ull> rank_ACGT = ACGT_last_rank[index];

    switch (ch){
        case 'A': return rank_ACGT[0];
        case 'C': return rank_ACGT[1];
        case 'G': return rank_ACGT[2];
        case 'T': return rank_ACGT[3];        
    }
    return -1;
}

ll findFirstOccurenceInLastCol(const char ch, const vector<int>& lastColBitMap, ll start, ll end) {
    ll iter = start;
    while(lastColBitMap[iter] != getNumericFormat(ch) && iter < end) {
        iter++;
    }
    if(iter == end) return -1;
    return iter;
}

ll findLastOccurenceInLastCol(const char ch, const vector<int>& lastColBitMap, ll start, ll end) {
    ll iter = end - 1;
    while(lastColBitMap[iter] != getNumericFormat(ch) && iter >= start) {
        iter--;
    }
    if(iter < start) return -1;
    return iter;
}

void getFirstCol(vector<ull> &firstCol) {
    fstream firstColRankFile("FirstCol.txt", ios::in);
    string readStr;

    while(getline(firstColRankFile, readStr)){
        firstCol.push_back(stoull(readStr));
    }

    if(firstColRankFile.is_open())  firstColRankFile.close();
}

void getACGTLastRank(vector<vector<ull>> &ACGT_last_rank){
    fstream lastColRankFile("last_col_rank.txt", ios::in);
    string readStr;

    while(getline(lastColRankFile, readStr)){
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

void getLastColBitMap(vector<int> &lastColBitMap) {
    fstream lastColBitmapFile("chrX_last_col_bitmap.txt", ios::in);
    string readStr;
    while(getline(lastColBitmapFile, readStr)){
        lastColBitMap.push_back(stoull(readStr));
    }
    if(lastColBitmapFile.is_open())  lastColBitmapFile.close();
}

int getNumericFormat(char ch){
    switch(ch) {
        case 'A': return 1000;
        case 'C': return 100;
        case 'G': return 10;
        case 'T': return 1;
    }
    return 0;
}
