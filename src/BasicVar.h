#pragma once
#ifndef __BASICVAR__
#define  __BASICVAR__
#include <string>
#include <cassert>
#include <stdexcept>
#include <memory>
#include <set>
#include <iostream>
#include <sstream>
#include <map>

namespace thz {

	class VarBase;

#define TYPEMAP(XX)  \
	XX(VOID,"void") \
	XX(INT,"int")    \
	XX(INT_REF,"int&") \
	XX(INT_PTR,"int*")  \
	XX(DOUBLE,"double")  \
	XX(DOUBLE_REF,"double&")  \
	XX(DOUBLE_PTR,"double*")  \
	XX(CHAR,"char")  \
	XX(CHAR_REF,"char&")  \
	XX(CHAR_PTR,"char*")  \
	XX(NOTYPE,"no_type")

	enum class VarType : unsigned int {
#define GEN_TYPE(t,s) t,
		TYPEMAP(GEN_TYPE)
#undef GEN_TYPE
	};

	inline const char* type2Str(VarType type) {
		static const char* typeTab[] = {
	#define GEN_TYPESTR(t,s) s,
			TYPEMAP(GEN_TYPESTR)
	#undef GEN_TYPESTR
		};

		assert(unsigned(type) >= 0 && unsigned(type) < sizeof(typeTab) / sizeof(typeTab[0]));
		return typeTab[unsigned(type)];
	};


	inline VarType str2Type(const std::string& str) {
		static const std::map<std::string, VarType> typeMap = {
	#define GEN_TYPEPAIR(t, s) {s, VarType::t},
			TYPEMAP(GEN_TYPEPAIR)
	#undef GEN_TYPEPAIR
		};

		auto it = typeMap.find(str);
		if (it != typeMap.end()) {
			return it->second;
		}
		throw std::runtime_error("Unsupported parameter type: " + str);
	}


	typedef std::map<std::string, std::shared_ptr<VarBase>> __var_map__;

	bool isBasicVar(const std::string& type);
	bool isPtr(VarType type);
	bool isRef(VarType type);
	VarType getReferenceType(VarType baseType);
	VarType getPointerType(VarType baseType);

	std::shared_ptr<VarBase> createVariable(VarType type, const std::string& name, const std::string& initialValue = "");
	std::shared_ptr<VarBase> createVariable(VarType type, const std::string& name, double value);
	std::shared_ptr<VarBase> createVariable(VarType type, const std::string& name, std::shared_ptr<VarBase> value);


	std::shared_ptr<VarBase> createVariableRef(VarType type, const std::string& name, std::shared_ptr<VarBase> tar);
	std::shared_ptr<VarBase> createVariablePtr(VarType type, const std::string& name, std::shared_ptr<VarBase> tar,bool shareData=false);


	std::shared_ptr<VarBase> createVarByTemp(VarType type, std::string paramName, std::string argValue);
	std::shared_ptr<VarBase> createVarByVar(VarType type, std::string name, std::shared_ptr<VarBase> rightVar,
		bool createPtr = false, bool createRef = false, bool getAddr = false, bool deRef = false);
	std::shared_ptr<VarBase> createVarByVarMap(VarType type, std::string name, std::string argValue, __var_map__ varMap);


	std::shared_ptr<VarBase> deReference(std::shared_ptr<VarBase> var);
	void rePtr(std::shared_ptr<VarBase> var, std::shared_ptr<VarBase> target);
	void sharePtr(std::shared_ptr<VarBase> var, std::shared_ptr<VarBase> target);
	void shareRef(std::shared_ptr<VarBase> var, std::shared_ptr<VarBase> target);

	bool isRef(VarType type);
	void setBasicVar(std::shared_ptr<VarBase> target, double value);
	void displayVar(std::shared_ptr<VarBase> var);

	class VarBase {
	public:

		const std::string& getName() const {
			return m_name;
		}
		VarType getType() const {
			return m_type;
		}
		virtual void setData(const std::string& dataStr) {};       //为了接口统一 参数设置为string
		virtual std::string getData() const {
			throw std::runtime_error("VarBase No Data");
			return "VarBase No Data";
		};
		virtual void* getDataAddr() {
			throw std::runtime_error("VarBase No Data");
			return nullptr;
		};

