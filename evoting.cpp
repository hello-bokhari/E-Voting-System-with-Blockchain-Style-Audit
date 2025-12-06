#include <iostream>
#include <string>
#include <ctime>
#include <vector>
#include <fstream>
#include <stdexcept>
#include <iomanip>
#include <sstream>
using namespace std;

// Hash function for hash table indexing
int simpleHash(const string& key, int tableSize) {
    int hash = 0;
    for (char c : key) {
        hash = (hash * 31 + c) % tableSize;
    }
    return abs(hash);
}

// Generate hash for blockchain block hashing
string generateHash(const string& data) {
    unsigned long hash = 5381;
    for (char c : data) {
        hash = ((hash << 5) + hash) + c;
    }
    stringstream ss;
    ss << hex << hash;
    return ss.str();
}

struct Voter {
    string voterID;
    string name;
    bool hasVoted;
    Voter* next;
    
    Voter(string id, string n) : voterID(id), name(n), hasVoted(false), next(nullptr) {}
};

class VoterDatabase {
private:
    Voter* head;
    
public:
    VoterDatabase() : head(nullptr) {}
    
    void insertVoter(string voterID, string name) {
        try {
            // Validate input
            if (voterID.empty()) {
                throw invalid_argument("Voter ID cannot be empty");
            }
            if (name.empty()) {
                throw invalid_argument("Name cannot be empty");
            }
            
            Voter* temp = head;
            while (temp != nullptr) {
                if (temp->voterID == voterID) {
                    cout << "Voter ID already exists!\n";
                    return;
                }
                temp = temp->next;
            }
            
            Voter* newVoter = new Voter(voterID, name);
            if (newVoter == nullptr) {
                throw runtime_error("Memory allocation failed");
            }
            newVoter->next = head;
            head = newVoter;
            cout << "Voter registered successfully: " << name << " (ID: " << voterID << ")\n";
        } catch (const invalid_argument& e) {
            cout << "Invalid input: " << e.what() << "\n";
        } catch (const bad_alloc& e) {
            cout << "Memory allocation error: " << e.what() << "\n";
        } catch (const exception& e) {
            cout << "Error registering voter: " << e.what() << "\n";
        }
    }
    
    Voter* findVoter(string voterID) {
        Voter* temp = head;
        while (temp != nullptr) {
            if (temp->voterID == voterID) {
                return temp;
            }
            temp = temp->next;
        }
        return nullptr;
    }
    
    bool markAsVoted(string voterID) {
        Voter* voter = findVoter(voterID);
        if (voter == nullptr) {
            return false;
        }
        if (voter->hasVoted) {
            return false;
        }
        voter->hasVoted = true;
        return true;
    }
    
    void displayAllVoters() {
        cout << "\nREGISTERED VOTERS:\n";
        int count = 0;
        Voter* temp = head;
        while (temp != nullptr) {
            cout << "ID: " << temp->voterID << " | Name: " << temp->name 
                 << " | Voted: " << (temp->hasVoted ? "Yes" : "No") << "\n";
            count++;
            temp = temp->next;
        }
        cout << "Total registered voters: " << count << "\n";
    }
    
    // Save voter database to file
    bool saveToFile(const string& filename) {
        try {
            ofstream file(filename);
            if (!file.is_open()) {
                throw runtime_error("Unable to open file for writing");
            }
            
            Voter* temp = head;
            while (temp != nullptr) {
                file << temp->voterID << "|" << temp->name << "|" 
                     << temp->hasVoted << "\n";
                temp = temp->next;
            }
            file.close();
            return true;
        } catch (const exception& e) {
            cout << "Error saving voter database: " << e.what() << "\n";
            return false;
        }
    }
    
    // Load voter database from file
    bool loadFromFile(const string& filename) {
        try {
            ifstream file(filename);
            if (!file.is_open()) {
                return false; // File doesn't exist, not an error
            }
            
            string line;
            while (getline(file, line)) {
                stringstream ss(line);
                string voterID, name, votedStr;
                
                getline(ss, voterID, '|');
                getline(ss, name, '|');
                getline(ss, votedStr, '|');
                
                if (!voterID.empty() && !name.empty()) {
                    Voter* newVoter = new Voter(voterID, name);
                    newVoter->hasVoted = (votedStr == "1");
                    newVoter->next = head;
                    head = newVoter;
                }
            }
            file.close();
            return true;
        } catch (const exception& e) {
            cout << "Error loading voter database: " << e.what() << "\n";
            return false;
        }
    }
    
