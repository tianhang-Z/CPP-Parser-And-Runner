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
	XX(NOTYPE,"no_type")

	enum class VarType : unsigned int {
#define GEN_TYPE(t,s) t,
		TYPEMAP(GEN_TYPE)
#undef GEN_TYPE
	};

	inline const char* typeStr(VarType type) {
		static const char* typeTab[] = {
	#define GEN_TYPESTR(t,s) s,
			TYPEMAP(GEN_TYPESTR)
	#undef GEN_TYPESTR
		};

		assert(unsigned(type) >= 0 && unsigned(type) < sizeof(typeTab) / sizeof(typeTab[0]));
		return typeTab[unsigned(type)];
	};

	typedef std::map<std::string, std::shared_ptr<VarBase>> __var_map__;


	bool isBasicVar(const std::string& type);
	bool isPtr(VarType type);
	bool isRef(VarType type);
	VarType str2Type(const std::string& str);

	std::shared_ptr<VarBase> createVariable(VarType type, const std::string& name, const std::string& initialValue = "");
	std::shared_ptr<VarBase> createVariable(VarType type, const std::string& name, double value);
	std::shared_ptr<VarBase> createVariable(VarType type, const std::string& name, std::shared_ptr<VarBase> value);


	std::shared_ptr<VarBase> createVariableRef(VarType type, const std::string& name, std::shared_ptr<VarBase> tar);
	std::shared_ptr<VarBase> createVariablePtr(VarType type, const std::string& name, std::shared_ptr<VarBase> tar,bool shareData=false);


	std::shared_ptr<VarBase> createArgsByTemp(VarType type, std::string paramName, std::string argValue);
	std::shared_ptr<VarBase> createVarByVar(VarType type, std::string name, std::shared_ptr<VarBase> rightVar,
		bool createPtr = false, bool createRef = false, bool getAddr = false, bool deRef = false);
	std::shared_ptr<VarBase> createVarByVarMap(VarType type, std::string name, std::string argValue, __var_map__ varMap);


	std::shared_ptr<VarBase> deReference(std::shared_ptr<VarBase> var);
	void rePtr(std::shared_ptr<VarBase> var, std::shared_ptr<VarBase> target);
	void sharePtr(std::shared_ptr<VarBase> var, std::shared_ptr<VarBase> target);

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

	
	class VarInt : public VarBase {
	public:
		VarInt(const std::string& name) {
			setName(name);
			m_data = 0;
			setType(VarType::INT);
		}
		VarInt(const std::string& name, std::shared_ptr<VarBase> data) {
			setName(name);
			std::shared_ptr<VarInt> temp = std::dynamic_pointer_cast<VarInt>(data);
			m_data = temp->m_data;
			setType(VarType::INT);
		}
		VarInt(const std::string& name, int data) {
			setName(name);
			m_data = data;
			setType(VarType::INT);
		};
		VarInt(const std::string& name, const std::string& dataStr) {
			setName(name);
			setData(dataStr);
			setType(VarType::INT);
		};
		std::string getData() const {
			return std::to_string(m_data);
		};
		void* getDataAddr() {
			return &m_data;
		};
		void setData(const std::string& dataStr) {
			try {
				m_data = std::stoi(dataStr);
			}
			catch (...) {
				throw std::runtime_error("Invalid int value: " + dataStr);
			}
		}
	private:
		int m_data;
	};

	class VarDouble : public VarBase {
	public:
		VarDouble(const std::string& name) {
			setName(name);
			m_data = 0;
			setType(VarType::DOUBLE);
		}
		VarDouble(const std::string& name, std::shared_ptr<VarBase> data) {
			setName(name);
			std::shared_ptr<VarDouble> temp = std::dynamic_pointer_cast<VarDouble>(data);
			m_data = temp->m_data;
			setType(VarType::DOUBLE);
		}
		VarDouble(const std::string& name, double data) {
			setName(name);
			m_data = data;
			setType(VarType::DOUBLE);
		};
		VarDouble(const std::string& name, const std::string& dataStr) {
			setName(name);
			setData(dataStr);
			setType(VarType::DOUBLE);
		};
		std::string getData() const {
			return std::to_string(m_data);
		}
		void* getDataAddr() {
			return &m_data;
		};
		void setData(const std::string& dataStr) {
			try {
				m_data = std::stod(dataStr);
			}
			catch (...) {
				throw std::runtime_error("Invalid int value: " + dataStr);
			}
		}

	private:
		double m_data;
	};

	class VarChar : public VarBase {
	public:
		VarChar(const std::string& name) {
			setName(name);
			m_data = '\0';
			setType(VarType::CHAR);
		}
		VarChar(const std::string& name, std::shared_ptr<VarBase> data) {
			setName(name);
			std::shared_ptr<VarChar> temp = std::dynamic_pointer_cast<VarChar>(data);
			m_data = temp->m_data;
			setType(VarType::CHAR);
		}
		VarChar(const std::string& name, char data) {
			setName(name);
			m_data = data;
			setType(VarType::CHAR);
		};
		VarChar(const std::string& name, const std::string& dataStr) {
			setName(name);
			setData(dataStr);
			setType(VarType::CHAR);
		};
		std::string getData() const {
			return std::string(1, m_data);
		}
		void* getDataAddr() {
			return &m_data;
		};
		void setData(const std::string& dataStr) {
			if (dataStr.size() != 1) {
				throw std::runtime_error("Char must be exactly 1 character");
			}
			m_data = dataStr[0];
		}

	private:
		char m_data;
	};



	template <typename T, VarType BASE_TYPE, VarType REF_TYPE, VarType PTR_TYPE>
	class VarPtr;

	template <typename T, VarType BASE_TYPE, VarType REF_TYPE, VarType PTR_TYPE>
	class VarRef : public VarBase {
	public:
		friend class VarPtr<T, BASE_TYPE, REF_TYPE, PTR_TYPE>;
		friend std::shared_ptr<VarBase> deReference(std::shared_ptr<VarBase> var);
		friend void rePtr(std::shared_ptr<VarBase> var, std::shared_ptr<VarBase> target);
		friend void sharePtr(std::shared_ptr<VarBase> var, std::shared_ptr<VarBase> target);

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

	// 特化int类型的引用和指针
	using VarIntRef = VarRef<int, VarType::INT, VarType::INT_REF, VarType::INT_PTR>;
	using VarIntPtr = VarPtr<int, VarType::INT, VarType::INT_REF, VarType::INT_PTR>;

	// 特化double类型的引用和指针
	using VarDoubleRef = VarRef<double, VarType::DOUBLE, VarType::DOUBLE_REF, VarType::DOUBLE_PTR>;
	using VarDoublePtr = VarPtr<double, VarType::DOUBLE, VarType::DOUBLE_REF, VarType::DOUBLE_PTR>;



#undef TYPEMAP

}
#endif