		virtual ~VarBase() = default;
	private:
		std::string m_name;
		VarType m_type;
	protected:
		void setName(const std::string& name) {
			m_name = name;
		}
		void setType(VarType type) {
			m_type = type;
		}
	};

	class VarVoid : public VarBase {
	public:
		VarVoid(const std::string& name) {
			setName(name);
			setType(VarType::VOID);
		}

		std::string getData() const {
			throw std::runtime_error("void has no data");
			return "void has no data";
		};
		void* getDataAddr() {
			throw std::runtime_error("void has no data");
			return NULL;
		};
		void setData(const std::string& dataStr) {
			throw std::runtime_error("void has no data");
		}
	};


	template <typename T, VarType TYPE, VarType REF_TYPE, VarType PTR_TYPE>
	class VarNumeric;
	template <typename T, VarType BASE_TYPE, VarType REF_TYPE, VarType PTR_TYPE>
	class VarPtr;
	template <typename T, VarType BASE_TYPE, VarType REF_TYPE, VarType PTR_TYPE>
	class VarRef;


	// 特化版本类型别名
	using VarInt = VarNumeric<int, VarType::INT,VarType::INT_REF, VarType::INT_PTR>;
	using VarDouble = VarNumeric<double, VarType::DOUBLE, VarType::DOUBLE_REF, VarType::DOUBLE_PTR>;
	using VarChar = VarNumeric<char, VarType::CHAR, VarType::CHAR_REF, VarType::CHAR_PTR>;

	// 特化int类型的引用和指针
	using VarIntRef = VarRef<int, VarType::INT, VarType::INT_REF, VarType::INT_PTR>;
	using VarIntPtr = VarPtr<int, VarType::INT, VarType::INT_REF, VarType::INT_PTR>;

	// 特化double类型的引用和指针
	using VarDoubleRef = VarRef<double, VarType::DOUBLE, VarType::DOUBLE_REF, VarType::DOUBLE_PTR>;
	using VarDoublePtr = VarPtr<double, VarType::DOUBLE, VarType::DOUBLE_REF, VarType::DOUBLE_PTR>;

	// 特化double类型的引用和指针
	using VarCharRef = VarRef<char, VarType::CHAR, VarType::CHAR_REF, VarType::CHAR_PTR>;
	using VarCharPtr = VarPtr<char, VarType::CHAR, VarType::CHAR_REF, VarType::CHAR_PTR>;


	template <typename T, VarType TYPE, VarType REF_TYPE, VarType PTR_TYPE >
	class VarNumeric : public VarBase {
	public:
		// 默认构造函数
		VarNumeric(const std::string& name) : m_data(T{}) {
			setName(name);
			setType(TYPE);
		}

		// 从VarBase构造
		VarNumeric(const std::string& name, std::shared_ptr<VarBase> data) {
			setName(name);
			if (isRef(data->getType())) {
				auto temp = std::dynamic_pointer_cast<VarRef<T, TYPE, REF_TYPE,PTR_TYPE >>(data);
				setData(temp->getData());
			}
			else {
				auto temp = std::dynamic_pointer_cast<VarNumeric>(data);
				m_data = temp->m_data;
			}
			setType(TYPE);
		}

		// 从具体值构造
		VarNumeric(const std::string& name, T data) : m_data(data) {
			setName(name);
			setType(TYPE);
		}

		// 从字符串构造
		VarNumeric(const std::string& name, const std::string& dataStr) {
			setName(name);
			setData(dataStr);
			setType(TYPE);
		}

		// 获取数据字符串表示
		std::string getData() const override {
			if constexpr (std::is_same_v<T, char>) {
				return std::string(1, m_data);
			}
			else {
				return std::to_string(m_data);
			}
		}

		// 获取数据地址
		void* getDataAddr() override {
			return &m_data;
		}

