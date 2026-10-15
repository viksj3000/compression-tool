#ifndef HUFFMAN_TREE_H
#define HUFFMAN_TREE_H

#include "HuffmanNode.h"
#include <map>
#include <string>
#include <queue>
#include <memory>

// ============================================================
// HuffmanTree - Builds and manages the Huffman binary tree
// ============================================================
// The tree is built from character frequencies using a min-heap
// (priority queue). Characters with lower frequency get longer
// codes, and characters with higher frequency get shorter codes.
// This is what makes Huffman coding efficient for compression.

class HuffmanTree {
private:
    std::shared_ptr<HuffmanNode> root;        // root of the Huffman tree
    std::map<char, std::string> codeMap;      // maps each character to its binary code

    // Recursive helper to generate binary codes by traversing the tree
    // Going left adds '0', going right adds '1' to the current code
    void generateCodesHelper(std::shared_ptr<HuffmanNode> node, const std::string& currentCode);

    // Recursive helper for displaying tree structure with indentation
    void displayTreeHelper(std::shared_ptr<HuffmanNode> node, const std::string& indent, bool isRight) const;

    // Recursive helper for serializing the tree into a string
    void serializeHelper(std::shared_ptr<HuffmanNode> node, std::string& result) const;

    // Recursive helper for deserializing tree from string
    std::shared_ptr<HuffmanNode> deserializeHelper(const std::string& data, int& index);

public:
    HuffmanTree();

    // Build the Huffman tree from a frequency map
    // Uses a min priority queue to always combine the two lowest frequency nodes
    void buildTree(const std::map<char, int>& freqMap);

    // Generate binary codes for each character by traversing the tree
    void generateCodes();

    // Get the map of character -> binary code
    std::map<char, std::string> getCodeMap() const;

    // Convert tree structure to a string for saving in compressed file
    // Format: '1' followed by character for leaf, '0' for internal node
    std::string serializeTree() const;

    // Rebuild tree from the serialized string
    void deserializeTree(const std::string& data);

    // Display tree structure in console for visualization
    void displayTree() const;

    // Get root node (needed for decompression traversal)
    std::shared_ptr<HuffmanNode> getRoot() const;
};

#endif // HUFFMAN_TREE_H
