#include <iostream>
#include <fstream>
#include <regex>
#include <string>
#include <unordered_map>
#include <vector>
#include <bitset>

using namespace std;

string getComputationBits(string expression) {
    string a = (expression.find('M') != string::npos) ? "1" : "0";

    string cBits;

    if (expression == "0") {
        cBits = "101010";
    } else if (expression == "1") {
        cBits = "111111";
    } else if (expression == "-1") {
        cBits = "111010";
    } else if (expression == "D") {
        cBits = "001100";
    } else if (expression == "A" || expression == "M") {
        cBits = "110000";
    } else if (expression == "!D") {
        cBits = "001101";
    } else if (expression == "!A" || expression == "!M") {
        cBits = "110001";
    } else if (expression == "-D") {
        cBits = "001111";
    } else if (expression == "-A" || expression == "-M") {
        cBits = "110011";
    } else if (expression == "D+1" || expression == "1+D") {
        cBits = "011111";
    } else if (expression == "A+1" || expression == "M+1" || expression == "1+A" || expression == "1+M") {
        cBits = "110111";
    } else if (expression == "D-1") {
        cBits = "001110";
    } else if (expression == "A-1" || expression == "M-1") {
        cBits = "110010";
    } else if (expression == "D+A" || expression == "D+M" || expression == "A+D" || expression == "M+D") {
        cBits = "000010";
    } else if (expression == "D-A" || expression == "D-M") {
        cBits = "010011";
    } else if (expression == "A-D" || expression == "M-D") {
        cBits = "000111";
    } else if (expression == "D&A" || expression == "D&M" || expression == "A&D" || expression == "M&D") {
        cBits = "000000";
    } else if (expression == "D|A" || expression == "D|M" || expression == "A|D" || expression == "M|D") {
        cBits = "010101";
    } else {
        cerr << "Invalid expression: " << expression << endl;
    }

    return (a + cBits);
}

string getJumpBits(string jump) {
    if (jump == "null") {
        return "000";
    } else if (jump == "JGT") {
        return "001";
    } else if (jump == "JEQ") {
        return "010";
    } else if (jump == "JGE") {
        return "011";
    } else if (jump == "JLT") {
        return "100";
    } else if (jump == "JNE") {
        return "101";
    } else if (jump == "JLE") {
        return "110";
    } else if (jump == "JMP") {
        return "111";
    } else {
        cerr << "Invalid jump: " << jump << endl;
    }

    return "000";
}

