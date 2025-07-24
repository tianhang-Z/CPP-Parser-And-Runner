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
        // 空字符串直接返回false
        if (stmt.empty())
            return false;

        size_t idx = 0;
        // 检查函数名：必须以字母或下划线开头
        if (!std::isalpha(stmt[idx]) && stmt[idx] != '_')
            return false;
        idx++;
        // 读取函数名（允许字母、数字、下划线）
        while (idx < stmt.size() && (std::isalnum(stmt[idx]) || stmt[idx] == '_')) {
            idx++;
        }
        // 跳过函数名后的空格
        while (idx < stmt.size() && std::isspace(stmt[idx])) {
            idx++;
        }
        // 此时必须有左括号 '('
        if (idx >= stmt.size() || stmt[idx] != '(')
            return false;
        idx++; // 跳过 '('
        int parenCount = 1;  // 括号计数器，初始为1（已有一个左括号）
        bool inString = false; // 是否在字符串字面量内（防止括号被误判）

        // 遍历剩余字符，进行括号匹配
        while (idx < stmt.size() && parenCount > 0) {
            char c = stmt[idx];
            if (c == '"') {
                inString = !inString;  // 切换字符串状态
            }
            else if (!inString) {
                if (c == '(') parenCount++;     // 遇到左括号，计数增加
                else if (c == ')') parenCount--; // 遇到右括号，计数减少
            }
            idx++;
        }
        // 检查：括号未匹配完 或 最终位置不在末尾
        if (parenCount != 0 || idx != stmt.size())
            return false;

        return true; // 所有条件满足，是单独的函数调用
    }

    size_t FindMatchingBrace(const std::string& blockBody, size_t startPos) {
        int level = 1;  // 初始层级为1（已经跳过了第一个'{'）
        for (size_t pos = startPos; pos < blockBody.length(); ++pos) {
            char c = blockBody[pos];
            if (c == '{') level++;
            else if (c == '}') {
                if (--level == 0) return pos;  // 找到匹配的右括号
            }
            // 跳过字符串字面量（避免将字符串内的"}"误判为结束）
            else if (c == '"') {
                do { pos++; } while (pos < blockBody.length() && blockBody[pos] != '"');
            }
        }
        return std::string::npos;  // 未找到匹配
    }

    bool IsControlKeyword(const std::string& blockBody, size_t pos) {
        static const std::vector<std::string> keywords = { "for", "else", "if", "while", "do" };
        for (const auto& kw : keywords) {
            if (blockBody.compare(pos, kw.length(), kw) == 0) {
                // 检查关键字后是否跟空格/括号  
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
        std::stack<size_t> controlStack; // 存储控制结构的起始位置
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

    // 向上查找block 但是不允许跨函数查找，当parent为funcblock时，只查一次
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
                return nullptr; // 没有这个var
        }
            
        return it->second;
    }

    // 支持向上查找父block 找到函数的block
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

    // 添加if else for等的处理
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
            // 解析int& a=b语句; 引用必须声明时初始化 语法较为固定
            // 解析int* a=&b;  int* a=a; 
            // 解析int* a= doNothing(b);
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
            } // 解析char c = 'A';
            else if (type == VarType::Char && valuePart.size() >= 3 &&
                valuePart[0] == '\'' && valuePart.back() == '\'') {
                char c = valuePart[1];
                var = std::make_shared<VarChar>(name, c);
            } // 解析复杂计算语句  返回计算结果 
            else {
                // 解析 int a=fun(b)
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
            if (IsPtr(type)) // 解析int* a;
                var = CreateVariablePtr(type, name, nullptr);
            else  //解析 int a;
                var = CreateVariable(type, name);
        }

        m_varMap[name] = std::move(var);
    }

    void Block::do_assignment(std::shared_ptr<VarBase>& leftVar, std::string& rightExpr, bool leftDeref) {
        VarType leftType = leftVar->get_type();

        // 处理 (int)a=sum(arg1,arg2)   (int*) a=sum(arg1,arg2)   (int&) a=sum(arg1,arg2)
        if (IsFunCall(rightExpr)) {
            std::shared_ptr<VarBase> ret = create_var_by_funcall_ret(rightExpr);
            VarType retType = ret->get_type();
            if (IsPtr(retType)) {
                SharePtr(leftVar, ret);  // 如果返回指针和引用 , 则共享数据
            }
            else if (IsRef(retType)) {
                ShareRef(leftVar, ret);
            }
            else {
                SetBasicVar(leftVar, GetDoubleValueByVar(ret));
            }
        }
        // 解析引用
        else if (IsRef(leftType)) {
            // 当leftType是返回值时 需要特殊处理
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
        // 解析*p1=a+3;
        else if (leftDeref && IsPtr(leftType)) {
            double result = m_calc.evaluate_expression(rightExpr, this);
            SetBasicVar(leftVar, result);
        }
        // 处理p1=p2  (p1,p2都是指针）和 p1=&a
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
        } //处理常规情况 a=b;
        else {
            double result = m_calc.evaluate_expression(rightExpr, this);
            SetBasicVar(leftVar, result);
        }
    }

    // 赋值语句解析
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



    // 返回语句解析
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


    // 从函数调用返回值创建变量
    std::shared_ptr<VarBase> Block::create_var_by_funcall_ret(std::string funcExpr) {
        std::shared_ptr<VarBase> ret = m_calc.evaluate_funCall(funcExpr, this);
        return ret;
    }

    // 传参
    std::shared_ptr<VarBase> Block::create_args_by_parent_var(VarType type, std::string name, std::string argValue) {
        std::shared_ptr<VarBase> var = CreateVarByBlockVarMap(type, name, argValue, m_parentBlock);
        return var;
    }

    // 从已有变量创建
    std::shared_ptr<VarBase> Block::create_var_by_self_var(VarType type, std::string name, std::string argValue) {
        std::shared_ptr<VarBase> var = CreateVarByBlockVarMap(type, name, argValue, this);
        return var;
    }

}
