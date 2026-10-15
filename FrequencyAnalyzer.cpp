#include "FrequencyAnalyzer.h"
#include <iostream>
#include <iomanip>

// ============================================================
// FrequencyAnalyzer Implementation
// ============================================================

FrequencyAnalyzer::FrequencyAnalyzer() {}

// Count how many times each character appears in the text
// Simply iterate through every character and increment its count in the map
// Time complexity: O(n) where n is the length of the text
void FrequencyAnalyzer::calculateFrequency(const std::string& text) {
    frequencyMap.clear();

    for (char ch : text) {
        frequencyMap[ch]++;
    }
}

// Return the frequency map
std::map<char, int> FrequencyAnalyzer::getFrequencyMap() const {
    return frequencyMap;
}

// Store the Huffman codes after tree is built (so we can display them alongside frequencies)
void FrequencyAnalyzer::setCodeMap(const std::map<char, std::string>& codes) {
    codeMap = codes;
}

// Display a nicely formatted frequency table
// Shows character, its frequency, and its assigned Huffman code
void FrequencyAnalyzer::displayFrequencyTable() const {
    if (frequencyMap.empty()) {
        std::cout << "  (No frequency data. Compress a file first.)" << std::endl;
        return;
    }

    std::cout << std::endl;
    std::cout << "  " << std::string(45, '-') << std::endl;
    std::cout << "  " << std::left << std::setw(10) << "Char"
              << "| " << std::setw(12) << "Frequency"
              << "| " << "Code" << std::endl;
    std::cout << "  " << std::string(45, '-') << std::endl;

    for (const auto& pair : frequencyMap) {
        std::string charDisplay;

        // Handle special characters so they display nicely
        if (pair.first == '\n') {
            charDisplay = "\\n";
        } else if (pair.first == '\t') {
            charDisplay = "\\t";
        } else if (pair.first == ' ') {
            charDisplay = "SPACE";
        } else if (pair.first == '\r') {
            charDisplay = "\\r";
        } else {
            charDisplay = std::string(1, pair.first);
        }

        std::cout << "   " << std::left << std::setw(9) << charDisplay
                  << "| " << std::setw(12) << pair.second
                  << "| ";

        // Show Huffman code if available
        auto it = codeMap.find(pair.first);
        if (it != codeMap.end()) {
            std::cout << it->second;
        } else {
            std::cout << "(not generated)";
        }
        std::cout << std::endl;
    }

    std::cout << "  " << std::string(45, '-') << std::endl;
    std::cout << "  Total unique characters: " << frequencyMap.size() << std::endl;
    std::cout << std::endl;
}

// Get number of unique characters
int FrequencyAnalyzer::getUniqueCharCount() const {
    return static_cast<int>(frequencyMap.size());
}
