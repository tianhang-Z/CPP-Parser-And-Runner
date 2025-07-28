#pragma once
#ifndef __CONTROLBLOCK__
#define __CONTROLBLOCK__
#include <BasicBlock.h>
#include <BasicClass.h>

namespace thz {
	
	
	class LoopBlock :public Block {
	public:

		LoopBlock(std::string blockBody,Block* parent):Block(blockBody,parent) {
			m_type = BlockType::LoopBlock;
			m_owner = nullptr;
		}

		void run_loop() {
			parse_loop_for();
		}
		void set_owner(BasicClass* owner) {
			m_owner = owner;
		}
		BasicClass* get_owner() {
			return m_owner;
		}
		std::shared_ptr<VarBase> find_var(std::string varName) {
			std::shared_ptr<VarBase> memberVar=nullptr;
			if (m_owner) {
				memberVar = m_owner->find_var(varName);
			}
			std::shared_ptr<VarBase> blockVar = nullptr;
			blockVar = Block::find_var(varName);
			if (blockVar && memberVar)
				throw std::runtime_error("redifine var ");
			else if (memberVar)
				return memberVar;
			else
				return blockVar;
		}
	private:
		void parse_loop_for();
		BasicClass* m_owner;

	};

	class IfBlock : public Block {
	public :
		IfBlock(std::string blockbody, Block* parent) :Block(blockbody, parent) {
			m_type = BlockType::IfBlock;
			m_owner = nullptr;
		}

		void run_if() {
			parse_if_block();
		}
		void set_owner(BasicClass* owner) {
			m_owner = owner;
		}
		BasicClass* get_owner() {
			return m_owner;
		}
		std::shared_ptr<VarBase> find_var(std::string varName) {
			std::shared_ptr<VarBase> memberVar = nullptr;
			if (m_owner) {
				memberVar = m_owner->find_var(varName);
			}
			std::shared_ptr<VarBase> blockVar = nullptr;
			blockVar = Block::find_var(varName);
			if (blockVar && memberVar)
				throw std::runtime_error("redifine var ");
			else if (memberVar)
				return memberVar;
			else
				return blockVar;
		}

	private:
		void parse_if_block();
		BasicClass* m_owner;
	};
}

#endif