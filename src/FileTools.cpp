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


// 去除可能的引号  如'a'
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

// 压缩连续空白符为单个空格
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

// 移除注释 (简单实现)
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
                i++; // 跳过 '/'
            }
            continue;
        }

        if (str[i] == '/' && i + 1 < str.size()) {
            if (str[i + 1] == '/') {
                inSingleLineComment = true;
                i++; // 跳过第二个 '/'
                continue;
            }
            if (str[i + 1] == '*') {
                inMultiLineComment = true;
                i++; // 跳过 '*'
                continue;
            }
        }

        result += str[i];
    }

    return result;
}

bool IsAccessSpecifier(const std::string& str, size_t pos) {
    return str.compare(pos, 6, "public") == 0 ||
        str.compare(pos, 7, "private") == 0 ||
        str.compare(pos, 9, "protected") == 0;
}

void SkipAccessSpecifier(const std::string& str, size_t& pos) {
    while (pos < str.size() && str[pos] != ':')
        pos++;
    if (pos < str.size()) pos++;
}

std::string ExtractIdentifier(const std::string& str, size_t start, size_t end) {
    size_t nameEnd = end;
    while (nameEnd > start &&
        (std::isspace(str[nameEnd - 1]) ||
            str[nameEnd - 1] == '*' ||
            str[nameEnd - 1] == '&'))
        nameEnd--;

    size_t nameStart = nameEnd;
    while (nameStart > start &&
        !std::isspace(str[nameStart - 1]) &&
        str[nameStart - 1] != '*' &&
        str[nameStart - 1] != '&' &&
        str[nameStart - 1] != '(')
        nameStart--;

    if (nameStart >= nameEnd) return "";
    return Trim(str.substr(nameStart, nameEnd - nameStart));
}

std::string ExtractReturnType(const std::string& str, size_t start, size_t nameStart) {
    size_t returnTypeStart = nameStart;
    while (returnTypeStart > start &&
        (std::isspace(str[returnTypeStart - 1]) ||
            str[returnTypeStart - 1] == '*' ||
            str[returnTypeStart - 1] == '&' ||
            std::isalnum(str[returnTypeStart - 1])))
        returnTypeStart--;

    return Trim(str.substr(returnTypeStart, nameStart - returnTypeStart));
}

std::string ExtractParameters(const std::string& str, size_t parenStart, size_t& pos) {
    size_t paramEnd = parenStart + 1;
    int parenCount = 1;
    while (paramEnd < str.size() && parenCount > 0) {
        if (str[paramEnd] == '(') parenCount++;
        else if (str[paramEnd] == ')') parenCount--;
        paramEnd++;
    }

    if (parenCount != 0) {
        pos = paramEnd;
        return "";
    }

    pos = paramEnd;
    return Trim(str.substr(parenStart + 1, paramEnd - parenStart - 2));
}

std::string ExtractFunctionBody(const std::string& str, size_t bodyStart, size_t& pos) {
    int braceCount = 1;
    size_t bodyEnd = bodyStart + 1;
    while (bodyEnd < str.size() && braceCount > 0) {
        if (str[bodyEnd] == '{') braceCount++;
        else if (str[bodyEnd] == '}') braceCount--;
        bodyEnd++;
    }

    if (braceCount != 0) {
        pos = bodyStart + 1;
        return "";
    }

    pos = bodyEnd;
    return CompressWhitespace(str.substr(bodyStart + 1, bodyEnd - bodyStart - 2));
}

std::vector<std::string> ExtractVariableDeclarations(const std::string& classBody) {
    std::vector<std::string> variables;
    size_t pos = 0;

    while (pos < classBody.size()) {
        // 跳过空白字符
        while (pos < classBody.size() && std::isspace(classBody[pos]))
            pos++;

        if (pos >= classBody.size()) break;

        // 跳过访问修饰符
        if (IsAccessSpecifier(classBody, pos)) {
            SkipAccessSpecifier(classBody, pos);
            continue;
        }

        // 跳过函数定义
        size_t parenPos = classBody.find('(', pos);
        size_t semicolonPos = classBody.find(';', pos);

        if (parenPos != std::string::npos && parenPos < semicolonPos) {
            // 这是函数定义，跳过
            size_t bodyStart = classBody.find('{', parenPos);
            if (bodyStart != std::string::npos) {
                int braceCount = 1;
                pos = bodyStart + 1;
                while (pos < classBody.size() && braceCount > 0) {
                    if (classBody[pos] == '{') braceCount++;
                    else if (classBody[pos] == '}') braceCount--;
                    pos++;
                }
            }
            else {
                pos = semicolonPos + 1;
            }
            continue;
        }

        // 处理变量声明
        if (semicolonPos != std::string::npos) {
            std::string varDecl = Trim(classBody.substr(pos, semicolonPos - pos));
            if (!varDecl.empty()) {
                // 处理可能的多个变量声明，如: int a, b, c;
                size_t commaPos = varDecl.find(',');
                if (commaPos != std::string::npos) {
                    std::string baseType = Trim(varDecl.substr(0, varDecl.find_last_of(" \t\n\r", commaPos)));
                    std::vector<std::string> varNames = Split(varDecl.substr(commaPos + 1), ',');

                    variables.push_back(baseType + " " + Trim(varDecl.substr(0, commaPos)));
                    for (const auto& name : varNames) {
                        variables.push_back(baseType + " " + Trim(name));
                    }
                }
                else {
                    variables.push_back(varDecl);
                }
            }
            pos = semicolonPos + 1;
        }
        else {
            break;
        }
    }

    return variables;
}


