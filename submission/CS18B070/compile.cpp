#include <iostream>
#include <fstream>
#include <sstream>
#include <regex>
#include <unordered_map>
#include <stdlib.h>

using namespace std;

struct variable {
    string name;
    string kind;    // segment name
    string type;    // datatype
    int index;

    variable(string varName, string varKind, string varType, int varIndex) {
        name = varName;
        kind = varKind;
        type = varType;
        index = varIndex;
    }

    variable() {};
};

class CodeGenerationEngine {        // process .xml into .vm
    vector<string> lines;
    vector<string>::iterator currentLine;
    string vmFile;
    string errorFile;
    stringstream vmDump;

    unordered_map<string, variable> classSymbolTable;
    unordered_map<string, variable> subroutineSymbolTable;
    int staticCount;
    int fieldCount;
    int localCount;
    int argumentCount;
    int labelNum;

    string currentClassName;
    string currentSubroutineName;
    string currentSubroutineType;

    bool isToken() {
        string line = *currentLine;
        if (line.substr(0,9) == "<keyword>" || line.substr(0, 12) == "<identifier>" || line.substr(0, 17) == "<integerConstant>" || line.substr(0,16) == "<stringConstant>" || line.substr(0,8) == "<symbol>") {
            return true;
        } else {
            return false;
        }
    }

    bool isToken(string line) {
        if (line.substr(0,9) == "<keyword>" || line.substr(0, 12) == "<identifier>" || line.substr(0, 17) == "<integerConstant>" || line.substr(0,16) == "<stringConstant>" || line.substr(0,8) == "<symbol>") {
            return true;
        } else {
            return false;
        }
    }

    string getTokenType() {
        string token = *currentLine;
        return token.substr(1, token.find('>') - 1);
    }

    string getTokenContent() {
        string token = *currentLine;
        string tokenType = getTokenType();
        if (!isToken()) return "";
        return token.substr(tokenType.length() + 3, token.length() - 2 * tokenType.length() - 7);
    }

    string getTokenType(string token) {
        return token.substr(1, token.find('>') - 1);
    }

    string getTokenContent(string token) {
        string tokenType = getTokenType(token);
        if (!isToken(token)) return "";
        return token.substr(tokenType.length() + 3, token.length() - 2 * tokenType.length() - 7);
    }

    void writeError(string error) {
        //cout << "Failed at line " << "'" <<  *currentLine << "'" << endl << endl;
        //cout << vmDump.str() << endl;
        ofstream err(errorFile);
        err << error << endl;
        err.close();
        exit(EXIT_FAILURE);
    }

    void writeVM() {
        ofstream vm(vmFile);
        vm << vmDump.str();
        vm.close();
    }

    string trim(const string s) { // removes whitespace characters from beginnig and end of string s
        string x = s;
        for(int i=0; i<(int)s.length(); i++) {
            const int l = (int)x.length()-1;
            if(x[l]==' '||x[l]=='\t'||x[l]=='\n'||x[l]=='\v'||x[l]=='\f'||x[l]=='\r'||x[l]=='\0') x.erase(l, 1);
            if(x[0]==' '||x[0]=='\t'||x[0]=='\n'||x[0]=='\v'||x[0]=='\f'||x[0]=='\r'||x[0]=='\0') x.erase(0, 1);
        }
        return x;
    }

    void dump(string instruction) {
        vmDump << instruction << "\n";
    }

    bool isOp() {
        return (currentLine->substr(0,8) == "<symbol>" && (getTokenContent() == "+" || getTokenContent() == "-" || getTokenContent() == "*" || getTokenContent() == "/" || getTokenContent() == "&amp;" || getTokenContent() == "|" || getTokenContent() == "&lt;" || getTokenContent() == "&gt;" || getTokenContent() == "="));
    }

    bool isUnaryOp() {
        return (*currentLine == "<symbol> - </symbol>" || *currentLine == "<symbol> ~ </symbol>");
    }

    string op2vm(string op, bool isUnary=false) {
        if (op == "+") {
            return "add";
        } else if (op == "-" && !isUnary) {
            return "sub";
        } else if (op == "&amp;") {
            return "and";
        } else if (op == "|") {
            return "or";
        } else if (op == "&gt;") {
            return "gt";
        } else if (op == "&lt;") {
            return "lt";
        } else if (op == "=") {
            return "eq";
        } else if (op == "*") {
            return "call Math.multiply 2";
        } else if (op == "/") {
            return "call Math.divide 2";
        } else if (op == "-" && isUnary) {
            return "neg";
        } else if (op == "~") {
            return "not";
        }

        writeError("Syntax error: operator expected.");
        return "";
    }

    void incrementLine() {
        //cout << *currentLine << endl;
        currentLine++;
    }
    public:

    CodeGenerationEngine(stringstream &analysis, string vmPath, string errPath) {
        string line;
        while(getline(analysis, line, '\n')) {
            lines.push_back(trim(line));
        }
        currentLine = lines.begin();
        vmFile = vmPath;
        errorFile = errPath;
        fieldCount = 0;
        staticCount = 0;
        labelNum = 0;

        incrementLine();
        compileClass();
        
        writeVM();
    }

    void compileClass() {
        classSymbolTable.clear();
        staticCount = 0;
        fieldCount = 0;

        incrementLine();      // <keyword> class </keyword>
        currentClassName = getTokenContent(); incrementLine(); // class name identifier
        incrementLine();      // <symbol> { </symbol>

        while (*currentLine == "<classVarDec>") {
            incrementLine();    // <classVarDec>
            compileClassVarDec();
        }

        while (*currentLine == "<subroutineDec>") {
            incrementLine();    // <subroutineVarDec>
            compileSubroutine();
        }

        incrementLine();      // <symbol> } </symbol>
        incrementLine();      // </class>
    }

