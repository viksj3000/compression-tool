#ifndef HUFFMAN_COMPRESSOR_H
#define HUFFMAN_COMPRESSOR_H

#include "HuffmanTree.h"
#include "FrequencyAnalyzer.h"
#include <string>

// ============================================================
// HuffmanCompressor - Main controller class
// ============================================================
// This class ties everything together. It uses:
//   - FrequencyAnalyzer to count character frequencies
//   - HuffmanTree to build the tree and generate codes
//   - BitStreamWriter to write compressed data
//   - BitStreamReader to read compressed data
//
// Compressed file format:
//   [4 bytes: tree header size] [Serialized Tree] [1 byte: padding bits count] [Encoded Binary Data]
//
// This format allows the decompressor to:
//   1. Read how big the tree header is
//   2. Extract and rebuild the Huffman tree
//   3. Know how many padding bits to ignore at the end
//   4. Decode the binary data back to original text

class HuffmanCompressor {
private:
    HuffmanTree huffTree;                   // the Huffman tree used for encoding/decoding
    FrequencyAnalyzer analyzer;             // frequency counter
    std::string lastInputFile;              // stores last compressed file name
    std::string lastOutputFile;             // stores last output file name
    long originalSize;                      // size of original file in bytes
    long compressedSize;                    // size of compressed file in bytes
    int totalCharsEncoded;                  // number of characters encoded
    bool hasStats;                          // flag to check if compression was done

public:
    HuffmanCompressor();

    // Compress input text file to output binary file
    // Returns true if compression was successful
    bool compress(const std::string& inputFile, const std::string& outputFile);

    // Decompress input binary file to output text file
    // Returns true if decompression was successful
    bool decompress(const std::string& inputFile, const std::string& outputFile);

    // Display compression statistics (sizes, ratio, etc.)
    void showCompressionStats() const;

    // Display the frequency table (character, frequency, code)
    void showFrequencyTable() const;

    // Display the Huffman codes table
    void showCodesTable() const;

    // Display the Huffman tree structure
    void showTree() const;

    // Check if stats are available
    bool statsAvailable() const;
};

#endif // HUFFMAN_COMPRESSOR_H
