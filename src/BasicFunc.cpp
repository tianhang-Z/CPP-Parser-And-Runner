#include <BasicFunc.h>
#include <FileTools.h>
namespace thz {



    bool IsVarDeclStmt(const std::string& stmt) {
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


    // �Ӻ������÷���ֵ��������
    std::shared_ptr<VarBase> FuncBase::create_var_by_funcall_ret(std::string funcExpr) {
        std::shared_ptr<VarBase> ret = m_calc.evaluate_funCall(funcExpr, this);
        return ret;
    }

    // ����
    std::shared_ptr<VarBase> FuncBase::create_args_by_parentv_var(VarType type, std::string name, std::string argValue) {
        VarMap& parentVarMap = parentFunc->get_var_map();
        std::shared_ptr<VarBase> var = CreateVarByVarMap(type,name,argValue,parentVarMap);
        return var;
    }

    // �����б�������
    std::shared_ptr<VarBase> FuncBase::create_var_by_self_var(VarType type, std::string name, std::string argValue) {
        std::shared_ptr<VarBase> var = CreateVarByVarMap(type, name, argValue, this->get_var_map());
        return var;
    }

    void FuncBase::set_args(const std::string& actualArgs) {
        // �����β��������� "char a, int b, double c"
        std::vector<std::string> formalParamDecls = Split(m_formalArgs, ',');

        // ����ʵ�ʲ���ֵ���� " 'x', 42, 3.14, a, b "  ������value��Ҳ�����Ǹ������ı���
        std::vector<std::string> argValues = Split(actualArgs, ',');

        // Ҫ��ʵ�����β�����һ�� ��֧��Ĭ�ϲ���
        if (formalParamDecls.size() != argValues.size()) {
            throw std::runtime_error("Argument count mismatch");
        }

        m_varMap.clear();

        for (size_t i = 0; i < formalParamDecls.size(); ++i) {
            std::string paramDecl = Trim(formalParamDecls[i]);
            std::string argValue = Trim(argValues[i]);

            // ���������������� "char a" �� "int b" ;
            // ���� int* c Ҫ��*�ڱ�������һ��
            size_t spacePos = paramDecl.find(' ');
            if (spacePos == std::string::npos) {
                throw std::runtime_error("Invalid parameter declaration: " + paramDecl);
            }

            std::string typeStr = Trim(paramDecl.substr(0, spacePos));
            std::string paramName = Trim(paramDecl.substr(spacePos + 1));


            VarType type = str2Type(typeStr);

            // ������������ӵ�m_varMap 
            std::shared_ptr<VarBase> var;

            if (!argValue.empty()) {
                if (parentFunc!=nullptr) 
                    var = create_args_by_parentv_var(type, paramName, argValue);
                else {   // �������Ǳ��� ���Ǿ����ֵ ��Ϊ���Ǹ��������õ�
                        var = CreateVarByTemp(type, paramName, argValue);
                }
            } else {
                throw std::runtime_error("Argument mismatch");
            } 
            m_varMap[paramName] = std::move(var);
        }
    }

    void FuncBase::set_return_var(const std::string& returnTypeStr) {
        VarType returnType = str2Type(returnTypeStr);
        if (IsPtr(returnType))
            m_returnVar = CreateVariablePtr(returnType, "return_var", nullptr);
        else if (IsRef(returnType))
            m_returnVar = CreateVariableRef(returnType, "return_var", nullptr);
        else 
            m_returnVar = CreateVariable(returnType, "return_var", "");
    }

    std::shared_ptr<VarBase> FuncBase::run_func(const std::string& actualArgs, FuncBase* parent) {
        set_parent_func(parent);
        set_args(actualArgs);
        parse_function_body();
        return m_returnVar;   // ����ֵ��m_returnVar�Ŀ���
    } 

    void FuncBase::parse_function_body() {
        std::vector<std::string> statements = Split(m_funcStatements, ';');

        for (const std::string& stmt : statements) {
            std::string trimmedStmt = Trim(stmt);
            if (trimmedStmt.empty()) continue;
            parse_statement(trimmedStmt);
        }
    }

    void FuncBase::parse_statement(const std::string& stmt) {

        if (IsVarDeclStmt(stmt))
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

    void FuncBase::parse_variable_declaration(const std::string& stmt) {
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
            if (createPtr||createRef) {
                if (IsFunCall(valuePart)) {
                    // int* a=fun(b)     int& a=fun(b)
                    std::shared_ptr<VarBase> retVar = create_var_by_funcall_ret(valuePart);
                    bool returnPtr = IsPtr(retVar->get_type());
                    bool returnRef = IsRef(retVar->get_type());
                    var = CreateVarByVar(type, name, retVar,createPtr,createRef);
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

    void FuncBase::do_assignment(std::shared_ptr<VarBase>& leftVar, const std::string& rightExpr,bool leftDeref) {
        VarType leftType = leftVar->get_type();

        // ���� (int)a=sum(arg1,arg2)   (int*) a=sum(arg1,arg2)   (int&) a=sum(arg1,arg2)
        if (IsFunCall(rightExpr)) {
            std::shared_ptr<VarBase> ret = create_var_by_funcall_ret(rightExpr);
            VarType retType = ret->get_type();
            if (IsPtr(retType) ) {
                SharePtr(leftVar, ret);  // �������ָ������� , ��������
            }
            else if (IsRef(retType)) {
                ShareRef(leftVar, ret);
            }
            else {
                SetBasicVar(leftVar, GetDoubleValue(ret));
            }
        }
        // ��������
        else if (IsRef(leftType)) {
            // ��leftType�Ƿ���ֵʱ ��Ҫ���⴦��
            // int& donothing(int& a) {return a;}
            if (leftVar->get_name() == "return_var") {
                auto it = m_varMap.find(rightExpr);
                if (it == m_varMap.end())
                    throw std::runtime_error("can not get rightVar");
                std::shared_ptr<VarBase> rightVar = it->second;
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
        // ����p1=p2  (p1,p2����ָ�룩
        else if (IsPtr(leftType) && !leftDeref) {
            auto rightVar = m_varMap.find(rightExpr);
            if (rightVar == m_varMap.end())
                throw std::runtime_error("Undefined right variable: " + leftVar->get_name());
            if (!IsPtr(rightVar->second->get_type()))
                throw std::runtime_error("rigth var not a pointer");
            SharePtr(leftVar, rightVar->second);
        } //��������� a=b;
        else { 
            double result = m_calc.evaluate_expression(rightExpr, this);
            SetBasicVar(leftVar, result);
        }
    }

    // ��ֵ������
    void FuncBase::parse_assignment(const std::string& stmt) {
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

        auto it = m_varMap.find(varName);
        if (it == m_varMap.end()) {
            throw std::runtime_error("Undefined variable: " + varName);
        }

        std::shared_ptr<VarBase> var = it->second;

        do_assignment(var, exprStr, varDeRef);
    }

    // ����������
    void FuncBase::parse_return_statement(const std::string& stmt) {
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

}