    void compileSubroutine() {
        subroutineSymbolTable.clear();
        localCount = 0;
        argumentCount = 0;

        currentSubroutineType = getTokenContent(); incrementLine();
        string returnType = getTokenContent(); incrementLine();
        currentSubroutineName = getTokenContent(); incrementLine();
        if (currentSubroutineType == "method") {
            subroutineSymbolTable["this"] = variable("this", "argument", currentClassName, argumentCount++);
        }
        incrementLine();      // <symbol> ( </symbol>
        incrementLine();      // <parameterList>
        compileParameterList();
        incrementLine();      // <symbol> ) </symbol>
        incrementLine();      // <subroutineBody>
        compileSubroutineBody();
        incrementLine();      // </subroutineDec>
    }

    void compileSubroutineBody() {
        incrementLine();      // <symbol> { </symbol>
        while (*currentLine == "<varDec>") {
            incrementLine();      // <varDec>
            compileVarDec();
        }

        dump("function " + currentClassName + "." + currentSubroutineName + " " + to_string(localCount));
        if (currentSubroutineType == "constructor") {
            dump("push constant " + to_string(fieldCount));
            dump("call Memory.alloc 1");
            dump("pop pointer 0");
        } else if (currentSubroutineType == "method") {
            dump("push argument 0");
            dump("pop pointer 0");
        }
        
        incrementLine();      // <statements>
        compileStatements();
        incrementLine();      // <symbol> } </symbol>
        incrementLine();      // </subroutineBody>
    }

    void compileClassVarDec() {
        string kind = getTokenContent(); incrementLine();   // <keyword> kind </keyword>
        string type = getTokenContent(); incrementLine();   // <keyword> datatype </keyword>
        string name = getTokenContent(); incrementLine();   // <identifier> name </identifier>
        if (kind == "field") kind = "this";

        if (classSymbolTable.count(name) == 0) {
            classSymbolTable[name] = variable(name, kind, type, kind == "this" ? fieldCount++ : staticCount++);
        }/* else {
            writeError("Declaration error: Repeated declaration of '" + name + "'");
        }*/

        while(getTokenContent() == ",") {
            incrementLine();                                // <symbol> , </symbol>
            name = getTokenContent(); incrementLine();      // <identifier> name </identifier>

            if (classSymbolTable.count(name) == 0) {
                classSymbolTable[name] = variable(name, kind, type, kind == "this" ? fieldCount++ : staticCount++);
            }/* else {
                writeError("Declaration error: Repeated declaration of '" + name + "'");
            }*/
        }

        incrementLine();      // ;
        incrementLine();      // </classVarDec>
    }

    void compileParameterList() {
        while (*currentLine != "</parameterList>") {
            string type = getTokenContent(); incrementLine();   // <keyword> datatype </keyword>
            string name = getTokenContent(); incrementLine();   // <identifier> name </identifier>
            if (subroutineSymbolTable.count(name) == 0) {
                subroutineSymbolTable[name] = variable(name, "argument", type, argumentCount++);
            }/* else {
                writeError("Declaration error: Repeated declaration of '" + name + "'");
            }*/

            if (getTokenContent() == ",") incrementLine();      // <symbol> , </symbol>
        }

        incrementLine();      // </paramenterList>
    }

    void compileStatements() {
        while(*currentLine != "</statements>") {
            if (*currentLine == "<letStatement>") {
                incrementLine();                        // <letStatement>
                compileLetStatement();
            } else if (*currentLine == "<ifStatement>") {
                incrementLine();                        // <ifStatement>
                compileIfStatement();
            } else if (*currentLine == "<whileStatement>") {
                incrementLine();                        // <whileStatement>
                compileWhileStatement();
            } else if (*currentLine == "<doStatement>") {
                incrementLine();                        // <doStatement>
                compileDoStatement();
            } else if (*currentLine == "<returnStatement>") {
                incrementLine();                        // <returnStatement>
                compileReturnStatement();
            }
        }
        incrementLine();                                // </statements>
    }
    
    void compileVarDec() {
        incrementLine();      // <keyword> var </keyword>
        string type = getTokenContent(); incrementLine();
        string name = getTokenContent(); incrementLine();
        
        if (subroutineSymbolTable.count(name) == 0) {
            subroutineSymbolTable[name] = variable(name, "local", type, localCount++);
        }/* else {
            writeError("Declaration error: Repeated declaration of '" + name + "'");
        }*/

        while(getTokenContent() == ",") {
            incrementLine();            // <symbol> , </symbol>
            name = getTokenContent();
            incrementLine();            // <identifier> varName </identifier>

            if (subroutineSymbolTable.count(name) == 0) {
                subroutineSymbolTable[name] = variable(name, "local", type, localCount++);
            }/* else {
                writeError("Declaration error: Repeated declaration of '" + name + "'");
            }*/
        }

        incrementLine();      // ;
        incrementLine();      // </varDec>
    }

    void compileIfStatement() {
        int TlabelNum = labelNum; labelNum += 2;
        incrementLine();      // <keyword> if </keyword>
        incrementLine();      // <symbol> ( </symbol>
        incrementLine();      // <expression>
        compileExpression();
        incrementLine();      // <symbol> ) </symbol>
        incrementLine();      // <symbol> { </symbol>
        dump("not");
        dump("if-goto " + currentClassName + "." + to_string(TlabelNum));
        incrementLine();      // <statments>
        compileStatements();
        incrementLine();      // <symbol> } </symbol>
        dump("goto " + currentClassName + "." + to_string(TlabelNum + 1));
        dump("label " + currentClassName + "." + to_string(TlabelNum));
        if (getTokenContent() == "else") {
            incrementLine();      // <keyword> else </keyword>
            incrementLine();      // <symbol> { </symbol>
            incrementLine();      // <statments>
            compileStatements();
            incrementLine();      // <symbol> } </symbol>
        }
        dump("label " + currentClassName + "." + to_string(TlabelNum + 1));
        incrementLine();      // </ifStatement>
    }
    
