#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <cmath>
#include "Image.h"

void printUsage();

void addMidCrackCodeOfImageToFile(const std::string &fileName, Image *image);

std::string convertToMidCrackCode(Image *image);

Image *convertFromMidCrackCode(const std::string &fileName);

void compressMidCrackCode(const std::string& midCrackCodeFile, const std::string& compressionOutputFile);

void decompressMidCrackCode(const std::string& inputCompressedFile, const std::string& outputMidCrackCodeFile);

int main(int argc, char *argv[]) {
    if (argc < 4 || argc > 5) {
        std::cout << "Wrong arguments." << std::endl;
        printUsage();
        exit(1);
    }
    std::string option = argv[1];
    if (option == "-m") { // convert to mid-crack code
        auto *image = new Image(argv[2]);
        addMidCrackCodeOfImageToFile(argv[3], image);
        delete image;
    } else if (option == "-i") { // convert from mid-crack code
        auto *convertedImage = convertFromMidCrackCode(argv[2]);
        convertedImage->saveImage(argv[3]);
        delete convertedImage;
    } else if (option == "-c") { // compress mid-crack code
        compressMidCrackCode(argv[2], argv[3]);
    } else if (option == "-d") { // decompressing mid-crack code
        decompressMidCrackCode(argv[2], argv[3]);
    } else printUsage();

    return 0;
}

void decompressMidCrackCode(const std::string& inputCompressedFile, const std::string& outputMidCrackCodeFile) {
    // open .bin file with compressed mid-crack code
    std::ifstream inputFile(inputCompressedFile, std::ios::in | std::ios::binary);
    if (!inputFile.is_open()) {
        std::cout << "The inputFile could not be opened." << std::endl;
        exit(1);
    }

    // LZW decompression
    // create output .txt file for the mid-crack code
    std::ofstream midCrackCodeFile(outputMidCrackCodeFile);
    if (!midCrackCodeFile.is_open()) {
        std::cout << "the midCrackCodeFile could not be opened." << std::endl;
        exit(1);
    }

    // get number of bytes required for the output codes
    unsigned char numberOfBytesInCode;
    inputFile >> numberOfBytesInCode;

    unsigned int prevCode;
    inputFile.read((char*)&prevCode, numberOfBytesInCode);

    std::vector<std::string> table = {"0", "1", "2", "3", "4", "5", "6", "7"}; // init table
    std::string S, C;
    midCrackCodeFile << table[prevCode];
    while (inputFile.peek() != EOF) {
        unsigned int nextCode;
        inputFile.read((char*) (&nextCode), numberOfBytesInCode);
        if (nextCode >= table.size()) { // if nextCode is not in table
            S = table[prevCode];
            S.append(C);
        } else { // nextCode is in table
            S = table[nextCode];
        }
        midCrackCodeFile << S;
        C = S[0];
        table.push_back(table[prevCode] + C); // add translation of prevCode + C to table
        prevCode = nextCode;
    }

    inputFile.close();
    midCrackCodeFile.close();
}

void compressMidCrackCode(const std::string& midCrackCodeFile, const std::string& compressionOutputFile) {
    // read mid-crack code from inputFile
    std::string midCrackCode;
    std::ifstream inputFile(midCrackCodeFile, std::ios::binary);
    if (!inputFile.is_open()) {
        std::cout << "The inputFile could not be opened." << std::endl;
        exit(1);
    }
    std::istreambuf_iterator<char> inputIt(inputFile), emptyInputIt;
    std::back_insert_iterator<std::string> stringInsert(midCrackCode);
    copy(inputIt, emptyInputIt, stringInsert);
    inputFile.close();

    // LZW
    std::vector<int> output;
    std::vector<std::string> table = {"0", "1", "2", "3", "4", "5", "6", "7"}; // init table
    std::string P(1, midCrackCode[0]); // set P to the first character of the mid-crack code
    bool first = true;
    for (char C : midCrackCode) {
        if (first) { first = false; continue; } // skip first iteration
        if (std::find(table.begin(), table.end(), P + C) != table.end()) { // if P + C is in table
            P += C; // add C to P
        } else {
            // add code for P to output
            auto it = find(table.begin(), table.end(), P);
            int index = it - table.begin();
            output.push_back(index);
            // add P + C to table
            table.push_back(P + C);
            // change P
            P = C;
        }
    }
    // add code for P to output
    auto it = find(table.begin(), table.end(), P);
    int index = it - table.begin();
    output.push_back(index);

    // create inputFile and add the compressed mid-crack code
    // output to .bin
    std::ofstream outputFile(compressionOutputFile, std::ios::binary);
    if (!outputFile.is_open()) {
        std::cout << "The inputFile could not be opened." << std::endl;
        exit(1);
    }

    // get number of bytes required for the output codes
    unsigned char numberOfBytesInCode = (int)ceil(log2(output.size())/8);
    outputFile << numberOfBytesInCode;
    // output the codes
    for(auto o: output) {
        outputFile.write((char*)&o, numberOfBytesInCode);
    }

    outputFile.close();
}