    ~VoterDatabase() {
        Voter* temp = head;
        while (temp != nullptr) {
            Voter* toDelete = temp;
            temp = temp->next;
            delete toDelete;
        }
    }
};

struct VoteRecord {
    string voterID;
    string candidate;
    time_t timestamp;
    string hash;
    string previousHash;
    VoteRecord* next;
    
    VoteRecord(string vID, string cand, string prevHash = "0") 
        : voterID(vID), candidate(cand), previousHash(prevHash), next(nullptr) {
        timestamp = time(nullptr);
        hash = calculateHash();
    }
    
    string calculateHash() {
        stringstream ss;
        ss << voterID << candidate << timestamp << previousHash;
        return generateHash(ss.str());
    }
};

class VoteLedger {
private:
    VoteRecord* head;
    VoteRecord* tail;
    int recordCount;
    
public:
    VoteLedger() : head(nullptr), tail(nullptr), recordCount(0) {}
    
    void addVote(string voterID, string candidate) {
        try {
            string prevHash = (tail != nullptr) ? tail->hash : "0";
            VoteRecord* newRecord = new VoteRecord(voterID, candidate, prevHash);
            
            if (newRecord == nullptr) {
                throw runtime_error("Memory allocation failed");
            }
            
            if (head == nullptr) {
                head = tail = newRecord;
            } else {
                tail->next = newRecord;
                tail = newRecord;
            }
            recordCount++;
            cout << "Vote recorded (Record #" << recordCount << ")\n";
        } catch (const exception& e) {
            cout << "Error adding vote to ledger: " << e.what() << "\n";
        }
    }
    
    void displayLedger() {
        cout << "\nVOTE LEDGER:\n";
        VoteRecord* current = head;
        int recordNum = 1;
        
        while (current != nullptr) {
            time_t t = current->timestamp;
            cout << "\nRecord #" << recordNum << "\n";
            cout << "Voter ID: " << current->voterID << "\n";
            cout << "Candidate: " << current->candidate << "\n";
            cout << "Timestamp: " << ctime(&t);
            cout << "Hash: " << current->hash << "\n";
            cout << "Previous Hash: " << current->previousHash << "\n";
            
            current = current->next;
            recordNum++;
        }
        cout << "\nTotal records: " << recordCount << "\n";
    }
    
    int getTotalVotes() {
        return recordCount;
    }
    
    // Verify blockchain integrity
    bool verifyChain() {
        if (head == nullptr) {
            return true;
        }
        
        VoteRecord* current = head;
        while (current != nullptr) {
            // Verify current block's hash
            string calculatedHash = current->calculateHash();
            if (calculatedHash != current->hash) {
                cout << "Chain integrity compromised: Hash mismatch in record\n";
                return false;
            }
            
            // Verify link to previous block
            if (current->next != nullptr) {
                if (current->hash != current->next->previousHash) {
                    cout << "Chain integrity compromised: Previous hash mismatch\n";
                    return false;
                }
            }
            current = current->next;
        }
        cout << "Blockchain verification successful: All records valid\n";
        return true;
    }
    
    // Save vote ledger to file
    bool saveToFile(const string& filename) {
        try {
            ofstream file(filename);
            if (!file.is_open()) {
                throw runtime_error("Unable to open file for writing");
            }
            
            VoteRecord* current = head;
            while (current != nullptr) {
                file << current->voterID << "|" << current->candidate << "|"
                     << current->timestamp << "|" << current->hash << "|"
                     << current->previousHash << "\n";
                current = current->next;
            }
            file.close();
            return true;
        } catch (const exception& e) {
            cout << "Error saving vote ledger: " << e.what() << "\n";
            return false;
        }
    }
    
    // Load vote ledger from file
    bool loadFromFile(const string& filename) {
        try {
            ifstream file(filename);
            if (!file.is_open()) {
                return false; // File doesn't exist
            }
            
            string line;
            while (getline(file, line)) {
                stringstream ss(line);
                string voterID, candidate, timestampStr, hash, previousHash;
                
                getline(ss, voterID, '|');
                getline(ss, candidate, '|');
                getline(ss, timestampStr, '|');
                getline(ss, hash, '|');
                getline(ss, previousHash, '|');
                
                if (!voterID.empty() && !candidate.empty()) {
                    VoteRecord* newRecord = new VoteRecord(voterID, candidate, previousHash);
                    newRecord->timestamp = stol(timestampStr);
                    newRecord->hash = hash;
                    
                    if (head == nullptr) {
                        head = tail = newRecord;
                    } else {
                        tail->next = newRecord;
                        tail = newRecord;
                    }
                    recordCount++;
                }
            }
            file.close();
            return true;
        } catch (const exception& e) {
            cout << "Error loading vote ledger: " << e.what() << "\n";
            return false;
        }
    }
    