		// 设置数据
		void setData(const std::string& dataStr) override {
			if constexpr (std::is_same_v<T, char>) {
				if (dataStr.size() != 1) {
					throw std::runtime_error("Char must be exactly 1 character");
				}
				m_data = dataStr[0];
			}
			else if constexpr (std::is_same_v<T, int>) {
				try {
					m_data = std::stoi(dataStr);
				}
				catch (...) {
					throw std::runtime_error("Invalid int value: " + dataStr);
				}
			}
			else if constexpr (std::is_same_v<T, double>) {
				try {
					m_data = std::stod(dataStr);
				}
				catch (...) {
					throw std::runtime_error("Invalid double value: " + dataStr);
				}
			}
		}

		// 获取原始数据
		T getRawData() const {
			return m_data;
		}

	private:
		T m_data;
	};


	template <typename T, VarType BASE_TYPE, VarType REF_TYPE, VarType PTR_TYPE>
	class VarRef : public VarBase {
	public:
		friend class VarNumeric<T, BASE_TYPE, REF_TYPE, PTR_TYPE>;
		friend class VarPtr<T, BASE_TYPE, REF_TYPE, PTR_TYPE>;
		friend std::shared_ptr<VarBase> deReference(std::shared_ptr<VarBase> var);
		friend void rePtr(std::shared_ptr<VarBase> var, std::shared_ptr<VarBase> target);
		friend void sharePtr(std::shared_ptr<VarBase> var, std::shared_ptr<VarBase> target);
		friend void shareRef(std::shared_ptr<VarBase> var, std::shared_ptr<VarBase> target);

		VarRef(const std::string& name, std::shared_ptr<VarBase> tar) {
			if (tar->getType() == BASE_TYPE)
				m_data = tar;
			else if (tar->getType() == REF_TYPE) {
				auto temp = std::dynamic_pointer_cast<VarRef>(tar);
				m_data = temp->m_data;
			}
			else
				throw std::runtime_error("params not match ref type");
			setName(name);
			setType(REF_TYPE);
		}

		VarRef(const std::string& name) {
			m_data = nullptr;
			setName(name);
			setType(REF_TYPE);
		}

		std::string getData() const {
			return m_data->getData();
		};

		void* getDataAddr() {
			return m_data->getDataAddr();
		};

		void setData(const std::string& dataStr) {
			m_data->setData(dataStr);
		};

	private:
		std::shared_ptr<VarBase> m_data;
	};

	template <typename T, VarType BASE_TYPE, VarType REF_TYPE, VarType PTR_TYPE>
	class VarPtr : public VarBase {
	public:
		friend class VarRef<T, BASE_TYPE, REF_TYPE, PTR_TYPE>;
		friend std::shared_ptr<VarBase> deReference(std::shared_ptr<VarBase> var);
		friend void rePtr(std::shared_ptr<VarBase> var, std::shared_ptr<VarBase> target);
		friend void sharePtr(std::shared_ptr<VarBase> var, std::shared_ptr<VarBase> target);

		VarPtr(const std::string& name, std::shared_ptr<VarBase> tar, bool shareData) {
			if (shareData) {
				// 可以从指针创建指针 int* p1=p2，二者共享数据
				if (tar->getType() == PTR_TYPE) {
					auto temp = std::dynamic_pointer_cast<VarPtr>(tar);
					m_data = temp->m_data;
				}
				else if (tar->getType() == REF_TYPE) {
					// 可以从引用创建指针 int* p=&r，其中r是引用，此时共享数据
					auto temp = std::dynamic_pointer_cast<VarRef<T, BASE_TYPE, REF_TYPE, PTR_TYPE>>(tar);
					m_data = temp->m_data;
				}
				else {
					throw std::runtime_error("tar is not a pointer");
				}
			}
			else {
				if (tar->getType() == BASE_TYPE || tar->getType() == REF_TYPE)
					m_data = tar;
				else
					throw std::runtime_error("params not match ref type");
			}
			setName(name);
			setType(PTR_TYPE);
		}

		VarPtr(const std::string& name) {
			m_data = nullptr;
			setName(name);
			setType(PTR_TYPE);
		}

		void setData(const std::string& dataStr) {
			m_data->setData(dataStr);
		};

		std::string getData() const {
			std::stringstream ss;
			ss << m_data;
			std::string addressStr = ss.str();
			return addressStr;
		};

		void* getDataAddr() {
			return &m_data;
		};

	private:
		std::shared_ptr<VarBase> m_data;
	};




#undef TYPEMAP

}
#endif