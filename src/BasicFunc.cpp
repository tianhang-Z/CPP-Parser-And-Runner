#include <BasicFunc.h>
#include <FileTools.h>
namespace thz {

    class ControlBlock;


    FuncBlock::FuncBlock(const std::string& returnType, const std::string& name, const std::string& formalArgs, const std::string& funcStmt)
        :Block(funcStmt),
        m_name(name),
        m_formalArgs(formalArgs) {
        m_type = BlockType::FunBlock;
        create_return_var(returnType);
    };
    FuncBlock::FuncBlock(const FuncBlock& func) : Block(func.m_blockBody) {
        m_type = BlockType::FunBlock;
        m_name = func.m_name;
        m_formalArgs = func.m_formalArgs;
        VarType returnType = func.get_return_var()->get_type();
        create_return_var(type2Str(returnType));
    };


    void FuncBlock::set_args(const std::string& actualArgs) {
        // 解析形参声明，如 "char a, int b, double c"
        std::vector<std::string> formalParamDecls = Split(m_formalArgs, ',');

        // 解析实际参数值，如 " 'x', 42, 3.14, a, b "  可以是value，也可以是父函数的变量
        std::vector<std::string> argValues = Split(actualArgs, ',');

        // 要求实参与形参数量一致 不支持默认参数
        if (formalParamDecls.size() != argValues.size()) {
            throw std::runtime_error("Argument count mismatch");
        }

        m_varMap.clear();

        for (size_t i = 0; i < formalParamDecls.size(); ++i) {
            std::string paramDecl = Trim(formalParamDecls[i]);
            std::string argValue = Trim(argValues[i]);

            // 解析参数声明，如 "char a" 或 "int b" ;
            // 或者 int* c 要求*在变量类型一侧
            size_t spacePos = paramDecl.find(' ');
            if (spacePos == std::string::npos) {
                throw std::runtime_error("Invalid parameter declaration: " + paramDecl);
            }

            std::string typeStr = Trim(paramDecl.substr(0, spacePos));
            std::string paramName = Trim(paramDecl.substr(spacePos + 1));


            VarType type = str2Type(typeStr);

            // 创建变量并添加到m_varMap 
            std::shared_ptr<VarBase> var;

            if (!argValue.empty()) {
                if (m_parentBlock!=nullptr) 
                    var = create_args_by_parent_var(type, paramName, argValue);
                else {   // 参数并非变量 而是具体的值 因为并非父函数调用的
                        var = CreateVarByTemp(type, paramName, argValue);
                }
            } else {
                throw std::runtime_error("Argument mismatch");
            } 
            m_varMap[paramName] = std::move(var);
        }
    }

    void FuncBlock::create_return_var(const std::string& returnTypeStr) {
        VarType returnType = str2Type(returnTypeStr);
        if (IsPtr(returnType))
            m_returnVar = CreateVariablePtr(returnType, "return_var", nullptr);
        else if (IsRef(returnType))
            m_returnVar = CreateVariableRef(returnType, "return_var", nullptr);
        else 
            m_returnVar = CreateVariable(returnType, "return_var", "");
    }

    std::shared_ptr<VarBase> FuncBlock::run_func(const std::string& actualArgs, Block* parent) {
        set_parent_func(parent);
        set_args(actualArgs);
        parse_function_body();
        return m_returnVar;   // 返回值是m_returnVar的拷贝
    } 


    void FuncBlock::parse_function_body() {
        parse_block_body(m_blockBody);
    }

    // 支持重载 name+actualArgs组成唯一标识 name+formalArgs也组成唯一标识
    std::shared_ptr<VarBase> FuncMap::call_func(const std::string name, const std::string& actualArgs, Block* parent) {

        auto it = m_funcMap.find(name);
        if (it == m_funcMap.end()) {
            throw std::runtime_error("Function not found: " + name);
        }
        FuncBlock copyFunc(*(it->second));

        return copyFunc.run_func(actualArgs, parent);

    }

    FuncBlock* FuncMap::create_func(std::string name) {
        auto it = m_funcMap.find(name);
        if (it == m_funcMap.end()) {
            throw std::runtime_error("Function not found: " + name);
        }
        FuncBlock* newFunc = new FuncBlock(*(it->second));
        return newFunc;

    }
    void FuncMap::show_func() {
        for (auto ele : m_funcMap) {
            std::cout << "name: " << ele.first << std::endl;
            std::cout << "params:" << ele.second->get_formal_args() << std::endl;
        }
    }

}