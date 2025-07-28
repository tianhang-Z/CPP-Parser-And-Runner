#include <Calculator.h>
#include <BasicFunc.h>
#include <BasicClass.h>

namespace thz {

    bool IsOp(const std::string& str) {
        if (str == "+" || str == "-" || str == "*" || str == "/" || str == "@")
            return true;
        else return false;
    }



    std::string GetStringValue(std::shared_ptr<VarBase> var) {
        if (!var) throw std::runtime_error("Null variable");

        if (var->get_type() == VarType::Char) {
            double c = var->get_data_to_str()[0];
            return std::to_string(static_cast<int>(c));
        } // double����Ϊint��������
        return var->get_data_to_str();
    }

    template<typename DataType,VarType Type>
    DataType GetDataByVar(std::shared_ptr<VarBase> var) {
        auto temp = std::dynamic_pointer_cast<typename Type2ClassMap<Type>::ClassName>(var);
        return temp->get_raw_data();
    }

    template<typename DataType, VarType Type>
    double GetDoubleByVarImpl(std::shared_ptr<VarBase> var) {
        DataType ret = GetDataByVar<DataType, Type>(var);
        return static_cast<double>(ret);
    }

    double GetDoubleValueByVar(std::shared_ptr<VarBase> var) {
        if (!var) throw std::runtime_error("Null variable");

        VarType type = var->get_type();
        switch(type) {
            case VarType::Double:  return GetDoubleByVarImpl<double, VarType::Double>(var);
            case VarType::DoubleRef: return GetDoubleByVarImpl<double, VarType::DoubleRef>(var);
            case VarType::DoublePtr: return GetDoubleByVarImpl<double, VarType::DoublePtr>(var);

            case VarType::Char: return GetDoubleByVarImpl<char, VarType::Char>(var);
            case VarType::CharRef: return GetDoubleByVarImpl<char, VarType::CharRef>(var);
            case VarType::CharPtr: return GetDoubleByVarImpl<char, VarType::CharPtr>(var);

            case VarType::Int: return GetDoubleByVarImpl<int, VarType::Int>(var);
            case VarType::IntRef: return GetDoubleByVarImpl<int, VarType::IntRef>(var);
            case VarType::IntPtr: return GetDoubleByVarImpl<int, VarType::IntPtr>(var);

            case VarType::Bool: return GetDoubleByVarImpl<bool, VarType::Bool>(var);
            case VarType::BoolRef: return GetDoubleByVarImpl<bool, VarType::BoolRef>(var);
            case VarType::BoolPtr: return GetDoubleByVarImpl<bool, VarType::BoolPtr>(var);
            default:
                throw std::runtime_error("Unsupported target type");
        }

    }


    enum class TokenType { NONE, NUMBER, IDENTIFIER, OP };