void printUsage() {
    std::cout << "Usage:" << std::endl;
    std::cout << "\tConverting to mid-crack code: -m [imageFile.bmp] [midCrackCode.txt]" << std::endl;
    std::cout << "\tConverting from mid-crack code: -i [midCrackCode.txt] [imageFile.bmp]" << std::endl;
    std::cout << "\tCompressing mid-crack code: -c [midCrackCode.txt] [compressedMidCrackCode.bin]" << std::endl;
    std::cout << "\tDecompressing mid-crack code: -d [compressedMidCrackCode.bin] [midCrackCode.txt]" << std::endl;
}

void addMidCrackCodeOfImageToFile(const std::string &fileName, Image *image) {
//    get mid-crack code
    std::string solution = convertToMidCrackCode(image);

//    create text file and add the mid-crack code
    std::ofstream outputFile;
    outputFile.open(fileName);
    if (!outputFile.is_open()) {
        std::cout << "The file could not be opened." << std::endl;
        exit(1);
    }
    outputFile << solution;
    outputFile.close();
}

std::string convertToMidCrackCode(Image *image) {
    // find starting position
    std::pair<int, int> startingPosition;
    for (int y = 0; y < image->getImageHeight(); ++y) {
        for (int x = 0; x < image->getImageWidth(); ++x) {
            if (image->getBitImage()[y][x] == 1) {
                startingPosition = std::make_pair(y, x);
                goto startingPositionSet;
            }
        }
    }
    startingPositionSet:

    int previousDirectionOfEdge; // direction of the edge in the last found edge (top/right/bottom/left)
    int newDirectionOfEdge = 2; // 2 = top; 0 = right; 6 = bottom; 4 = left;
    int yDir[] = {1, 1, 1, 0, -1, -1, -1, 0}; // where to look in the y axis relative to the current edge
    int xDir[] = {1, 0, -1, -1, -1, 0, 1, 1}; // where to look in the x axis relative to the current edge

    std::string midCrackCode; // solution

    std::pair<int, int> previousPositionOnOuterEdge; // position of pixel of last detected edge
    std::pair<int, int> newPositionOnOuterEdge = startingPosition;
    do { // trace edges of the image
        previousPositionOnOuterEdge = newPositionOnOuterEdge;
        int y = previousPositionOnOuterEdge.first;
        int x = previousPositionOnOuterEdge.second;
        previousDirectionOfEdge = newDirectionOfEdge; // current direction of edge (top/right/bottom/left)
        int indexOfDirectionToLookAt =
                8 - previousDirectionOfEdge; // first element in the xDir and yDir arrays to look at

        for (int i = 0; i < 7; ++i) { // go through all neighbouring pixels in clockwise direction
            int yDirToLookAt = yDir[(indexOfDirectionToLookAt + i) % 8];
            int xDirToLookAt = xDir[(indexOfDirectionToLookAt + i) % 8];

            int xToLookAt = x + xDirToLookAt;
            int yToLookAt = y + yDirToLookAt;

            int digitToAddToMidCrackCode = (previousDirectionOfEdge - i + 7) % 8;
            if (i != 0 && i % 2 == 0)
                midCrackCode += std::to_string(digitToAddToMidCrackCode); // add digit to mid-crack code

//            if x and y are in bounds, then check, otherwise skip
            if (xToLookAt >= 0 && yToLookAt >= 0 && xToLookAt < image->getImageWidth() &&
                yToLookAt < image->getImageHeight()) {
                if (image->getBitImage()[yToLookAt][xToLookAt] == 1) { // new pixel of the edge found
                    newPositionOnOuterEdge = std::make_pair(yToLookAt, xToLookAt);
                    int newDirectionDifference = 2 - 2 * ((i + 1) / 2);
                    newDirectionOfEdge = (previousDirectionOfEdge + newDirectionDifference + 8) % 8;

                    midCrackCode += std::to_string(digitToAddToMidCrackCode);
                    break;
                }
            } else continue;
        }
    } while (startingPosition != newPositionOnOuterEdge);
    // add additional edges around the starting position
    if (newDirectionOfEdge == 0) midCrackCode += "531"; // if right
    else if (newDirectionOfEdge == 6) midCrackCode += "31"; // if bottom
    else if (newDirectionOfEdge == 4) midCrackCode += "1"; // if left

    return midCrackCode;
}

