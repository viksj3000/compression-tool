#include "HuffmanCompressor.h"
#include "BitStreamWriter.h"
#include "BitStreamReader.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>

// ============================================================
// HuffmanCompressor Implementation
// ============================================================

HuffmanCompressor::HuffmanCompressor()
    : originalSize(0), compressedSize(0), totalCharsEncoded(0), hasStats(false) {}

// ============================================================
// COMPRESSION - Step by step:
// 1. Read the entire input file into a string
// 2. Count frequency of each character
// 3. Build Huffman tree from frequencies
// 4. Generate binary codes for each character
// 5. Encode the text using generated codes
// 6. Write compressed file in format:
//    [4 bytes: tree size] [serialized tree] [1 byte: padding] [encoded data]
// ============================================================

bool HuffmanCompressor::compress(const std::string& inputFile, const std::string& outputFile) {
    // Step 1: Open and read the input file
    std::ifstream inFile(inputFile, std::ios::binary);
    if (!inFile.is_open()) {
        std::cout << "\n  [ERROR] Could not open file: " << inputFile << std::endl;
        return false;
    }

    // Read entire file content into a string
    std::stringstream contentStream;
    contentStream << inFile.rdbuf();
    std::string fileContent = contentStream.str();
    inFile.close();

    // Check if file is empty
    if (fileContent.empty()) {
        std::cout << "\n  [ERROR] File is empty: " << inputFile << std::endl;
        return false;
    }

    std::cout << "\n  [*] Reading file: " << inputFile << std::endl;
    std::cout << "  [*] File size: " << fileContent.size() << " bytes" << std::endl;

    // Step 2: Count frequency of each character
    std::cout << "  [*] Analyzing character frequencies..." << std::endl;
    analyzer.calculateFrequency(fileContent);
    std::map<char, int> freqMap = analyzer.getFrequencyMap();

    // Step 3: Build Huffman tree using the frequency map
    std::cout << "  [*] Building Huffman tree..." << std::endl;
    huffTree.buildTree(freqMap);

    // Step 4: Generate binary codes for each character
    std::cout << "  [*] Generating Huffman codes..." << std::endl;
    huffTree.generateCodes();
    std::map<char, std::string> codeMap = huffTree.getCodeMap();

    // Store codes in analyzer for display purposes
    analyzer.setCodeMap(codeMap);

    // Step 5: Encode the entire text using Huffman codes
    // Each character is replaced by its binary code string
    std::cout << "  [*] Encoding text..." << std::endl;
    std::string encodedBits = "";
    for (char ch : fileContent) {
        encodedBits += codeMap[ch];
    }

    // Step 6: Write compressed file
    std::cout << "  [*] Writing compressed file: " << outputFile << std::endl;

    std::ofstream outFile(outputFile, std::ios::binary);
    if (!outFile.is_open()) {
        std::cout << "\n  [ERROR] Could not create output file: " << outputFile << std::endl;
        return false;
    }

    // Serialize the Huffman tree to save in file header
    std::string serializedTree = huffTree.serializeTree();
    uint32_t treeSize = static_cast<uint32_t>(serializedTree.size());

    // Write tree header size (4 bytes, little-endian)
    // This tells the decompressor how many bytes to read for the tree
    outFile.put(static_cast<char>((treeSize >> 0) & 0xFF));
    outFile.put(static_cast<char>((treeSize >> 8) & 0xFF));
    outFile.put(static_cast<char>((treeSize >> 16) & 0xFF));
    outFile.put(static_cast<char>((treeSize >> 24) & 0xFF));

    // Write the serialized tree data
    outFile.write(serializedTree.c_str(), serializedTree.size());

    // Write total number of encoded characters (4 bytes)
    // Needed to know exactly when to stop during decompression
    uint32_t totalChars = static_cast<uint32_t>(fileContent.size());
    outFile.put(static_cast<char>((totalChars >> 0) & 0xFF));
    outFile.put(static_cast<char>((totalChars >> 8) & 0xFF));
    outFile.put(static_cast<char>((totalChars >> 16) & 0xFF));
    outFile.put(static_cast<char>((totalChars >> 24) & 0xFF));

    // Write encoded binary data using BitStreamWriter
    // This converts the string of '0's and '1's into actual bits
    BitStreamWriter bitWriter(outFile);
    bitWriter.writeBitString(encodedBits);

    // Flush remaining bits (pad with zeros to complete the last byte)
    int paddingBits = bitWriter.flush();

    outFile.close();

    // Save stats for display
    lastInputFile = inputFile;
    lastOutputFile = outputFile;
    originalSize = static_cast<long>(fileContent.size());
    totalCharsEncoded = static_cast<int>(fileContent.size());

    // Get compressed file size
    std::ifstream checkFile(outputFile, std::ios::binary | std::ios::ate);
    compressedSize = static_cast<long>(checkFile.tellg());
    checkFile.close();

    hasStats = true;

    std::cout << "  [*] Compression complete!" << std::endl;
    std::cout << "  [*] Padding bits added: " << paddingBits << std::endl;

    return true;
}

