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

    class Block;

    double GetDoubleValue(std::shared_ptr<VarBase> var);
    std::string GetStringValue(std::shared_ptr<VarBase> var);

    class Calculator {
    public:
        Calculator() {
            block = nullptr;
        }
        Calculator(Block* block)
            :block(block) {
        }

        double evaluate_expression(const std::string& expr, Block* parent);
        std::shared_ptr<VarBase> evaluate_funCall(const std::string& expr, Block* parent);
    private:            

        Block*  block;

        std::vector<std::string> tokenize(const std::string& expr);
        double evaluate_tokens(const std::vector<std::string>& tokens, Block* parent);
        std::shared_ptr<VarBase> evaluate_funcall_tokens(const std::vector<std::string>& tokens,size_t& token_idx , Block* parent);

        void apply_stack_operation(std::vector<double>& values,
            std::vector<std::string>& ops);
        double apply_operation_double(double a, double b, const std::string& op);
        int get_precedence(const std::string& op);


    };
}


#endif // !__CALCULATOR__
