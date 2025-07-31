#include <Calculator.h>
#include <BasicVar.h>
#include <BasicFunc.h>
#include <BasicClass.h>
#include <logger.h>
#include <FileTools.h>
#include <iostream>
#include <Timer.hpp>
#include <filesystem>


namespace test {
    using namespace thz;

	void test_cal() {
		std::string expr= { "2+3*5+(1+1)"};
        Calculator calc;
		double result = calc.evaluate_expression(expr,nullptr);
		std::cout << "Result: " << result << std::endl;
	}
	void test_var() {
        std::vector<std::shared_ptr<VarBase>> variables;

        // 测试Int类型
        variables.push_back(CreateVariable(VarType::Int, "age", "25"));
        variables.push_back(CreateVariable(VarType::Int, "count"));  // 默认值0
        variables.push_back(std::make_shared<VarInt>("test", 11));    // 直接构造

        // 测试Char类型
        variables.push_back(CreateVariable(VarType::Char, "initial", "A"));
        variables.push_back(CreateVariable(VarType::Char, "empty"));  // 默认值'\0'
        variables.push_back(std::make_shared<VarChar>("ch", 'X'));    // 直接构造

        // 测试Double类型
        variables.push_back(CreateVariable(VarType::Double, "price", "19.99"));
        variables.push_back(CreateVariable(VarType::Double, "ratio"));  // 默认值0.0
        variables.push_back(std::make_shared<VarDouble>("pi", 3.14159)); // 直接构造

        // 打印所有变量
        for (const auto& var : variables) {
            std::cout << "Name: " << var->get_name()
                << ", Type: " << type2Str(var->get_type())
                << ", Value: " << var->get_data_to_str() << std::endl;
        }

        // 设置新值并验证
        variables[0]->set_data_by_str("30");  // age从25改为30
        variables[1]->set_data_by_str("100"); // count从0改为100
        variables[2]->set_data_by_str("40");  // temp从30改为40

        variables[3]->set_data_by_str("B");   // initial从A改为B
        variables[4]->set_data_by_str("Y");   // empty从'\0'改为Y
        variables[5]->set_data_by_str("Z");   // ch从X改为Z

        variables[6]->set_data_by_str("29.95"); // price从19.99改为29.95
        variables[7]->set_data_by_str("0.5");   // ratio从0.0改为0.5
        variables[8]->set_data_by_str("2.71828"); // pi从3.14159改为2.71828

        // 打印所有变量
        for (const auto& var : variables) {
            std::cout << "Name: " << var->get_name()
                << ", Type: " << type2Str(var->get_type())
                << ", Value: " << var->get_data_to_str() << std::endl;
        }

        // 测试错误处理
        try {
            variables[0]->set_data_by_str("abc");  // 无效int值
        }
        catch (const std::runtime_error& e) {
            std::cout << "Error: " << e.what() << std::endl;
        }

        try {
            variables[3]->set_data_by_str("AB");   // 无效char值
        }
        catch (const std::runtime_error& e) {
            std::cout << "Error: " << e.what() << std::endl;
        }

        try {
            variables[6]->set_data_by_str("xyz");  // 无效char值
        }
        catch (const std::runtime_error& e) {
            std::cout << "Error: " << e.what() << std::endl;
        }


	}

    void test_func() {      
        std::string returnType = "int";
        std::string name = "mainFunc";
        std::string formalArgs = "int arg1,int arg2";
        std::string funcStmt =
            " int a = arg1;"
            "int b = 2;"
            "a = b;"
            "int c = a+arg2;"
            "return c+arg1; ";
        FuncBlock testFunc(returnType,name,formalArgs,funcStmt);
        testFunc.run_func("1,2",nullptr);
        //testFunc.display_var_map();
        DisplayVar(testFunc.get_return_var());
    }