// ============================================================
// DECOMPRESSION - Step by step:
// 1. Read the compressed file header (tree size)
// 2. Extract and deserialize the Huffman tree
// 3. Read total character count
// 4. Read encoded bits and traverse tree to decode
// 5. Write decoded text to output file
// ============================================================

bool HuffmanCompressor::decompress(const std::string& inputFile, const std::string& outputFile) {
    // Step 1: Open the compressed file
    std::ifstream inFile(inputFile, std::ios::binary);
    if (!inFile.is_open()) {
        std::cout << "\n  [ERROR] Could not open file: " << inputFile << std::endl;
        return false;
    }

    std::cout << "\n  [*] Reading compressed file: " << inputFile << std::endl;

    // Read tree header size (4 bytes, little-endian)
    uint32_t treeSize = 0;
    unsigned char sizeBuf[4];
    inFile.read(reinterpret_cast<char*>(sizeBuf), 4);
    if (inFile.gcount() != 4) {
        std::cout << "\n  [ERROR] Invalid compressed file format." << std::endl;
        return false;
    }
    treeSize = sizeBuf[0] | (sizeBuf[1] << 8) | (sizeBuf[2] << 16) | (sizeBuf[3] << 24);

    // Step 2: Read and deserialize the Huffman tree
    std::cout << "  [*] Rebuilding Huffman tree from header..." << std::endl;
    std::string treeData(treeSize, '\0');
    inFile.read(&treeData[0], treeSize);
    if (inFile.gcount() != static_cast<std::streamsize>(treeSize)) {
        std::cout << "\n  [ERROR] Corrupted tree header in compressed file." << std::endl;
        return false;
    }
    huffTree.deserializeTree(treeData);

    // Step 3: Read total number of characters to decode (4 bytes)
    unsigned char charCountBuf[4];
    inFile.read(reinterpret_cast<char*>(charCountBuf), 4);
    if (inFile.gcount() != 4) {
        std::cout << "\n  [ERROR] Invalid compressed file format." << std::endl;
        return false;
    }
    uint32_t totalChars = charCountBuf[0] | (charCountBuf[1] << 8) |
                          (charCountBuf[2] << 16) | (charCountBuf[3] << 24);

    std::cout << "  [*] Characters to decode: " << totalChars << std::endl;

    // Step 4: Read encoded bits and decode using tree traversal
    std::cout << "  [*] Decoding data..." << std::endl;

    std::ofstream outFile(outputFile, std::ios::binary);
    if (!outFile.is_open()) {
        std::cout << "\n  [ERROR] Could not create output file: " << outputFile << std::endl;
        return false;
    }

    BitStreamReader bitReader(inFile);
    auto currentNode = huffTree.getRoot();
    uint32_t charsDecoded = 0;

    // Traverse tree bit by bit:
    // - bit 0 = go left
    // - bit 1 = go right
    // When we reach a leaf, output the character and restart from root
    while (charsDecoded < totalChars) {
        int bit = bitReader.readBit();
        if (bit == -1) {
            std::cout << "\n  [ERROR] Unexpected end of compressed data." << std::endl;
            break;
        }

        // Navigate tree based on bit value
        if (bit == 0) {
            currentNode = currentNode->left;
        } else {
            currentNode = currentNode->right;
        }

        // If we've reached a leaf, we've decoded one character
        if (currentNode != nullptr && currentNode->isLeaf()) {
            outFile.put(currentNode->character);
            charsDecoded++;
            currentNode = huffTree.getRoot();  // restart from root for next character
        }
    }

    inFile.close();
    outFile.close();

    std::cout << "  [*] Decompression complete!" << std::endl;
    std::cout << "  [*] Characters decoded: " << charsDecoded << std::endl;
    std::cout << "  [*] Output written to: " << outputFile << std::endl;

    return true;
}

