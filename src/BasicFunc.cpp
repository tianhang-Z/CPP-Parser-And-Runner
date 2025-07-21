#include <BasicFunc.h>
#include <FileTools.h>
namespace thz {



    bool isVarDeclStmt(const std::string& stmt) {
        for (const char* type : { "int ", "int* ", "int& ",
                                "double ", "double* ", "double& ",
                                "char ", "char* ", "char& ",
                                "void ", "void* " }) {
            if (stmt.find(type) == 0) {
                return true;
            }
        }
        return false;
    }

    bool isAssignStmt(const std::string& stmt) {
        if (stmt.find('=') != std::string::npos) return true;
        else return false;
    }
    bool isReturnStmt(const std::string& stmt) {
        if (stmt.find("return") == 0) return true;
        else return false;
    };

    bool isFunCall(const std::string& stmt) {
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


    // 从函数调用返回值创建变量
    std::shared_ptr<VarBase> FuncBase::createVarByFunCallRet(std::string funcExpr) {
        std::shared_ptr<VarBase> ret = m_calc.evaluateFunCall(funcExpr, this);
        return ret;
    }

    // 传参
    std::shared_ptr<VarBase> FuncBase::createArgsByParentVar(VarType type, std::string name, std::string argValue) {
        __var_map__& parentVarMap = parentFunc->getVarMap();
        std::shared_ptr<VarBase> var = createVarByVarMap(type,name,argValue,parentVarMap);
        return var;
    }

    // 从已有变量创建
    std::shared_ptr<VarBase> FuncBase::createVarBySelfVar(VarType type, std::string name, std::string argValue) {
        std::shared_ptr<VarBase> var = createVarByVarMap(type, name, argValue, this->getVarMap());
        return var;
    }

    void FuncBase::setArgs(const std::string& actualArgs) {
        // 解析形参声明，如 "char a, int b, double c"
        std::vector<std::string> formalParamDecls = split(m_formalArgs, ',');

        // 解析实际参数值，如 " 'x', 42, 3.14, a, b "  可以是value，也可以是父函数的变量
        std::vector<std::string> argValues = split(actualArgs, ',');

        // 要求实参与形参数量一致 不支持默认参数
        if (formalParamDecls.size() != argValues.size()) {
            throw std::runtime_error("Argument count mismatch");
        }

        m_varMap.clear();

        for (size_t i = 0; i < formalParamDecls.size(); ++i) {
            std::string paramDecl = trim(formalParamDecls[i]);
            std::string argValue = trim(argValues[i]);

            // 解析参数声明，如 "char a" 或 "int b" ;
            // 或者 int* c 要求*在变量类型一侧
            size_t spacePos = paramDecl.find(' ');
            if (spacePos == std::string::npos) {
                throw std::runtime_error("Invalid parameter declaration: " + paramDecl);
            }

            std::string typeStr = trim(paramDecl.substr(0, spacePos));
            std::string paramName = trim(paramDecl.substr(spacePos + 1));


            VarType type = str2Type(typeStr);

            // 创建变量并添加到m_varMap 
            std::shared_ptr<VarBase> var;

            if (!argValue.empty()) {
                if (parentFunc!=nullptr) 
                    var = createArgsByParentVar(type, paramName, argValue);
                else {   // 参数并非变量 而是具体的值 因为并非父函数调用的
                        var = createVarByTemp(type, paramName, argValue);
                }
            } else {
                throw std::runtime_error("Argument mismatch");
            } 
            m_varMap[paramName] = std::move(var);
        }
    }

    void FuncBase::setReturnVar(const std::string& returnTypeStr) {
        VarType returnType = str2Type(returnTypeStr);
        if (isPtr(returnType))
            m_returnVar = createVariablePtr(returnType, "return_var", nullptr);
        else if (isRef(returnType))
            m_returnVar = createVariableRef(returnType, "return_var", nullptr);
        else 
            m_returnVar = createVariable(returnType, "return_var", "");
    }

    std::shared_ptr<VarBase> FuncBase::runFunc(const std::string& actualArgs, FuncBase* parent) {
        setParentFunc(parent);
        setArgs(actualArgs);
        parseFunctionBody();
        return m_returnVar;   // 返回值是m_returnVar的拷贝
    } 

    void FuncBase::parseFunctionBody() {
        std::vector<std::string> statements = split(m_funcStatements, ';');

        for (const std::string& stmt : statements) {
            std::string trimmedStmt = trim(stmt);
            if (trimmedStmt.empty()) continue;
            parseStatement(trimmedStmt);
        }
    }

    void FuncBase::parseStatement(const std::string& stmt) {

        if (isVarDeclStmt(stmt))
            parseVariableDeclaration(stmt);
        else if (isAssignStmt(stmt))
            parseAssignment(stmt);
        else if (isReturnStmt(stmt))
            parseReturnStatement(stmt);
        else if (isFunCall(stmt)) 
            m_calc.evaluateFunCall(stmt, this);
        else
            throw std::runtime_error("can not match stmt");
    }

    void FuncBase::parseVariableDeclaration(const std::string& stmt) {
        std::vector<std::string> parts = split(stmt, ' ');
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
            valuePart = trim(stmt.substr(equalPos + 1));
            if (!valuePart.empty() && valuePart.back() == ';') {
                valuePart.pop_back();
            }
        }

        VarType type = str2Type(typeStr);
        bool createRef = isRef(type);
        bool createPtr = isPtr(type);

        std::shared_ptr<VarBase> var;
        if (!valuePart.empty()) {
            // 解析int& a=b语句; 引用必须声明时初始化 语法较为固定
            // 解析int* a=&b;  int* a=a; 
            // 解析int* a= doNothing(b);
            if (createPtr||createRef) {
                if (isFunCall(valuePart)) {
                    // int* a=fun(b)     int& a=fun(b)
                    std::shared_ptr<VarBase> retVar = createVarByFunCallRet(valuePart);
                    bool returnPtr = isPtr(retVar->getType());
                    bool returnRef = isRef(retVar->getType());
                    var = createVarByVar(type, name, retVar,createPtr,createRef);
                }
                else 
                    var = createVarBySelfVar(type, name, valuePart); 
            } // 解析char c = 'A';
            else if (type == VarType::CHAR && valuePart.size() >= 3 &&
                valuePart[0] == '\'' && valuePart.back() == '\'') {
                char c = valuePart[1];
                var = std::make_shared<VarChar>(name, c);
            } // 解析复杂计算语句  返回计算结果 
            else {
                // 解析 int a=fun(b)
                if (isFunCall(valuePart)) {
                    std::shared_ptr<VarBase> ret = createVarByFunCallRet(valuePart);
                    var = createVariable(type, name, ret);
                }
                else {
                    double result = m_calc.evaluateExpression(valuePart, this);
                    var = createVariable(type, name, result);
                }
            }
        }
        else {
            if (isPtr(type)) // 解析int* a;
                var = createVariablePtr(type, name, nullptr);
            else  //解析 int a;
                var = createVariable(type, name);
        }

        m_varMap[name] = std::move(var);
    }

