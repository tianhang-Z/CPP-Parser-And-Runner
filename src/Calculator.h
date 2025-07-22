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
    double GetDoubleValue(std::shared_ptr<VarBase> var);
    std::string GetStringValue(std::shared_ptr<VarBase> var);

    class Calculator {
    public:
        Calculator() {
            m_varMap = nullptr;
        }
        Calculator(std::map<std::string, std::shared_ptr<VarBase>>* varMap)
            : m_varMap(varMap) {
        }

        double evaluate_expression(const std::string& expr, FuncBase* parent);
        std::shared_ptr<VarBase> evaluate_funCall(const std::string& expr, FuncBase* parent);
    private:                 
        std::map<std::string, std::shared_ptr<VarBase>>* m_varMap;

        std::vector<std::string> tokenize(const std::string& expr);
        double evaluate_tokens(const std::vector<std::string>& tokens, FuncBase* parent);
        std::shared_ptr<VarBase> evaluate_funcall_tokens(const std::vector<std::string>& tokens,size_t& token_idx ,FuncBase* parent);

        void apply_stack_operation(std::vector<double>& values,
            std::vector<std::string>& ops);
        double apply_operation_double(double a, double b, const std::string& op);
        int get_precedence(const std::string& op);


    };
}


#endif // !__CALCULATOR__