    void compileReturnStatement() {
        incrementLine();          // <keyword> return </keyword>
        if (*currentLine == "<expression>") {
            incrementLine();      // <expression>
            compileExpression();
            dump("return");
        } else {
            dump("push constant 0");
            dump("return");
        }
        incrementLine();      // <symbol> ; </symbol>
        incrementLine();      // </returnStatement>
    }

    void compileWhileStatement() {
        int TlabelNum = labelNum; labelNum += 2;
        incrementLine();      // <keyword> while </keyword>
        incrementLine();      // <symbol> ( </symbol>
        incrementLine();      // <expression>
        dump("label " + currentClassName + "." + to_string(TlabelNum));
        compileExpression();
        incrementLine();      // <symbol> ) </symbol>
        dump("not");
        dump("if-goto " + currentClassName + "." + to_string(TlabelNum + 1));
        incrementLine();      // <symbol> { </symbol>
        incrementLine();      // <statements>
        compileStatements();
        incrementLine();      // <symbol> } </symbol>
        dump("goto " + currentClassName + "." + to_string(TlabelNum));
        dump("label " + currentClassName + "." + to_string(TlabelNum + 1));
        incrementLine();      // </whileStatement>
    }
    
    void compileDoStatement() {                 
        incrementLine();      // <keyword> do </keyword>
        string id1 = getTokenContent();
        incrementLine();      // <identifier> subroutineName </identifier>
        
        if (getTokenContent() == ".") {
            incrementLine();     // <symbol> . </symbol>
            string id2 = getTokenContent();
            string kind, type;
            int index;
            incrementLine();     // <identifier> id2 </identifier>
            if (subroutineSymbolTable.count(id1) > 0) {
                kind = subroutineSymbolTable[id1].kind;
                type = subroutineSymbolTable[id1].type;
                index = subroutineSymbolTable[id1].index;
                
                dump("push " + kind + " " + to_string(index));
                incrementLine();      // <symbol> ( </symbol>
                incrementLine();      // <expressionList>
                int nP = compileExpressionList();
                incrementLine();      // <symbol> ) </symbol>
                incrementLine();      // <symbol> ; </symbol>

                dump("call " + type + "." + id2 + " " + to_string(nP + 1));
                dump("pop temp 0");
            } else if (classSymbolTable.count(id1) > 0) {
                kind = classSymbolTable[id1].kind;
                type = classSymbolTable[id1].type;
                index = classSymbolTable[id1].index;
                
                dump("push " + kind + " " + to_string(index));
                incrementLine();      // <symbol> ( </symbol>
                incrementLine();      // <expressionList>
                int nP = compileExpressionList();
                incrementLine();      // <symbol> ) </symbol>
                incrementLine();      // <symbol> ; </symbol>

                dump("call " + type + "." + id2 + " " + to_string(nP + 1));
                dump("pop temp 0");
            } else {
                incrementLine();      // <symbol> ( </symbol>
                incrementLine();      // <expressionList>
                int nP = compileExpressionList();
                incrementLine();      // <symbol> ) </symbol>
                incrementLine();      // <symbol> ; </symbol>

                dump("call " + id1 + "." + id2 + " " + to_string(nP));
                dump("pop temp 0");
            }
        } else {
            dump("push pointer 0");
            incrementLine();      // <symbol> ( </symbol>
            incrementLine();      // <expressionList>
            int nP = compileExpressionList();
            incrementLine();      // <symbol> ) </symbol>
            incrementLine();      // <symbol> ; </symbol>

            dump("call " + currentClassName + "." + id1 + " " + to_string(nP + 1));
            dump("pop temp 0");
        }

        incrementLine();     // </doStatement>
    }
    
    void compileLetStatement() {            // add check in classSymbolTable?
        incrementLine();      // <keyword> let </keyword>
        string name = getTokenContent(); incrementLine();     // <identifier> name </identifier>
        string kind, type;
        int index;
        if (subroutineSymbolTable.count(name) > 0)  {
            kind = subroutineSymbolTable[name].kind;
            type = subroutineSymbolTable[name].type;
            index = subroutineSymbolTable[name].index;
        } else if (classSymbolTable.count(name) > 0) {
            kind = classSymbolTable[name].kind;
            type = classSymbolTable[name].type;
            index = classSymbolTable[name].index;
        } else {
            writeError("Declaration error: " + name + " undeclared.");
        }
        
        if (getTokenContent() == "[") {
            incrementLine();      // <symbol> [ </symbol>
            incrementLine();      // <expression>
            compileExpression();
            incrementLine();      // <symbol> ] </symbol>
            dump("push " + kind + " " + to_string(index));
            dump("add");
            incrementLine();      // <symbol> = </symbol>
            incrementLine();      // <expression>
            compileExpression();
            dump("pop temp 0");
            dump("pop pointer 1");
            dump("push temp 0");
            dump("pop that 0");    
        } else {
            incrementLine();      // <symbol> = </symbol>
            incrementLine();      // <expression>
            compileExpression();
            dump("pop " + kind + " " + to_string(index));
        }

        incrementLine();          // <symbol> ; </symbol>
        incrementLine();          // </letStatement>
    }

    void compileExpression() {
        incrementLine();            // <term>
        compileTerm();
        
        string op;
        while (isOp()) {
            op = getTokenContent();
            incrementLine();            // <symbol> op </symbol>
            incrementLine();            // <term>
            compileTerm();
            dump(op2vm(op));
        }

        incrementLine();          // </expression>
    }

