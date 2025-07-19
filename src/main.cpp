#include <Calculator.h>
#include <BasicVar.h>
#include <BasicFunc.h>
#include <logger.h>
#include <FileTools.h>
#include <iostream>


namespace test {
    using namespace thz;

	void test_cal() {
		std::string expr= { "2+3*5+(1+1)"};
        Calculator calc;
		double result = calc.evaluateExpression(expr,nullptr);
		std::cout << "Result: " << result << std::endl;
	}
	void test_var() {
        std::vector<std::shared_ptr<VarBase>> variables;

        // 测试Int类型
        variables.push_back(createVariable(VarType::INT, "age", "25"));
        variables.push_back(createVariable(VarType::INT, "count"));  // 默认值0
        variables.push_back(std::make_shared<VarInt>("test", 11));    // 直接构造

        // 测试Char类型
        variables.push_back(createVariable(VarType::CHAR, "initial", "A"));
        variables.push_back(createVariable(VarType::CHAR, "empty"));  // 默认值'\0'
        variables.push_back(std::make_shared<VarChar>("ch", 'X'));    // 直接构造

        // 测试Double类型
        variables.push_back(createVariable(VarType::DOUBLE, "price", "19.99"));
        variables.push_back(createVariable(VarType::DOUBLE, "ratio"));  // 默认值0.0
        variables.push_back(std::make_shared<VarDouble>("pi", 3.14159)); // 直接构造

        // 打印所有变量
        for (const auto& var : variables) {
            std::cout << "Name: " << var->getName()
                << ", Type: " << typeStr(var->getType())
                << ", Value: " << var->getData() << std::endl;
        }

        // 设置新值并验证
        variables[0]->setData("30");  // age从25改为30
        variables[1]->setData("100"); // count从0改为100
        variables[2]->setData("40");  // temp从30改为40

        variables[3]->setData("B");   // initial从A改为B
        variables[4]->setData("Y");   // empty从'\0'改为Y
        variables[5]->setData("Z");   // ch从X改为Z

        variables[6]->setData("29.95"); // price从19.99改为29.95
        variables[7]->setData("0.5");   // ratio从0.0改为0.5
        variables[8]->setData("2.71828"); // pi从3.14159改为2.71828

        // 打印所有变量
        for (const auto& var : variables) {
            std::cout << "Name: " << var->getName()
                << ", Type: " << typeStr(var->getType())
                << ", Value: " << var->getData() << std::endl;
        }

        // 测试错误处理
        try {
            variables[0]->setData("abc");  // 无效int值
        }
        catch (const std::runtime_error& e) {
            std::cout << "Error: " << e.what() << std::endl;
        }

        try {
            variables[3]->setData("AB");   // 无效char值
        }
        catch (const std::runtime_error& e) {
            std::cout << "Error: " << e.what() << std::endl;
        }

        try {
            variables[6]->setData("xyz");  // 无效char值
        }
        catch (const std::runtime_error& e) {
            std::cout << "Error: " << e.what() << std::endl;
        }


	}

    void test_func() {
        /*
         int mainFunc()
         {
            int a = 0;
            int b = 1;
            a = b;
            int c = a + b;
            return c;
        }
        */
        /*
        FuncBase(const std::string& returnType, const std::string& name, const std::string& formalArgs, const std::string& funcState)
        :m_name(name), m_formalArgs(formalArgs), m_funcStatements(funcState), m_calc(&m_varMap) {
        setReturnVar(returnType);
        };
        */
        
        std::string returnType = "int";
        std::string name = "mainFunc";
        std::string formalArgs = "int arg1,int arg2";
        std::string funcStmt =
            " int a = arg1;"
            "int b = 2;"
            "a = b;"
            "int c = a+arg2;"
            "return c+arg1; ";
        FuncBase testFunc(returnType,name,formalArgs,funcStmt);
        testFunc.runFunc("1,2",nullptr);
        //testFunc.displayVarMap();
        displayVar(testFunc.getReturnVar());
    }

    void test_funCall() {
        std::string returnType1 = "int";
        std::string name1 = "sum";
        std::string formalArgs1 = "int a,int b";
        std::string funcStmt1 =
            " return doNothing(a)+b";
        FuncBase fun1(returnType1, name1, formalArgs1, funcStmt1);


        std::string returnType2 = "int";
        std::string name2 = "doNothing";
        std::string formalArgs2 = "int b";
        std::string funcStmt2 =
            " return b";
        FuncBase fun2(returnType2, name2, formalArgs2, funcStmt2);

        FuncMap::getFuncMap().addFunc(fun1.getName(), &fun1);
        FuncMap::getFuncMap().addFunc(fun2.getName(), &fun2);

        //FuncMap::showFunc();

        std::string returnType3 = "int";
        std::string name3 = "mainFunc";
        std::string formalArgs3 = "int arg1, int arg2, int arg3";
        std::string funcStmt3 =
            " int a = 100 ;"
            " a = sum(arg1, arg2);"
            "int b = sum(a, arg3);"
            "int c = b;"
            "return c;";
        FuncBase mainFunc(returnType3, name3, formalArgs3, funcStmt3);
        
        mainFunc.runFunc("100,100,100",nullptr);
        displayVar(mainFunc.getReturnVar());
    }
    void test_EasyRefAndPtr() {
        std::string returnType1 = "int";
        std::string name1 = "sum";
        std::string formalArgs1 = "int arg1,int arg2";
        std::string funcStmt1 =
            "int a=1;"
            "int* b=&a;"
            "int& c=a;"
            "return c;";
        FuncBase fun1(returnType1, name1, formalArgs1, funcStmt1);
        fun1.runFunc("1,2", nullptr);
        displayVar(fun1.getReturnVar());
    }

