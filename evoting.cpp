#include <iostream>
#include <string>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <vector>
#include <fstream>
using namespace std;

// ==================== HASH FUNCTION ====================
size_t simpleHash(const string& key, int tableSize) {
    size_t hash = 0;
    for (char c : key) {
        hash = (hash * 31 + c) % tableSize;
    }
    return hash;
}

string generateHash(const string& data) {
    size_t hash = 5381;
    for (char c : data) {
        hash = ((hash << 5) + hash) + c;
    }
    stringstream ss;
    ss << hex << hash;
    return ss.str();
}

// ==================== VOTER STRUCTURE ====================
struct Voter {
    string voterID;
    string name;
    bool hasVoted;
    Voter* next;
    
    Voter(string id, string n) : voterID(id), name(n), hasVoted(false), next(nullptr) {}
};

// ==================== HASH TABLE (VOTER DATABASE) ====================
class VoterHashTable {
private:
    static const int TABLE_SIZE = 100;
    Voter* table[TABLE_SIZE];
    
public:
    VoterHashTable() {
        for (int i = 0; i < TABLE_SIZE; i++) {
            table[i] = nullptr;
        }
    }
    
    void insertVoter(string voterID, string name) {
        size_t index = simpleHash(voterID, TABLE_SIZE);
        Voter* newVoter = new Voter(voterID, name);
        
        if (table[index] == nullptr) {
            table[index] = newVoter;
        } else {
            Voter* temp = table[index];
            while (temp->next != nullptr) {
                if (temp->voterID == voterID) {
                    cout << "❌ Voter ID already exists!\n";
                    delete newVoter;
                    return;
                }
                temp = temp->next;
            }
            if (temp->voterID == voterID) {
                cout << "❌ Voter ID already exists!\n";
                delete newVoter;
                return;
            }
            temp->next = newVoter;
        }
        cout << "✅ Voter registered successfully: " << name << " (ID: " << voterID << ")\n";
    }
    
    Voter* findVoter(string voterID) {
        size_t index = simpleHash(voterID, TABLE_SIZE);
        Voter* temp = table[index];
        
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
        cout << "\n========== REGISTERED VOTERS ==========\n";
        int count = 0;
        for (int i = 0; i < TABLE_SIZE; i++) {
            Voter* temp = table[i];
            while (temp != nullptr) {
                cout << "ID: " << temp->voterID << " | Name: " << temp->name 
                     << " | Voted: " << (temp->hasVoted ? "Yes" : "No") << "\n";
                count++;
                temp = temp->next;
            }
        }
        cout << "Total registered voters: " << count << "\n";
    }
    
    ~VoterHashTable() {
        for (int i = 0; i < TABLE_SIZE; i++) {
            Voter* temp = table[i];
            while (temp != nullptr) {
                Voter* toDelete = temp;
                temp = temp->next;
                delete toDelete;
            }
        }
    }
};

// ==================== BLOCK STRUCTURE (BLOCKCHAIN NODE) ====================
struct Block {
    string voterID;
    string candidate;
    time_t timestamp;
    string hash;
    string previousHash;
    Block* next;
    
    Block(string vID, string cand, string prevHash) 
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

// ==================== BLOCKCHAIN (LINKED LIST) ====================
class Blockchain {
private:
    Block* head;
    Block* tail;
    int blockCount;
    
public:
    Blockchain() : head(nullptr), tail(nullptr), blockCount(0) {
        // Create genesis block
        addGenesisBlock();
    }
    
    void addGenesisBlock() {
        Block* genesis = new Block("GENESIS", "GENESIS", "0");
        head = tail = genesis;
        blockCount++;
    }
    
    void addBlock(string voterID, string candidate) {
        string prevHash = tail->hash;
        Block* newBlock = new Block(voterID, candidate, prevHash);
        tail->next = newBlock;
        tail = newBlock;
        blockCount++;
        cout << "✅ Vote recorded in blockchain (Block #" << blockCount << ")\n";
    }
    
    bool verifyChain() {
        cout << "\n========== BLOCKCHAIN AUDIT ==========\n";
        Block* current = head->next; // Skip genesis
        Block* previous = head;
        bool isValid = true;
        int blockNum = 1;
        
        while (current != nullptr) {
            // Verify hash integrity
            string recalculatedHash = current->calculateHash();
            if (current->hash != recalculatedHash) {
                cout << "❌ Block #" << blockNum << " has been tampered! Hash mismatch.\n";
                isValid = false;
            }
            
            // Verify chain linkage
            if (current->previousHash != previous->hash) {
                cout << "❌ Block #" << blockNum << " chain linkage broken!\n";
                isValid = false;
            }
            
            previous = current;
            current = current->next;
            blockNum++;
        }
        
        if (isValid) {
            cout << "✅ Blockchain integrity verified! All " << blockCount << " blocks are valid.\n";
        } else {
            cout << "⚠️  Blockchain has been compromised!\n";
        }
        return isValid;
    }
    
