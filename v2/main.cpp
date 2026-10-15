#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <queue>
#include <iomanip>
#include <limits>

using namespace std;

// Minimal single-file Huffman Compression Tool v2
// Designed for clarity, simplicity, and efficiency.

// A Node in the Huffman Tree
struct Node {
    char character;
    int frequency;
    Node *left, *right;
    
    // Constructor initializes the node. Default left/right children are nullptr.
    Node(char ch, int freq, Node* l = nullptr, Node* r = nullptr) 
        : character(ch), frequency(freq), left(l), right(r) {}
        
    // Destructor recursively deletes children to prevent memory leaks
    ~Node() { 
        delete left; 
        delete right; 
    }
};

// Comparator for the priority queue (Min-Heap)
// We want nodes with the lowest frequency to have the highest priority.
struct CompareNodes {
    bool operator()(Node* leftNode, Node* rightNode) { 
        return leftNode->frequency > rightNode->frequency; 
    }
};

class HuffmanV2 {
    Node* root = nullptr;                           // Root of the Huffman Tree
    unordered_map<char, string> huffmanCodes;       // Stores mapping of Character -> Binary Code (e.g., 'a' -> "101")
    unordered_map<char, int> characterFrequencies;  // Stores frequency of each character
    size_t originalSize = 0, compressedSize = 0;    // Tracks file sizes for statistics

    // Recursively traverses the tree to build '0' and '1' strings for each leaf
    void buildCodes(Node* node, string currentCode) {
        if (!node) return;
        
        // If it's a leaf node, assign the accumulated code to the character
        if (!node->left && !node->right) {
            huffmanCodes[node->character] = currentCode;
        }
        
        // Traverse left (append '0') and right (append '1')
        buildCodes(node->left, currentCode + "0");
        buildCodes(node->right, currentCode + "1");
    }

    // Displays the tree in the console (helpful for debugging/understanding)
    void printTree(Node* node, string indent, bool isLast) {
        if (!node) return;
        
        // Print current node
        cout << indent << "+- " 
             << (node->left || node->right ? "*" : (node->character == '\n' ? "\\n" : string(1, node->character))) 
             << " (" << node->frequency << ")\n";
             
        indent += isLast ? "   " : "|  ";
        
        // Recursively print children
        if (node->left) printTree(node->left, indent, !node->right);
        if (node->right) printTree(node->right, indent, true);
    }

    // Constructs the Huffman Tree using a Min-Heap
    void buildTree() {
        delete root; 
        root = nullptr; 
        huffmanCodes.clear();
        
        if (characterFrequencies.empty()) return;
        
        // Priority Queue (Min-Heap) to constantly fetch the two lowest frequency nodes
        priority_queue<Node*, vector<Node*>, CompareNodes> minHeap;
        
        // 1. Push all characters as leaf nodes into the min-heap
        for (auto& pair : characterFrequencies) {
            minHeap.push(new Node(pair.first, pair.second));
        }
        
        // 2. Continually merge the two smallest nodes until 1 tree remains
        while (minHeap.size() > 1) {
            Node *leftChild = minHeap.top(); minHeap.pop();
            Node *rightChild = minHeap.top(); minHeap.pop();
            
            // Create a parent node (internal node, arbitrary character '\0')
            minHeap.push(new Node('\0', leftChild->frequency + rightChild->frequency, leftChild, rightChild));
        }
        
        // 3. The final node is the root of the tree
        root = minHeap.top();
        
        // 4. Generate the prefix codes
        buildCodes(root, "");
    }

    // Pre-order traversal to encode the tree into a string
    // '1' indicates a leaf node (followed by character), '0' indicates an internal node
    void serializeTree(Node* node, string& outStructure) {
        if (!node) return;
        
        if (!node->left && !node->right) {
            outStructure += '1';
            outStructure += node->character;
        } else {
            outStructure += '0';
            serializeTree(node->left, outStructure);
            serializeTree(node->right, outStructure);
        }
    }