    ~VoteLedger() {
        VoteRecord* current = head;
        while (current != nullptr) {
            VoteRecord* toDelete = current;
            current = current->next;
            delete toDelete;
        }
    }
};

struct CandidateNode {
    string name;
    int voteCount;
    CandidateNode* left;
    CandidateNode* right;
    
    CandidateNode(string n) : name(n), voteCount(0), left(nullptr), right(nullptr) {}
};

class CandidateBST {
private:
    CandidateNode* root;
    
    CandidateNode* insert(CandidateNode* node, string name) {
        if (node == nullptr) {
            return new CandidateNode(name);
        }
        if (name < node->name) {
            node->left = insert(node->left, name);
        } else if (name > node->name) {
            node->right = insert(node->right, name);
        }
        return node;
    }
    
    CandidateNode* search(CandidateNode* node, string name) {
        if (node == nullptr || node->name == name) {
            return node;
        }
        if (name < node->name) {
            return search(node->left, name);
        }
        return search(node->right, name);
    }
    
    void inorderTraversal(CandidateNode* node) {
        if (node != nullptr) {
            inorderTraversal(node->left);
            cout << node->name << ": " << node->voteCount << " votes\n";
            inorderTraversal(node->right);
        }
    }
    
    void collectCandidates(CandidateNode* node, vector<pair<string, int>>& candidates) {
        if (node != nullptr) {
            collectCandidates(node->left, candidates);
            candidates.push_back({node->name, node->voteCount});
            collectCandidates(node->right, candidates);
        }
    }
    
    int getTotalVotes(CandidateNode* node) {
        if (node == nullptr) return 0;
        return node->voteCount + getTotalVotes(node->left) + getTotalVotes(node->right);
    }
    
    void destroyTree(CandidateNode* node) {
        if (node != nullptr) {
            destroyTree(node->left);
            destroyTree(node->right);
            delete node;
        }
    }
    
    void saveToFileHelper(CandidateNode* node, ofstream& file) {
        if (node != nullptr) {
            file << node->name << "|" << node->voteCount << "\n";
            saveToFileHelper(node->left, file);
            saveToFileHelper(node->right, file);
        }
    }
    
public:
    CandidateBST() : root(nullptr) {}
    
    void addCandidate(string name) {
        root = insert(root, name);
        cout << "Candidate added: " << name << "\n";
    }
    
    bool addVote(string name) {
        CandidateNode* candidate = search(root, name);
        if (candidate == nullptr) {
            return false;
        }
        candidate->voteCount++;
        return true;
    }
    
    void displayResults() {
        cout << "\nELECTION RESULTS (Sorted):\n";
        inorderTraversal(root);
    }
    
    string getWinner() {
        if (root == nullptr) return "No candidates";
        vector<pair<string, int>> allCandidates;
        collectCandidates(root, allCandidates);
        string winner = "";
        int maxVotes = -1;
        for (const auto& candidate : allCandidates) {
            if (candidate.second > maxVotes) {
                maxVotes = candidate.second;
                winner = candidate.first;
            }
        }
        return winner;
    }
    
    void getVotePercentages() {
        if (root == nullptr) {
            cout << "No votes cast yet.\n";
            return;
        }
        vector<pair<string, int>> allCandidates;
        collectCandidates(root, allCandidates);
        int total = getTotalVotes(root);
        if (total == 0) {
            cout << "No votes cast yet.\n";
            return;
        }
        cout << "\nVOTE PERCENTAGES:\n";
        for (const auto& candidate : allCandidates) {
            double percentage = (candidate.second * 100.0) / total;
            cout << candidate.first << ": " << candidate.second << " votes (" 
                 << fixed << setprecision(2) << percentage << "%)\n";
        }
    }
    
