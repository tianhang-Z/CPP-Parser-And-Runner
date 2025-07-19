#pragma once
#ifndef __BASICFUNC__
#define __BASICFUNC__

#include <BasicVar.h>
#include <Calculator.h>
#include <logger.h>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <mutex>
#include <sstream>
#include <algorithm>
#include <cctype>

namespace thz {

	class FuncBase {
	public:

		FuncBase(const std::string& returnType, const std::string& name, const std::string& formalArgs, const std::string& funcStmt)
			:m_varMap(),
			m_calc(&m_varMap),  // 使用m_varMap的地址初始化m_calc
			m_name(name),
			m_formalArgs(formalArgs), m_funcStatements(funcStmt) {
			setReturnVar(returnType);
		};
		FuncBase(const FuncBase& func) {
			m_varMap = std::map<std::string, std::shared_ptr<VarBase>>();
			m_calc = Calculator(&m_varMap);
			m_name = func.m_name;
			m_formalArgs = func.m_formalArgs;
			m_funcStatements = func.m_funcStatements;
			VarType returnType = func.getReturnVar()->getType();
			setReturnVar(typeStr(returnType));
		};


		std::shared_ptr<VarBase> runFunc(const std::string& actualArgs,FuncBase* parent);
		std::shared_ptr<VarBase> getReturnVar() {
			return m_returnVar;
		}
		const std::shared_ptr<VarBase> getReturnVar() const {
			return m_returnVar;
		}
		const std::string& getName() {
			return m_name;
		}
		const std::string& getFormalArgs() {
			return m_formalArgs;
		}
		void displayVarMap() {
			for (auto& ele : m_varMap) {
				displayVar(ele.second);
			}
		}
		void setParentFunc(FuncBase* parent) {
			parentFunc = parent;
		}
		__var_map__& getVarMap() {
			return m_varMap;
		}
		const __var_map__& getVarMap() const {
			return m_varMap;
		}
		Calculator& getCalc() {
			return m_calc;
		}

	private:
		FuncBase* parentFunc;
		Calculator m_calc;        // 每个func有自己的calculator

		__var_map__ m_varMap;

		std::string m_name;
		std::string m_formalArgs;       // 形参语句
		std::string m_funcStatements;
		std::shared_ptr<VarBase> m_returnVar;

		// 辅助方法

		void setArgs(const std::string& actualArgs);
		std::shared_ptr<VarBase> createArgsByParentVar(VarType type, std::string name, std::string argValue);
		std::shared_ptr<VarBase> createVarBySelfVar(VarType type, std::string name, std::string argValue);
		std::shared_ptr<VarBase> createVarByFunCallRet(std::string funcExpr);

		void setReturnVar(const std::string& returnTypeStr);
		void parseFunctionBody();
		void parseStatement(const std::string& stmt);
		void parseVariableDeclaration(const std::string& stmt);
		void parseAssignment(const std::string& stmt);
		void doAssignment(std::shared_ptr<VarBase> leftVar, const std::string& rightExpr, bool leftDeref=false);
		void parseReturnStatement(const std::string& stmt);
	};


	class FuncMap {
	public:
		FuncMap(const FuncMap&) = delete;
		FuncMap& operator=(const FuncMap&) = delete;

		static FuncMap& getFuncMap() {
			static FuncMap instance;
			return instance;
		}


		void addFunc(const std::string& name, FuncBase* func) {
			std::lock_guard<std::mutex> lock(m_mutex);
			m_funcMap[name] = func;
		}
		std::shared_ptr<VarBase> callFunc(const std::string name, const std::string& actualArgs, FuncBase* parent) {

			auto it = m_funcMap.find(name);
			if (it == m_funcMap.end()) {
				throw std::runtime_error("Function not found: " + name);
			}
			FuncBase copyFunc(*(it->second));

			return copyFunc.runFunc(actualArgs,parent);

		}
		FuncBase* createFunc(std::string name) {
			auto it = m_funcMap.find(name);
			if (it == m_funcMap.end()) {
				throw std::runtime_error("Function not found: " + name);
			}
			FuncBase* newFunc = new FuncBase(*(it->second));
			return newFunc;

		}
		void showFunc() {
			for (auto ele : m_funcMap) {
				std::cout << "name: " << ele.first << std::endl;
				std::cout << "params:" << ele.second->getFormalArgs() << std::endl;
			}
		}
	private:
		FuncMap() = default;
		// 互斥锁保证线程安全
		std::mutex m_mutex;
		std::map <std::string, FuncBase*> m_funcMap;
	};


}
#endif // !__BASICFUNC__