    void test_RefAndPtr() {
        std::string returnType1 = "void";
        std::string name1 = "sum";
        std::string formalArgs1 = "int& a,int& b,int& out";
        std::string funcStmt1 =
            " int* c = doNothing(&a);"
            "int& c1 = *c;"
            "out = c1 + b; ";
        FuncBase fun1(returnType1, name1, formalArgs1, funcStmt1);


        std::string returnType2 = "int*";
        std::string name2 = "doNothing";
        std::string formalArgs2 = "int* a";
        std::string funcStmt2 =
            " return a";
        FuncBase fun2(returnType2, name2, formalArgs2, funcStmt2);

        FuncMap::getFuncMap().addFunc(fun1.getName(), &fun1);
        FuncMap::getFuncMap().addFunc(fun2.getName(), &fun2);

        //FuncMap::showFunc();

        std::string returnType3 = "int";
        std::string name3 = "mainFunc";
        std::string formalArgs3 = "int arg1, int arg2, int arg3";
        std::string funcStmt3 =
            " int a = 0;"
            "sum(arg1, arg2, a);"
            "int b = 0;"
            "sum(a, arg3, b);"
            "int c = b;"
            "return c; ";
        FuncBase mainFunc(returnType3, name3, formalArgs3, funcStmt3);

        mainFunc.runFunc("1,2,3", nullptr);
        displayVar(mainFunc.getReturnVar());
    }

    void test_extractFunc() {
        std::map<std::string, FuncInfo> funcMap = extractFunctions("E:/hw2/testGroups/group1/cppCode3.cpp");

        if (funcMap.find("mainFunc") != funcMap.end()) {
            FuncInfo mainFunc = funcMap["mainFunc"];
            std::cout << "Function: " << mainFunc.funcName << "\n"
                << "Return Type: " << mainFunc.returnType << "\n"
                << "Arguments: " << mainFunc.formalArgs << "\n"
                << "Body: " << mainFunc.funcStmt << std::endl;
        }
    }
    void test_file(std::string filePath, std::string actualArgs) {
        std::map<std::string, FuncInfo> funcInfoMap = extractFunctions(filePath);
        
        for (auto& func : funcInfoMap) {
            std::string returnType = func.second.returnType;
            std::string name = func.second.funcName;
            std::string formalArgs = func.second.formalArgs;
            std::string funcStmt = func.second.funcStmt;
            FuncBase* fun = new FuncBase(returnType, name, formalArgs, funcStmt);
            FuncMap::getFuncMap().addFunc(fun->getName(), fun);
        }

        FuncBase* mainFunc = FuncMap::getFuncMap().createFunc("mainFunc");
        mainFunc->runFunc(actualArgs, nullptr);
        displayVar(mainFunc->getReturnVar());
    }
    void test_pipeLine() {
        std::cout << "--------------test pipeline------------" << std::endl;
        std::string file_path = "E:/hw2/testGroups/group1/cppCode3.cpp";
        std::string actualArgs = "100,2,3";
        test_file(file_path, actualArgs);
    }

    void test_char() {
        std::cout << "--------------test char------------" << std::endl;
        std::string file_path = "E:/hw2/testGroups/group1/testChar.cpp";
        std::string actualArgs = "'a',1,1";
        test_file(file_path, actualArgs);
    }
    void test_double() {
        std::cout << "--------------test double------------" << std::endl;
        std::string file_path = "E:/hw2/testGroups/group1/testDouble.cpp";
        std::string actualArgs = "1.1,2.2,3.3";
        test_file(file_path, actualArgs);
    }


}
int main() {  
    logger::setLogFile("log.txt", false);
    logger::setLogLevel(logger::LOG_LEVEL::LOG_LEVEL_DEBUG);
	//test::test_cal();
    //test::test_var();
    //test::test_func();
    //test::test_funCall();
    //test::test_EasyRefAndPtr();
    //test::test_RefAndPtr();
    //test::test_extractFunc();
    test::test_pipeLine();
    test::test_char();
    test::test_double();


}