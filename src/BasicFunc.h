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
			m_calc(&m_varMap,this),  // 使用m_varMap的地址初始化m_calc
			m_name(name),
			m_formalArgs(formalArgs), m_funcStatements(funcStmt) {
			set_return_var(returnType);
		};
		FuncBase(const FuncBase& func) {
			m_varMap = std::map<std::string, std::shared_ptr<VarBase>>();
			m_calc = Calculator(&m_varMap,this);
			m_name = func.m_name;
			m_formalArgs = func.m_formalArgs;
			m_funcStatements = func.m_funcStatements;
			VarType returnType = func.get_return_var()->get_type();
			set_return_var(type2Str(returnType));
		};


		std::shared_ptr<VarBase> run_func(const std::string& actualArgs,FuncBase* parent);
		std::shared_ptr<VarBase> get_return_var() {
			return m_returnVar;
		}
		const std::shared_ptr<VarBase> get_return_var() const {
			return m_returnVar;
		}
		const std::string& get_name() {
			return m_name;
		}
		const std::string& get_formal_args() {
			return m_formalArgs;
		}
		void display_var_map() {
			for (auto& ele : m_varMap) {
				DisplayVar(ele.second);
			}
		}
		void set_parent_func(FuncBase* parent) {
			parentFunc = parent;
		}
		VarMap& get_var_map() {
			return m_varMap;
		}
		const VarMap& get_var_map() const {
			return m_varMap;
		}
		Calculator& get_calc() {
			return m_calc;
		}

	private:
		FuncBase* parentFunc;
		Calculator m_calc;        // 每个func有自己的calculator

		VarMap m_varMap;

		std::string m_name;
		std::string m_formalArgs;       // 形参语句
		std::string m_funcStatements;
		std::shared_ptr<VarBase> m_returnVar;

		// 辅助方法

		void set_args(const std::string& actualArgs);
		std::shared_ptr<VarBase> create_args_by_parentv_var(VarType type, std::string name, std::string argValue);
		std::shared_ptr<VarBase> create_var_by_self_var(VarType type, std::string name, std::string argValue);
		std::shared_ptr<VarBase> create_var_by_funcall_ret(std::string funcExpr);

		void set_return_var(const std::string& returnTypeStr);
		void parse_braces_body(const std::string& bracesBody);
		void parse_function_body();
		void parse_statement(const std::string& stmt);
		void parse_variable_declaration(const std::string& stmt);
		void parse_assignment(const std::string& stmt);
		void parse_loop_for(const std::string& stmt);
		void do_assignment(std::shared_ptr<VarBase>& leftVar, std::string& rightExpr, bool leftDeref=false);
		void parse_return_statement(const std::string& stmt);
	};

	/*
	singleton
	*/
	class FuncMap {
	public:
		FuncMap(const FuncMap&) = delete;
		FuncMap& operator=(const FuncMap&) = delete;

		static FuncMap& get_func_map() {
			static FuncMap instance;
			return instance;
		}


		void add_func(const std::string& name, FuncBase* func) {
			std::lock_guard<std::mutex> lock(m_mutex);
			m_funcMap[name] = func;
		}

		// 支持重载 name+actualArgs组成唯一标识 name+formalArgs也组成唯一标识
		std::shared_ptr<VarBase> call_func(const std::string name, const std::string& actualArgs, FuncBase* parent) {

			auto it = m_funcMap.find(name);
			if (it == m_funcMap.end()) {
				throw std::runtime_error("Function not found: " + name);
			}
			FuncBase copyFunc(*(it->second));

			return copyFunc.run_func(actualArgs,parent);

		}
		FuncBase* create_func(std::string name) {
			auto it = m_funcMap.find(name);
			if (it == m_funcMap.end()) {
				throw std::runtime_error("Function not found: " + name);
			}
			FuncBase* newFunc = new FuncBase(*(it->second));
			return newFunc;

		}
		void show_func() {
			for (auto ele : m_funcMap) {
				std::cout << "name: " << ele.first << std::endl;
				std::cout << "params:" << ele.second->get_formal_args() << std::endl;
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
