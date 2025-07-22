#pragma once
#ifndef __FILETOOLS__
#define __FILETOOLS__

#include <string>
#include <map>
#include <sstream>
#include <vector>

std::vector<std::string> Split(const std::string& s, char delimiter);
std::string Trim(const std::string& str);
char ParseCharLiteral(const std::string& charValue);   // ȥ�����ܵ�����  ��'a'


// �洢������Ϣ�Ľṹ��
struct FuncInfo {
    std::string returnType;
    std::string funcName;
    std::string formalArgs;
    std::string funcStmt;
};

// ������������
std::string CompressWhitespace(const std::string& str);
std::map<std::string, FuncInfo> ExtractFunctions(const std::string& filePath);


#endif // !__FILETOOLS_