Image *convertFromMidCrackCode(const std::string &fileName) {
//    read mid-crack code from file
    std::string midCrackCode;
    std::ifstream file(fileName, std::ios::binary);
    if (!file.is_open()) {
        std::cout << "The file could not be opened." << std::endl;
        exit(1);
    }
    std::istreambuf_iterator<char> inputIt(file), emptyInputIt;
    std::back_insert_iterator<std::string> stringInsert(midCrackCode);
    copy(inputIt, emptyInputIt, stringInsert);

//    find bounds of image by keeping track of the relative position from the starting pixel
    float xBoundsRight = 0, xBoundsLeft = 0, yBoundsDown = 0; // farthest relative x and y positions
    float xCurrent = 0, yCurrent = 0; // current relative x and y positions
    for (char &i : midCrackCode) {
        int digit = i - '0';
        if (digit == 0) xCurrent++; // right
        else if (digit == 1) { // up and right
            xCurrent += 0.5;
            yCurrent -= 0.5;
        } else if (digit == 2) yCurrent--; // up
        else if (digit == 3) { // up and left
            xCurrent -= 0.5;
            yCurrent -= 0.5;
        } else if (digit == 4) xCurrent--; // left
        else if (digit == 5) { // down and left
            xCurrent -= 0.5;
            yCurrent += 0.5;
        } else if (digit == 6) yCurrent++; // down
        else if (digit == 7) { // down and right
            xCurrent += 0.5;
            yCurrent += 0.5;
        } else {
            std::cout << "Wrong digit." << std::endl;
        }

        // update bounds
        if (xCurrent < xBoundsLeft) xBoundsLeft = xCurrent;
        if (xCurrent > xBoundsRight) xBoundsRight = xCurrent;
        if (yCurrent > yBoundsDown) yBoundsDown = yCurrent;
        if (yCurrent < 0) {
            std::cout << "Wrong bounds." << std::endl;
        }
    }

//    set starting point
    std::pair<int, int> startingPoint(0, (int) -xBoundsLeft);

//    initialize image
    auto *image = new Image();

    int imageWidth = 1 + (int) xBoundsRight - (int) xBoundsLeft;
    int imageHeight = (int) yBoundsDown;
    image->setImageWidth(imageWidth);
    image->setImageHeight(imageHeight);

    int **bitImage;
    bitImage = new int *[imageHeight];
    for (int i = 0; i < imageHeight; ++i) {
        bitImage[i] = new int[imageWidth];
        for (int j = 0; j < imageWidth; ++j) {
            bitImage[i][j] = 0;
        }
    }

//    set edges from mid-crack code into bit image
    int currentDirectionOfEdge; // direction of current edge
    int newDirectionOfEdge = 2; // top = 2; right = 0; bottom = 6; left = 4;
    std::pair<int, int> currentPoint;
    std::pair<int, int> newPoint = startingPoint;
    bitImage[newPoint.first][newPoint.second] = 1;

    int yDir[] = {0, 1, 1, 0, 0, -1, 0, -1, -1, 0, 0, 1}; // where to look in the y axis relative to the current edge
    int xDir[] = {0, 0, 1, 0, 1, 1, 0, 0, -1, 0, -1, -1}; // where to look in the x axis relative to the current edge

    for (char &i : midCrackCode) {
        int digit = i - '0';
        currentDirectionOfEdge = newDirectionOfEdge;
        currentPoint = newPoint;

        newDirectionOfEdge = (12 - currentDirectionOfEdge + (digit * 2)) % 8;

        int indexForXAndYArrays = (digit + (3 - currentDirectionOfEdge)) % 8 + ((currentDirectionOfEdge / 2) * 3);
        int newY = currentPoint.first + yDir[indexForXAndYArrays];
        int newX = currentPoint.second + xDir[indexForXAndYArrays];
        newPoint = std::make_pair(newY, newX);

        bitImage[newPoint.first][newPoint.second] = 1;
    }
    image->setBitImage(bitImage);

    return image;
}
