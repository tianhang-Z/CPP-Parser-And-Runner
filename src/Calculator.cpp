#include <Calculator.h>
#include <BasicFunc.h>

namespace thz {

bool IsOp(const std::string& str) {
    if (str == "+" || str == "-" || str == "*" || str == "/" || str == "@")
        return true;
    else return false;
}



std::string GetStringValue(std::shared_ptr<VarBase> var) {
    if (!var) throw std::runtime_error("Null variable");

    if (var->get_type() == VarType::Char) {
        double c = var->get_data()[0];
        return std::to_string(static_cast<int>(c));
    } // double����Ϊint��������
    return var->get_data();
}

double GetDoubleValue(std::shared_ptr<VarBase> var) {
    if (!var) throw std::runtime_error("Null variable");

    try {
        if (var->get_type() == VarType::Char) {
            return static_cast<double>(var->get_data()[0]);
        }
        return std::stod(var->get_data());     // int��charͳһ��double����
    }
    catch (...) {
        throw std::runtime_error("Conversion failed for: " + var->get_data());
    }
}

namespace {
    enum class TokenType { NONE, NUMBER, IDENTIFIER, OP };

}

//�����ͷָ�expr
std::vector<std::string> Calculator::tokenize(const std::string& expr) {
    std::vector<std::string> tokens;
    std::string currentToken;
    TokenType currentType = TokenType::NONE;
    for (char c : expr) {
        if (isspace(c)) {
            if (!currentToken.empty()) {
                tokens.push_back(currentToken);
                currentToken.clear();
                currentType = TokenType::NONE;
            }
            continue;
        }

        // ��������������԰�����ĸ�����֡��»��ߣ�
        if (isalpha(c) || c == '_' ||
            (isdigit(c) && currentType == TokenType::IDENTIFIER)) {

            if (currentType == TokenType::NUMBER && !currentToken.empty()) {
                tokens.push_back(currentToken);
                currentToken.clear();
            }

            currentToken += c;
            currentType = TokenType::IDENTIFIER;
        }
        // �������֣��������š�С���㣩
        else if (isdigit(c) || c == '.' ||
            (c == '-' && currentToken.empty())) {

            if (currentType == TokenType::IDENTIFIER && !currentToken.empty()) {
                tokens.push_back(currentToken);
                currentToken.clear();
            }

            currentToken += c;
            currentType = TokenType::NUMBER;
        }
        // ���������ַ�������� + - * /  & ���� �ȣ�
        else {
            if (!currentToken.empty()) {
                tokens.push_back(currentToken);
                currentToken.clear();
                currentType = TokenType::NONE;
            }

            // ���⴦��������
            if (c == '(' || c == ')' || c == ',') {
                tokens.push_back(std::string(1, c));
            }
            // ���⴦��һԪ����
            else if (c == '-' && (tokens.empty() ||
                tokens.back() == "(" ||
                tokens.back() == "+" ||
                tokens.back() == "-" ||
                tokens.back() == "*" ||
                tokens.back() == "/")) {
                tokens.push_back("@");
            }
            else {
                tokens.push_back(std::string(1, c));
            }
        }
    }

    if (!currentToken.empty()) {
        tokens.push_back(currentToken);
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
        LOG_DEBUG("%d token %s", tokens_idx, token.c_str());

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
            std::shared_ptr<VarBase> ret = FuncMap::get_func_map().call_func(funcName, actualArgs, parent);
            return ret;
        }
    }
    return nullptr;
}



// ���ӹؼ��ִ����߼�
// static_cast 
double Calculator::evaluate_tokens(const std::vector<std::string>& tokens, Block* parent) {
    std::vector<double> values;
    std::vector<std::string> ops;
    size_t i = 0;


    while (i < tokens.size()) {
        const std::string& token = tokens[i];
        LOG_DEBUG("%d token %s", i, token.c_str());
        
        if (token.empty()) {
            i++;
            continue;
        }
        // ����������
        if (i + 1 < tokens.size() && tokens[i + 1] == "(" && isalpha(token[0])) { 
            std::shared_ptr<VarBase> ret = evaluate_funcall_tokens(tokens, i, parent);
            values.push_back(GetDoubleValue(ret));
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
            auto var = block->find_var(token);
            if (var == nullptr) {
                throw std::runtime_error("Undefined variable: " + token);
            }
            // ��������ǰ����һ��*��*ǰ��һ����������ʱ����Ҫ������
            if ((i == 1 && tokens[i - 1] == "*") || 
                (i >= 2 && tokens[i - 1] == "*") ||
                (i >= 2 && tokens[i - 1] == "*" && IsOp(tokens[i - 2])) ||
                (i >= 2 && tokens[i - 1] == "*" && tokens[i - 2] == "(")) {
                ops.pop_back(); // ȡ��*
                values.push_back(GetDoubleValue(DeReference(var)));
            }
            else if (IsRef(var->get_type())) {
                // �����õĴ���
                values.push_back(GetDoubleValue(DeReference(var)));
            }
            else {
                values.push_back(GetDoubleValue(var));
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