    void test_funCall() {
        std::string returnType1 = "int";
        std::string name1 = "sum";
        std::string formalArgs1 = "int a,int b";
        std::string funcStmt1 =
            " return doNothing(a)+b";
        FuncBlock fun1(returnType1, name1, formalArgs1, funcStmt1);


        std::string returnType2 = "int";
        std::string name2 = "doNothing";
        std::string formalArgs2 = "int b";
        std::string funcStmt2 =
            " return b";
        FuncBlock fun2(returnType2, name2, formalArgs2, funcStmt2);

        FuncMap::get_func_map().add_func(fun1.get_name(), &fun1);
        FuncMap::get_func_map().add_func(fun2.get_name(), &fun2);

        //FuncMap::show_func();

        std::string returnType3 = "int";
        std::string name3 = "mainFunc";
        std::string formalArgs3 = "int arg1, int arg2, int arg3";
        std::string funcStmt3 =
            " int a = 100 ;"
            " a = sum(arg1, arg2);"
            "int b = sum(a, arg3);"
            "int c = b;"
            "return c;";
        FuncBlock mainFunc(returnType3, name3, formalArgs3, funcStmt3);
        
        mainFunc.run_func("100,100,100",nullptr);
        DisplayVar(mainFunc.get_return_var());
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
        FuncBlock fun1(returnType1, name1, formalArgs1, funcStmt1);
        fun1.run_func("1,2", nullptr);
        DisplayVar(fun1.get_return_var());
    }

    void test_RefAndPtr() {
        std::string returnType1 = "void";
        std::string name1 = "sum";
        std::string formalArgs1 = "int& a,int& b,int& out";
        std::string funcStmt1 =
            " int* c = doNothing(&a);"
            "int& c1 = *c;"
            "out = c1 + b; ";
        FuncBlock fun1(returnType1, name1, formalArgs1, funcStmt1);


        std::string returnType2 = "int*";
        std::string name2 = "doNothing";
        std::string formalArgs2 = "int* a";
        std::string funcStmt2 =
            " return a";
        FuncBlock fun2(returnType2, name2, formalArgs2, funcStmt2);

        FuncMap::get_func_map().add_func(fun1.get_name(), &fun1);
        FuncMap::get_func_map().add_func(fun2.get_name(), &fun2);

        //FuncMap::show_func();

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
        FuncBlock mainFunc(returnType3, name3, formalArgs3, funcStmt3);

        mainFunc.run_func("1,2,3", nullptr);
        DisplayVar(mainFunc.get_return_var());
    }

