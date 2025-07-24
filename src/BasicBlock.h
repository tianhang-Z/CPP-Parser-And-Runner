
#include <BasicVar.h>
#include <Calculator.h>

namespace thz {


	bool IsVarDeclStmt(const std::string& stmt);
	bool IsAssignStmt(const std::string& stmt);
	bool IsReturnStmt(const std::string& stmt);
	bool IsFunCall(const std::string& stmt);

	enum class BlockType :unsigned {
		BasicBlock,
		FunBlock,
		LoopBlock,
	};

	class Block {
	public:
		friend class Calculator;



		Block(std::string blockBody) :m_type(BlockType::BasicBlock),m_varMap(), m_blockBody(blockBody), m_calc(this) {
			m_parentBlock = nullptr;
		};

		Block(std::string blockBody,Block* parentBlock) :m_type(BlockType::BasicBlock), m_varMap(), m_blockBody(blockBody),m_calc(this),m_parentBlock(parentBlock) {};

		BlockType get_block_type(){
			return m_type;
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



		/*
		block可以是func control_block 共同点是管理一些变量，可以声明和赋值，可以提前return，其中可以嵌套block
		block可以从父block查找var并对其赋值
		*/
		void parse_statement(const std::string& stmt);
		void parse_block_body(const std::string& blockBody);
		void parse_assignment(const std::string& stmt);
		void do_assignment(std::shared_ptr<VarBase>& leftVar, std::string& rightExpr, bool leftDeref);
		void parse_variable_declaration(const std::string& stmt);

		std::shared_ptr<VarBase> find_var(std::string varName);
		
		// 支持向上查找父block 找到函数的block
		virtual std::shared_ptr<VarBase> get_return_var();

		void parse_return_statement(const std::string& stmt);        // 从父block获取return_var，若为funcBlock，本身有return_var
		void parse_loop_for(const std::string& stmt) ; 

		std::shared_ptr<VarBase> create_args_by_parent_var(VarType type, std::string name, std::string argValue);
		std::shared_ptr<VarBase> create_var_by_self_var(VarType type, std::string name, std::string argValue);
		std::shared_ptr<VarBase> create_var_by_funcall_ret(std::string funcExpr);

	protected:

		BlockType m_type;
		VarMap m_varMap;
		std::string m_blockBody;
		Calculator m_calc;
		Block* m_parentBlock;

	};

} // namespace thz