    int compileExpressionList() {
        int nP = 0;
        while (*currentLine == "<expression>") {
            incrementLine();          // <expression>
            compileExpression();
            nP++;
            
            if (isToken() && getTokenContent() == ",") incrementLine();     // <symbol> , </symbol>
        }
        
        incrementLine();              // </expressionList>
        return nP; 
    }

    void compileTerm() {                        // add errors
        if (isUnaryOp()) {  
            string op = getTokenContent();
            incrementLine();      // <symbol> unaryOp </symbol>
            incrementLine();      // <expression>
            compileTerm();
            dump(op2vm(op, true));
        } else if (getTokenContent() == "(") {
            incrementLine();      // <symbol> ( </symbol>
            incrementLine();      // <expression>
            compileExpression();
            incrementLine();      // <symbol> ) </symbol>
        } else if (getTokenType() == "integerConstant") {
            dump("push constant " + getTokenContent());
            incrementLine();      // <integerConstant> integer </integerConstant>
        } else if (getTokenType() == "keyword") {
            if (getTokenContent() == "true") {
                dump("push constant 0");
                dump("not");
            } else if (getTokenContent() == "false") {
                dump("push constant 0");
            } else if (getTokenContent() == "null") {
                dump("push constant 0");
            } else if (getTokenContent() == "this") {
                dump("push pointer 0");
            }
            incrementLine();      // <keyword> keywordConstant </keyword>
        } else if (getTokenType() == "stringConstant") {
            int STRLEN = getTokenContent().size();
            dump("push constant " + to_string(STRLEN));
            dump("call String.new 1");
            for(int i=0; i < STRLEN; i++) {
                dump("push constant " + to_string(int(getTokenContent()[i])));
                dump("call String.appendChar 2");
            }
            incrementLine();      // <stringConstant> string </stringConstant>
        } else if (getTokenType() == "identifier") {
            currentLine++;
            vector<string>::iterator next = currentLine;
            currentLine--;
            string name = getTokenContent();

            if (getTokenContent(*next) != "(" && getTokenContent(*next) != "[" && getTokenContent(*next) != ".") {
                if (subroutineSymbolTable.count(getTokenContent()) > 0) {
                    string kind = subroutineSymbolTable[name].kind;
                    int index = subroutineSymbolTable[name].index;

                    dump("push " + kind + " " + to_string(index));
                } else if (classSymbolTable.count(getTokenContent()) > 0) {
                    string kind = classSymbolTable[name].kind;
                    int index = classSymbolTable[name].index;

                    dump("push " + kind + " " + to_string(index));
                } else {
                    writeError("Declaration error: " + name + " undeclared.");
                }
                
                incrementLine();          // <identifier> varName </identifier>
            } else if (getTokenContent(*next) == "[") {
                if (subroutineSymbolTable.count(getTokenContent()) > 0) {
                    string kind = subroutineSymbolTable[name].kind;
                    int index = subroutineSymbolTable[name].index;

                    incrementLine();          // <identifier> varName </identifier>
                    incrementLine();          // <symbol> [ </symbol>
                    incrementLine();          // <expression>
                    compileExpression();
                    incrementLine();          // <symbol> ] </symbol>
                    dump("push " + kind + " " + to_string(index));
                    dump("add");
                    dump("pop pointer 1");
                    dump("push that 0");

                } else if (classSymbolTable.count(getTokenContent()) > 0) {
                    string kind = classSymbolTable[name].kind;
                    int index = classSymbolTable[name].index;

                    incrementLine();          // <identifier> varName </identifier>
                    incrementLine();          // <symbol> [ </symbol>
                    incrementLine();          // <expression>
                    compileExpression();
                    incrementLine();          // <symbol> ] </symbol>
                    dump("push " + kind + " " + to_string(index));
                    dump("add");
                    dump("pop pointer 1");
                    dump("push that 0");
                } else {
                    writeError("Declaration error: " + name + " undeclared.");
                }
            } else if (getTokenContent(*next) == "(" || getTokenContent(*next) == ".") {
                string id1 = getTokenContent();
                incrementLine();      // <identifier> subroutineName </identifier>
                
                if (getTokenContent() == ".") {
                    incrementLine();     // <symbol> . </symbol>
                    string id2 = getTokenContent();
                    incrementLine();     // <identifier> subroutineName </identifier>
                    string kind, type;
                    int index;
                    if (subroutineSymbolTable.count(id1) > 0) {
                        kind = subroutineSymbolTable[id1].kind;
                        type = subroutineSymbolTable[id1].type;
                        index = subroutineSymbolTable[id1].index;
                        
                        incrementLine();      // <symbol> ( </symbol>
                        incrementLine();      // <expressionList>
                        dump("push " + kind + " " + to_string(index));
                        int nP = compileExpressionList();
                        incrementLine();      // <symbol> ) </symbol>

                        
                        dump("call " + type + "." + id2 + " " + to_string(nP + 1));
                        //dump("pop temp 0");
                    } else if (classSymbolTable.count(id1) > 0) {
                        kind = classSymbolTable[id1].kind;
                        type = classSymbolTable[id1].type;
                        index = classSymbolTable[id1].index;
                        
                        incrementLine();      // <symbol> ( </symbol>
                        incrementLine();      // <expressionList>
                        dump("push " + kind + " " + to_string(index));
                        int nP = compileExpressionList();
                        incrementLine();      // <symbol> ) </symbol>

                        
                        dump("call " + type + "." + id2 + " " + to_string(nP + 1));
                        //dump("pop temp 0");
                    } else {
                        incrementLine();      // <symbol> ( </symbol>
                        incrementLine();      // <expressionList>
                        int nP = compileExpressionList();
                        incrementLine();      // <symbol> ) </symbol>

                        dump("call " + id1 + "." + id2 + " " + to_string(nP));
                        //dump("pop temp 0");
                    }
                } else {
                    dump("push pointer 0");
                    incrementLine();      // <symbol> ( </symbol>
                    incrementLine();      // <expressionList>
                    int nP = compileExpressionList();
                    incrementLine();      // <symbol> ) </symbol>

                    dump("call " + currentClassName + "." + id1 + " " + to_string(nP + 1));
                    //dump("pop temp 0");
                }
            }
        }

        incrementLine();          // </term>
    }
};

