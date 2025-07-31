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

		FuncBlock(const std::string& returnType, const std::string& name,
			const std::string& formalArgs, const std::string& funcStmt);

		FuncBlock(const FuncBlock& func);


		std::shared_ptr<VarBase> run_func(const std::string& actualArgs, Block* parent);

		void set_parent_func(Block* parent) {
			m_parentBlock = parent;
		}
		std::shared_ptr<VarBase> get_return_var() {
			return m_returnVar;
		}
		std::string get_return_typestr() const {
			return type2Str(m_returnVar->get_type());
		}
		const std::shared_ptr<VarBase> get_return_var() const {
			return m_returnVar;
		}
		const std::string& get_name() const  {
			return m_name;
		}
		const std::string& get_formal_args() const {
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
		std::shared_ptr<VarBase> call_func(const std::string name, const std::string& actualArgs, Block* parent);

		FuncBlock* create_func(std::string name);
		void show_func();

	private:
		FuncMap() = default;
		// 互斥锁保证线程安全
		std::mutex m_mutex;
		std::map <std::string, FuncBlock*> m_funcMap;
	};


}
#endif // !__BASICFUNC__
