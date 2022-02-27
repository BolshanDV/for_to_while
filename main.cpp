//
// Created by Дмитрий on 15.02.2022.
//
#include <iostream>
#include "fstream"
#include <vector>
#include <regex>

using namespace std;

//**Utils function**
// function for delete space in start string
string& leftTrim(string& str)
{
    string chars = "/ ";

    str.erase(0, str.find_first_not_of(chars));
    return str;
}

//**Utils function**
// just simple function for split string by regex element [got from StackOverFlow]
vector<string> split (const string& s, const string& delimiter) {
    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    string token;
    vector<string> res;

    while ((pos_end = s.find (delimiter, pos_start)) != string::npos) {
        token = s.substr (pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        res.push_back (token);
    }

    res.push_back (s.substr (pos_start));
    return res;
}


struct For {
        string type; //type For [example: ClassicFor, NotValidFor , etc...]
        vector<string> conditionals; //All conditionals in for [example: int i=0, i < 55, etc...]
        vector<string> body; //Body for cycle after "{" or no brackets cycle
};

struct While {
        vector<string> initializeLines; // Start lines [example: int i=0; j=0 before while cycle]
        vector<string> conditionals; // All conditionals from For cycle
        vector<string> body; // Body while cycle on evert line
        string otherText;
        string constantInBody = leftTrim(conditionals[2]); // Usually it's auto-increment argument [example: i++, i--, etc...]
        string mainConditional = leftTrim(conditionals[1]); // Main conditional while cycle [example: i < 56, etc.. while(mainConditional)]
};

// reading file function
string readingFile(){
    ifstream file("/Users/dmitrij/CLionProjects/textConverter/text.txt");
    string str;
    string content;
    while (getline(file, str)) {
        content += str;
        content.push_back('\n');
    };
    return content;
}

void creatureReadyFile( const string& text) {
    ofstream out;
    out.open("/Users/dmitrij/CLionProjects/textConverter/result.txt", ios::app);
    out << text << endl;
};

// Some String builder function for while cycle
vector<string> builderStringifyWhileCycles(const vector<While>& whiles) {
    vector<string> whilesVector;

    for (auto & i : whiles) {
        string whileCycle;

        for (auto & line : i.initializeLines) {
            whileCycle.append(line).append(";\n"); // append ";" to initialize lines [example: before[i = 0], after this operation int i = 0;]
        }

        whileCycle.append("\twhile(").append(i.mainConditional).append(")").append("{\n\t\t"); // Create while cycle

        for (auto & bodyLine : i.body) {
            whileCycle.append(bodyLine).append(";\n\t\t");
        }

        whileCycle.append(i.constantInBody).append(";\n\t");
        whileCycle.append("}"); // close while cycle
        whileCycle.append(i.otherText);
        whilesVector.push_back(whileCycle); // Push stringify while cycle in Vector of cycles
    }

    return whilesVector;
}

// just a convert function to While struct in head project
vector<While> convertToWhileCycleObjects(vector<For> fors) {
    vector<While> vectorList;

    // this forEach in c++
    for (auto & i : fors) {
        vector<string> initializeLines;
            initializeLines.push_back('\t' + i.conditionals[0]);


        vector<string> conditionals = i.conditionals;
        vector<string> body;
        string otherText;
        if ( i.type != "NotValidFor" ) {
            for(int j = 0; j < i.body.size() - 1; j++){
                body.push_back(i.body[j]);
            }
            otherText = i.body.back();
        } else {
            otherText = i.body.back();
        }
        While obj = {initializeLines, conditionals, body, otherText};
        vectorList.push_back(obj); // Push created object while in Vector<While>
    }

    return vectorList;
}

vector<string> extractClassicBodyLines(const string& forBlock) {
    // Double split text
    string splitedText = split(split(forBlock, "{")[1], "}")[0];
    string otherText = forBlock.substr(forBlock.find('}') + 1);
    // Replace all \n and " " to ""
    splitedText = regex_replace(regex_replace(splitedText, regex("\n"), ""), regex(" "), "");

    // Split elements by ";" and create Vector
    vector<string> body = split(splitedText, ";");

    // Delete last element, because it always empty
    body.pop_back();
    body.push_back(otherText);

    return body;
}

// cope under function with small changes
vector<string> extractWithoutBracketsBodyLines(const string& forBlock) {
    string splitedText = split(split(forBlock, ")")[1], ";")[0];
    splitedText = regex_replace(regex_replace(splitedText, regex("\n"), ""), regex(" "), "");

    vector<string> body;
    body.push_back(splitedText);
    body.push_back(forBlock.substr(forBlock.find(splitedText) + splitedText.size() + 1));
    return body;
}
vector<string> extractOtherText( string forBlock) {
    string splitedText = forBlock.replace(0, forBlock.find(')') + 2, "");

    vector<string> body;
    body.push_back(splitedText);
    return body;
}

// process type of for
string processForType(const string& forBlock) {
    string extractedBody = leftTrim(split(forBlock, ")")[1]);

    // check first symbol after "for(cond) { or ; or \n"
    if (extractedBody.find_first_not_of('{')) { return "ClassicFor"; }
    if (extractedBody.find_first_not_of(';')) { return "NotValidFor"; }
    if (extractedBody.find_first_not_of('\n')) { return "ForWithoutBrackets"; }

    // if check throw with exception
    return "Unknown for";
}


// extract conditionals function for all type For cycles
vector<string> extractConditionals( const string& stringifyFor) {
    vector<string> conditionals;
    string stringifyForElement =  stringifyFor;
    stringifyForElement.insert(0, 1, '(');
    stringifyForElement = split(split(stringifyForElement, "(")[1], ")")[0];
    conditionals = split(stringifyForElement, ";");

    return conditionals;
}

// extract body function
vector<string> extractBody(const string& body, const string& type) {
    vector<string> bodyVector;

    if (type == "ClassicFor") bodyVector = extractClassicBodyLines(body);
    if (type == "ForWithoutBrackets") bodyVector = extractWithoutBracketsBodyLines(body);
    if (type == "NotValidFor") bodyVector = extractOtherText(body);

    return bodyVector;
}


vector<For> extractAndProcessFors(const string& text) {
    vector<For> list;
    vector<string> fors = split(text, "for(");
    creatureReadyFile(fors[0]);
    for (int i = 0; i < fors.size(); i++) {
        // skip i = 0 because it's always not for element
        if (i == 0) continue;

        // check type for with simple realization
        string type = processForType(fors[i]);

        // different functions which help you extract data from "for" cycles
        vector<string> conditionals = extractConditionals(fors[i]);
        vector<string> body = extractBody(fors[i], type);

        // build For object and push them in vector
        For obj = {type, conditionals, body};
        list.push_back(obj);
    }

    return list;
}

vector<string> injectToFile (const vector<string>& stringifyList ){
    string resultWhile;
    for(const auto & i : stringifyList) {
        creatureReadyFile(i);
    }
}

int main() {
    string text = readingFile();

    // extract and process all for's in document
    vector<For> fors = extractAndProcessFors(text);

    //Convert for's object to While's objects
    vector<While> whiles = convertToWhileCycleObjects(fors);

    //Build stringify while cycles
    vector<string> stringifyList = builderStringifyWhileCycles(whiles);

    // TODO Implement last function which will be record string whiles in file in the necessary places
    injectToFile(stringifyList);
    return 0;
}