    void generateStatistics() {
        if (root == nullptr) {
            cout << "No candidates in the system.\n";
            return;
        }
        vector<pair<string, int>> allCandidates;
        collectCandidates(root, allCandidates);
        int total = getTotalVotes(root);
        cout << "\nELECTION STATISTICS:\n";
        cout << "Total Votes Cast: " << total << "\n";
        cout << "Number of Candidates: " << allCandidates.size() << "\n";
        if (total > 0) {
            string winner = getWinner();
            cout << "Leading Candidate: " << winner << "\n";
            int winnerVotes = 0;
            for (const auto& candidate : allCandidates) {
                if (candidate.first == winner) {
                    winnerVotes = candidate.second;
                    break;
                }
            }
            double winnerPercentage = (winnerVotes * 100.0) / total;
            cout << "Leading Candidate Votes: " << winnerVotes << " (" 
                 << fixed << setprecision(2) << winnerPercentage << "%)\n";
            double avgVotes = (double)total / allCandidates.size();
            cout << "Average Votes per Candidate: " << fixed << setprecision(2) << avgVotes << "\n";
            int zeroVoteCandidates = 0;
            for (const auto& candidate : allCandidates) {
                if (candidate.second == 0) {
                    zeroVoteCandidates++;
                }
            }
            cout << "Candidates with Zero Votes: " << zeroVoteCandidates << "\n";
        }
    }
    
    bool exportResults(const string& filename) {
        try {
            ofstream file(filename);
            if (!file.is_open()) {
                throw runtime_error("Unable to open file for writing");
            }
            vector<pair<string, int>> allCandidates;
            collectCandidates(root, allCandidates);
            int total = getTotalVotes(root);
            
            time_t now = time(nullptr);
            file << "ELECTION RESULTS REPORT\n";
            file << "Generated: " << ctime(&now) << "\n";
            file << "CANDIDATE VOTES:\n";
            for (const auto& candidate : allCandidates) {
                double percentage = (total > 0) ? (candidate.second * 100.0) / total : 0;
                file << candidate.first << ": " << candidate.second << " votes (" 
                     << fixed << setprecision(2) << percentage << "%)\n";
            }
            file << "\nSTATISTICS:\n";
            file << "Total Votes: " << total << "\n";
            file << "Total Candidates: " << allCandidates.size() << "\n";
            if (total > 0) {
                string winner = getWinner();
                file << "Winner: " << winner << "\n";
            }
            file.close();
            cout << "Results exported to " << filename << "\n";
            return true;
        } catch (const exception& e) {
            cout << "Error exporting results: " << e.what() << "\n";
            return false;
        }
    }
    
    bool candidateExists(string name) {
        return search(root, name) != nullptr;
    }
    
    bool saveToFile(const string& filename) {
        try {
            ofstream file(filename);
            if (!file.is_open()) {
                throw runtime_error("Unable to open file for writing");
            }
            saveToFileHelper(root, file);
            file.close();
            return true;
        } catch (const exception& e) {
            cout << "Error saving candidates: " << e.what() << "\n";
            return false;
        }
    }
    
    bool loadFromFile(const string& filename) {
        try {
            ifstream file(filename);
            if (!file.is_open()) {
                return false;
            }
            
            string line;
            while (getline(file, line)) {
                stringstream ss(line);
                string name, voteCountStr;
                
                getline(ss, name, '|');
                getline(ss, voteCountStr, '|');
                
                if (!name.empty()) {
                    root = insert(root, name);
                    CandidateNode* node = search(root, name);
                    if (node != nullptr) {
                        node->voteCount = stoi(voteCountStr);
                    }
                }
            }
            file.close();
            return true;
        } catch (const exception& e) {
            cout << "Error loading candidates: " << e.what() << "\n";
            return false;
        }
    }
    
    ~CandidateBST() {
        destroyTree(root);
    }
};

class VotingSystem {
private:
    VoterDatabase voterDB;
    VoteLedger ledger;
    CandidateBST candidates;
    
public:
    void initializeCandidates() {
        candidates.addCandidate("Akram");
        candidates.addCandidate("Kashan");
        candidates.addCandidate("Mubashir");
        candidates.addCandidate("Suleman");
    }
    
    void registerVoter(string voterID, string name) {
        voterDB.insertVoter(voterID, name);
    }
    