    // Recursively reconstructs the tree from the serialized string
    Node* deserializeTree(const string& data, int& index) {
        if (index >= data.size()) return nullptr;
        
        // If it's a leaf node ('1'), read the character and return a leaf
        if (data[index++] == '1') {
            return new Node(data[index++], 0);
        }
        
        // If it's an internal node ('0'), recursively build left and right children
        Node* leftChild = deserializeTree(data, index);
        Node* rightChild = deserializeTree(data, index);
        
        return new Node('\0', 0, leftChild, rightChild);
    }

    // Writes the serialized Huffman Tree to the output file header
    void writeHeader(ofstream& outStream) {
        string treeData;
        serializeTree(root, treeData);
        
        int treeSize = treeData.size();
        
        // Write the size of the tree data, then the tree data itself
        outStream.write((char*)&treeSize, sizeof(treeSize));
        outStream.write(treeData.c_str(), treeSize);
    }

    // Reads and reconstructs the Huffman Tree from the input file header
    void readHeader(ifstream& inStream) {
        int treeSize;
        
        // Read the size of the tree data
        inStream.read((char*)&treeSize, sizeof(treeSize));
        
        // Read the exact amount of tree string data
        string treeData(treeSize, '\0');
        inStream.read(&treeData[0], treeSize);
        
        // Reconstruct the tree and generate the codes for decompression
        int index = 0;
        root = deserializeTree(treeData, index);
        
        huffmanCodes.clear();
        buildCodes(root, "");
    }

public:
    ~HuffmanV2() { delete root; }

    bool compress(const string& inputPath, const string& outputPath) {
        ifstream inFile(inputPath, ios::binary);
        if (!inFile) { cout << "  [ERROR] Cannot open input file!\n"; return false; }
        
        characterFrequencies.clear();
        originalSize = 0;
        char ch;
        
        // 1. Calculate frequencies
        while (inFile.get(ch)) { 
            characterFrequencies[ch]++; 
            originalSize++; 
        }
        
        // Rewind completely for the encoding pass
        inFile.clear(); 
        inFile.seekg(0);

        // 2. Build Tree and Codes
        buildTree();

        ofstream outFile(outputPath, ios::binary);
        if (!outFile) { cout << "  [ERROR] Cannot open output file!\n"; return false; }
        
        // 3. Write Tree to Header
        writeHeader(outFile);
        
        // 4. Encode Text and write as actual BITS using bitwise shifting
        char bitBuffer = 0;
        int bitsInCurrentByte = 0;
        compressedSize = outFile.tellp(); // Get the size of the header so far
        
        // 5. Save the exact original file size (prevents extra padding bits from being decoded)
        outFile.write((char*)&originalSize, sizeof(originalSize)); 
        
        while (inFile.get(ch)) {
            for (char bit : huffmanCodes[ch]) {
                // Shift buffer to the left by 1 and insert the bit (0 or 1)
                bitBuffer = (bitBuffer << 1) | (bit - '0');
                bitsInCurrentByte++;
                
                // When we've filled 8 bits, write the byte to the file
                if (bitsInCurrentByte == 8) { 
                    outFile.put(bitBuffer); 
                    bitBuffer = 0; 
                    bitsInCurrentByte = 0; 
                    compressedSize++; 
                }
            }
        }
        
        // Write any remaining bits (padding with 0s on the right)
        if (bitsInCurrentByte > 0) {
            bitBuffer <<= (8 - bitsInCurrentByte);
            outFile.put(bitBuffer);
            compressedSize++;
        }
        
        cout << "  [SUCCESS] File compressed successfully!\n";
        return true;
    }

