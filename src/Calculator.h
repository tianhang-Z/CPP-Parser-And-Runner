#pragma once
#ifndef __CALCULATOR__
#define __CALCULATOR__

#include <BasicVar.h>
#include <logger.h>
#include <vector>
#include <string>
#include <stack>
#include <map>
#include <cctype>
#include <stdexcept>
#include <sstream>
#include <algorithm>

namespace thz {

    class FuncMap;
    class FuncBase;
    double getDoubleValue(std::shared_ptr<VarBase> var);
    std::string getStringValue(std::shared_ptr<VarBase> var);

    class Calculator {
    public:
        Calculator() {
            m_varMap = nullptr;
        }
        Calculator(std::map<std::string, std::shared_ptr<VarBase>>* varMap)
            : m_varMap(varMap) {
        }

        double evaluateExpression(const std::string& expr, FuncBase* parent);
        std::shared_ptr<VarBase> evaluateFunCall(const std::string& expr, FuncBase* parent);
    private:                 
        std::map<std::string, std::shared_ptr<VarBase>>* m_varMap;

        std::vector<std::string> tokenize(const std::string& expr);
        double evaluateTokens(const std::vector<std::string>& tokens, FuncBase* parent);
        std::shared_ptr<VarBase> evaluateFunCallTokens(const std::vector<std::string>& tokens,size_t& token_idx ,FuncBase* parent);

        void applyStackOperation(std::vector<double>& values,
            std::vector<std::string>& ops);
        double applyOperationDouble(double a, double b, const std::string& op);
        int getPrecedence(const std::string& op);


    };
}


#endif // !__CALCULATOR__
