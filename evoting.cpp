#include <iostream>
#include <string>
#include <ctime>
#include <vector>
#include <fstream>
#include <stdexcept>
#include <iomanip>
using namespace std;

// TODO: Add hash functions here for blockchain implementation
// - simpleHash(key, tableSize) for hash table indexing
// - generateHash(data) for blockchain block hashing

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
        // TODO: Add exception handling for invalid input
        try {
            Voter* temp = head;
            while (temp != nullptr) {
                if (temp->voterID == voterID) {
                    cout << "Voter ID already exists!\n";
                    return;
                }
                temp = temp->next;
            }
            
            Voter* newVoter = new Voter(voterID, name);
            newVoter->next = head;
            head = newVoter;
            cout << "Voter registered successfully: " << name << " (ID: " << voterID << ")\n";
        } catch (const exception& e) {
            // TODO: Implement proper exception handling
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
    
    // TODO: Add file persistence methods
    // - saveToFile(filename)
    // - loadFromFile(filename)
    
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
    VoteRecord* next;
    
    // TODO: Add hash field for blockchain implementation
    // string hash;
    // string previousHash;
    
    VoteRecord(string vID, string cand) 
        : voterID(vID), candidate(cand), next(nullptr) {
        timestamp = time(nullptr);
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
        VoteRecord* newRecord = new VoteRecord(voterID, candidate);
        
        if (head == nullptr) {
            head = tail = newRecord;
        } else {
            tail->next = newRecord;
            tail = newRecord;
        }
        recordCount++;
        cout << "Vote recorded (Record #" << recordCount << ")\n";
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
            
            current = current->next;
            recordNum++;
        }
        cout << "\nTotal records: " << recordCount << "\n";
    }
    
    int getTotalVotes() {
        return recordCount;
    }
    
    // TODO: Add blockchain integrity verification
    // - verifyChain()
    // - calculateHash()
    
    // TODO: Add vote ledger file handling
    // - saveToFile(filename)
    // - loadFromFile(filename)
    
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
        ofstream file(filename);
        if (!file.is_open()) {
            cout << "Error: Unable to open file for writing.\n";
            return false;
        }
        vector<pair<string, int>> allCandidates;
        collectCandidates(root, allCandidates);
        int total = getTotalVotes(root);
        file << "ELECTION RESULTS REPORT\n";
        file << "Generated: " << time(nullptr) << "\n\n";
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
    }
    
    bool candidateExists(string name) {
        return search(root, name) != nullptr;
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
        // TODO: Add comprehensive exception handling
        try {
            Voter* voter = voterDB.findVoter(voterID);
            if (voter == nullptr) {
                cout << "Voter ID not found! Please register first.\n";
                return;
            }
            if (voter->hasVoted) {
                cout << "This voter has already cast their vote!\n";
                return;
            }
            if (!candidates.candidateExists(candidate)) {
                cout << "Invalid candidate name!\n";
                return;
            }
            voterDB.markAsVoted(voterID);
            ledger.addVote(voterID, candidate);
            candidates.addVote(candidate);
            cout << "Vote successfully cast for " << candidate << "!\n";
        } catch (const exception& e) {
            // TODO: Implement proper exception handling
            cout << "Error casting vote: " << e.what() << "\n";
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
    
    // TODO: Add system-wide file handling
    // - saveSystemState(filename)
    // - loadSystemState(filename)
    // - exportReport(filename)
    
    // TODO: Add blockchain verification
    // - auditBlockchain()
    // - detectTampering()
};

void displayMenu() {
    cout << "\nSIMPLE E-VOTING SYSTEM\n";
    cout << "1. Register Voter\n";
    cout << "2. Cast Vote\n";
    cout << "3. Display Election Results\n";
    cout << "4. View Vote Ledger\n";
    cout << "5. View Registered Voters\n";
    cout << "6. Show Vote Percentages\n";
    cout << "7. Show Statistics\n";
    cout << "8. Export Results to File\n";
    cout << "0. Exit\n";
    cout << "Choose an option: ";
}

int main() {
    // TODO: Add exception handling for main program
    try {
        VotingSystem system;
        cout << "Initializing E-Voting System...\n\n";
        system.initializeCandidates();
        
        // TODO: Add file loading here
        // system.loadSystemState("voting_data.txt");
        
        system.registerVoter("V001", "Abbad");
        system.registerVoter("V002", "Talal");
        system.registerVoter("V003", "Haziq");
        
        int choice;
        string voterID, name, candidate, filename;
        
        while (true) {
            displayMenu();
            cin >> choice;
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
                    
                case 0:
                    // TODO: Add file saving here before exit
                    // system.saveSystemState("voting_data.txt");
                    cout << "Exiting E-Voting System. Thank you!\n";
                    return 0;
                    
                default:
                    cout << "Invalid choice! Please try again.\n";
            }
        }
    } catch (const exception& e) {
        // TODO: Implement comprehensive error handling
        cout << "Critical error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
