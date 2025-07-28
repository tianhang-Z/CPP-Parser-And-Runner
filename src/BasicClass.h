#ifndef __BASICCLASS__
#define __BASICCLASS__

#include <BasicFunc.h>
#include <FileTools.h>

namespace thz {

	/*
	singleton
	*/
	class ClassManager {
	public:
		ClassManager(const ClassManager&) = delete;
		ClassManager& operator=(const ClassManager&) = delete;

		static ClassManager& get_manager() {
			static ClassManager instance;
			return instance;
		}


		void set_class_info(std::map<std::string, ClassInfo> info) {
			m_initMap = info;
		}

		bool find_class_type(const std::string& name) {
			if (m_initMap.find(name) != m_initMap.end()) {
				return true;
			}
			else return false;
		}

		std::shared_ptr<BasicClass> create_class(const std::string& className, const std::string typeName) {

			auto it = m_initMap.find(typeName);
			if (it == m_initMap.end()) {
				throw std::runtime_error("Class not found: " + typeName);
			}
			return std::make_shared<BasicClass>(className, it->second.funcMap,it->second.varInfo);

		}

		void show_class() {
			for (auto ele : m_initMap) {
				std::cout << ele.second;
			}
		}

	private:
		ClassManager() = default;
		std::mutex m_mutex;
		std::map<std::string, ClassInfo> m_initMap;

	};


	class BasicClass;

	class MemberFunc : public FuncBlock {
	public:

		MemberFunc(const std::string& returnType, const std::string& name,
			const std::string& formalArgs, const std::string& funcStmt, BasicClass* owner)
			:FuncBlock(returnType, name, formalArgs, funcStmt) {
			m_owner = owner;
			m_type = BlockType::InClass;
			m_calc.set_block(this);
		};
		MemberFunc(const MemberFunc& func, BasicClass* owner)
			: FuncBlock(func.get_return_typestr(), func.get_name(), func.get_formal_args(), func.get_block_body()) {
			m_owner = owner;
			m_type = BlockType::InClass;
			m_calc.set_block(this);

		};

		BasicClass* get_owner() {
			return m_owner;
		}

		std::shared_ptr<VarBase> find_var(std::string name);
	private:
		BlockType m_type;
		BasicClass* m_owner;
	};


	class MemberFuncManager {
	public:
		MemberFuncManager() {
		}

		void add_func(const std::string& name, MemberFunc* func) {
			std::lock_guard<std::mutex> lock(m_mutex);
			m_funcMap[name] = func;
		}


		std::shared_ptr<VarBase> call_func(const std::string name, const std::string& actualArgs, Block* parent) {

			auto it = m_funcMap.find(name);
			if (it == m_funcMap.end()) {
				throw std::runtime_error("Function not found: " + name);
			}
			MemberFunc copyFunc(*(it->second));

			return copyFunc.run_func(actualArgs, parent);

		}

		MemberFunc* create_func(std::string name) {
			auto it = m_funcMap.find(name);
			if (it == m_funcMap.end()) {
				throw std::runtime_error("Function not found: " + name);
			}
			MemberFunc* newFunc = new MemberFunc(*(it->second));
			return newFunc;

		}

		void show_func() {
			for (auto ele : m_funcMap) {
				std::cout << "name: " << ele.first << std::endl;
				std::cout << "params:" << ele.second->get_formal_args() << std::endl;
			}
		}

	private:

		// 互斥锁保证线程安全
		std::mutex m_mutex;
		std::map <std::string, MemberFunc*> m_funcMap;
	};

	class BasicClass  {
	public:
		// 构造class对象
		BasicClass(const std::string& className,std::map<std::string, FuncInfo> funcMap, std::vector<std::string> varInfo) {
			m_name = className;
			for (auto& funcInfo : funcMap) {
				std::string returnType = funcInfo.second.returnType;
				std::string name = funcInfo.second.funcName;
				std::string formalArgs = funcInfo.second.formalArgs;
				std::string funcStmt = funcInfo.second.funcStmt;
				MemberFunc* fun = new MemberFunc(returnType, name, formalArgs, 
					         funcStmt, this);
				m_funcManager.add_func(fun->get_name(), fun);
			}

			for (auto& varDecl : varInfo) {
				/* varInfo格式为
				int a
				char c
				*/
				std::vector<std::string> parts = Split(varDecl, ' ');
				if (parts.size() < 2) {
					throw std::runtime_error("Invalid variable declaration: " + varDecl);
				}
				std::string typeStr = Trim(parts[0]);
				std::string name = Trim(parts[1]);
				VarType type = str2Type(typeStr);
				std::shared_ptr<VarBase> var;
				if (IsPtr(type)) // 解析int* a;
					var = CreateVariablePtr(type, name, nullptr);
				else  //解析 int a;
					var = CreateVariable(type, name);
				m_memberVar[name] = var;
			}

		}



		std::shared_ptr<VarBase> call_func(const std::string name, const std::string& actualArgs, Block* parent) {
			return m_funcManager.call_func(name, actualArgs, parent);
		}

		// 只查找class管理的member_var;  供外界访问和供成员函数访问
		std::shared_ptr<VarBase> find_var(std::string name) {
			auto it = m_memberVar.find(name);
			if (it == m_memberVar.end())
				return nullptr;
			else
				return it->second;
		}
	private:
		std::string m_name;
		VarMap m_memberVar;
		MemberFuncManager m_funcManager;
	};

	using ClassMap = std::map<std::string, std::shared_ptr<BasicClass>>;

	



}

#endif