class AnalysisEngine {              // process T.xml into .xml
    private:
    vector<string> tokens;
    vector<string>::iterator currentToken;
    stringstream xmlDump;
    string errorFile;
    string anzFile;
    int numSpaces;

    string getTokenType() {
        string token = *currentToken;
        return token.substr(1, token.find('>') - 1);
    }

    string getTokenContent() {
        string token = *currentToken;
        string tokenType = getTokenType(token);
        return token.substr(tokenType.length() + 3, token.length() - 2 * tokenType.length() - 7);
    }

    string getTokenType(string token) {
        return token.substr(1, token.find('>') - 1);
    }

    string getTokenContent(string token) {
        string tokenType = getTokenType(token);
        return token.substr(tokenType.length() + 3, token.length() - 2 * tokenType.length() - 6);
    }

    string generateSpaces() {
        return string(numSpaces, ' ');
    }

    bool isType() {
        string content = getTokenContent();
        return getTokenType() == "identifier" || (getTokenType() == "keyword" && (content == "int" || content == "char" || content == "boolean"));
    }

    bool isFunctionType() {
        string content = getTokenContent();
        return (getTokenType() == "keyword" && (content == "constructor" || content == "function" || content == "method"));
    }

    bool isOp() {
        string content = getTokenContent();
        return (getTokenType() == "symbol" && (content == "+" || content == "-" || content == "*" || content == "/" || content == "&amp;" || content == "|" || content == "&lt;" || content == "&gt;" || content == "="));
    }

    bool isUnaryOp() {
        string content = getTokenContent();
        return (getTokenType() == "symbol" && (content == "~" || content == "-"));
    }

    void incrementSpaces() {
        numSpaces += 2;
    }

    void decrementSpaces() {
        numSpaces -= 2;
    }

    void dump(string line) {
        //cout << generateSpaces() << line << "\n";
        xmlDump << generateSpaces() << line << "\n";
    }

    void dump() {
        //cout << generateSpaces() << *currentToken << "\n";
        xmlDump << generateSpaces() << *currentToken << "\n";
        currentToken++;
    }

    void writeError(string type) {
        //cout << "Failed at " << "'" <<  *currentToken << "'" << endl << "'" << getTokenType() << "'" << endl << "'" <<  getTokenContent()  << "'" << endl << endl;
        //cout << xmlDump.str() << endl;
        ofstream err(errorFile);
        if (getTokenType() != type) {
            err << "ERROR: Expecting <" + type + "> but " + getTokenContent() << endl;
        } else {
            err << "ERROR: " << getTokenContent() << endl;
        }
        err.close();
        exit(EXIT_FAILURE);
    }

    void writeAnalysis() {
        ofstream anz(anzFile);
        anz << xmlDump.str();
        anz.close();
    }

    public:
    AnalysisEngine(stringstream &tokenized, string anzPath, string errPath) {
        string token;
        while(getline(tokenized, token, '\n')) {
            tokens.push_back(token);
        }
        currentToken = ++tokens.begin();
        numSpaces = 0;

        anzFile = anzPath;
        errorFile = errPath;
        compileClass();

        writeAnalysis();
    }

    string getXMLDump() {
        return xmlDump.str();
    }

    void compileClass() {
        if (getTokenType() == "keyword" && getTokenContent() == "class") {
            dump("<class>");
            incrementSpaces();
            dump();
        } else {
            writeError("keyword");
        }

        if (getTokenType() == "identifier") {
            dump();
        } else {
            writeError("identifier");
        }

        if (getTokenType() == "symbol" && getTokenContent() == "{") {
            dump();
        } else {
            writeError("symbol");
        }
        
        while (getTokenType() == "keyword" && (getTokenContent() == "static" || getTokenContent() == "field")) {
            compileClassVarDec();
        }

        while (getTokenType() == "keyword" && (getTokenContent() == "constructor" || getTokenContent() == "function" || getTokenContent() == "method")) {
            compileSubroutine();
        }

        if (getTokenType() == "symbol" && getTokenContent() == "}") {
            dump();
        } else {
            writeError("symbol");
        }

        decrementSpaces();
        dump("</class>");
    }

    void compileClassVarDec() {
        dump("<classVarDec>");
        incrementSpaces();
        dump();

        if (isType()) {
            dump();
        } else {
            writeError("keyword");
        }

        if (getTokenType() == "identifier") {
            dump();
        } else {
            writeError("identifier");
        }

        while (getTokenType() == "symbol" && getTokenContent() == ",") {
            dump();
            if (getTokenType() == "identifier") {
                dump();
            } else {
                writeError("identifier");
            }
        }

        if (getTokenType() == "symbol" && getTokenContent() == ";") {
            dump();
        } else {
            writeError("symbol");
        }

        decrementSpaces();
        dump("</classVarDec>");
    }

