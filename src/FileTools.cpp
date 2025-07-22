#include <FileTools.h>
#include <fstream>
#include <sstream>
#include <cctype>
#include <iostream>
#include <algorithm>
#include <vector>

std::vector<std::string> Split(const std::string& s, char delimiter) {
    std::vector < std::string> tokens;
    std::istringstream tokenStream(s);
    std::string token;
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
};

std::string Trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, (last - first + 1));
}


// ȥ�����ܵ�����  ��'a'
char ParseCharLiteral(const std::string& charValue) {
    std::string value = Trim(charValue);
    if (value.size() >= 2 && value.front() == '\'' && value.back() == '\'') {
        value = value.substr(1, value.size() - 2);
    }
    if (value.size() != 1) {
        throw std::runtime_error("Char must be exactly 1 character: " + charValue);
    }
    return value[0];
}

// ѹ�������հ׷�Ϊ�����ո�
std::string CompressWhitespace(const std::string& str) {
    std::string result;
    bool lastWasSpace = false;

    for (char c : str) {
        if (std::isspace(static_cast<unsigned char>(c))) {
            if (!lastWasSpace) {
                result += ' ';
                lastWasSpace = true;
            }
        }
        else {
            result += c;
            lastWasSpace = false;
        }
    }
    return Trim(result);
}

// �Ƴ�ע�� (��ʵ��)
std::string RemoveComments(const std::string& str) {
    std::string result;
    bool inSingleLineComment = false;
    bool inMultiLineComment = false;

    for (size_t i = 0; i < str.size(); i++) {
        if (inSingleLineComment) {
            if (str[i] == '\n') {
                inSingleLineComment = false;
                result += str[i];
            }
            continue;
        }
        if (inMultiLineComment) {
            if (str[i] == '*' && i + 1 < str.size() && str[i + 1] == '/') {
                inMultiLineComment = false;
                i++; // ���� '/'
            }
            continue;
        }

        if (str[i] == '/' && i + 1 < str.size()) {
            if (str[i + 1] == '/') {
                inSingleLineComment = true;
                i++; // �����ڶ��� '/'
                continue;
            }
            if (str[i + 1] == '*') {
                inMultiLineComment = true;
                i++; // ���� '*'
                continue;
            }
        }

        result += str[i];
    }

    return result;
}

// ���ļ�����ȡ������Ϣ
std::map<std::string, FuncInfo> ExtractFunctions(const std::string& filePath) {
    std::map<std::string, FuncInfo> funcMap;
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filePath << std::endl;
        return funcMap;
    }

    // ��ȡ�����ļ����ݲ��Ƴ�ע��
    std::ostringstream oss;
    oss << file.rdbuf();
    std::string content = RemoveComments(oss.str());
    file.close();

    size_t pos = 0;
    while (pos < content.size()) {
        // �����հ��ַ�
        while (pos < content.size() && std::isspace(static_cast<unsigned char>(content[pos])))
            pos++;

        if (pos >= content.size()) break;

        // ���Һ�����������: <return-type> <name>(<args>) {
        // 1. �ҵ���������Ϊ������ʼ��
        size_t parenStart = content.find('(', pos);
        if (parenStart == std::string::npos) break;

        // 2. ������ҵ���������ʼ
        size_t nameEnd = parenStart;
        while (nameEnd > 0 &&
            (std::isspace(static_cast<unsigned char>(content[nameEnd - 1])) ||
                content[nameEnd - 1] == '*' ||
                content[nameEnd - 1] == '&'))
            nameEnd--;

        size_t nameStart = nameEnd;
        while (nameStart > 0 &&
            !std::isspace(static_cast<unsigned char>(content[nameStart - 1])) &&
            content[nameStart - 1] != '*' &&
            content[nameStart - 1] != '&' &&
            content[nameStart - 1] != '(')
            nameStart--;

        if (nameStart >= nameEnd) {
            pos = parenStart + 1;
            continue;
        }

        std::string funcName = content.substr(nameStart, nameEnd - nameStart);
        funcName = Trim(funcName);

        // 3. �Ӻ�������ǰ�ҷ�������
        size_t returnTypeStart = nameStart;
        while (returnTypeStart > 0 &&
            (std::isspace(static_cast<unsigned char>(content[returnTypeStart - 1])) ||
                content[returnTypeStart - 1] == '*' ||
                content[returnTypeStart - 1] == '&' ||
                std::isalnum(static_cast<unsigned char>(content[returnTypeStart - 1]))))
            returnTypeStart--;

        std::string returnType = content.substr(returnTypeStart, nameStart - returnTypeStart);
        returnType = Trim(returnType);

        // ����ָ������÷������� (ȷ������*��&����)
        if (returnType.find('*') != std::string::npos || returnType.find('&') != std::string::npos) {
            // ȷ������������������
            size_t posStar = returnType.find('*');
            size_t posAmp = returnType.find('&');
            size_t symbolPos = (posStar != std::string::npos) ? posStar : posAmp;

            if (symbolPos > 0 && std::isspace(static_cast<unsigned char>(returnType[symbolPos - 1]))) {
                // �Ƴ�����ǰ�Ŀո�
                returnType = Trim(returnType.substr(0, symbolPos)) +
                    returnType.substr(symbolPos);
            }
        }

        // 4. ��ȡ�����б�
        size_t paramEnd = parenStart + 1;
        int parenCount = 1;
        while (paramEnd < content.size() && parenCount > 0) {
            if (content[paramEnd] == '(') parenCount++;
            else if (content[paramEnd] == ')') parenCount--;
            paramEnd++;
        }

        if (parenCount != 0) {
            pos = paramEnd;
            continue;
        }

        std::string formalArgs = content.substr(parenStart + 1, paramEnd - parenStart - 2);
        formalArgs = Trim(formalArgs);

        // 5. ���Һ�������ʼλ��
        size_t bodyStart = paramEnd;
        while (bodyStart < content.size() && content[bodyStart] != '{')
            bodyStart++;

        if (bodyStart >= content.size()) {
            pos = paramEnd;
            continue;
        }

        // 6. ��ȡ������
        size_t bodyEnd = bodyStart + 1;
        int braceCount = 1;
        while (bodyEnd < content.size() && braceCount > 0) {
            if (content[bodyEnd] == '{') braceCount++;
            else if (content[bodyEnd] == '}') braceCount--;
            bodyEnd++;
        }

        if (braceCount != 0) {
            pos = bodyStart + 1;
            continue;
        }

        std::string funcBody = content.substr(bodyStart + 1, bodyEnd - bodyStart - 2);
        funcBody = CompressWhitespace(funcBody);

        // ����map
        if (!funcName.empty() && !returnType.empty()) {
            FuncInfo info{ returnType, funcName, formalArgs, funcBody };
            funcMap[funcName] = info;
        }

        // �ƶ������������λ��
        pos = bodyEnd;
    }

    return funcMap;
}