int main(int argc, char** argv) {
    
    // PASS 1
    // make sure command is executed properly
    if (argc == 1) {
        cerr << "ERROR: Provide asm file as command line argument" << endl;
        return 1;
    } else if (argc > 2) {
        cerr << "ERROR: Provide only one asm file as command line argument" << endl;
        return 1;
    }

    string filename = argv[1];
    string asmFileContents;
    
    // read file contents into asmFileContents
    if (ifstream asmFile = ifstream(filename.c_str())) {
        asmFileContents.assign((istreambuf_iterator<char>(asmFile)), (istreambuf_iterator<char>()));
        asmFile.close();
    } else {
        cerr << "ERROR: File not found" << endl;
        return 1;
    }

    // remove all comments, whitespace, multi-newlines from asmFileContents
    regex anyComment("(\\/\\/.*|\\/\\*[\\s\\S]*?\\*\\/)");
    regex newline("[\\r\\n]+");
    regex whitespace("[     ]");

    asmFileContents = regex_replace(asmFileContents, anyComment, "");
    asmFileContents = regex_replace(asmFileContents, whitespace, "");
    asmFileContents = regex_replace(asmFileContents, newline, "\n");
    if (asmFileContents[0] == '\n' || asmFileContents[0] == '\r') asmFileContents.erase(asmFileContents.begin());
    if (asmFileContents[0] == '\n' || asmFileContents[0] == '\r') asmFileContents.erase(asmFileContents.begin());
    if (asmFileContents.back() == '\n' || asmFileContents.back() == '\r') asmFileContents.erase(asmFileContents.end() - 1);
    if (asmFileContents.back() == '\n' || asmFileContents.back() == '\r') asmFileContents.erase(asmFileContents.end() - 1);

    // save cleaned asmFileContents into assemberOut.ir
    ofstream irFile("assemblerOut.ir");
    irFile << asmFileContents;
    irFile.close();
    
    // populate symbolTable with standard symbols
    unordered_map<string, int> symbolTable;

    symbolTable["SP"] = 0;
    symbolTable["LCL"] = 1;
    symbolTable["ARG"] = 2;
    symbolTable["THIS"] = 3;
    symbolTable["THAT"] = 4;
    symbolTable["R0"] = 0;
    symbolTable["R1"] = 1;
    symbolTable["R2"] = 2;
    symbolTable["R3"] = 3;
    symbolTable["R4"] = 4;
    symbolTable["R5"] = 5;
    symbolTable["R6"] = 6;
    symbolTable["R7"] = 7;
    symbolTable["R8"] = 8;
    symbolTable["R9"] = 9;
    symbolTable["R10"] = 10;
    symbolTable["R11"] = 11;
    symbolTable["R12"] = 12;
    symbolTable["R13"] = 13;
    symbolTable["R14"] = 14;
    symbolTable["R15"] = 15;
    symbolTable["SCREEN"] = 16384;
    symbolTable["KBD"] = 24576;

    // insert all labels into symbolTable
    istringstream asmStream(asmFileContents);
    int variableCount = 0;
    int lineNumber = 0;

    for(string line; getline(asmStream, line);) {
        if (line[0] == '(') {
            string label = line.substr(1, line.size() - 2);
            symbolTable[label] = lineNumber;
        } else {
            lineNumber++;
        }
    }

    // insert all variables into symbolTable
    asmStream.clear();
    asmStream.seekg(0);

    for(string line; getline(asmStream, line); lineNumber++) {
        if (line[0] == '@') {
            string variableName = line.substr(1, line.size() - 1);
            bool isNum = (variableName[0] >= '0' && variableName[0] <= '9');
            bool isKnown = (symbolTable.count(variableName) > 0);
            
            if (!isNum && !isKnown) {
                symbolTable[variableName] = 16 + variableCount;
                variableCount++;
            }
        }
    }

    // PASS 2
    
    asmStream.clear();
    asmStream.seekg(0);
    vector<string> instructions;

    // convert assembly to machine code
    for(string line; getline(asmStream, line); lineNumber++) {
        if (line[0] == '(') continue;                       // label

        string instruction;
        if (line[0] == '@') {                               // A instruction
            instruction += '0';
            int address;

            if (symbolTable.count(line.substr(1)) > 0) {
                address = symbolTable[line.substr(1)];
            } else {
                address = atoi(line.substr(1).c_str());
            }

            instruction += bitset<15>(address).to_string();
        } else {                                            // C instruction
            instruction += "111";

            if (line.find('=') != string::npos) {           // command insturction
                int index = line.find('=');
                string LHS = line.substr(0, index);
                string RHS = line.substr(index + 1);

                string destination = "000";
                if (LHS.find('A') != string::npos) destination[0] = '1';
                if (LHS.find('D') != string::npos) destination[1] = '1';
                if (LHS.find('M') != string::npos) destination[2] = '1';

                instruction += getComputationBits(RHS);
                instruction += destination;
                instruction += "000";
            } else if (line.find(';') != string::npos) {    // jump insturction
                int index = line.find(';');
                string LHS = line.substr(0, index);
                string RHS = line.substr(index + 1);
                instruction += getComputationBits(LHS);
                instruction += "000";
                instruction += getJumpBits(RHS);
            } else {
                cerr << "ERROR: Invalid instruction: " << line << endl;
                return 1;
            }
        }
        instructions.push_back(instruction);
    }

    // save machine code into assembler.out
    ofstream outFile("assembler.hack");
    for(vector<string>::iterator s = instructions.begin(); s != instructions.end(); s++) {
        outFile << *s << endl;
    }
    outFile.close();

    return 0;
}