    //�����ͷָ�expr
    std::vector<std::string> Calculator::tokenize(const std::string& expr) {
        std::vector<std::string> tokens;
        std::string current;
        auto commit = [&] {
            if (!current.empty()) tokens.push_back(current);
            current.clear();
            };

        for (size_t i = 0; i < expr.size(); ++i) {
            const char c = expr[i];

            if (isspace(c)) {
                commit();
                continue;
            }

            // �������������ĸ��ͷ��������ĸ�����֡��»��ߣ�
            if (isalpha(c) || c == '_') {
                current += c;
                // ������ȡ��������ĸ�����ֻ��»���
                while (i + 1 < expr.size() && (isalnum(expr[i + 1]) || expr[i + 1] == '_' || expr[i + 1] == '.')) {
                    i++;
                    // �����ţ�ȷ���ǳ�Ա���ʣ�
                    if (expr[i] == '.') {
                        current += '.';
                        // ��ź�������ĸ/�»���
                        if (i + 1 >= expr.size() || !(isalpha(expr[i + 1]) || expr[i + 1] == '_')) {
                            throw std::runtime_error("Invalid member access syntax");
                        }
                        continue;
                    }
                    current += expr[i];
                }
                commit();
            }
            // �������֣������š�С����Ϳ�ѧ��������
            else if (isdigit(c) || (c == '-' && (current.empty() || tokens.back() == "("))) {
                if (!current.empty() && isalpha(current.back()))
                    throw std::runtime_error("Invalid number format");
                current += (c == '-' && current.empty()) ? '@' : c; // һԪ����ת@
                // ������ȡ���֡�С������ѧ������
                while (i + 1 < expr.size() && (isdigit(expr[i + 1]) || expr[i + 1] == '.' ||
                    tolower(expr[i + 1]) == 'e')) {
                    current += expr[++i];
                    // �����ѧ�������������ţ��� 1e-2��
                    if (tolower(expr[i]) == 'e' && i + 1 < expr.size() &&
                        (expr[i + 1] == '+' || expr[i + 1] == '-')) {
                        current += expr[++i];
                    }
                }
                commit();
            }
            // �����ţ���Ա���ʻ��Ա�������ã��� obj.p1 �� obj.fun()��
            else if (c == '.' && !tokens.empty() &&
                (isalpha(tokens.back().back()) || tokens.back().back() == '_')) {
                // �������ύ�����Ǽ�����ȡ��ź�ı�ʶ������ obj.fun��
                current += c;
                // ȷ����ź��ǺϷ��ı�����
                if (i + 1 >= expr.size() || !isalpha(expr[i + 1]))
                    throw std::runtime_error("Invalid member access syntax");
                // ������ȡ��ź�Ĳ��֣��� fun��
                while (i + 1 < expr.size() && (isalnum(expr[i + 1]) || expr[i + 1] == '_')) {
                    current += expr[++i];
                }
                // ��������� '('����������Ϊ�������ã��� obj.fun()��
                if (i + 1 < expr.size() && expr[i + 1] == '(') {
                    // ���ύ���ȴ����� '('
                }
                else {
                    commit();
                }
            }
            // ���������
            else {
                commit();
                if (c == '(' || c == ')' || c == ',') {
                    tokens.emplace_back(1, c);
                }
                else if (strchr("+-*/&|=", c)) {
                    tokens.emplace_back(1, c);
                }
                else {
                    throw std::runtime_error(std::string("Invalid character: ") + c);
                }
            }
        }
        commit();

        // У�����÷��������Ե�Ž�β��
        for (const auto& tok : tokens) {
            if (tok.back() == '.' || (tok.size() > 1 && tok.find("..") != std::string::npos))
                throw std::runtime_error("Invalid dot operator usage");
        }

        return tokens;
    }

    double Calculator::evaluate_expression(const std::string& expr, Block* parent) {
        if (expr.empty()) return 0.0;

        std::vector<std::string> tokens = tokenize(expr);
        return evaluate_tokens(tokens,parent);
    }

    std::shared_ptr<VarBase> Calculator::evaluate_funCall(const std::string& expr, Block* parent) {
        if (expr.empty()) return nullptr;

        std::vector<std::string> tokens = tokenize(expr);
        size_t token_idx = 0;
        return evaluate_funcall_tokens(tokens, token_idx, parent);
    }

    std::shared_ptr<VarBase> Calculator::evaluate_funcall_tokens(const std::vector<std::string>& tokens,size_t& tokens_idx, Block* parent){
        while (tokens_idx < tokens.size()) {
            const std::string& token = tokens[tokens_idx];

            if (token.empty()) {
                tokens_idx++;
                continue;
            }
            // ����������
            if (tokens_idx + 1 < tokens.size() && tokens[tokens_idx + 1] == "(" && isalpha(token[0])) {
                std::string funcName = token;
                tokens_idx += 2; // ������������������
                // ֱ�ӽ�sum(a,b)��ʵ��"a,b"���� call_func, ���ڲ�����ݸ���������
                std::string actualArgs;
                while (tokens_idx < tokens.size() && tokens[tokens_idx] != ")") {
                    actualArgs += tokens[tokens_idx];
                    tokens_idx++;
                }
                if (tokens_idx < tokens.size() && tokens[tokens_idx] == ")") tokens_idx++;
                std::shared_ptr<VarBase> ret;
                if (funcName.find('.') != std::string::npos) {
                    // �ຯ������
                    size_t dotPos = funcName.find('.');
                    std::string className = funcName.substr(0, dotPos);
                    std::string memberFunName = funcName.substr(dotPos + 1);
                    std::shared_ptr<BasicClass> cls = m_block->find_class(className);
                    if (cls == nullptr)
                        throw std::runtime_error("can not find class");
                    else
                        ret = cls->call_func(memberFunName, actualArgs, parent);
                }
                else 
                    ret = FuncMap::get_func_map().call_func(funcName, actualArgs, parent);
                return ret;
            }
        }
        return nullptr;
    }



    // ���ӹؼ��ִ����߼�

