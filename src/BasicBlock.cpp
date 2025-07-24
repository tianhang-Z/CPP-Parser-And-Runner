#include <BasicBlock.h>
#include <ControlBlock.h>
#include <FileTools.h>

namespace thz {


    bool IsVarDeclStmt(const std::string& stmt) {
        for (const std::string& varType : VgTypeMap) {
            if (stmt.find(varType) == 0) {
                return true;
            }
        }
        return false;
    }

    bool IsAssignStmt(const std::string& stmt) {
        if (stmt.find('=') != std::string::npos) return true;
        else return false;
    }
    bool IsReturnStmt(const std::string& stmt) {
        if (stmt.find("return") == 0) return true;
        else return false;
    };

    bool IsFunCall(const std::string& stmt) {
        // ���ַ���ֱ�ӷ���false
        if (stmt.empty())
            return false;

        size_t idx = 0;
        // ��麯��������������ĸ���»��߿�ͷ
        if (!std::isalpha(stmt[idx]) && stmt[idx] != '_')
            return false;
        idx++;
        // ��ȡ��������������ĸ�����֡��»��ߣ�
        while (idx < stmt.size() && (std::isalnum(stmt[idx]) || stmt[idx] == '_')) {
            idx++;
        }
        // ������������Ŀո�
        while (idx < stmt.size() && std::isspace(stmt[idx])) {
            idx++;
        }
        // ��ʱ������������ '('
        if (idx >= stmt.size() || stmt[idx] != '(')
            return false;
        idx++; // ���� '('
        int parenCount = 1;  // ���ż���������ʼΪ1������һ�������ţ�
        bool inString = false; // �Ƿ����ַ����������ڣ���ֹ���ű����У�

        // ����ʣ���ַ�����������ƥ��
        while (idx < stmt.size() && parenCount > 0) {
            char c = stmt[idx];
            if (c == '"') {
                inString = !inString;  // �л��ַ���״̬
            }
            else if (!inString) {
                if (c == '(') parenCount++;     // ���������ţ���������
                else if (c == ')') parenCount--; // ���������ţ���������
            }
            idx++;
        }
        // ��飺����δƥ���� �� ����λ�ò���ĩβ
        if (parenCount != 0 || idx != stmt.size())
            return false;

        return true; // �����������㣬�ǵ����ĺ�������
    }

    size_t FindMatchingBrace(const std::string& blockBody, size_t startPos) {
        int level = 1;  // ��ʼ�㼶Ϊ1���Ѿ������˵�һ��'{'��
        for (size_t pos = startPos; pos < blockBody.length(); ++pos) {
            char c = blockBody[pos];
            if (c == '{') level++;
            else if (c == '}') {
                if (--level == 0) return pos;  // �ҵ�ƥ���������
            }
            // �����ַ��������������⽫�ַ����ڵ�"}"����Ϊ������
            else if (c == '"') {
                do { pos++; } while (pos < blockBody.length() && blockBody[pos] != '"');
            }
        }
        return std::string::npos;  // δ�ҵ�ƥ��
    }

    bool IsControlKeyword(const std::string& blockBody, size_t pos) {
        static const std::vector<std::string> keywords = { "for", "else", "if", "while", "do" };
        for (const auto& kw : keywords) {
            if (blockBody.compare(pos, kw.length(), kw) == 0) {
                // ���ؼ��ֺ��Ƿ���ո�/����  
                size_t next_pos = pos + kw.length();
                if (isspace(blockBody[next_pos]) ||
                    blockBody[next_pos] == '(' ||
                    blockBody[next_pos] == '{') {
                    return true;
                }
            }
        }
        return false;
    }

    void SplitBody(std::vector<std::string>& stmts, const std::string& blockBody) {
        std::stack<size_t> controlStack; // �洢���ƽṹ����ʼλ��
        size_t pos = 0;
        size_t len = blockBody.length();

        while (pos < len) {

            while (pos < len && isspace(blockBody[pos])) pos++;
            if (pos >= len) break;

            if (IsControlKeyword(blockBody, pos)) {
                size_t block_start = blockBody.find('{',pos);
                if (block_start == std::string::npos) {
                    throw std::runtime_error(" control block must have { }");
                }
                size_t block_end = FindMatchingBrace(blockBody, block_start + 1);
                std::string stmt = blockBody.substr(pos, block_end - pos + 1);
                stmts.push_back(stmt);
                pos = block_end + 1;
                LOG_DEBUG("split result: %s", stmt.c_str());
            }
            else {
                size_t end = blockBody.find(';', pos);
                if (end == std::string::npos) end = len;
                std::string stmt = blockBody.substr(pos, end - pos);
                if (!stmt.empty()) stmts.push_back(stmt);
                pos = end + 1;
                LOG_DEBUG("split result: %s", stmt.c_str());

            }
        }
    }