    void compileSubroutine() {
        dump("<subroutineDec>");
        incrementSpaces();

        if (isFunctionType()) {
            dump();
        } else {
            writeError("keyword");
        }

        if ((getTokenType() == "keyword" && getTokenContent() == "void") || isType()) {
            dump();
        } else {
            writeError("keyword");
        }

        if (getTokenType() == "identifier") {
            dump();
        } else {
            writeError("identifier");
        }

        if (getTokenType() == "symbol" && getTokenContent() == "(") {
            dump();
        } else {
            writeError("symbol");
        }

        compileParameterList();

        if (getTokenType() == "symbol" && getTokenContent() == ")") {
            dump();
        } else {
            writeError("symbol");
        }

        dump("<subroutineBody>");
        incrementSpaces();

        if (getTokenType() == "symbol" && getTokenContent() == "{") {
            dump();
        } else {
            writeError("symbol");
        }

        while (getTokenType() == "keyword" && getTokenContent() == "var") {
            dump("<varDec>");
            incrementSpaces();

            dump();
            if (isType()) {
                dump();
            } else {
                writeError("keyword");
            }

            if (getTokenType() == "identifier") {
                dump();
            } else {
                writeError("identifier");
            }

            while (getTokenType() == "symbol" && getTokenContent() == ",") {
                dump();
                if (getTokenType() == "identifier") {
                    dump();
                } else {
                    writeError("identifier");
                }
            }

            if (getTokenType() == "symbol" && getTokenContent() == ";") {
                dump();
            } else {
                writeError("symbol");
            }

            decrementSpaces();
            dump("</varDec>");
        }

        compileStatements();

        if (getTokenType() == "symbol" && getTokenContent() == "}") {
            dump();
        } else {
            writeError("symbol");
        }

        decrementSpaces();
        dump("</subroutineBody>");

        decrementSpaces();
        dump("</subroutineDec>");
    }
    
    void compileParameterList() { 
        dump("<parameterList>");
        incrementSpaces();

        if (isType()) {
            dump();
            
            if (getTokenType() == "identifier") {
                dump();
            } else {
                writeError("identifier");
            }

            while (getTokenType() == "symbol" && getTokenContent() == ",") {
                dump();
                
                if (isType()) {
                    dump();
                } else {
                    writeError("keyword");
                }
                
                if (getTokenType() == "identifier") {
                    dump();
                } else {
                    writeError("identifier");
                }
            }
        } else if (getTokenType() != "symbol" || getTokenContent() != ")") {
            writeError("symbol");
        }

        decrementSpaces();
        dump("</parameterList>");
    }

    void compileStatements() {
        dump("<statements>");
        incrementSpaces();

        while(getTokenType() == "keyword") {
            string content = getTokenContent();
            if (content == "let") {
                compileLetStatement();
            } else if (content == "if") {
                compileIfStatement();
            } else if (content == "while") {
                compileWhileStatement();
            } else if (content == "do") {
                compileDoStatement();
            } else if (content == "return") {
                compileReturnStatement();
            } else {
                writeError("keyword");
            }
        }

        decrementSpaces();
        dump("</statements>");
    }

    void compileIfStatement() {
        dump("<ifStatement>");
        incrementSpaces();
        dump();

        if (getTokenType() == "symbol" && getTokenContent() == "(") {
            dump();
        } else {
            writeError("symbol");
        }

        compileExpression();

        if (getTokenType() == "symbol" && getTokenContent() == ")") {
            dump();
        } else {
            writeError("symbol");
        }

        if (getTokenType() == "symbol" && getTokenContent() == "{") {
            dump();
        } else {
            writeError("symbol");
        }

        compileStatements();

        if (getTokenType() == "symbol" && getTokenContent() == "}") {
            dump();
        } else {
            writeError("symbol");
        }

        if (getTokenType() == "keyword" && getTokenContent() == "else") {
            dump();

            if (getTokenType() == "symbol" && getTokenContent() == "{") {
                dump();
            } else {
                writeError("symbol");
            }

            compileStatements();

            if (getTokenType() == "symbol" && getTokenContent() == "}") {
                dump();
            } else {
                writeError("symbol");
            }
        }

        decrementSpaces();
        dump("</ifStatement>");
    }
    
    void compileReturnStatement() {
        dump("<returnStatement>");
        incrementSpaces();
        dump();

        string content = getTokenContent();
        if (getTokenType() == "integerConstant") {
            compileExpression();
        } else if (getTokenType() == "stringConstant") {
            compileExpression();
        } else if (getTokenType() == "keyword" && (content == "true" || content == "false" || content == "null" || content == "this")) {
            compileExpression();
        } else if (getTokenType() == "identifier") {
            compileExpression();
        } else if (getTokenType() == "symbol" && getTokenContent() == "(") {
            compileExpression();
        } else if (isUnaryOp()) {
            compileExpression();
        }
        
        if (getTokenType() == "symbol" && getTokenContent() == ";") {
            dump();
        } else {
            writeError("symbol");
        }

        decrementSpaces();
        dump("</returnStatement>");
    }

    void compileWhileStatement() {
        dump("<whileStatement>");
        incrementSpaces();
        dump();

        if (getTokenType() == "symbol" && getTokenContent() == "(") {
            dump();
        } else {
            writeError("symbol");
        }

        compileExpression();

        if (getTokenType() == "symbol" && getTokenContent() == ")") {
            dump();
        } else {
            writeError("symbol");
        }

        if (getTokenType() == "symbol" && getTokenContent() == "{") {
            dump();
        } else {
            writeError("symbol");
        }

        compileStatements();

        if (getTokenType() == "symbol" && getTokenContent() == "}") {
            dump();
        } else {
            writeError("symbol");
        }

        decrementSpaces();
        dump("</whileStatement>");
    }

    void compileDoStatement() {
        dump("<doStatement>");
        incrementSpaces();
        dump();

        compileSubroutineCall();

        if (getTokenType() == "symbol" && getTokenContent() == ";") {
            dump();
        } else {
            writeError("symbol");
        }

        decrementSpaces();
        dump("</doStatement>");
    }

