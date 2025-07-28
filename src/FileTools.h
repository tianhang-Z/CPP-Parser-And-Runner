#pragma once
#ifndef __FILETOOLS__
#define __FILETOOLS__

#include <string>
#include <map>
#include <sstream>
#include <vector>

std::vector<std::string> Split(const std::string& s, char delimiter);
std::string Trim(const std::string& str);
char ParseCharLiteral(const std::string& charValue);   // 去除可能的引号  如'a'


// 存储函数信息的结构体
struct FuncInfo {
    std::string returnType;
    std::string funcName;
    std::string formalArgs;
    std::string funcStmt;
};

inline std::ostream& operator<<(std::ostream& os, const FuncInfo& info) {
    os << info.returnType << " " << info.funcName << " (" << info.formalArgs << ")" << std::endl;
    os << "{" << std::endl << info.funcStmt << std::endl << "}" << std::endl;
    return os;
}

struct ClassInfo {
    std::string className;
    std::map<std::string, FuncInfo> funcMap;
    std::vector<std::string> varInfo;
};


inline std::ostream& operator<<(std::ostream& os, const ClassInfo& classInfo) {
    os << "class name" << classInfo.className << std::endl;
    os << "var decl ------------- " << std::endl;
    for (auto& var : classInfo.varInfo)
        os << var << std::endl;
    for (auto& memberFuncInfo : classInfo.funcMap) {
        os << memberFuncInfo.second;
    }
    return os;
}


struct FileInfo {
    std::map<std::string, FuncInfo> allFuncInfo;
    std::map<std::string, ClassInfo> allClassInfo;
};

// 辅助函数声明
std::string CompressWhitespace(const std::string& str);
//std::map<std::string, FuncInfo> ExtractFunctions(const std::string& filePath);
FileInfo ExtractFileInfo(const std::string& filePath);

#endif // !__FILETOOLS_