// 类信息提取
ClassInfo ExtractClassInfo(const std::string& className, const std::string& classBody) {
    ClassInfo classInfo;
    classInfo.className = className;
    // 先提取所有变量声明
    classInfo.varInfo = ExtractVariableDeclarations(classBody);

    size_t bodyPos = 0;
    while (bodyPos < classBody.size()) {
        // 跳过空白字符
        while (bodyPos < classBody.size() && std::isspace(classBody[bodyPos]))
            bodyPos++;

        if (bodyPos >= classBody.size()) break;

        // 处理访问修饰符
        if (IsAccessSpecifier(classBody, bodyPos)) {
            SkipAccessSpecifier(classBody, bodyPos);
            continue;
        }

        // 检查是否是函数
        size_t parenPos = classBody.find('(', bodyPos);
        if (parenPos != std::string::npos && parenPos < classBody.size()) {
            // 提取函数信息
            std::string funcName = ExtractIdentifier(classBody, bodyPos, parenPos);
            if (!funcName.empty()) {
                std::string returnType = ExtractReturnType(classBody, bodyPos, parenPos - funcName.size());
                std::string formalArgs = ExtractParameters(classBody, parenPos, bodyPos);

                // 检查是否有函数体
                size_t bodyStart = classBody.find('{', bodyPos);
                if (bodyStart != std::string::npos) {
                    std::string funcBody = ExtractFunctionBody(classBody, bodyStart, bodyPos);
                    if (!funcBody.empty()) {
                        FuncInfo funcInfo{ returnType, funcName, formalArgs, funcBody };
                        classInfo.funcMap[funcName] = funcInfo;
                    }
                }
            }
        }
        else {
            // 成员变量处理
            size_t semicolonPos = classBody.find(';', bodyPos);
            if (semicolonPos == std::string::npos) break;

            std::string varDecl = Trim(classBody.substr(bodyPos, semicolonPos - bodyPos));
            if (!varDecl.empty()) {
                classInfo.varInfo.push_back(varDecl);
            }
            bodyPos = semicolonPos + 1;
        }
    }

    return classInfo;
}



// 文件信息提取
FileInfo ExtractFileInfo(const std::string& filePath) {
    FileInfo fileInfo;
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filePath << std::endl;
        return fileInfo;
    }

    // 读取文件内容并移除注释
    std::ostringstream oss;
    oss << file.rdbuf();
    std::string content = RemoveComments(oss.str());
    file.close();

    size_t pos = 0;
    while (pos < content.size()) {
        // 跳过空白字符
        while (pos < content.size() && std::isspace(content[pos]))
            pos++;

        if (pos >= content.size()) break;

        // 处理类定义
        if (content.compare(pos, 6, "class ") == 0) {
            pos += 6;
            // 提取类名
            size_t classNameStart = pos;
            while (pos < content.size() && !std::isspace(content[pos]) &&
                content[pos] != '{' && content[pos] != ':')
                pos++;

            if (pos >= content.size()) break;

            std::string className = content.substr(classNameStart, pos - classNameStart);
            className = Trim(className);

            // 跳过继承部分
            while (pos < content.size() && content[pos] != '{')
                pos++;

            if (pos >= content.size()) break;

            // 提取类体
            size_t classBodyStart = pos + 1;
            int braceCount = 1;
            pos++;
            while (pos < content.size() && braceCount > 0) {
                if (content[pos] == '{') braceCount++;
                else if (content[pos] == '}') braceCount--;
                pos++;
            }

            if (braceCount != 0) continue;

            std::string classBody = content.substr(classBodyStart, pos - classBodyStart - 1);
            fileInfo.allClassInfo[className] = ExtractClassInfo(className, classBody);
            continue;
        }

        // 处理普通函数
        size_t parenStart = content.find('(', pos);
        if (parenStart == std::string::npos || parenStart >= content.size()) {
            pos++;
            continue;
        }

        // 提取函数信息
        std::string funcName = ExtractIdentifier(content, pos, parenStart);
        if (!funcName.empty()) {
            std::string returnType = ExtractReturnType(content, pos, parenStart - funcName.size());
            std::string formalArgs = ExtractParameters(content, parenStart, pos);

            // 查找函数体
            size_t bodyStart = content.find('{', pos);
            if (bodyStart != std::string::npos) {
                std::string funcBody = ExtractFunctionBody(content, bodyStart, pos);
                if (!funcBody.empty()) {
                    FuncInfo funcInfo{ returnType, funcName, formalArgs, funcBody };
                    fileInfo.allFuncInfo[funcName] = funcInfo;
                }
            }
        }
        else {
            pos++;
        }
    }

    return fileInfo;
}