    void compileLetStatement() {
        dump("<letStatement>");
        incrementSpaces();
        dump();

        if (getTokenType() == "identifier") {
            dump();
        } else {
            writeError("identifier");
        }

        if (getTokenType() == "symbol" && getTokenContent() == "[") {
            dump();
            compileExpression();
            if (getTokenType() == "symbol" && getTokenContent() == "]") {
                dump();
            } else {
                writeError("symbol");
            }
        }

        if (getTokenType() == "symbol" && getTokenContent() == "=") {
            dump();
        } else {
            writeError("symbol");
        }

        compileExpression();

        if (getTokenType() == "symbol" && getTokenContent() == ";") {
            dump();
        } else {
            writeError("symbol");
        }

        decrementSpaces();
        dump("</letStatement>");
    }

    void compileExpression() {
        dump("<expression>");
        incrementSpaces();

        compileTerm();

        while (isOp()) {
            dump();
            compileTerm();
        }


        decrementSpaces();
        dump("</expression>");
    }

    int compileExpressionList() {
        dump("<expressionList>");
        incrementSpaces();
        
        string content = getTokenContent();
        int numExpressions = 0;
        if (getTokenType() == "integerConstant") {
            numExpressions++;
            compileExpression();
        } else if (getTokenType() == "stringConstant") {
            numExpressions++;
            compileExpression();
        } else if (getTokenType() == "keyword" && (content == "true" || content == "false" || content == "null" || content == "this")) {
            numExpressions++;
            compileExpression();
        } else if (getTokenType() == "identifier") {
            numExpressions++;
            compileExpression();
        } else if (getTokenType() == "symbol" && getTokenContent() != ")") {
            numExpressions++;
            compileExpression();
        }
        
        while(getTokenType() == "symbol" && getTokenContent() == ",") {
            numExpressions++;
            dump();
            compileExpression();
        }

        decrementSpaces();
        dump("</expressionList>");

        return numExpressions;
    }
    
    void compileTerm() {
        dump("<term>");
        incrementSpaces();

        string content = getTokenContent();

        if (getTokenType() == "integerConstant") {
            dump();
        } else if (getTokenType() == "stringConstant") {
            dump();
        } else if (getTokenType() == "keyword" && (content == "true" || content == "false" || content == "null" || content == "this")) {
            dump();
        } else if (getTokenType() == "identifier") {
            dump();
            if (getTokenType() == "symbol" && getTokenContent() == "[") {
                dump();
                compileExpression();
                if (getTokenType() == "symbol" && getTokenContent() == "]") {
                    dump();
                } else {
                    writeError("symbol");
                }
            } else if (getTokenType() == "symbol" && getTokenContent() == "(") {
                compileSubroutineCall(true);
            } else if (getTokenType() == "symbol" && getTokenContent() == ".") {
                dump();
                if (getTokenType() == "identifier") {
                    dump();
                } else {
                    writeError("identifier");
                }
                if (getTokenType() == "symbol" && getTokenContent() == "(") {
                    compileSubroutineCall(true);
                }
            }
        } else if (getTokenType() == "symbol" && getTokenContent() == "(") {
            dump();
            compileExpression();
            if (getTokenType() == "symbol" && getTokenContent() == ")") {
                dump();
            } else {
                writeError("symbol");
            }
        } else if (isUnaryOp()){
            dump();
            compileTerm();
        } else {
            writeError("identifier");
        }

        decrementSpaces();
        dump("</term>");
        
        
    }

    void compileSubroutineCall(bool identifierProvided = false) {
        if (!identifierProvided) {
            if (getTokenType() == "identifier") {
                dump();
            } else {
                writeError("identifier");
            }
        

            if (getTokenType() == "symbol" && getTokenContent() == ".") {
                dump();
                if (getTokenType() == "identifier") {
                    dump();
                } else {
                    writeError("identifier");
                }
            }
        }

        if (getTokenType() == "symbol" && getTokenContent() == "(") {
            dump();
        } else {
            writeError("symbol");
        }

        compileExpressionList();

        if (getTokenType() == "symbol" && getTokenContent() == ")") {
            dump();
        } else {
            writeError("symbol");
        }
    }

};

class TokenizationEngine {          // process .jack into T.xml
    string errorFile;

    void writeError(string error) {
        ofstream err(errorFile);
        err << error << endl;
        err.close();
        exit(EXIT_FAILURE);
    }

    
    public:
    TokenizationEngine(string errPath) {
        errorFile = errPath;
    }

    stringstream padSymbols(string original) {
        stringstream processed;
        bool inString = false;
        for(char c: original) {
            switch (c) {
                case '{':
                case '}':
                case '(':
                case ')':
                case '[':
                case ']':
                case '.':
                case ',':
                case ';':
                case '+':
                case '-':
                case '*':
                case '/':
                case '|':
                case '=':
                case '~':
                    if (!inString) processed << " " << c << " ";
                    else processed << c;
                    break;
                case '&':
                    if (!inString) processed << " &amp; ";
                    else processed << c;
                    break;
                case '<':
                    if (!inString) processed << " &lt; ";
                    else processed << " < ";
                    break;
                case '>':
                    if (!inString) processed << " &gt; ";
                    else processed << " > ";
                    break;
                case '"':
                    processed << c;
                    inString = !inString;
                    break;
                case '\t':
                    if (!inString) processed << " ";
                    else processed << c;
                    break;
                default:
                    processed << c;
                    break;
            }
        }

        string superPadded = processed.str();
        processed.str("");

        bool lastCharSpace = false;
        inString = false;
        for (char c: superPadded) {
            if (!inString) {
                if (c == ' ' && lastCharSpace) {
                    lastCharSpace = true;
                } else if (c != ' ' && lastCharSpace) {
                    processed << c;
                    lastCharSpace = false;
                } else if (c == ' ' && !lastCharSpace) {
                    processed << c;
                    lastCharSpace = true;
                } else if (c !=  ' ' && !lastCharSpace) {
                    processed << c;
                    lastCharSpace = false;
                }
            } else {
                processed << c;
                lastCharSpace = false;
            }
            if (c == '"') inString = !inString;
        }
        
        return processed;
    }