    bool decompress(const string& inputPath, const string& outputPath) {
        ifstream inFile(inputPath, ios::binary);
        if (!inFile) { cout << "  [ERROR] Cannot open compressed file!\n"; return false; }
        
        // 1. Extract tree from header
        readHeader(inFile);

        ofstream outFile(outputPath, ios::binary);
        if (!outFile) { cout << "  [ERROR] Cannot open output file!\n"; return false; }

        // 2. Read the original uncompressed file size
        size_t expectedSize = 0;
        inFile.read((char*)&expectedSize, sizeof(expectedSize));

        // 3. Walk through tree using bits to retrieve characters
        Node* currentNode = root;
        char byte;
        size_t charactersWritten = 0;
        
        // Read file byte-by-byte
        while (inFile.get(byte) && charactersWritten < expectedSize) {
            
            // Analyze bit-by-bit inside this byte, from left (bit 7) to right (bit 0)
            for (int i = 7; i >= 0 && charactersWritten < expectedSize; i--) {
                
                // Get value of i-th bit (1 or 0)
                int bitValue = (byte >> i) & 1;
                
                // Traverse tree
                currentNode = (bitValue == 1) ? currentNode->right : currentNode->left;
                
                // Check if we reached a leaf
                if (!currentNode->left && !currentNode->right) {
                    outFile.put(currentNode->character);
                    currentNode = root; // Reset to top for the next character
                    charactersWritten++;
                }
            }
        }
        
        cout << "  [SUCCESS] File decompressed to " << outputPath << "!\n";
        return true;
    }

    void showFrequencyTable() {
        if (characterFrequencies.empty()) { cout << "  No data available.\n"; return; }
        for (auto& p : characterFrequencies) 
            cout << "  '" << (p.first == '\n' ? "\\n" : string(1, p.first)) << "' : " << p.second << "\n";
    }

    void showCodesTable() {
        if (huffmanCodes.empty()) { cout << "  No data available.\n"; return; }
        for (auto& p : huffmanCodes) 
            cout << "  '" << (p.first == '\n' ? "\\n" : string(1, p.first)) << "' : " << p.second << "\n";
    }

    void showCompressionStats() {
        if (originalSize == 0) { cout << "  No compression data.\n"; return; }
        cout << "\n  ---- Compression Stats ----\n";
        cout << "  Original Size   : " << originalSize << " bytes\n";
        cout << "  Compressed Size : " << compressedSize << " bytes\n";
        cout << "  Ratio           : " << fixed << setprecision(2) << ((float)compressedSize / originalSize * 100) << "%\n";
    }

    void showTree() {
        if (!root) { cout << "  No tree available.\n"; return; }
        printTree(root, "", true);
    }
};

int main() {
    HuffmanV2 huff;
    int choice;
    cout << "\n  ==============================================\n";
    cout << "    Huffman File Compression Tool V2\n";
    cout << "  ==============================================\n";

    while (true) {
        cout << "\n  1. Compress a File\n  2. Decompress a File\n  3. View Frequency Table\n  4. View Huffman Codes Table\n  5. View Compression Stats\n  6. View Huffman Tree Structure\n  0. Exit\n\n  Enter choice: ";
        if (!(cin >> choice)) { cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n'); continue; }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        if (choice == 0) break;

        if (choice == 1 || choice == 2) {
            string inPath, outPath;
            cout << (choice == 1 ? "  Enter input file path: " : "  Enter compressed file (.huff): ");
            getline(cin, inPath);
            if (inPath.front() == '"' && inPath.back() == '"') inPath = inPath.substr(1, inPath.size() - 2);

            cout << "  Enter output file path (or press Enter for default): ";
            getline(cin, outPath);
            if (outPath.empty()) {
                size_t dot = inPath.find_last_of('.');
                string base = (dot != string::npos) ? inPath.substr(0, dot) : inPath;
                outPath = base + (choice == 1 ? ".huff" : "_decoded.txt");
            }
            if (outPath.front() == '"' && outPath.back() == '"') outPath = outPath.substr(1, outPath.size() - 2);

            if (choice == 1) {
                if (huff.compress(inPath, outPath)) huff.showCompressionStats();
            } else {
                huff.decompress(inPath, outPath);
            }
        }
        else if (choice == 3) huff.showFrequencyTable();
        else if (choice == 4) huff.showCodesTable();
        else if (choice == 5) huff.showCompressionStats();
        else if (choice == 6) huff.showTree();
    }
    cout << "\n  Exiting... Thank you!\n";
    return 0;
}