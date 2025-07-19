#include <Calculator.h>
#include <BasicFunc.h>

namespace thz {

bool isOp(const std::string& str) {
    if (str == "+" || str == "-" || str == "*" || str == "/" || str == "@")
        return true;
    else return false;
}



std::string getStringValue(std::shared_ptr<VarBase> var) {
    if (!var) throw std::runtime_error("Null variable");

    if (var->getType() == VarType::CHAR) {
        double c = var->getData()[0];
        return std::to_string(static_cast<int>(c));
    } // double����Ϊint��������
    return var->getData();
}

double getDoubleValue(std::shared_ptr<VarBase> var) {
    if (!var) throw std::runtime_error("Null variable");

    try {
        if (var->getType() == VarType::CHAR) {
            return static_cast<double>(var->getData()[0]);
        }
        return std::stod(var->getData());     // int��charͳһ��double����
    }
    catch (...) {
        throw std::runtime_error("Conversion failed for: " + var->getData());
    }
}


enum class TokenType { NONE, NUMBER, IDENTIFIER ,OP };

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

double Calculator::evaluateExpression(const std::string& expr, FuncBase* parent) {
    if (expr.empty()) return 0.0;

    std::vector<std::string> tokens = tokenize(expr);
    return evaluateTokens(tokens,parent);
}

std::shared_ptr<VarBase> Calculator::evaluateFunCall(const std::string& expr, FuncBase* parent) {
    if (expr.empty()) return nullptr;

    std::vector<std::string> tokens = tokenize(expr);
    size_t token_idx = 0;
    return evaluateFunCallTokens(tokens, token_idx, parent);
}

std::shared_ptr<VarBase> Calculator::evaluateFunCallTokens(const std::vector<std::string>& tokens,size_t& tokens_idx, FuncBase* parent){
    while (tokens_idx < tokens.size()) {
        const std::string& token = tokens[tokens_idx];
        DEBUG("%d token %s", tokens_idx, token.c_str());

        if (token.empty()) {
            tokens_idx++;
            continue;
        }
        // ����������
        if (tokens_idx + 1 < tokens.size() && tokens[tokens_idx + 1] == "(" && isalpha(token[0])) {
            std::string funcName = token;
            tokens_idx += 2; // ������������������
            // ֱ�ӽ�sum(a,b)��ʵ��"a,b"���� callFunc, ���ڲ�����ݸ���������
            std::string actualArgs;
            while (tokens_idx < tokens.size() && tokens[tokens_idx] != ")") {
                actualArgs += tokens[tokens_idx];
                tokens_idx++;
            }
            if (tokens_idx < tokens.size() && tokens[tokens_idx] == ")") tokens_idx++;
            std::shared_ptr<VarBase> ret = FuncMap::getFuncMap().callFunc(funcName, actualArgs, parent);
            return ret;
        }
    }
}


double Calculator::evaluateTokens(const std::vector<std::string>& tokens, FuncBase* parent) {
    std::vector<double> values;
    std::vector<std::string> ops;
    size_t i = 0;


    while (i < tokens.size()) {
        const std::string& token = tokens[i];
        DEBUG("%d token %s", i, token.c_str());
        
        if (token.empty()) {
            i++;
            continue;
        }
        // ����������
        if (i + 1 < tokens.size() && tokens[i + 1] == "(" && isalpha(token[0])) { 
            std::shared_ptr<VarBase> ret = evaluateFunCallTokens(tokens, i, parent);
            values.push_back(getDoubleValue(ret));
            continue;
        }

        // ���������  
        if (isalpha(token[0])) {
            auto it = m_varMap->find(token);
            if (it == m_varMap->end()) {
                throw std::runtime_error("Undefined variable: " + token);
            }
            // ��������ǰ����һ��*��*ǰ��һ����������ʱ����Ҫ������
            if ((i == 1 && tokens[i - 1] == "*") || 
                (i >= 2 && tokens[i - 1] == "*") ||
                (i >= 2 && tokens[i - 1] == "*" && isOp(tokens[i - 2])) ||
                (i >= 2 && tokens[i - 1] == "*" && tokens[i - 2] == "(")) {
                ops.pop_back(); // ȡ��*
                values.push_back(getDoubleValue(deReference(it->second)));
            }
            else if (isRef(it->second->getType())) {
                // �����õĴ���
                values.push_back(getDoubleValue(deReference(it->second)));
            }
            else {
                values.push_back(getDoubleValue(it->second));
            }
            i++;
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
            i++;
        }
        // ����һԪ����
        else if (token == "@") {
            ops.push_back("@");
            i++;
        }
        else if (token == "(") {
            ops.push_back(token);
            i++;
        }
        else if (token == ")") {
            while (!ops.empty() && ops.back() != "(") {
                applyStackOperation(values, ops);
            }
            if (ops.empty()) throw std::runtime_error("Mismatched parentheses");
            ops.pop_back(); // �Ƴ� '('
            i++;
        }
        else if (token == "+" || token == "-" || token == "*" || token == "/") {
            while (!ops.empty() &&
                (ops.back() == "@" || // һԪ��������ȼ����
                    (getPrecedence(ops.back()) >= getPrecedence(token)))) {
                applyStackOperation(values, ops);
            }
            ops.push_back(token);
            i++;
        }
        else {
            throw std::runtime_error("Invalid token: " + token);
        }
    }

    // ����ʣ�������
    while (!ops.empty()) {
        applyStackOperation(values, ops);
    }

    if (values.size() != 1) {
        throw std::runtime_error("Expression evaluation failed");
    }

    return values.back();
}


void Calculator::applyStackOperation(std::vector<double>& values,
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
        values.push_back(applyOperationDouble(a, b, op));
    }
}


double Calculator::applyOperationDouble(double a, double b,
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

int Calculator::getPrecedence(const std::string& op) {
    if (op == "@") return 3;  // һԪ�����������ȼ�
    if (op == "*" || op == "/") return 2;
    if (op == "+" || op == "-") return 1;
    return 0; // �������
}

}
