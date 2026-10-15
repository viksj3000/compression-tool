#include "HuffmanCompressor.h"
#include <iostream>
#include <string>
#include <limits>

// ============================================================
// Main Program - CLI Menu for File Compression Tool
// ============================================================
// Provides a simple menu-driven interface for the user to:
//   1. Compress text files using Huffman coding
//   2. Decompress .huff files back to text
//   3. View frequency analysis, codes, stats, and tree structure

void displayMenu() {
    std::cout << "\n===== File Compression Tool (Huffman Coding) =====" << std::endl;
    std::cout << std::endl;
    std::cout << "  1. Compress a File" << std::endl;
    std::cout << "  2. Decompress a File" << std::endl;
    std::cout << "  3. View Frequency Table" << std::endl;
    std::cout << "  4. View Huffman Codes Table" << std::endl;
    std::cout << "  5. View Compression Stats" << std::endl;
    std::cout << "  6. View Huffman Tree Structure" << std::endl;
    std::cout << "  0. Exit" << std::endl;
    std::cout << std::endl;
    std::cout << "  Enter choice: ";
}

int main() {
    HuffmanCompressor compressor;
    int choice;

    std::cout << std::endl;
    std::cout << "  ================================================" << std::endl;
    std::cout << "    Huffman File Compression Tool" << std::endl;
    std::cout << "    Using Huffman Coding Algorithm" << std::endl;
    std::cout << "    DSA College Project" << std::endl;
    std::cout << "  ================================================" << std::endl;

    while (true) {
        displayMenu();

        // Read user choice with input validation
        if (!(std::cin >> choice)) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "\n  [ERROR] Invalid input. Please enter a number." << std::endl;
            continue;
        }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        switch (choice) {

        case 1: {
            // ---- Compress a File ----
            std::string inputFile, outputFile;

            std::cout << "\n  Enter input file path: ";
            std::getline(std::cin, inputFile);

            // Remove surrounding quotes if the user pastes a path with quotes
            if (inputFile.size() >= 2 && inputFile.front() == '"' && inputFile.back() == '"') {
                inputFile = inputFile.substr(1, inputFile.size() - 2);
            }

            std::cout << "  Enter output file path (or press Enter for default): ";
            std::getline(std::cin, outputFile);

            // Default output: replace extension with .huff
            if (outputFile.empty()) {
                size_t dotPos = inputFile.find_last_of('.');
                if (dotPos != std::string::npos) {
                    outputFile = inputFile.substr(0, dotPos) + ".huff";
                } else {
                    outputFile = inputFile + ".huff";
                }
            }

            // Remove surrounding quotes from output file too
            if (outputFile.size() >= 2 && outputFile.front() == '"' && outputFile.back() == '"') {
                outputFile = outputFile.substr(1, outputFile.size() - 2);
            }

            if (compressor.compress(inputFile, outputFile)) {
                compressor.showCompressionStats();
            }
            break;
        }

        case 2: {
            // ---- Decompress a File ----
            std::string inputFile, outputFile;

            std::cout << "\n  Enter compressed file path (.huff): ";
            std::getline(std::cin, inputFile);

            if (inputFile.size() >= 2 && inputFile.front() == '"' && inputFile.back() == '"') {
                inputFile = inputFile.substr(1, inputFile.size() - 2);
            }

            std::cout << "  Enter output file path (or press Enter for default): ";
            std::getline(std::cin, outputFile);

            // Default output: replace .huff with _decoded.txt
            if (outputFile.empty()) {
                size_t dotPos = inputFile.find_last_of('.');
                if (dotPos != std::string::npos) {
                    outputFile = inputFile.substr(0, dotPos) + "_decoded.txt";
                } else {
                    outputFile = inputFile + "_decoded.txt";
                }
            }

            if (outputFile.size() >= 2 && outputFile.front() == '"' && outputFile.back() == '"') {
                outputFile = outputFile.substr(1, outputFile.size() - 2);
            }

            compressor.decompress(inputFile, outputFile);
            break;
        }

        case 3:
            // ---- View Frequency Table ----
            std::cout << "\n  ---- Character Frequency Table ----" << std::endl;
            compressor.showFrequencyTable();
            break;

        case 4:
            // ---- View Huffman Codes ----
            std::cout << "\n  ---- Huffman Codes Table ----" << std::endl;
            compressor.showCodesTable();
            break;

        case 5:
            // ---- View Stats ----
            compressor.showCompressionStats();
            break;

        case 6:
            // ---- View Tree Structure ----
            std::cout << "\n  ---- Huffman Tree Structure ----" << std::endl;
            compressor.showTree();
            break;

        case 0:
            std::cout << "\n  Exiting... Thank you!" << std::endl;
            std::cout << std::endl;
            return 0;

        default:
            std::cout << "\n  [ERROR] Invalid choice. Please try again." << std::endl;
            break;
        }
    }

    return 0;
}
