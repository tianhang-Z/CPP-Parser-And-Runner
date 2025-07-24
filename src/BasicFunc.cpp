#include <BasicFunc.h>
#include <FileTools.h>
namespace thz {

    class ControlBlock;

    void FuncBlock::set_args(const std::string& actualArgs) {
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
                if (m_parentBlock!=nullptr) 
                    var = create_args_by_parent_var(type, paramName, argValue);
                else {   // �������Ǳ��� ���Ǿ����ֵ ��Ϊ���Ǹ��������õ�
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
        return m_returnVar;   // ����ֵ��m_returnVar�Ŀ���
    } 


    void FuncBlock::parse_function_body() {
        parse_block_body(m_blockBody);
    }

}