    // ���ϲ���block ���ǲ�����纯�����ң���parentΪfuncblockʱ��ֻ��һ��
    std::shared_ptr<VarBase> Block::find_var(std::string varName) {
        Block* curBlock = this;
        VarMap* curMap = &m_varMap;
        auto it = curMap->find(varName);
        while (it == curMap->end()) {
            if (curBlock->m_parentBlock != nullptr ) {
                curBlock = curBlock->m_parentBlock;
                curMap = &(curBlock->m_varMap);
                it = curMap->find(varName);
                if (curBlock->m_type == BlockType::FunBlock) {
                    if (it == curMap->end()) return nullptr;
                    else return it->second;
                }
            }
            else
                return nullptr; // û�����var
        }
            
        return it->second;
    }

    // ֧�����ϲ��Ҹ�block �ҵ�������block
    std::shared_ptr<VarBase> Block::get_return_var() {
        Block* searchBlock = m_parentBlock;
        while (searchBlock != nullptr) {
            if (searchBlock->get_block_type() != BlockType::FunBlock)
                searchBlock = searchBlock->m_parentBlock;
            else
                return searchBlock->get_return_var();
        }
        return nullptr;
    };


    void Block::parse_block_body(const std::string& blockBody) {
        std::vector<std::string> statements;
        SplitBody(statements, blockBody);

        for (int i = 0; i < statements.size(); i++) {
            std::string stmt = statements[i];
            std::string trimmedStmt = Trim(stmt);
            if (trimmedStmt.empty()) continue;
            if (stmt.find("if") == 0) {
                while (i + 1 < statements.size()) {
                    std::string nextStmt = Trim(statements[i + 1]);
                    if (nextStmt.find("else if") != std::string::npos || nextStmt.find("else") != std::string::npos) {
                        trimmedStmt += nextStmt;
                        i++;
                    }
                    else
                        break;
                }
            }
            parse_statement(trimmedStmt);
        }
    }

    // ���if else for�ȵĴ���
    void Block::parse_statement(const std::string& stmt) {
        if (stmt.find("for") == 0) {
            LoopBlock(stmt,this).run_loop();
        }
        else if (stmt.find("if") == 0) {

        }
        else if (IsVarDeclStmt(stmt))
            parse_variable_declaration(stmt);
        else if (IsAssignStmt(stmt))
            parse_assignment(stmt);
        else if (IsReturnStmt(stmt))
            parse_return_statement(stmt);
        else if (IsFunCall(stmt))
            m_calc.evaluate_funCall(stmt, this);
        else
            throw std::runtime_error("can not match stmt");
    }



    void Block::parse_variable_declaration(const std::string& stmt) {
        std::vector<std::string> parts = Split(stmt, ' ');
        if (parts.size() < 2) {
            throw std::runtime_error("Invalid variable declaration: " + stmt);
        }

        std::string typeStr = parts[0];
        size_t nameEnd = parts[1].find_first_of(" ;=");
        std::string name = (nameEnd == std::string::npos) ?
            parts[1] : parts[1].substr(0, nameEnd);

        if (m_varMap.find(name) != m_varMap.end()) {
            throw std::runtime_error("Variable redeclaration: " + name);
        }

        size_t equalPos = stmt.find('=');
        std::string valuePart;
        if (equalPos != std::string::npos) {
            valuePart = Trim(stmt.substr(equalPos + 1));
            if (!valuePart.empty() && valuePart.back() == ';') {
                valuePart.pop_back();
            }
        }

        VarType type = str2Type(typeStr);
        bool createRef = IsRef(type);
        bool createPtr = IsPtr(type);

        std::shared_ptr<VarBase> var;
        if (!valuePart.empty()) {
            // ����int& a=b���; ���ñ�������ʱ��ʼ�� �﷨��Ϊ�̶�
            // ����int* a=&b;  int* a=a; 
            // ����int* a= doNothing(b);
            if (createPtr || createRef) {
                if (IsFunCall(valuePart)) {
                    // int* a=fun(b)     int& a=fun(b)
                    std::shared_ptr<VarBase> retVar = create_var_by_funcall_ret(valuePart);
                    bool returnPtr = IsPtr(retVar->get_type());
                    bool returnRef = IsRef(retVar->get_type());
                    var = CreateVarByVar(type, name, retVar, createPtr, createRef);
                }
                else
                    var = create_var_by_self_var(type, name, valuePart);
            } // ����char c = 'A';
            else if (type == VarType::Char && valuePart.size() >= 3 &&
                valuePart[0] == '\'' && valuePart.back() == '\'') {
                char c = valuePart[1];
                var = std::make_shared<VarChar>(name, c);
            } // �������Ӽ������  ���ؼ����� 
            else {
                // ���� int a=fun(b)
                if (IsFunCall(valuePart)) {
                    std::shared_ptr<VarBase> ret = create_var_by_funcall_ret(valuePart);
                    var = CreateVariable(type, name, ret);
                }
                else {
                    double result = m_calc.evaluate_expression(valuePart, this);
                    var = CreateVariable(type, name, result);
                }
            }
        }
        else {
            if (IsPtr(type)) // ����int* a;
                var = CreateVariablePtr(type, name, nullptr);
            else  //���� int a;
                var = CreateVariable(type, name);
        }

        m_varMap[name] = std::move(var);
    }