    void test_extractFunc() {
        std::map<std::string, FuncInfo> funcMap = ExtractFileInfo("E:/hw2/testGroups/group1/cppCode3.cpp").allFuncInfo;

        if (funcMap.find("mainFunc") != funcMap.end()) {
            FuncInfo mainFunc = funcMap["mainFunc"];
            std::cout << "Function: " << mainFunc.funcName << "\n"
                << "Return Type: " << mainFunc.returnType << "\n"
                << "Arguments: " << mainFunc.formalArgs << "\n"
                << "Body: " << mainFunc.funcStmt << std::endl;
        }
    }
    void test_file(std::string filePath, std::string actualArgs, bool display = false) {
        FileInfo fileInfo = ExtractFileInfo(filePath);
        std::map<std::string, FuncInfo> funcInfoMap = fileInfo.allFuncInfo;
        auto classInfo = fileInfo.allClassInfo;
        ClassManager::get_manager().set_class_info(classInfo);
        //ClassManager::get_manager().show_class();

        for (auto& func : funcInfoMap) {
            std::string returnType = func.second.returnType;
            std::string name = func.second.funcName;
            std::string formalArgs = func.second.formalArgs;
            std::string funcStmt = func.second.funcStmt;
            FuncBlock* fun = new FuncBlock(returnType, name, formalArgs, funcStmt);
            FuncMap::get_func_map().add_func(fun->get_name(), fun);
        }

        FuncBlock* mainFunc = FuncMap::get_func_map().create_func("mainFunc");
        (*mainFunc).run_func(actualArgs, nullptr);
        if (display) {
            //std::cout << "test file:" << filePath << std::endl;
            //DisplayVar(mainFunc->get_return_var());
            std::cout << "output return value:" << mainFunc->get_return_var()->get_data_to_str() << std::endl;
            std::cout << std::endl;
        }
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

    void self_test_int(bool display=false) {
        std::string file_path = "E:/hw2/testGroups/self/testcode_int.cpp";
        std::string actualArgs = "0,0";
        test_file(file_path, actualArgs,display);
    }
    void self_test_double(bool display = false) {
        std::string file_path = "E:/hw2/testGroups/self/testcode_double.cpp";
        std::string actualArgs = "0,0";
        test_file(file_path, actualArgs, display);
    }
    void self_test_char(bool display = false) {
        std::string file_path = "E:/hw2/testGroups/self/testcode_char.cpp";
        std::string actualArgs = "'a'";
        test_file(file_path, actualArgs, display);
    }

    void self_test_bool(bool display = false) {
        std::string file_path = "E:/hw2/testGroups/self/testcode_bool.cpp";
        std::string actualArgs = "true";
        test_file(file_path, actualArgs, display);
    }


    void test_hw2_file(std::string filePath, std::string paraPath, bool display = false) {
        std::ifstream input(paraPath);
        if (display) {
            std::cout << "-------------------------------------------------------" << std::endl;
            std::cout << "test file :" << filePath << std::endl;
            std::cout << "param path:" << paraPath << std::endl;
        }
        std::string line;
        size_t i = 0;
        while (std::getline(input, line)) {
            if (display) {
                std::cout << "----------test file " << i++ << "------------" << std::endl;
                std::cout << "expected result: " << line << std::endl;
            }
            size_t parenthesisBegin = line.find('(');
            size_t parenthesisEnd = line.find(')');
            std::string params = Trim(line.substr(parenthesisBegin + 1, parenthesisEnd - parenthesisBegin - 1));
            test_file(filePath, params, display);
        }
        

    }
    void hw2_test(bool display=false) {
        //test_hw2_file("E:/题目/第一题/Q1-1.cpp","E:/题目/第一题/Q1-1.txt",display);
        //test_hw2_file("E:/题目/第一题/Q1-2.cpp", "E:/题目/第一题/Q1-2.txt",display);
        //test_hw2_file("E:/题目/第一题/Q1-3.cpp", "E:/题目/第一题/Q1-3.txt",display);


        //test_hw2_file("E:/题目/第二题/Q2-1.cpp", "E:/题目/第二题/Q2-1.txt",display);
        //test_hw2_file("E:/题目/第二题/Q2-2.cpp", "E:/题目/第二题/Q2-2.txt",display);
        //test_hw2_file("E:/题目/第二题/Q2-3.cpp", "E:/题目/第二题/Q2-3.txt",display);

        test_hw2_file("E:/题目/第三题/Q3-1.cpp", "E:/题目/第三题/Q3-1.txt",display);
        test_hw2_file("E:/题目/第三题/Q3-2.cpp", "E:/题目/第三题/Q3-2.txt",display);
        test_hw2_file("E:/题目/第三题/Q3-3.cpp", "E:/题目/第三题/Q3-3.txt",display);
    }
}
int main() {  
    Logger::SetLogFile("log.txt", false);
    Logger::SetLogLevel(Logger::LogLevel::Info);
	//test::test_cal();
    //test::test_var();
    //test::test_func();
    //test::test_funCall();
    //test::test_EasyRefAndPtr();
    //test::test_RefAndPtr();
    //test::test_extractFunc();
    //test::test_pipeLine();
    //test::test_char();
    //test::test_double();
    //test::self_test_double();
    //test::self_test_char();
    //{
    //    TimeGuard guard("loop 100 times:");
    //    //for (int i = 0; i < 100; i++) {
    //    //    test::self_test_bool();
    //    //    test::self_test_int();
    //    //}
    //    //test::self_test_bool(true);
    //    test::self_test_int(true);
    //    //test::self_test_double(true);
    //    //test::self_test_char(true);
    //}
    //test::self_test_int(true);
    {
        TimeGuard guard("loop 1 times:");
        for (int i = 0; i < 100; i++) {
            test::hw2_test(false);
        }
    }


}