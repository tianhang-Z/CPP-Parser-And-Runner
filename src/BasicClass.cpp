#include <BasicClass.h>

namespace thz {
	std::shared_ptr<VarBase> MemberFunc::find_var(std::string name) {
		auto memberVar = m_owner->find_var(name);
		auto blockVar = Block::find_var(name);
		if (memberVar && blockVar)
			throw std::runtime_error("redefine var");
		else if (memberVar)
			return memberVar;
		else
			return blockVar;

	}
}