// Display compression statistics
void HuffmanCompressor::showCompressionStats() const {
    if (!hasStats) {
        std::cout << "\n  (No stats available. Compress a file first.)" << std::endl;
        return;
    }

    double ratio = 0.0;
    if (originalSize > 0) {
        ratio = (1.0 - (static_cast<double>(compressedSize) / originalSize)) * 100.0;
    }

    std::cout << std::endl;
    std::cout << "  ---- Compression Stats ----" << std::endl;
    std::cout << "  Original File     : " << lastInputFile << std::endl;
    std::cout << "  Compressed File   : " << lastOutputFile << std::endl;
    std::cout << "  Original Size     : " << originalSize << " bytes" << std::endl;
    std::cout << "  Compressed Size   : " << compressedSize << " bytes" << std::endl;
    std::cout << "  Compression Ratio : " << std::fixed << std::setprecision(2) << ratio << "%" << std::endl;
    std::cout << "  Characters Encoded: " << totalCharsEncoded << std::endl;
    std::cout << "  Unique Characters : " << analyzer.getUniqueCharCount() << std::endl;
    std::cout << "  ---------------------------" << std::endl;
    std::cout << std::endl;
}

// Display frequency table
void HuffmanCompressor::showFrequencyTable() const {
    analyzer.displayFrequencyTable();
}

// Display Huffman codes table
void HuffmanCompressor::showCodesTable() const {
    std::map<char, std::string> codeMap = huffTree.getCodeMap();

    if (codeMap.empty()) {
        std::cout << "\n  (No codes available. Compress a file first.)" << std::endl;
        return;
    }

    std::cout << std::endl;
    std::cout << "  " << std::string(40, '-') << std::endl;
    std::cout << "  " << std::left << std::setw(10) << "Char"
              << "| " << std::setw(12) << "Code"
              << "| " << "Length" << std::endl;
    std::cout << "  " << std::string(40, '-') << std::endl;

    for (const auto& pair : codeMap) {
        std::string charDisplay;

        if (pair.first == '\n') charDisplay = "\\n";
        else if (pair.first == '\t') charDisplay = "\\t";
        else if (pair.first == ' ') charDisplay = "SPACE";
        else if (pair.first == '\r') charDisplay = "\\r";
        else charDisplay = std::string(1, pair.first);

        std::cout << "   " << std::left << std::setw(9) << charDisplay
                  << "| " << std::setw(12) << pair.second
                  << "| " << pair.second.size() << " bits" << std::endl;
    }

    std::cout << "  " << std::string(40, '-') << std::endl;
    std::cout << std::endl;
}

// Display tree structure
void HuffmanCompressor::showTree() const {
    huffTree.displayTree();
}

// Check if stats are available
bool HuffmanCompressor::statsAvailable() const {
    return hasStats;
}
