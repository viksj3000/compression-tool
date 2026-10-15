#include "HuffmanTree.h"
#include <iostream>
#include <queue>

// ============================================================
// HuffmanTree Implementation
// ============================================================

HuffmanTree::HuffmanTree() : root(nullptr) {}

// Build the Huffman tree from character frequencies
// Algorithm:
//   1. Create a leaf node for each character and push into min-heap
//   2. While there's more than one node in the heap:
//      a. Pop two nodes with lowest frequency
//      b. Create a new internal node with these two as children
//      c. New node's frequency = sum of both children's frequencies
//      d. Push new node back into the heap
//   3. Last remaining node is the root of the Huffman tree

void HuffmanTree::buildTree(const std::map<char, int>& freqMap) {
    // Edge case: empty frequency map
    if (freqMap.empty()) {
        root = nullptr;
        return;
    }

    // Min priority queue - lowest frequency gets highest priority
    std::priority_queue<std::shared_ptr<HuffmanNode>,
                        std::vector<std::shared_ptr<HuffmanNode>>,
                        CompareNodes> minHeap;

    // Step 1: Create leaf nodes for each character and push to heap
    for (const auto& pair : freqMap) {
        minHeap.push(std::make_shared<HuffmanNode>(pair.first, pair.second));
    }

    // Special case: only one unique character in the text
    // We still need a proper tree, so we add a dummy node
    if (minHeap.size() == 1) {
        auto onlyNode = minHeap.top();
        minHeap.pop();
        // Create a parent node with the only node as left child
        auto parentNode = std::make_shared<HuffmanNode>(onlyNode->frequency, onlyNode, nullptr);
        root = parentNode;
        return;
    }

    // Step 2: Keep combining two smallest nodes until one remains
    while (minHeap.size() > 1) {
        // Pop two nodes with lowest frequency
        auto leftNode = minHeap.top();
        minHeap.pop();
        auto rightNode = minHeap.top();
        minHeap.pop();

        // Create internal node with combined frequency
        int combinedFreq = leftNode->frequency + rightNode->frequency;
        auto parentNode = std::make_shared<HuffmanNode>(combinedFreq, leftNode, rightNode);

        // Push combined node back into the heap
        minHeap.push(parentNode);
    }

    // Step 3: The last node is the root
    root = minHeap.top();
}

// Generate binary codes by traversing the tree
// Starts the recursive traversal from root with empty code string
void HuffmanTree::generateCodes() {
    codeMap.clear();
    if (root != nullptr) {
        generateCodesHelper(root, "");
    }
}

// Recursive helper: traverse tree and build codes
// Left edge = '0', Right edge = '1'
// When we hit a leaf, the accumulated string is that character's code
void HuffmanTree::generateCodesHelper(std::shared_ptr<HuffmanNode> node, const std::string& currentCode) {
    if (node == nullptr) return;

    // If it's a leaf node, we've found a character - save its code
    if (node->isLeaf()) {
        // Handle edge case where root itself is a leaf (single character text)
        if (currentCode.empty()) {
            codeMap[node->character] = "0";
        } else {
            codeMap[node->character] = currentCode;
        }
        return;
    }

    // Recurse left (add '0') and right (add '1')
    generateCodesHelper(node->left, currentCode + "0");
    generateCodesHelper(node->right, currentCode + "1");
}

// Return the generated code map
std::map<char, std::string> HuffmanTree::getCodeMap() const {
    return codeMap;
}

// Serialize the tree to a string for storage in compressed file
// Format: '1' + character byte for leaf nodes, '0' for internal nodes
// We use preorder traversal (root -> left -> right)
// This way we can reconstruct the exact same tree during decompression
std::string HuffmanTree::serializeTree() const {
    std::string result = "";
    serializeHelper(root, result);
    return result;
}

void HuffmanTree::serializeHelper(std::shared_ptr<HuffmanNode> node, std::string& result) const {
    if (node == nullptr) return;

    if (node->isLeaf()) {
        result += '1';              // marker: this is a leaf
        result += node->character;  // the actual character
    } else {
        result += '0';              // marker: this is an internal node
        serializeHelper(node->left, result);
        serializeHelper(node->right, result);
    }
}

// Deserialize tree from string - rebuild the tree structure
// Reads the serialized string and reconstructs nodes recursively
void HuffmanTree::deserializeTree(const std::string& data) {
    int index = 0;
    root = deserializeHelper(data, index);
}

std::shared_ptr<HuffmanNode> HuffmanTree::deserializeHelper(const std::string& data, int& index) {
    if (index >= static_cast<int>(data.size())) return nullptr;

    char marker = data[index++];

    if (marker == '1') {
        // It's a leaf node - next character is the stored character
        char ch = data[index++];
        return std::make_shared<HuffmanNode>(ch, 0);  // frequency doesn't matter for decoding
    } else {
        // It's an internal node - recursively build left and right subtrees
        auto leftChild = deserializeHelper(data, index);
        auto rightChild = deserializeHelper(data, index);
        return std::make_shared<HuffmanNode>(0, leftChild, rightChild);
    }
}

// Display tree structure in console using indentation
void HuffmanTree::displayTree() const {
    if (root == nullptr) {
        std::cout << "  (Tree is empty. Compress a file first.)" << std::endl;
        return;
    }
    std::cout << std::endl;
    displayTreeHelper(root, "", false);
    std::cout << std::endl;
}

// Recursive tree display with visual connectors
// Uses box-drawing style characters for a clean look
void HuffmanTree::displayTreeHelper(std::shared_ptr<HuffmanNode> node, const std::string& indent, bool isRight) const {
    if (node == nullptr) return;

    std::cout << indent;

    if (indent.empty()) {
        std::cout << "  ";  // root node
    } else if (isRight) {
        std::cout << "|-- ";  // right branch
    } else {
        std::cout << "`-- ";  // left branch
    }

    if (node->isLeaf()) {
        // Print leaf with character info
        // Handle special characters for display
        if (node->character == '\n') {
            std::cout << "[\\n] (freq: " << node->frequency << ")" << std::endl;
        } else if (node->character == '\t') {
            std::cout << "[\\t] (freq: " << node->frequency << ")" << std::endl;
        } else if (node->character == ' ') {
            std::cout << "[SPACE] (freq: " << node->frequency << ")" << std::endl;
        } else {
            std::cout << "['" << node->character << "'] (freq: " << node->frequency << ")" << std::endl;
        }
    } else {
        // Print internal node with combined frequency
        std::cout << "(*) (freq: " << node->frequency << ")" << std::endl;
    }

    // Build indent string for children
    std::string childIndent = indent;
    if (indent.empty()) {
        childIndent = "  ";
    } else if (isRight) {
        childIndent += "|   ";
    } else {
        childIndent += "    ";
    }

    // Print right child first (appears on top), then left
    displayTreeHelper(node->right, childIndent, true);
    displayTreeHelper(node->left, childIndent, false);
}

// Get root node pointer
std::shared_ptr<HuffmanNode> HuffmanTree::getRoot() const {
    return root;
}