    void castVote(string voterID, string candidate) {
        try {
            if (voterID.empty()) {
                throw invalid_argument("Voter ID cannot be empty");
            }
            if (candidate.empty()) {
                throw invalid_argument("Candidate name cannot be empty");
            }
            
            Voter* voter = voterDB.findVoter(voterID);
            if (voter == nullptr) {
                throw runtime_error("Voter ID not found! Please register first.");
            }
            if (voter->hasVoted) {
                throw runtime_error("This voter has already cast their vote!");
            }
            if (!candidates.candidateExists(candidate)) {
                throw invalid_argument("Invalid candidate name!");
            }
            
            if (!voterDB.markAsVoted(voterID)) {
                throw runtime_error("Failed to mark voter as voted");
            }
            
            ledger.addVote(voterID, candidate);
            
            if (!candidates.addVote(candidate)) {
                throw runtime_error("Failed to record vote for candidate");
            }
            
            cout << "Vote successfully cast for " << candidate << "!\n";
        } catch (const invalid_argument& e) {
            cout << "Invalid input: " << e.what() << "\n";
        } catch (const runtime_error& e) {
            cout << "Error: " << e.what() << "\n";
        } catch (const exception& e) {
            cout << "Unexpected error casting vote: " << e.what() << "\n";
        }
    }
    
    void displayResults() {
        candidates.displayResults();
        cout << "Total votes cast: " << ledger.getTotalVotes() << "\n";
    }
    
    void displayLedger() {
        ledger.displayLedger();
    }
    
    void displayVoters() {
        voterDB.displayAllVoters();
    }
    
    void showPercentages() {
        candidates.getVotePercentages();
    }
    
    void showStatistics() {
        candidates.generateStatistics();
    }
    
    void exportElectionResults(const string& filename) {
        candidates.exportResults(filename);
    }
    
    // Save complete system state
    bool saveSystemState(const string& baseFilename) {
        try {
            bool success = true;
            success &= voterDB.saveToFile(baseFilename + "_voters.txt");
            success &= ledger.saveToFile(baseFilename + "_ledger.txt");
            success &= candidates.saveToFile(baseFilename + "_candidates.txt");
            
            if (success) {
                cout << "System state saved successfully\n";
            } else {
                cout << "Warning: Some files may not have been saved properly\n";
            }
            return success;
        } catch (const exception& e) {
            cout << "Error saving system state: " << e.what() << "\n";
            return false;
        }
    }
    
    // Load complete system state
    bool loadSystemState(const string& baseFilename) {
        try {
            bool votersLoaded = voterDB.loadFromFile(baseFilename + "_voters.txt");
            bool ledgerLoaded = ledger.loadFromFile(baseFilename + "_ledger.txt");
            bool candidatesLoaded = candidates.loadFromFile(baseFilename + "_candidates.txt");
            
            if (votersLoaded || ledgerLoaded || candidatesLoaded) {
                cout << "System state loaded successfully\n";
                return true;
            }
            return false;
        } catch (const exception& e) {
            cout << "Error loading system state: " << e.what() << "\n";
            return false;
        }
    }
    
    // Generate comprehensive report
    bool exportReport(const string& filename) {
        try {
            ofstream file(filename);
            if (!file.is_open()) {
                throw runtime_error("Unable to open file for writing");
            }
            
            time_t now = time(nullptr);
            file << "====================================\n";
            file << "COMPREHENSIVE ELECTION REPORT\n";
            file << "====================================\n";
            file << "Generated: " << ctime(&now) << "\n";
            
            file << "\n--- ELECTION RESULTS ---\n";
            candidates.exportResults("temp_results.txt");
            
            file << "\n--- BLOCKCHAIN STATUS ---\n";
            if (ledger.verifyChain()) {
                file << "Blockchain Integrity: VERIFIED\n";
            } else {
                file << "Blockchain Integrity: COMPROMISED\n";
            }
            file << "Total Records: " << ledger.getTotalVotes() << "\n";
            
            file.close();
            cout << "Comprehensive report exported to " << filename << "\n";
            return true;
        } catch (const exception& e) {
            cout << "Error exporting report: " << e.what() << "\n";
            return false;
        }
    }
    
    // Audit blockchain integrity
    void auditBlockchain() {
        cout << "\n=== BLOCKCHAIN AUDIT ===\n";
        if (ledger.verifyChain()) {
            cout << "Status: SECURE\n";
            cout << "All vote records are intact and verified.\n";
        } else {
            cout << "Status: COMPROMISED\n";
            cout << "WARNING: Tampering detected in blockchain!\n";
        }
    }
    
