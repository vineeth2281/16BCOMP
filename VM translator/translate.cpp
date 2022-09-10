#include <iostream>
#include <regex>
#include <fstream>
#include <string>

using namespace std;

string functionName = "NOFUNC";
string className = "NOCLASS";

string getAssembly(string vmInstruction) {
    static int instuctionIndex = 0;
    instuctionIndex++;

    if (vmInstruction == "add") {
        return "@SP\n"
                "AM=M-1\n"
                "D=M\n"
                "A=A-1\n"
                "M=M+D\n";
    } else if (vmInstruction == "sub") {
        return "@SP\n"
                "AM=M-1\n"
                "D=M\n"
                "A=A-1\n"
                "M=M-D\n";
    } else if (vmInstruction == "and") {
        return "@SP\n"
                "AM=M-1\n"
                "D=M\n"
                "A=A-1\n"
                "M=M&D\n";
    } else if (vmInstruction == "or") {
        return "@SP\n"
                "AM=M-1\n"
                "D=M\n"
                "A=A-1\n"
                "M=M|D\n";
    } else if (vmInstruction == "not") {
        return "@SP\n"
                "A=M-1\n"
                "M=!M\n";
    } else if (vmInstruction == "neg") {
        return "@SP\n"
                "A=M-1\n"
                "M=-M\n";
    } else if (vmInstruction == "eq") {
        return "@SP\n"
                "AM=M-1\n"
                "D=M+1\n"
                "A=A-1\n"
                "MD=M-D\n"
                "@IFEQ." + className + "." + to_string(instuctionIndex) + "\n"
                "D+1;JEQ\n"
                "@SP\n"
                "A=M-1\n"
                "M=0\n"
                "(IFEQ." + className + "." + to_string(instuctionIndex) + ")\n";
    } else if (vmInstruction == "lt") {
        return "@SP\n"
                "AM=M-1\n"
                "D=M\n"
                "A=A-1\n"
                "D=M-D\n"
                "M=0\n"
                "@IFNLT." + className + "." + to_string(instuctionIndex) + "\n"
                "D;JGE\n"
                "@SP\n"
                "A=M-1\n"
                "M=-1\n"
                "(IFNLT." + className + "." + to_string(instuctionIndex) + ")\n";
    } else if (vmInstruction == "gt") {
        return "@SP\n"
                "AM=M-1\n"
                "D=M\n"
                "A=A-1\n"
                "D=M-D\n"
                "M=0\n"
                "@IFNGT." + className + "." + to_string(instuctionIndex) + "\n"
                "D;JLE\n"
                "@SP\n"
                "A=M-1\n"
                "M=-1\n"
                "(IFNGT." + className + "." + to_string(instuctionIndex) + ")\n";
    } else if (vmInstruction.substr(0,5) == "push ") {
        if (vmInstruction.substr(5,6) == "local ") {
            return "@LCL\n"
                    "D=M\n"
                    "@" + vmInstruction.substr(11) + "\n"
                    "A=A+D\n"
                    "D=M\n"
                    "@SP\n"
                    "M=M+1\n"
                    "A=M-1\n"
                    "M=D\n";
        } else if (vmInstruction.substr(5,9) == "argument ") {
            return "@ARG\n"
                    "D=M\n"
                    "@" + vmInstruction.substr(14) + "\n"
                    "A=A+D\n"
                    "D=M\n"
                    "@SP\n"
                    "M=M+1\n"
                    "A=M-1\n"
                    "M=D\n";
        } else if (vmInstruction.substr(5,5) == "this ") {
            return "@THIS\n"
                    "D=M\n"
                    "@" + vmInstruction.substr(10) + "\n"
                    "A=A+D\n"
                    "D=M\n"
                    "@SP\n"
                    "M=M+1\n"
                    "A=M-1\n"
                    "M=D\n";
        } else if (vmInstruction.substr(5,5) == "that ") {
            return "@THAT\n"
                    "D=M\n"
                    "@" + vmInstruction.substr(10) + "\n"
                    "A=A+D\n"
                    "D=M\n"
                    "@SP\n"
                    "M=M+1\n"
                    "A=M-1\n"
                    "M=D\n";
        } else if (vmInstruction.substr(5,9) == "constant ") {
            return "@" + vmInstruction.substr(14) + "\n"
                    "D=A\n"
                    "@SP\n"
                    "M=M+1\n"
                    "A=M-1\n"
                    "M=D\n";
        } else if (vmInstruction.substr(5,8) == "pointer ") {
            if (vmInstruction.substr(13) == "0") {
                return "@THIS\n"
                        "D=M\n"
                        "@SP\n"
                        "M=M+1\n"
                        "A=M-1\n"
                        "M=D\n";
            } else if (vmInstruction.substr(13) == "1") {
                return "@THAT\n"
                        "D=M\n"
                        "@SP\n"
                        "M=M+1\n"
                        "A=M-1\n"
                        "M=D\n";
            }
        } else if (vmInstruction.substr(5,5) == "temp ") {
            int index = stoi(vmInstruction.substr(10));
            if (index >= 0 && index < 8) {
                return "@" + to_string(index + 5) + "\n"
                        "D=M\n"
                        "@SP\n"
                        "M=M+1\n"
                        "A=M-1\n"
                        "M=D\n";
            } else {
                return "ERROR: UNKNOWN INSTRUCTION";
            }
        } else if (vmInstruction.substr(5,7) == "static ") {
            string varName = vmInstruction.substr(12);
            return "@" + className + "." + varName + "\n"
                    "D=M\n"
                    "@SP\n"
                    "M=M+1\n"
                    "A=M-1\n"
                    "M=D\n";
        } else {
            return "ERROR: UNKNOWN INSTRUCTION";
        }
    } else if (vmInstruction.substr(0,4) == "pop ") {
        if (vmInstruction.substr(4,6) == "local ") {
            return "@LCL\n"
                    "D=M\n"
                    "@" + vmInstruction.substr(10) + "\n"
                    "D=A+D\n"
                    "@SP\n"
                    "AM=M-1\n"
                    "D=D+M\n"
                    "A=D-M\n"
                    "M=D-A\n";
        } else if (vmInstruction.substr(4,9) == "argument ") {
            return "@ARG\n"
                    "D=M\n"
                    "@" + vmInstruction.substr(13) + "\n"
                    "D=A+D\n"
                    "@SP\n"
                    "AM=M-1\n"
                    "D=D+M\n"
                    "A=D-M\n"
                    "M=D-A\n";
        } else if (vmInstruction.substr(4,5) == "this ") {
            return "@THIS\n"
                    "D=M\n"
                    "@" + vmInstruction.substr(9) + "\n"
                    "D=A+D\n"
                    "@SP\n"
                    "AM=M-1\n"
                    "D=D+M\n"
                    "A=D-M\n"
                    "M=D-A\n";
        } else if (vmInstruction.substr(4,5) == "that ") {
            return "@THAT\n"
                    "D=M\n"
                    "@" + vmInstruction.substr(9) + "\n"
                    "D=A+D\n"
                    "@SP\n"
                    "AM=M-1\n"
                    "D=D+M\n"
                    "A=D-M\n"
                    "M=D-A\n";
        } else if (vmInstruction.substr(4,8) == "pointer ") {
            if (vmInstruction.substr(12) == "0") {
                return "@SP\n"
                        "AM=M-1\n"
                        "D=M\n"
                        "@THIS\n"
                        "M=D\n";
            } else if (vmInstruction.substr(12) == "1") {
                return "@SP\n"
                        "AM=M-1\n"
                        "D=M\n"
                        "@THAT\n"
                        "M=D\n";
            }
        } else if (vmInstruction.substr(4,5) == "temp ") {
            int index = stoi(vmInstruction.substr(9));
            if (index >= 0 && index < 8) {
                return  "@" + to_string(index + 5) + "\n"
                        "D=A\n"
                        "@SP\n"
                        "AM=M-1\n"
                        "D=D+M\n"
                        "A=D-M\n"
                        "M=D-A\n";
            } else {
                return "ERROR: UNKNOWN INSTRUCTION";
            }
        } else if (vmInstruction.substr(4,7) == "static ") {
            string varName = vmInstruction.substr(11);
            return "@SP\n"
                    "AM=M-1\n"
                    "D=M\n"
                    "@" + className + "." + varName + "\n"
                    "M=D\n";
        }
    } else if (vmInstruction.substr(0,9) == "function ") {
        int functionNameLength = vmInstruction.substr(9).find(' ');
        functionName = vmInstruction.substr(9, functionNameLength);
        string numLocalVars = vmInstruction.substr(9 + functionNameLength);

        return "(" + functionName + ")\n"
                "@" + numLocalVars + "\n"
                "D=A\n"
                "(" + functionName + ".localLoop)\n"
                "@" + functionName + ".localEnd\n"
                "D;JLE\n"
                "@SP\n"
                "M=M+1\n"
                "A=M-1\n"
                "M=0\n"
                "D=D-1\n"
                "@" + functionName + ".localLoop\n"
                "0;JMP\n"
                "(" + functionName + ".localEnd)\n";
    } else if (vmInstruction.substr(0,5) == "call ") {
        int functionNameLength = vmInstruction.substr(5).find(' ');
        string calledFunctionName = vmInstruction.substr(5, functionNameLength);
        string numArguments = vmInstruction.substr(5 + functionNameLength);

        return "@" + className + ".return." + to_string(instuctionIndex) + "\n" 
                "D=A\n"
                "@SP\n"
                "M=M+1\n"
                "A=M-1\n"
                "M=D\n"
                "@LCL\n"
                "D=M\n"
                "@SP\n"
                "M=M+1\n"
                "A=M-1\n"
                "M=D\n"
                "@ARG\n"
                "D=M\n"
                "@SP\n"
                "M=M+1\n"
                "A=M-1\n"
                "M=D\n"
                "@THIS\n"
                "D=M\n"
                "@SP\n"
                "M=M+1\n"
                "A=M-1\n"
                "M=D\n"
                "@THAT\n"
                "D=M\n"
                "@SP\n"
                "M=M+1\n"
                "A=M-1\n"
                "M=D\n"
                "@SP\n"
                "D=M\n"
                "@" + numArguments + "\n"
                "D=D-A\n"
                "@5\n"
                "D=D-A\n"
                "@ARG\n"
                "M=D\n"
                "@SP\n"
                "D=M\n"
                "@LCL\n"
                "M=D\n"
                "@" + calledFunctionName + "\n"
                "0;JMP\n"
                "(" + className + ".return." + to_string(instuctionIndex) + ")\n";

    } else if (vmInstruction == "return") {
        return "@LCL\n"
                "D=M\n"
                "@FRAME\n"
                "M=D\n"
                "@5\n"
                "D=A\n"
                "@FRAME\n"
                "A=M-D\n"
                "D=M\n"
                "@RET\n"
                "M=D\n"
                "@SP\n"
                "AM=M-1\n"
                "D=M\n"
                "@ARG\n"
                "A=M\n"
                "M=D\n"
                "D=A\n"
                "@SP\n"
                "M=D+1\n"
                "@FRAME\n"
                "A=M-1\n"
                "D=M\n"
                "@THAT\n"
                "M=D\n"
                "@2\n"
                "D=A\n"
                "@FRAME\n"
                "A=M-D\n"
                "D=M\n"
                "@THIS\n"
                "M=D\n"
                "@3\n"
                "D=A\n"
                "@FRAME\n"
                "A=M-D\n"
                "D=M\n"
                "@ARG\n"
                "M=D\n"
                "@4\n"
                "D=A\n"
                "@FRAME\n"
                "A=M-D\n"
                "D=M\n"
                "@LCL\n"
                "M=D\n"
                "@RET\n"
                "A=M\n"
                "0;JMP\n";
    } else if (vmInstruction.substr(0,6) == "label ") {
        string labelName = vmInstruction.substr(6);
        return "(" + functionName + "$" + labelName + ")\n";
    } else if (vmInstruction.substr(0,5) == "goto ") {
        string labelName = vmInstruction.substr(5);
        return "@" + functionName + "$" + labelName + "\n"
                "0;JMP\n";
    } else if (vmInstruction.substr(0,8) == "if-goto ") {
        string labelName = vmInstruction.substr(8);
        return "@SP\n"
                "AM=M-1\n"
                "D=M\n"
                "@" + functionName + "$" + labelName + "\n"
                "D;JNE\n";
    } else {
        return "ERROR: UNKNOWN INSTRUCTION";
    }
}

