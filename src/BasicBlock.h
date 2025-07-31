#pragma once 
#ifndef __BASICBLOCK__
#define __BASICBLOCK__
#include <BasicVar.h>
#include <Calculator.h>

namespace thz {
	class BasicClass;
	using ClassMap = std::map<std::string, std::shared_ptr<BasicClass>>;

	bool IsVarDeclStmt(const std::string& stmt);
	bool IsAssignStmt(const std::string& stmt);
	bool IsReturnStmt(const std::string& stmt);
	bool IsFunCall(const std::string& stmt);

	enum class BlockType :unsigned {
		BasicBlock,
		FunBlock,
		LoopBlock,
		IfBlock,
		InClass,
	};


	class Block {
	public:
		friend class Calculator;

		Block(std::string blockBody);
		Block(std::string blockBody, Block* parentBlock);

		BlockType get_block_type(){
			return m_type;
		}
		VarMap& get_var_map() {
			return m_varMap;
		}
		const VarMap& get_var_map() const {
			return m_varMap;
		}
		std::string get_block_body() const {
			return m_blockBody;
		}
		Calculator& get_calc() {
			return m_calc;
		}

		virtual BasicClass* get_owner() {
			return nullptr;
		}

		/*
		block可以是func control_block 共同点是管理一些变量，可以声明和赋值，可以提前return，其中可以嵌套block
		block可以从父block查找var并对其赋值
		*/
		void parse_statement(const std::string& stmt);
		void parse_block_body(const std::string& blockBody);
		void parse_assignment(const std::string& stmt);
		void do_assignment(std::shared_ptr<VarBase>& leftVar, std::string& rightExpr, bool leftDeref);
		void parse_variable_declaration(const std::string& stmt);
		void parse_class_decl(const std::string& stmt);

		virtual std::shared_ptr<VarBase> find_var(std::string varName);

		std::shared_ptr<BasicClass> find_class(std::string clsName);

		// 支持向上查找父block 找到函数的block
		virtual std::shared_ptr<VarBase> get_return_var();

		void parse_return_statement(const std::string& stmt);        // 从父block获取return_var，若为funcBlock，本身有return_var

		std::shared_ptr<VarBase> create_args_by_parent_var(VarType type, std::string name, std::string argValue);
		std::shared_ptr<VarBase> create_var_by_self_var(VarType type, std::string name, std::string argValue);
		std::shared_ptr<VarBase> create_var_by_funcall_ret(std::string funcExpr);

		virtual ~Block() = default;
	protected:

		BlockType m_type;
		VarMap m_varMap;
		ClassMap m_classMap;
		std::string m_blockBody;
		Calculator m_calc;
		Block* m_parentBlock;

	};

} // namespace thz

#endif