    // Detect tampering
    void detectTampering() {
        cout << "\n=== TAMPERING DETECTION ===\n";
        if (ledger.verifyChain()) {
            cout << "No tampering detected. System integrity maintained.\n";
        } else {
            cout << "ALERT: Potential tampering detected!\n";
            cout << "Please review the blockchain immediately.\n";
        }
    }
};

void displayMenu() {
    cout << "\n========================================\n";
    cout << "     SIMPLE E-VOTING SYSTEM\n";
    cout << "========================================\n";
    cout << "1.  Register Voter\n";
    cout << "2.  Cast Vote\n";
    cout << "3.  Display Election Results\n";
    cout << "4.  View Vote Ledger\n";
    cout << "5.  View Registered Voters\n";
    cout << "6.  Show Vote Percentages\n";
    cout << "7.  Show Statistics\n";
    cout << "8.  Export Results to File\n";
    cout << "9.  Audit Blockchain\n";
    cout << "10. Detect Tampering\n";
    cout << "11. Save System State\n";
    cout << "12. Load System State\n";
    cout << "0.  Exit\n";
    cout << "========================================\n";
    cout << "Choose an option: ";
}

int main() {
    try {
        VotingSystem system;
        cout << "Initializing E-Voting System...\n\n";
        
        // Try to load existing data
        if (!system.loadSystemState("voting_data")) {
            cout << "No previous data found. Starting fresh.\n";
            system.initializeCandidates();
            
            // Register some default voters
            system.registerVoter("V001", "Abbad");
            system.registerVoter("V002", "Talal");
            system.registerVoter("V003", "Haziq");
        }
        
        int choice;
        string voterID, name, candidate, filename;
        
        while (true) {
            try {
                displayMenu();
                
                if (!(cin >> choice)) {
                    cin.clear();
                    cin.ignore(10000, '\n');
                    throw invalid_argument("Invalid input! Please enter a number.");
                }
                cin.ignore();
                
                switch (choice) {
                    case 1:
                        cout << "Enter Voter ID: ";
                        getline(cin, voterID);
                        cout << "Enter Name: ";
                        getline(cin, name);
                        system.registerVoter(voterID, name);
                        break;
                        
                    case 2:
                        cout << "Enter Voter ID: ";
                        getline(cin, voterID);
                        cout << "Available Candidates:\n";
                        cout << "  - Akram\n";
                        cout << "  - Kashan\n";
                        cout << "  - Mubashir\n";
                        cout << "  - Suleman\n";
                        cout << "Enter Candidate Name: ";
                        getline(cin, candidate);
                        system.castVote(voterID, candidate);
                        break;
                        
                    case 3:
                        system.displayResults();
                        break;
                        
                    case 4:
                        system.displayLedger();
                        break;
                        
                    case 5:
                        system.displayVoters();
                        break;
                        
                    case 6:
                        system.showPercentages();
                        break;
                        
                    case 7:
                        system.showStatistics();
                        break;
                        
                    case 8:
                        cout << "Enter filename (e.g., results.txt): ";
                        getline(cin, filename);
                        system.exportElectionResults(filename);
                        break;
                        
                    case 9:
                        system.auditBlockchain();
                        break;
                        
                    case 10:
                        system.detectTampering();
                        break;
                        
                    case 11:
                        system.saveSystemState("voting_data");
                        break;
                        
                    case 12:
                        if (system.loadSystemState("voting_data")) {
                            cout << "System state loaded successfully!\n";
                        } else {
                            cout << "No saved data found or error loading.\n";
                        }
                        break;
                        
                    case 0:
                        cout << "\nSaving system state before exit...\n";
                        system.saveSystemState("voting_data");
                        cout << "Exiting E-Voting System. Thank you!\n";
                        return 0;
                        
                    default:
                        cout << "Invalid choice! Please try again.\n";
                }
            } catch (const invalid_argument& e) {
                cout << "Input error: " << e.what() << "\n";
            } catch (const runtime_error& e) {
                cout << "Runtime error: " << e.what() << "\n";
            } catch (const exception& e) {
                cout << "Error: " << e.what() << "\n";
            }
        }
    } catch (const exception& e) {
        cout << "Critical error: " << e.what() << "\n";
        cout << "The system encountered a fatal error and must exit.\n";
        return 1;
    } catch (...) {
        cout << "Unknown critical error occurred!\n";
        return 1;
    }
    return 0;
}