    string createToken(string atomic) {
        if (atomic == "{" || atomic == "}" || atomic == "(" || atomic == ")" || atomic == "[" || 
            atomic == "]" || atomic == "." || atomic == "," || atomic == ";" || atomic == "+" || 
            atomic == "-" || atomic == "*" || atomic == "/" || atomic == "&amp;" || atomic == "|" || 
            atomic == "&lt;" || atomic == "&gt;" || atomic == "=" || atomic == "~") {
            
            return "<symbol> " + atomic + " </symbol>\n";
        } else if (atomic[0] == '"' && atomic.back() == '"') {
            return "<stringConstant> " + atomic.substr(1, atomic.length() - 2) + " </stringConstant>\n";
        } else if (atomic[0] >= '0' && atomic[0] <= '9') {
            if (all_of(atomic.begin(), atomic.end(), [](char c){return (c >= '0' && c <= '9');})) {
                return "<integerConstant> " + atomic + " </integerConstant>\n";
            } else {
                writeError("Lexical error: invalid token '" + atomic + "'");
            }
        } else if (atomic == "class" || atomic == "constructor" || atomic == "function" || 
            atomic == "method" || atomic == "field" || atomic == "static" || atomic == "var" ||
            atomic == "int" || atomic == "char" || atomic == "boolean" || atomic == "void" || atomic == "true" ||
            atomic == "false" || atomic == "null" || atomic == "this" || atomic == "let" || atomic == "do" ||
            atomic == "if" || atomic == "else" || atomic == "while" || atomic == "return") {
            
            return "<keyword> " + atomic + " </keyword>\n";
        } else {
            return "<identifier> " + atomic + " </identifier>\n";
        }
    } 
};

int main(int argc, char** argv) {

    int numFiles;

    // make sure command is executed properly
    if (argc < 3) {
        cerr << "ERROR: Provide appropriate command line arguments" << endl;
        return 1;
    } else {
        numFiles = stoi(argv[1]);
        if (argc != numFiles + 2) {
            cerr << "ERROR: Provide appropriate command line arguments" << endl;
            return 1;
        }
    } 

    // process every file individually
    for(int fileNumber = 2; fileNumber < argc; fileNumber++) {
        stringstream tokenized;
        tokenized << "<tokens>\n";
        string filename = argv[fileNumber];
        string classname = filename.substr(0, filename.length() - 5);
        string jackFileContents;
        
        // read file contents into jackFileContents
        if (ifstream jackFile = ifstream(filename.c_str())) {
            jackFileContents.assign((istreambuf_iterator<char>(jackFile)), (istreambuf_iterator<char>()));
        } else {
            cerr << "ERROR: File " + filename +  " not found" << endl;
            return 1;
        }

        // remove all comments, whitespace, multi-newlines from jackFileContents
        regex anyComment("(\\/\\/.*|\\/\\*[\\s\\S]*?\\*\\/)");
        regex newline("[\\r\\n]+");
        regex whitespaceNewline(" \\n");
        regex whitespace("[ ]+");

        jackFileContents = regex_replace(jackFileContents, anyComment, "");
        jackFileContents = regex_replace(jackFileContents, whitespace, " ");
        jackFileContents = regex_replace(jackFileContents, newline, "\n");
        jackFileContents = regex_replace(jackFileContents, whitespaceNewline, "\n");
        if (jackFileContents[0] == '\n' || jackFileContents[0] == '\r') jackFileContents.erase(jackFileContents.begin());
        if (jackFileContents[0] == '\n' || jackFileContents[0] == '\r') jackFileContents.erase(jackFileContents.begin());
        if (jackFileContents.back() == '\n' || jackFileContents.back() == '\r') jackFileContents.erase(jackFileContents.end() - 1);
        if (jackFileContents.back() == '\n' || jackFileContents.back() == '\r') jackFileContents.erase(jackFileContents.end() - 1);


        // convert .jack to T.xml
        TokenizationEngine te(classname + ".err");
        stringstream jackStream = te.padSymbols(jackFileContents);
        string atomic, word = "";
        bool inString = false;

        // create tokens properly: attempt 3
        for (char c: jackStream.str()) {
            if (inString) {
                if (c == '"') {
                    word += c;
                    inString = false;
                } else {
                    word += c;
                }
            } else {
                if (c == '"') {
                    if (word != "") {
                        tokenized << te.createToken(word);
                    }
                    word = "";
                    word += c;
                    inString = true;
                } else if (c == ' ' || c == '\n') {
                    if (word != "") {
                        tokenized << te.createToken(word);
                    }
                    word = "";
                } else {
                    word += c;
                }
            } 
        }

        tokenized << "</tokens>\n";

        // save T.xml
        ofstream outFile(classname + "T.xml");
        outFile << tokenized.str();
        outFile.close();

        // convert T.xml into .xml and save
        AnalysisEngine ae(tokenized, classname + ".xml", classname + ".err");
        string xmlDump = ae.getXMLDump();
        stringstream xmlStream(xmlDump);  

        // convert .xml to .vm and save
        CodeGenerationEngine cge(xmlStream, classname + ".vm", classname + ".err");    
    }
    

    return 0;
};