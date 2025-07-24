#pragma once
#ifndef __CONTROLBLOCK__
#define __CONTROLBLOCK__
#include <BasicBlock.h>

namespace thz {
	
	
	class LoopBlock :public Block {
	public:

		LoopBlock(std::string blockBody,Block* parent):Block(blockBody,parent) {
			m_type = BlockType::LoopBlock;
		}

		void run_loop() {
			parse_loop_for();
		}
	private:
		void parse_loop_for();

	};

	class IfBlock : public Block {

	};
}

#endif