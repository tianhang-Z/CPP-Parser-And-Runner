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

#define TYPE_MAP(XX)  \
	XX(Void,"void") \
	XX(Int,"int")    \
	XX(IntRef,"int&") \
	XX(IntPtr,"int*")  \
	XX(Double,"double")  \
	XX(DoubleRef,"double&")  \
	XX(DoublePtr,"double*")  \
	XX(Char,"char")  \
	XX(CharRef,"char&")  \
	XX(CharPtr,"char*")  \
	XX(Bool,"bool")  \
	XX(BoolRef,"bool&")  \
	XX(BoolPtr,"bool*")  \
	XX(NoType,"no_type")

	enum class VarType : unsigned int {
	#define GEN_TYPE(t,s) t,
			TYPE_MAP(GEN_TYPE)
	#undef GEN_TYPE
	};

	inline const char* type2Str(VarType type) {
		static const char* typeTab[] = {
	#define GEN_TYPESTR(t,s) s,
			TYPE_MAP(GEN_TYPESTR)
	#undef GEN_TYPESTR
		};

		assert(unsigned(type) >= 0 && unsigned(type) < sizeof(typeTab) / sizeof(typeTab[0]));
		return typeTab[unsigned(type)];
	};


	inline VarType str2Type(const std::string& str) {
		static const std::map<std::string, VarType> typeMap = {
	#define GEN_TYPEPAIR(t, s) {s, VarType::t},
			TYPE_MAP(GEN_TYPEPAIR)
	#undef GEN_TYPEPAIR
		};

		auto it = typeMap.find(str);
		if (it != typeMap.end()) {
			return it->second;
		}
		throw std::runtime_error("Unsupported parameter type: " + str);
	}

	extern const std::set<std::string> VgTypeMap;
	using VarMap = std::map<std::string, std::shared_ptr<VarBase>>;

	bool IsValidVar(const std::string& type);
	bool IsBasicType(VarType type);
	bool IsPtr(VarType type);
	bool IsRef(VarType type);
	VarType GetReferenceType(VarType baseType);
	VarType GetPointerType(VarType baseType);

	std::shared_ptr<VarBase> CreateVariable(VarType type, const std::string& name, const std::string& initialValue = "");
	std::shared_ptr<VarBase> CreateVariable(VarType type, const std::string& name, double value);
	std::shared_ptr<VarBase> CreateVariable(VarType type, const std::string& name, std::shared_ptr<VarBase> value);


	std::shared_ptr<VarBase> CreateVariableRef(VarType type, const std::string& name, std::shared_ptr<VarBase> source);
	std::shared_ptr<VarBase> CreateVariablePtr(VarType type, const std::string& name, std::shared_ptr<VarBase> source,bool shareData=false);


	std::shared_ptr<VarBase> CreateVarByTemp(VarType type, std::string paramName, std::string argValue);
	std::shared_ptr<VarBase> CreateVarByVar(VarType type, std::string name, std::shared_ptr<VarBase> rightVar,
		bool createPtr = false, bool createRef = false, bool getAddr = false, bool deRef = false);
	std::shared_ptr<VarBase> CreateVarByVarMap(VarType type, std::string name, std::string argValue, VarMap varMap);


	template <VarType Type>
	std::shared_ptr<VarBase> DeReferenceImpl(std::shared_ptr<VarBase> var);
	template <VarType Type>
	void ShareRefImpl(std::shared_ptr<VarBase> var, std::shared_ptr<VarBase> target);
	template <VarType Type>
	void SharePtrImpl(std::shared_ptr<VarBase> var, std::shared_ptr<VarBase> target);
	template <VarType Type>
	void RePtrImpl(std::shared_ptr<VarBase> var, std::shared_ptr<VarBase> target);

	std::shared_ptr<VarBase> DeReference(std::shared_ptr<VarBase> var);
	void RePtr(std::shared_ptr<VarBase> var, std::shared_ptr<VarBase> target);
	void SharePtr(std::shared_ptr<VarBase> var, std::shared_ptr<VarBase> target);
	void ShareRef(std::shared_ptr<VarBase> var, std::shared_ptr<VarBase> target);

	void SetBasicVar(std::shared_ptr<VarBase> target, double value);
	void DisplayVar(std::shared_ptr<VarBase> var);

	class VarBase {
	public:

		const std::string& get_name() const {
			return m_name;
		}
		VarType get_type() const {
			return m_type;
		}
		virtual void set_data(const std::string& dataStr) {};       //为了接口统一 参数设置为string
		virtual std::string get_data() const {
			throw std::runtime_error("VarBase No Data");
			return "VarBase No Data";
		};
		virtual void* get_data_addr() {
			throw std::runtime_error("VarBase No Data");
			return nullptr;
		};

		virtual ~VarBase() = default;
	private:
		std::string m_name;
		VarType m_type;
	protected:
		void set_name(const std::string& name) {
			m_name = name;
		}
		void set_type(VarType type) {
			m_type = type;
		}
	};

	class VarVoid : public VarBase {
	public:
		VarVoid(const std::string& name) {
			set_name(name);
			set_type(VarType::Void);
		}

		std::string get_data() const {
			throw std::runtime_error("void has no data");
			return "void has no data";
		};
		void* get_data_addr() {
			throw std::runtime_error("void has no data");
			return NULL;
		};
		void set_data(const std::string& dataStr) {
			throw std::runtime_error("void has no data");
		}
	};


	template <typename T, VarType Type, VarType RefType, VarType PtrType>
	class VarNumeric;
	template <typename T, VarType BaseType, VarType RefType, VarType PtrType>
	class VarPtr;
	template <typename T, VarType BaseType, VarType RefType, VarType PtrType>
	class VarRef;


	// 特化版本类型别名
	using VarInt = VarNumeric<int, VarType::Int,VarType::IntRef, VarType::IntPtr>;
	using VarDouble = VarNumeric<double, VarType::Double, VarType::DoubleRef, VarType::DoublePtr>;
	using VarChar = VarNumeric<char, VarType::Char, VarType::CharRef, VarType::CharPtr>;
	using VarBool = VarNumeric<bool, VarType::Bool, VarType::BoolRef, VarType::BoolPtr>;


	// 特化int类型的引用和指针
	using VarIntRef = VarRef<int, VarType::Int, VarType::IntRef, VarType::IntPtr>;
	using VarIntPtr = VarPtr<int, VarType::Int, VarType::IntRef, VarType::IntPtr>;

	// 特化double类型的引用和指针
	using VarDoubleRef = VarRef<double, VarType::Double, VarType::DoubleRef, VarType::DoublePtr>;
	using VarDoublePtr = VarPtr<double, VarType::Double, VarType::DoubleRef, VarType::DoublePtr>;

	// 特化char类型的引用和指针
	using VarCharRef = VarRef<char, VarType::Char, VarType::CharRef, VarType::CharPtr>;
	using VarCharPtr = VarPtr<char, VarType::Char, VarType::CharRef, VarType::CharPtr>;

	// 特化bool类型的引用和指针
	using VarBoolRef = VarRef<bool, VarType::Bool, VarType::BoolRef, VarType::BoolPtr>;
	using VarBoolPtr = VarPtr<bool, VarType::Bool, VarType::BoolRef, VarType::BoolPtr>;

	template <typename T, VarType Type, VarType RefType, VarType PtrType >
	class VarNumeric : public VarBase {
	public:
		// 默认构造函数
		VarNumeric(const std::string& name) : m_data(T{}) {
			set_name(name);
			set_type(Type);
		}

		// 从VarBase构造
		VarNumeric(const std::string& name, std::shared_ptr<VarBase> data) {
			set_name(name);
			if (IsRef(data->get_type())) {
				auto temp = std::dynamic_pointer_cast<VarRef<T, Type, RefType,PtrType >>(data);
				set_data(temp->get_data());
			}
			else {
				auto temp = std::dynamic_pointer_cast<VarNumeric>(data);
				m_data = temp->m_data;
			}
			set_type(Type);
		}

		// 从具体值构造
		VarNumeric(const std::string& name, T data) : m_data(data) {
			set_name(name);
			set_type(Type);
		}

		// 从字符串构造
		VarNumeric(const std::string& name, const std::string& dataStr) {
			set_name(name);
			set_data(dataStr);
			set_type(Type);
		}

		// 获取数据字符串表示
		std::string get_data() const override {
			if constexpr (std::is_same_v<T, char>) {
				return std::string(1, m_data);
			}
			else {
				return std::to_string(m_data);
			}
		}

		// 获取数据地址
		void* get_data_addr() override {
			return &m_data;
		}

		// 设置数据
		void set_data(const std::string& dataStr) override {
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
			else if constexpr (std::is_same_v<T, bool>) {
				try {
					m_data = std::stoi(dataStr);
				}
				catch (...) {
					throw std::runtime_error("Invalid bool value: " + dataStr);
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
		T get_raw_data() const {
			return m_data;
		}

	private:
		T m_data;
	};


	template <typename T, VarType BaseType, VarType RefType, VarType PtrType>
	class VarRef : public VarBase {
	public:
		friend class VarNumeric<T, BaseType, RefType, PtrType>;
		friend class VarPtr<T, BaseType, RefType, PtrType>;
		template <VarType Type>
		friend std::shared_ptr<VarBase> DeReferenceImpl(std::shared_ptr<VarBase> var);
		template <VarType Type>
		friend void ShareRefImpl(std::shared_ptr<VarBase> var, std::shared_ptr<VarBase> target);
		template <VarType Type>
		friend void SharePtrImpl(std::shared_ptr<VarBase> var, std::shared_ptr<VarBase> target);
		template <VarType Type>
		friend void RePtrImpl(std::shared_ptr<VarBase> var, std::shared_ptr<VarBase> target);

		VarRef(const std::string& name, std::shared_ptr<VarBase> source) {
			if (source->get_type() == BaseType)
				m_data = source;
			else if (source->get_type() == RefType) {
				auto temp = std::dynamic_pointer_cast<VarRef>(source);
				m_data = temp->m_data;
			}
			else
				throw std::runtime_error("params not match ref type");
			set_name(name);
			set_type(RefType);
		}

		VarRef(const std::string& name) {
			m_data = nullptr;
			set_name(name);
			set_type(RefType);
		}

		std::string get_data() const {
			return m_data->get_data();
		};

		void* get_data_addr() {
			return m_data->get_data_addr();
		};

		void set_data(const std::string& dataStr) {
			m_data->set_data(dataStr);
		};

	private:
		std::shared_ptr<VarBase> m_data;
	};

	template <typename T, VarType BaseType, VarType RefType, VarType PtrType>
	class VarPtr : public VarBase {
	public:
		friend class VarRef<T, BaseType, RefType, PtrType>;
		template <VarType Type>
		friend std::shared_ptr<VarBase> DeReferenceImpl(std::shared_ptr<VarBase> var);
		template <VarType Type>
		friend void ShareRefImpl(std::shared_ptr<VarBase> var, std::shared_ptr<VarBase> target);
		template <VarType Type>
		friend void SharePtrImpl(std::shared_ptr<VarBase> var, std::shared_ptr<VarBase> target);
		template <VarType Type>
		friend void RePtrImpl(std::shared_ptr<VarBase> var, std::shared_ptr<VarBase> target);

		VarPtr(const std::string& name, std::shared_ptr<VarBase> source, bool shareData) {
			if (shareData) {
				// 可以从指针创建指针 int* p1=p2，二者共享数据
				if (source->get_type() == PtrType) {
					auto temp = std::dynamic_pointer_cast<VarPtr>(source);
					m_data = temp->m_data;
				}
				else if (source->get_type() == RefType) {
					// 可以从引用创建指针 int* p=&r，其中r是引用，此时共享数据
					auto temp = std::dynamic_pointer_cast<VarRef<T, BaseType, RefType, PtrType>>(source);
					m_data = temp->m_data;
				}
				else {
					throw std::runtime_error("source is not a pointer");
				}
			}
			else {
				if (source->get_type() == BaseType || source->get_type() == RefType)
					m_data = source;
				else
					throw std::runtime_error("params not match ref type");
			}
			set_name(name);
			set_type(PtrType);
		}

		VarPtr(const std::string& name) {
			m_data = nullptr;
			set_name(name);
			set_type(PtrType);
		}

		void set_data(const std::string& dataStr) {
			m_data->set_data(dataStr);
		};

		//获取地址
		std::string get_data() const {
			std::stringstream ss;
			ss << m_data;
			std::string addressStr = ss.str();
			return addressStr;
		};

		void* get_data_addr() {
			return &m_data;
		};

	private:
		std::shared_ptr<VarBase> m_data;
	};




#undef TYPE_MAP

}
#endif