    double Calculator::evaluate_tokens(const std::vector<std::string>& tokens, Block* parent) {
        std::vector<double> values;
        std::vector<std::string> ops;
        size_t i = 0;


        while (i < tokens.size()) {
            const std::string& token = tokens[i];
        
            if (token.empty()) {
                i++;
                continue;
            }
            // ����������
            if (i + 1 < tokens.size() && tokens[i + 1] == "(" && isalpha(token[0])) { 
                std::shared_ptr<VarBase> ret = evaluate_funcall_tokens(tokens, i, parent);
                values.push_back(GetDoubleValueByVar(ret));
                continue;
            }
            else if (token == "true") {
                values.push_back(1);
            }
            else if (token == "false") {
                values.push_back(0);
            }
            // ���������  
            else if (isalpha(token[0])) {
                auto var = m_block->find_var(token);
                
                if (var == nullptr) {
                    throw std::runtime_error("Undefined variable: " + token);
                }
                // ��������ǰ����һ��*��*ǰ��һ����������ʱ����Ҫ������
                if ((i == 1 && tokens[i - 1] == "*") ||
                    (i >= 2 && tokens[i - 1] == "*") ||
                    (i >= 2 && tokens[i - 1] == "*" && IsOp(tokens[i - 2])) ||
                    (i >= 2 && tokens[i - 1] == "*" && tokens[i - 2] == "(")) {
                    ops.pop_back(); // ȡ��*
                    values.push_back(GetDoubleValueByVar(DeReference(var)));
                }
                else if (IsRef(var->get_type())) {
                    // �����õĴ���
                    values.push_back(GetDoubleValueByVar(DeReference(var)));
                }
                else {
                    values.push_back(GetDoubleValueByVar(var));
                }
                

            }
            // ��������
            else if (token.find_first_not_of("0123456789.-") == std::string::npos &&
                token != "-" && token != ".") {
                try {
                    values.push_back(std::stod(token));
                }
                catch (...) {
                    throw std::runtime_error("Invalid number: " + token);
                }
            }
            // ����һԪ����
            else if (token == "@") {
                ops.push_back("@");
                i++;
            }
            else if (token == "(") {
                ops.push_back(token);
            }
            else if (token == ")") {
                while (!ops.empty() && ops.back() != "(") {
                    apply_stack_operation(values, ops);
                }
                if (ops.empty()) throw std::runtime_error("Mismatched parentheses");
                ops.pop_back(); // �Ƴ� '('
            }
            else if (token == "+" || token == "-" || token == "*" || token == "/") {
                while (!ops.empty() &&
                    (ops.back() == "@" || // һԪ��������ȼ����
                        (get_precedence(ops.back()) >= get_precedence(token)))) {
                    apply_stack_operation(values, ops);
                }
                ops.push_back(token);
            }
            else {
                throw std::runtime_error("Invalid token: " + token);
            }
            i++;
        }

        // ����ʣ�������
        while (!ops.empty()) {
            apply_stack_operation(values, ops);
        }

        if (values.size() != 1) {
            throw std::runtime_error("Expression evaluation failed");
        }

        return values.back();
    }


    void Calculator::apply_stack_operation(std::vector<double>& values,
        std::vector<std::string>& ops) {
        if (ops.empty()) return;

        std::string op = ops.back();
        ops.pop_back();

        if (op == "@") {  // һԪ����
            if (values.empty()) throw std::runtime_error("Missing operand for unary -");
            double a = values.back(); values.pop_back();
            values.push_back(-a);
        }
        else {  // ��Ԫ�����
            if (values.size() < 2)
                throw std::runtime_error("Not enough operands for operator " + op);

            double b = values.back(); values.pop_back();
            double a = values.back(); values.pop_back();
            values.push_back(apply_operation_double(a, b, op));
        }
    }


    double Calculator::apply_operation_double(double a, double b,
        const std::string& op) {
        if (op == "+") return a + b;
        if (op == "-") return a - b;
        if (op == "*") return a * b;
        if (op == "/") {
            if (b == 0) throw std::runtime_error("Division by zero");
            return a / b;
        }
        throw std::runtime_error("Unsupported operator: " + op);
    }

    int Calculator::get_precedence(const std::string& op) {
        if (op == "@") return 3;  // һԪ�����������ȼ�
        if (op == "*" || op == "/") return 2;
        if (op == "+" || op == "-") return 1;
        return 0; // �������
    }

}