    void displayChain() {
        cout << "\n========== BLOCKCHAIN LEDGER ==========\n";
        Block* current = head->next; // Skip genesis
        int blockNum = 1;
        
        while (current != nullptr) {
            time_t t = current->timestamp;
            cout << "\n--- Block #" << blockNum << " ---\n";
            cout << "Voter ID: " << current->voterID << "\n";
            cout << "Candidate: " << current->candidate << "\n";
            cout << "Timestamp: " << ctime(&t);
            cout << "Hash: " << current->hash.substr(0, 16) << "...\n";
            cout << "Prev Hash: " << current->previousHash.substr(0, 16) << "...\n";
            
            current = current->next;
            blockNum++;
        }
        cout << "\nTotal blocks: " << blockCount << "\n";
    }
    
    int getTotalVotes() {
        return blockCount - 1; // Exclude genesis block
    }
    
    ~Blockchain() {
        Block* current = head;
        while (current != nullptr) {
            Block* toDelete = current;
            current = current->next;
            delete toDelete;
        }
    }
};

// ==================== BST NODE (CANDIDATE) ====================
struct CandidateNode {
    string name;
    int voteCount;
    CandidateNode* left;
    CandidateNode* right;
    
    CandidateNode(string n) : name(n), voteCount(0), left(nullptr), right(nullptr) {}
};

// ==================== BINARY SEARCH TREE (CANDIDATE MANAGEMENT) ====================
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
            cout << "🗳️  " << node->name << ": " << node->voteCount << " votes\n";
            inorderTraversal(node->right);
        }
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
        cout << "✅ Candidate added: " << name << "\n";
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
        cout << "\n========== ELECTION RESULTS (Sorted) ==========\n";
        inorderTraversal(root);
        cout << "===============================================\n";
    }
    
    bool candidateExists(string name) {
        return search(root, name) != nullptr;
    }
    
    ~CandidateBST() {
        destroyTree(root);
    }
};

// ==================== MAIN VOTING SYSTEM ====================
class VotingSystem {
private:
    VoterHashTable voterDB;
    Blockchain blockchain;
    CandidateBST candidates;
    
public:
    void initializeCandidates() {
        candidates.addCandidate("Alice Johnson");
        candidates.addCandidate("Bob Smith");
        candidates.addCandidate("Charlie Brown");
        candidates.addCandidate("Diana Prince");
    }
    
    void registerVoter(string voterID, string name) {
        voterDB.insertVoter(voterID, name);
    }
    
    void castVote(string voterID, string candidate) {
        // Verify voter exists
        Voter* voter = voterDB.findVoter(voterID);
        if (voter == nullptr) {
            cout << "❌ Voter ID not found! Please register first.\n";
            return;
        }
        
        // Check if already voted
        if (voter->hasVoted) {
            cout << "❌ This voter has already cast their vote!\n";
            return;
        }
        
        // Verify candidate exists
        if (!candidates.candidateExists(candidate)) {
            cout << "❌ Invalid candidate name!\n";
            return;
        }
        
        // Record vote
        voterDB.markAsVoted(voterID);
        blockchain.addBlock(voterID, candidate);
        candidates.addVote(candidate);
        
        cout << "🎉 Vote successfully cast for " << candidate << "!\n";
    }
    
    void auditBlockchain() {
        blockchain.verifyChain();
    }
    
    void displayResults() {
        candidates.displayResults();
        cout << "Total votes cast: " << blockchain.getTotalVotes() << "\n";
    }
    
    void displayBlockchain() {
        blockchain.displayChain();
    }
    
    void displayVoters() {
        voterDB.displayAllVoters();
    }
};

// ==================== MENU INTERFACE ====================
void displayMenu() {
    cout << "\n╔════════════════════════════════════════════╗\n";
    cout << "║   E-VOTING SYSTEM WITH BLOCKCHAIN AUDIT   ║\n";
    cout << "╚════════════════════════════════════════════╝\n";
    cout << "1. Register Voter\n";
    cout << "2. Cast Vote\n";
    cout << "3. Display Election Results\n";
    cout << "4. Audit Blockchain\n";
    cout << "5. View Blockchain Ledger\n";
    cout << "6. View Registered Voters\n";
    cout << "0. Exit\n";
    cout << "Choose an option: ";
}

int main() {
    VotingSystem system;
    
    // Initialize candidates
    cout << "Initializing E-Voting System...\n\n";
    system.initializeCandidates();
    
    // Pre-register some voters for testing
    system.registerVoter("V001", "John Doe");
    system.registerVoter("V002", "Jane Smith");
    system.registerVoter("V003", "Mike Johnson");
    
    int choice;
    string voterID, name, candidate;
    
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
                cout << "  - Alice Johnson\n";
                cout << "  - Bob Smith\n";
                cout << "  - Charlie Brown\n";
                cout << "  - Diana Prince\n";
                cout << "Enter Candidate Name: ";
                getline(cin, candidate);
                system.castVote(voterID, candidate);
                break;
                
            case 3:
                system.displayResults();
                break;
                
            case 4:
                system.auditBlockchain();
                break;
                
            case 5:
                system.displayBlockchain();
                break;
                
            case 6:
                system.displayVoters();
                break;
                
            case 0:
                cout << "Exiting E-Voting System. Thank you!\n";
                return 0;
                
            default:
                cout << "Invalid choice! Please try again.\n";
        }
    }
    
    return 0;
}