    void FuncBase::doAssignment(std::shared_ptr<VarBase>& leftVar, const std::string& rightExpr,bool leftDeref) {
        VarType leftType = leftVar->getType();

        // 处理 (int)a=sum(arg1,arg2)   (int*) a=sum(arg1,arg2)   (int&) a=sum(arg1,arg2)
        if (isFunCall(rightExpr)) {
            std::shared_ptr<VarBase> ret = createVarByFunCallRet(rightExpr);
            VarType retType = ret->getType();
            if (isPtr(retType) ) {
                sharePtr(leftVar, ret);  // 如果返回指针和引用 , 则共享数据
            }
            else if (isRef(retType)) {
                shareRef(leftVar, ret);
            }
            else {
                setBasicVar(leftVar, getDoubleValue(ret));
            }
        }
        // 解析引用
        else if (isRef(leftType)) {
            // 当leftType是返回值时 需要特殊处理
            // int& donothing(int& a) {return a;}
            if (leftVar->getName() == "return_var") {
                auto it = m_varMap.find(rightExpr);
                if (it == m_varMap.end())
                    throw std::runtime_error("can not get rightVar");
                std::shared_ptr<VarBase> rightVar = it->second;
                shareRef(leftVar, rightVar);
            }
            else {
                double result = m_calc.evaluateExpression(rightExpr, this);
                setBasicVar(leftVar, result);
            }
        }
        // 解析*p1=a+3;
        else if (leftDeref && isPtr(leftType)) {
            double result = m_calc.evaluateExpression(rightExpr, this);
            setBasicVar(leftVar, result);
        }
        // 处理p1=p2  (p1,p2都是指针）
        else if (isPtr(leftType) && !leftDeref) {
            auto rightVar = m_varMap.find(rightExpr);
            if (rightVar == m_varMap.end())
                throw std::runtime_error("Undefined right variable: " + leftVar->getName());
            if (!isPtr(rightVar->second->getType()))
                throw std::runtime_error("rigth var not a pointer");
            sharePtr(leftVar, rightVar->second);
        } //处理常规情况 a=b;
        else { 
            double result = m_calc.evaluateExpression(rightExpr, this);
            setBasicVar(leftVar, result);
        }
    }

    // 赋值语句解析
    void FuncBase::parseAssignment(const std::string& stmt) {
        size_t equalPos = stmt.find('=');
        if (equalPos == std::string::npos) {
            throw std::runtime_error("Invalid assignment: " + stmt);
        }

        std::string varName = trim(stmt.substr(0, equalPos));
        std::string exprStr = trim(stmt.substr(equalPos + 1));
        bool varDeRef = false;
        if (varName[0] == '*') {
            varDeRef = true;
            varName = varName.substr(1);
        }

        auto it = m_varMap.find(varName);
        if (it == m_varMap.end()) {
            throw std::runtime_error("Undefined variable: " + varName);
        }

        std::shared_ptr<VarBase> var = it->second;

        doAssignment(var, exprStr, varDeRef);
    }

    // 返回语句解析
    void FuncBase::parseReturnStatement(const std::string& stmt) {
        size_t returnPos = stmt.find("return");
        if (returnPos != 0) {
            throw std::runtime_error("Invalid return statement: " + stmt);
        }

        std::string exprStr = trim(stmt.substr(6));
        if (!exprStr.empty() && exprStr.back() == ';') {
            exprStr.pop_back();
        }

        if (exprStr.empty()) {
            return;
        }

        std::shared_ptr<VarBase> retVar = this->getReturnVar();
        VarType retType = retVar->getType();

        doAssignment(retVar, exprStr, false);
    }

}