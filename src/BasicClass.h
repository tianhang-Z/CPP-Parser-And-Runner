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

		bool find_class_type(const std::string& name);

		std::shared_ptr<BasicClass> create_class(const std::string& className, const std::string typeName);

		void show_class();

	private:
		ClassManager() = default;
		std::mutex m_mutex;
		std::map<std::string, ClassInfo> m_initMap;

	};


	class BasicClass;

	class MemberFunc : public FuncBlock {
	public:

		MemberFunc(const std::string& returnType, const std::string& name,
			const std::string& formalArgs, const std::string& funcStmt, BasicClass* owner);
			
		MemberFunc(const MemberFunc& func, BasicClass* owner);

		BasicClass* get_owner() {
			return m_owner;
		}

		std::shared_ptr<VarBase> find_var(std::string name);
	private:
		BasicClass* m_owner;
	};


	class MemberFuncManager {
	public:
		MemberFuncManager() {
		}

		void add_func(const std::string& name, MemberFunc* func);

		std::shared_ptr<VarBase> call_func(const std::string name, const std::string& actualArgs, Block* parent);
		
		MemberFunc* create_func(std::string name);

		void show_func();

	private:

		// 互斥锁保证线程安全
		std::mutex m_mutex;
		std::map <std::string, MemberFunc*> m_funcMap;
	};

	class BasicClass  {
	public:
		// 构造class对象
		BasicClass(const std::string& className, std::map<std::string, FuncInfo> funcMap, std::vector<std::string> varInfo);


		std::shared_ptr<VarBase> call_func(const std::string name, const std::string& actualArgs, Block* parent) {
			return m_funcManager.call_func(name, actualArgs, parent);
		}

		// 只查找class管理的member_var;  供外界访问和供成员函数访问
		std::shared_ptr<VarBase> find_var(std::string name);

	private:
		std::string m_name;
		VarMap m_memberVar;
		MemberFuncManager m_funcManager;
	};

	using ClassMap = std::map<std::string, std::shared_ptr<BasicClass>>;

	



}

#endif