    void Block::do_assignment(std::shared_ptr<VarBase>& leftVar, std::string& rightExpr, bool leftDeref) {
        VarType leftType = leftVar->get_type();

        // ���� (int)a=sum(arg1,arg2)   (int*) a=sum(arg1,arg2)   (int&) a=sum(arg1,arg2)
        if (IsFunCall(rightExpr)) {
            std::shared_ptr<VarBase> ret = create_var_by_funcall_ret(rightExpr);
            VarType retType = ret->get_type();
            if (IsPtr(retType)) {
                SharePtr(leftVar, ret);  // �������ָ������� , ��������
            }
            else if (IsRef(retType)) {
                ShareRef(leftVar, ret);
            }
            else {
                SetBasicVar(leftVar, GetDoubleValueByVar(ret));
            }
        }
        // ��������
        else if (IsRef(leftType)) {
            // ��leftType�Ƿ���ֵʱ ��Ҫ���⴦��
            // int& donothing(int& a) {return a;}
            if (leftVar->get_name() == "return_var") {
                auto rightVar = find_var(rightExpr);
                if (rightVar == nullptr)
                    throw std::runtime_error("can not get rightVar");
                ShareRef(leftVar, rightVar);
            }
            else {
                double result = m_calc.evaluate_expression(rightExpr, this);
                SetBasicVar(leftVar, result);
            }
        }
        // ����*p1=a+3;
        else if (leftDeref && IsPtr(leftType)) {
            double result = m_calc.evaluate_expression(rightExpr, this);
            SetBasicVar(leftVar, result);
        }
        // ����p1=p2  (p1,p2����ָ�룩�� p1=&a
        else if (IsPtr(leftType) && !leftDeref) {
            if (rightExpr[0] == '&') {
                std::string rightVarStr = rightExpr.substr(1);
                auto rightVar = find_var(rightVarStr);
                if (rightVar == nullptr)
                    throw std::runtime_error("Undefined right variable: " + leftVar->get_name());
                RePtr(leftVar, rightVar);
            }
            else {
                auto rightVar = find_var(rightExpr);
                if (rightVar == nullptr)
                    throw std::runtime_error("Undefined right variable: " + leftVar->get_name());
                if (!IsPtr(rightVar->get_type()))
                    throw std::runtime_error("rigth var not a pointer");
                SharePtr(leftVar, rightVar);
            }
        } //��������� a=b;
        else {
            double result = m_calc.evaluate_expression(rightExpr, this);
            SetBasicVar(leftVar, result);
        }
    }

    // ��ֵ������
    void Block::parse_assignment(const std::string& stmt) {
        size_t equalPos = stmt.find('=');
        if (equalPos == std::string::npos) {
            throw std::runtime_error("Invalid assignment: " + stmt);
        }

        std::string varName = Trim(stmt.substr(0, equalPos));
        std::string exprStr = Trim(stmt.substr(equalPos + 1));
        bool varDeRef = false;
        if (varName[0] == '*') {
            varDeRef = true;
            varName = varName.substr(1);
        }

        auto leftVar = find_var(varName);
        if (leftVar == nullptr) {
            throw std::runtime_error("Undefined variable: " + varName);
        }

        do_assignment(leftVar, exprStr, varDeRef);
    }



    // ����������
    void Block::parse_return_statement(const std::string& stmt) {
        size_t returnPos = stmt.find("return");
        if (returnPos != 0) {
            throw std::runtime_error("Invalid return statement: " + stmt);
        }

        std::string exprStr = Trim(stmt.substr(6));
        if (!exprStr.empty() && exprStr.back() == ';') {
            exprStr.pop_back();
        }

        if (exprStr.empty()) {
            return;
        }

        std::shared_ptr<VarBase> retVar = this->get_return_var();
        VarType retType = retVar->get_type();

        do_assignment(retVar, exprStr, false);
    }


    // �Ӻ������÷���ֵ��������
    std::shared_ptr<VarBase> Block::create_var_by_funcall_ret(std::string funcExpr) {
        std::shared_ptr<VarBase> ret = m_calc.evaluate_funCall(funcExpr, this);
        return ret;
    }

    // ����
    std::shared_ptr<VarBase> Block::create_args_by_parent_var(VarType type, std::string name, std::string argValue) {
        std::shared_ptr<VarBase> var = CreateVarByBlockVarMap(type, name, argValue, m_parentBlock);
        return var;
    }

    // �����б�������
    std::shared_ptr<VarBase> Block::create_var_by_self_var(VarType type, std::string name, std::string argValue) {
        std::shared_ptr<VarBase> var = CreateVarByBlockVarMap(type, name, argValue, this);
        return var;
    }

}