int main(int argc, char** argv) {
    
    // make sure command is executed properly
    if (argc == 1) {
        cerr << "ERROR: Provide vm file as command line argument" << endl;
        return 1;
    } else if (argc > 2) {
        cerr << "ERROR: Provide only one vm file as command line argument" << endl;
        return 1;
    }

    string filename = argv[1];
    className = filename.substr(filename.find_last_of("/") + 1,filename.length() - 4 - filename.find_last_of("/"));
    string vmFileContents;
    
    // read file contents into vmFileContents
    if (ifstream vmFile = ifstream(filename.c_str())) {
        vmFileContents.assign((istreambuf_iterator<char>(vmFile)), (istreambuf_iterator<char>()));
        vmFile.close();
    } else {
        cerr << "ERROR: File not found" << endl;
        return 1;
    }

    // remove all comments, whitespace, multi-newlines from vmFileContents
    regex anyComment("(\\/\\/.*|\\/\\*[\\s\\S]*?\\*\\/)");
    regex newline("[\\r\\n]+");
    regex whitespaceNewline(" \\n");
    regex whitespace("[ ]+");

    vmFileContents = regex_replace(vmFileContents, anyComment, "");
    vmFileContents = regex_replace(vmFileContents, whitespace, " ");
    vmFileContents = regex_replace(vmFileContents, newline, "\n");
    vmFileContents = regex_replace(vmFileContents, whitespaceNewline, "\n");
    if (vmFileContents[0] == '\n' || vmFileContents[0] == '\r') vmFileContents.erase(vmFileContents.begin());
    if (vmFileContents[0] == '\n' || vmFileContents[0] == '\r') vmFileContents.erase(vmFileContents.begin());
    if (vmFileContents.back() == '\n' || vmFileContents.back() == '\r') vmFileContents.erase(vmFileContents.end() - 1);
    if (vmFileContents.back() == '\n' || vmFileContents.back() == '\r') vmFileContents.erase(vmFileContents.end() - 1);

    istringstream vmStream(vmFileContents);
    stringstream asmStream;
    for(string line; getline(vmStream, line);) {
        string asmString = getAssembly(line);
        if (asmString != "ERROR: UNKNOWN INSTRUCTION") {
            asmStream << asmString;
        } else {
            cerr << "ERROR: UNKNOWN INSTRUCTION" << endl << "'" << line << "'" << endl;
            return 1;
        }
    }
    
    // save machine code into asm file
    ofstream outFile(filename.substr(0,filename.find(".vm")) + ".asm");
    outFile << asmStream.rdbuf();
    outFile.close();

    return 0;
}