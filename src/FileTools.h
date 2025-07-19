#pragma once
#ifndef __FILETOOLS__
#define __FILETOOLS__

#include <string>
#include <map>
#include <sstream>
#include <vector>

std::vector<std::string> split(const std::string& s, char delimiter);
std::string trim(const std::string& str);
char parseCharLiteral(const std::string& charValue);   // ȥ�����ܵ�����  ��'a'


// �洢������Ϣ�Ľṹ��
struct FuncInfo {
    std::string returnType;
    std::string funcName;
    std::string formalArgs;
    std::string funcStmt;
};

// ������������
std::string compress_whitespace(const std::string& str);
std::map<std::string, FuncInfo> extractFunctions(const std::string& filePath);


#endif // !__FILETOOLS_
