#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
using namespace std;

 // g++ create_matrix.cpp -o cm 
void getLinesAfter(const std::string& filename, int startLineNumber, int endLineNumber) {
    // Open the file for reading and writing
    std::ifstream inputFile(filename);
    if (!inputFile.is_open()) {
        std::cerr << "Error: Unable to open file " << filename << " for reading." << std::endl;
        return;
    }

    std::ofstream outputFile("test/temp.txt"); // Temporary file to store modified content
    if (!outputFile.is_open()) {
        std::cerr << "Error: Unable to create temporary file." << std::endl;
        return;
    }

    std::string line;
    int currentLineNumber = 1;

    while(std::getline(inputFile, line) && currentLineNumber < startLineNumber){
        currentLineNumber++;
    }

    // Read lines from the input file and copy them to the temporary file until the given line number
    while (std::getline(inputFile, line) && currentLineNumber < endLineNumber) {
        outputFile << line << std::endl;
        currentLineNumber++;
    }

    // Close both files
    inputFile.close();
    outputFile.close();

}

int main(int argc, char *argv[])
{
 	std::string filename = "Implementation/test/matrixlarge.txt";
    int startLine = 793585;
    int endLine = 1793585;

    getLinesAfter(filename, startLine, endLine);
}