#ifndef FREQUENCY_ANALYZER_H
#define FREQUENCY_ANALYZER_H

#include <map>
#include <string>

// ============================================================
// FrequencyAnalyzer - Counts character frequencies in text
// ============================================================
// This class reads through the input text and counts how many
// times each character appears. This frequency data is the
// foundation of Huffman coding - characters that appear more
// often get shorter binary codes.

class FrequencyAnalyzer {
private:
    std::map<char, int> frequencyMap;         // stores character -> count
    std::map<char, std::string> codeMap;      // stores character -> huffman code (set after tree is built)

public:
    FrequencyAnalyzer();

    // Count frequency of each character in the given text
    // Simply iterates through every character and increments its count
    void calculateFrequency(const std::string& text);

    // Get the frequency map
    std::map<char, int> getFrequencyMap() const;

    // Set the code map (called after Huffman codes are generated)
    void setCodeMap(const std::map<char, std::string>& codes);

    // Display a formatted table showing character, frequency, and assigned code
    void displayFrequencyTable() const;

    // Get total number of unique characters
    int getUniqueCharCount() const;
};

#endif // FREQUENCY_ANALYZER_H
