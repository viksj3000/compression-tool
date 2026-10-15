#ifndef HUFFMAN_NODE_H
#define HUFFMAN_NODE_H

#include <memory>

// ============================================================
// HuffmanNode - Represents a single node in the Huffman Tree
// ============================================================
// Each node stores a character and its frequency.
// Leaf nodes hold actual characters from the input text.
// Internal nodes have left and right children but no character.
// We use shared_ptr to avoid manual memory management.

class HuffmanNode {
public:
    char character;                           // the character stored (only meaningful for leaf nodes)
    int frequency;                            // how often this character appears in the text
    std::shared_ptr<HuffmanNode> left;        // left child (represents bit '0')
    std::shared_ptr<HuffmanNode> right;       // right child (represents bit '1')

    // Constructor for leaf node - has character and frequency, no children
    HuffmanNode(char ch, int freq)
        : character(ch), frequency(freq), left(nullptr), right(nullptr) {}

    // Constructor for internal node - has frequency and two children, no character
    // We use '\0' (null character) to mark internal nodes since they don't represent any character
    HuffmanNode(int freq, std::shared_ptr<HuffmanNode> leftChild, std::shared_ptr<HuffmanNode> rightChild)
        : character('\0'), frequency(freq), left(leftChild), right(rightChild) {}

    // Check if this node is a leaf (has no children)
    // Leaf nodes are the ones that actually hold characters
    bool isLeaf() const {
        return (left == nullptr && right == nullptr);
    }
};

// ============================================================
// Comparator for priority queue (min-heap)
// ============================================================
// In Huffman coding, we always pick the two nodes with the
// LOWEST frequency first. But C++ priority_queue is a max-heap
// by default, so we reverse the comparison.
// If nodeA has higher frequency than nodeB, nodeA should have
// LOWER priority (go deeper in the heap), so we return true.

struct CompareNodes {
    bool operator()(const std::shared_ptr<HuffmanNode>& a, const std::shared_ptr<HuffmanNode>& b) {
        return a->frequency > b->frequency;  // min-heap: lower frequency = higher priority
    }
};

#endif // HUFFMAN_NODE_H
