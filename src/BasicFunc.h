#pragma once
#ifndef __BASICFUNC__
#define __BASICFUNC__

#include <BasicVar.h>
#include <BasicBlock.h>
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


	class FuncBlock :public Block{
	public:

		FuncBlock(const std::string& returnType, const std::string& name, const std::string& formalArgs, const std::string& funcStmt)
			:Block(funcStmt),
			m_name(name),
			m_formalArgs(formalArgs) {
			m_type = BlockType::FunBlock;
			create_return_var(returnType);
		};
		FuncBlock(const FuncBlock& func): Block(func.m_blockBody) {
			m_type = BlockType::FunBlock;
			m_name = func.m_name;
			m_formalArgs = func.m_formalArgs;
			VarType returnType = func.get_return_var()->get_type();
			create_return_var(type2Str(returnType));
		};


		std::shared_ptr<VarBase> run_func(const std::string& actualArgs, Block* parent);

		void set_parent_func(Block* parent) {
			m_parentBlock = parent;
		}
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



	private:

		std::string m_name;
		std::string m_formalArgs;       // 形参语句
		std::shared_ptr<VarBase> m_returnVar;

		void set_args(const std::string& actualArgs);
		void create_return_var(const std::string& returnTypeStr);
		void parse_function_body();

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


		void add_func(const std::string& name, FuncBlock* func) {
			std::lock_guard<std::mutex> lock(m_mutex);
			m_funcMap[name] = func;
		}

		// 支持重载 name+actualArgs组成唯一标识 name+formalArgs也组成唯一标识
		std::shared_ptr<VarBase> call_func(const std::string name, const std::string& actualArgs, Block* parent) {

			auto it = m_funcMap.find(name);
			if (it == m_funcMap.end()) {
				throw std::runtime_error("Function not found: " + name);
			}
			FuncBlock copyFunc(*(it->second));

			return copyFunc.run_func(actualArgs,parent);

		}

		FuncBlock* create_func(std::string name) {
			auto it = m_funcMap.find(name);
			if (it == m_funcMap.end()) {
				throw std::runtime_error("Function not found: " + name);
			}
			FuncBlock* newFunc = new FuncBlock(*(it->second));
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
		std::map <std::string, FuncBlock*> m_funcMap;
	};


}
#endif // !__BASICFUNC__
