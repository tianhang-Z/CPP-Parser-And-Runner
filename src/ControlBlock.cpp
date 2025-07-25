#include <ControlBlock.h>
#include <FileTools.h>
#include <regex>
namespace thz{


    // ��������  ���������ж�
    char GetCompareMethod(const std::string& compareStr) {
        if (compareStr.find(">=") != std::string::npos) return 'g'; // >=
        if (compareStr.find("<=") != std::string::npos) return 'l'; // <=
        if (compareStr.find("==") != std::string::npos) return 'e'; // ==

        if (compareStr.find('<') != std::string::npos) return '<';
        if (compareStr.find('>') != std::string::npos) return '>';

        throw std::runtime_error("Only support <, >, <=, >=, ==");
    }


    double GetStrValue(std::string str, Block* block) {
        if (isalpha(str[0])) {
            auto leftVar = block->find_var(str);
            if (leftVar == nullptr)
                throw std::runtime_error("can not find left var");
            return GetDoubleValueByVar(leftVar);
        }
        else if (isdigit(str[0]))
            return std::stod(str);
        else
            throw std::runtime_error("can not get value from str");
    }

    bool CompareForNum(int leftVal, int rightVal, char compareMethod) {
        switch (compareMethod) {
        case '<': return leftVal < rightVal;
        case '>': return leftVal > rightVal;
        case 'e': return leftVal == rightVal;
        case 'l': // �� 'l' ��ʾ <= (less or equal)
            return leftVal <= rightVal;
        case 'g': // �� 'g' ��ʾ >= (greater or equal)
            return leftVal >= rightVal;
        default:
            throw std::invalid_argument("Invalid comparison method. Supported: '<', '>', '=', 'l', 'g'");
        }
    }

    // ֧��a<b  �� bool ����
    bool Compare(const std::string& compareStr,Block* block) {
        char compareMethod = GetCompareMethod(compareStr);
        size_t opBegin = compareStr.find_first_of("<>=");
        if (opBegin == std::string::npos) {
            if (compareStr == "true") return true;
            else if (compareStr == "false") return false;
            // ��ʱblock��Ϊ����
            auto boolVar = block->find_var(compareStr);
            if (boolVar == nullptr || boolVar->get_type() != VarType::Bool);
                throw std::runtime_error("can not find var");
            
        }

        std::string leftStr = Trim(compareStr.substr(0, opBegin));

        size_t rigthPartBegin = 0;
        if (compareStr.substr(opBegin, 2) == "<=" || compareStr.substr(opBegin, 2) == ">=" || compareStr.substr(opBegin, 2) == "==") {
            rigthPartBegin = opBegin + 2;
        }
        else {
            rigthPartBegin = opBegin + 1;
        }
        std::string rightStr = Trim(compareStr.substr(rigthPartBegin));

        double leftValue = GetStrValue(leftStr,block);
        double rightValue = GetStrValue(rightStr,block);

        return CompareForNum(leftValue, rightValue, compareMethod);

    }


    // ǰ�úͺ���++ --
    std::string ExtractVariableName(const std::string& expr) {
        std::regex pattern(R"((\+\+|--)?(\w+)(\+\+|--)?)"); // ƥ��ǰ��׺
        std::smatch matches;
        if (std::regex_match(expr, matches, pattern)) {
            return matches[2].str(); 
        }
        else
            throw std::runtime_error("can not catch var");
        return ""; 
    }

    void stepCnt(const std::string& stepMethod,Block* block) {
        std::string cntVarStr = ExtractVariableName(stepMethod);
        auto cntVar = block->find_var(cntVarStr);
        if (cntVar == nullptr)
            throw std::runtime_error("do not decl cnt var");
        if (stepMethod.find("++") != std::string::npos) SetBasicVar(cntVar, GetDoubleValueByVar(cntVar) + 1);
        else if (stepMethod.find("--") != std::string::npos) SetBasicVar(cntVar, GetDoubleValueByVar(cntVar) - 1);
        else throw std::runtime_error("only suppprt ++ --");
    }


    void LoopBlock::parse_loop_for() {
        size_t parenthesesBegin = m_blockBody.find('(');
        size_t parenthesesEnd = m_blockBody.find(')');
        std::string loopCondition = m_blockBody.substr(parenthesesBegin + 1, parenthesesEnd - parenthesesBegin - 1);
        size_t bracesBegin = m_blockBody.find_first_of('{');
        size_t bracesEnd = m_blockBody.find_last_of('}');
        std::string loopBody = m_blockBody.substr(bracesBegin + 1, bracesEnd - bracesBegin-1 );

        std::vector<std::string> loopConditionTokens = Split(loopCondition, ';');
        if (loopConditionTokens.size() != 3) {
            throw std::runtime_error("Invalid for loop syntax");
        }

        // ֧��for(;i<10;)���ȱʡ
        std::string cntDecl = Trim(loopConditionTokens[0]);
        if(cntDecl.size()!=0)
            parse_variable_declaration(cntDecl);

        std::string compareStr = Trim(loopConditionTokens[1]);
        std::string stepMethod = Trim(loopConditionTokens[2]);


        std::string cntVarStr = "";
        if(stepMethod.size()!=0)
            cntVarStr=ExtractVariableName(stepMethod);
        for (; Compare(compareStr,this); ) {
            auto cntVarIter = m_varMap.find(cntVarStr);
            std::shared_ptr<VarBase> cntVar = nullptr;
            if (cntVarIter != m_varMap.end())
                cntVar = cntVarIter->second;
            m_varMap.clear();   // for loop�еĶ���ı���ÿ�ν���for��ʱ�����¶���
            if (cntVar != nullptr)
                m_varMap[cntVarStr] = cntVar;


            parse_block_body(loopBody);

            if(stepMethod.size()!=0)
                stepCnt(stepMethod, this);
        }
    }


    void IfBlock::parse_if_block() {
        std::vector<std::string> conditionVec;
        std::vector<std::string> branchVec;
        
        size_t curBranchPos = 0;
        size_t braceEndPos = 0;
        while ((braceEndPos = m_blockBody.find('}', curBranchPos))!= std::string::npos) {
            size_t braceBeginPos = m_blockBody.find_first_of('{', curBranchPos);
            std::string curBranch = m_blockBody.substr(braceBeginPos + 1, braceEndPos - braceBeginPos - 1);
            branchVec.push_back(curBranch);

            size_t parenthesesBeginPos = m_blockBody.find_first_of('(', curBranchPos);
            size_t parenthesesEndPos = m_blockBody.find_first_of(')', curBranchPos);
            if (parenthesesBeginPos != std::string::npos && parenthesesEndPos != std::string::npos) {
                std::string condition = m_blockBody.substr(parenthesesBeginPos + 1, parenthesesEndPos = parenthesesBeginPos - 1);
                conditionVec.push_back(condition);
            }
            else {
                conditionVec.push_back("");
            }

            curBranchPos = braceEndPos + 1;
        }

        

    }
}
