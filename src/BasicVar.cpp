#include <BasicVar.h>
#include <FileTools.h>
#include <logger.h>

namespace thz {

    bool IsValidVar(const std::string& type) {
        static const std::set<std::string> typeMap = { "int","char","double" ,
            "int*","int&" ,"char*","char*","double*","double&","void"};
        return typeMap.count(type);
    }

    bool IsBasicType(VarType type) {
        if (type ==VarType::Void|| type == VarType::Int || type == VarType::Char || type == VarType::Double)
            return true;
        else return false;
    }

    bool IsPtr(VarType type) {
        if (type == VarType::IntPtr ||type == VarType::DoublePtr||type == VarType::CharPtr)
            return true;
        else return false;
    }

    bool IsRef(VarType type) {
        if (type == VarType::IntRef||type==VarType::DoubleRef||type==VarType::CharRef)
            return true;
        else return false;
    }



    namespace {
        struct TypeRelation {
            VarType m_base;      
            VarType m_ref;       
            VarType m_ptr;       
        };
        // 支持的变量类型关系表
        static const std::vector<TypeRelation> SupportedTypes = {
            {VarType::Int,    VarType::IntRef,    VarType::IntPtr},
            {VarType::Char,   VarType::CharRef,   VarType::CharPtr},
            {VarType::Double, VarType::DoubleRef, VarType::DoublePtr}
        };

        // 特化映射
        template <VarType Type>
        struct Type2ClassMap;

        template <> struct Type2ClassMap<VarType::Int> { using ClassName = VarInt; };
        template <> struct Type2ClassMap<VarType::Double> { using ClassName = VarDouble; };
        template <> struct Type2ClassMap<VarType::Char> { using ClassName = VarChar; };

        template <> struct Type2ClassMap<VarType::IntPtr> { using ClassName = VarIntPtr; };
        template <> struct Type2ClassMap<VarType::CharPtr> { using ClassName = VarCharPtr; };
        template <> struct Type2ClassMap<VarType::DoublePtr> { using ClassName = VarDoublePtr; };

        template <> struct Type2ClassMap<VarType::IntRef> { using ClassName = VarIntRef; };
        template <> struct Type2ClassMap<VarType::CharRef> { using ClassName = VarCharRef; };
        template <> struct Type2ClassMap<VarType::DoubleRef> { using ClassName = VarDoubleRef; };

    } // anonymous namespace


    VarType GetReferenceType(VarType baseType) {
        for (const auto& group : SupportedTypes) {
            if (baseType == group.m_base) {
                return group.m_ref;
            }
        }
        return VarType::NoType;
    }

    VarType GetPointerType(VarType baseType) {
        for (const auto& group : SupportedTypes) {
            if (baseType == group.m_base) {
                return group.m_ptr;
            }
        }
        return VarType::NoType;
    }


    bool IsValidRefSource(VarType tar, VarType source) {
        for (const auto& group : SupportedTypes) {
            if (tar == group.m_ref) {
                return source == group.m_base || source == group.m_ref;
            }
        }
        return false;
    }

    /*
    指针可以从指针、引用和基本变量创建
    */
    bool IsValidPointerSource(VarType tar, VarType source) {
        for (const auto& group : SupportedTypes) {
            if (tar == group.m_ptr) {
                return source == group.m_base || source == group.m_ref || source == group.m_ptr;
            }
        }
        return false;
    }

    template <VarType Type>
    std::shared_ptr<VarBase> DeReferenceImpl(std::shared_ptr<VarBase> var) {
        using PtrType = typename Type2ClassMap<Type>::ClassName;
        auto ptr = std::dynamic_pointer_cast<PtrType>(var);
        if (!ptr) throw std::runtime_error("Invalid cast in DeReference");
        return ptr->m_data;
    }

    std::shared_ptr<VarBase> DeReference(std::shared_ptr<VarBase> var) {
        // 获取var指向的VarBase
        VarType type = var->get_type();
        if (!IsPtr(type)&&!IsRef(type))
            throw std::runtime_error("only pointer can dereference");

        switch (var->get_type()) {
        case VarType::IntPtr: return DeReferenceImpl<VarType::IntPtr>(var);
        case VarType::IntRef: return DeReferenceImpl<VarType::IntRef>(var);
        case VarType::DoublePtr: return DeReferenceImpl<VarType::DoublePtr>(var);
        case VarType::DoubleRef: return DeReferenceImpl<VarType::DoubleRef>(var);
        default: throw std::runtime_error("Unsupported variable type");
        }
    }


    template <VarType Type>
    void ShareRefImpl(std::shared_ptr<VarBase> var, std::shared_ptr<VarBase> target) {
        using RefType = typename Type2ClassMap<Type>::ClassName;
        auto refVar = std::dynamic_pointer_cast<RefType>(var);
        auto refTarget = std::dynamic_pointer_cast<RefType>(target);
        if (!refVar || !refTarget) throw std::runtime_error("Invalid cast in ShareRef");
        refVar->m_data = refTarget->m_data;
    }

    void ShareRef(std::shared_ptr<VarBase> var, std::shared_ptr<VarBase> target) {
        if (!IsRef(var->get_type()))
            throw std::runtime_error("only pointer can RePtr");

        switch (var->get_type()) {
        case VarType::IntRef:    ShareRefImpl<VarType::IntRef>(var, target); break;
        case VarType::DoubleRef: ShareRefImpl<VarType::DoubleRef>(var, target); break;
        case VarType::CharRef:   ShareRefImpl<VarType::CharRef>(var, target); break;
        default:  throw std::runtime_error("Unsupported variable type");
        }
    }


    template <VarType Type>
    void SharePtrImpl(std::shared_ptr<VarBase> var, std::shared_ptr<VarBase> target) {
        using PtrType = typename Type2ClassMap<Type>::ClassName;
        auto ptrVar = std::dynamic_pointer_cast<PtrType>(var);
        auto ptrTarget = std::dynamic_pointer_cast<PtrType>(target);
        if (!ptrVar || !ptrTarget) throw std::runtime_error("Invalid cast in SharePtr");
        ptrVar->m_data = ptrTarget->m_data;
    }


    void SharePtr(std::shared_ptr<VarBase> var, std::shared_ptr<VarBase> target) {
        if (!IsPtr(var->get_type())) {
            throw std::runtime_error("Only pointer can SharePtr");
        }

        switch (var->get_type()) {
        case VarType::IntPtr:    SharePtrImpl<VarType::IntPtr>(var, target); break;
        case VarType::DoublePtr: SharePtrImpl<VarType::DoublePtr>(var, target); break;
        case VarType::CharPtr:   SharePtrImpl<VarType::CharPtr>(var, target); break;
        default: throw std::runtime_error("Unsupported variable type");
        }
    }

    template <VarType Type>
    void RePtrImpl(std::shared_ptr<VarBase> var, std::shared_ptr<VarBase> target) {
        using PtrType = typename Type2ClassMap<Type>::ClassName;
        auto ptr = std::dynamic_pointer_cast<PtrType>(var);
        if (!ptr) throw std::runtime_error("Invalid cast in RePtr");
        ptr->m_data = target;
    }

    // 修改指针指向 让var的m_data指向target
    void RePtr(std::shared_ptr<VarBase> var, std::shared_ptr<VarBase> target) {
        if (!IsPtr(var->get_type())) {
            throw std::runtime_error("Only pointer can RePtr");
        }

        switch (var->get_type()) {
        case VarType::IntPtr:    RePtrImpl<VarType::IntPtr>(var, target); break;
        case VarType::DoublePtr: RePtrImpl<VarType::DoublePtr>(var, target); break;
        case VarType::CharPtr:   RePtrImpl<VarType::CharPtr>(var, target); break;
        default: throw std::runtime_error("Unsupported variable type");
        }
    }


    void DisplayVar(std::shared_ptr<VarBase> var) {
        std::cout << "name:" << var->get_name() << std::endl
            << "type:" << type2Str(var->get_type()) << std::endl
            << "data:" << var->get_data() << std::endl;
    }


    void SetBasicVar(std::shared_ptr<VarBase> target, double value) {
        switch (target->get_type()) {
        case VarType::Double:
        case VarType::DoubleRef:
        case VarType::DoublePtr:
            target->set_data(std::to_string(value));
            break;
        case VarType::Char:
        case VarType::CharRef:
        case VarType::CharPtr:
            target->set_data(std::string(1, static_cast<char>(value)));
            break;
        case VarType::Int:
        case VarType::IntRef:
        case VarType::IntPtr:
            target->set_data(std::to_string(static_cast<int>(value)));
            break;
        default:
            throw std::runtime_error("Unsupported target type");
        }
    }


    // CreateVariable by double value , 运算器的实现是基于double的 , 所以应该支持从double创建基本变量
    std::shared_ptr<VarBase> CreateVariable(VarType type, const std::string& name, double value) {
        if (!IsBasicType(type))
            throw std::runtime_error("Unsupported variable type; only support basic type; not ptr; not ref");

        std::shared_ptr<VarBase> var;
        switch (type) {
        case VarType::Void: {
            throw std::runtime_error("void has no data");
            break;
        }
        case VarType::Int: {
            var = std::make_shared<VarInt>(name, static_cast<int>(value));
            break;
        }
        case VarType::Double: {
            var = std::make_shared<VarDouble>(name, value);
            break;
        }
        case VarType::Char: {
            // 更安全的字符转换
            char c = static_cast<char>(value);
            var = std::make_shared<VarChar>(name, c);
            break;
        }
        default: {
            throw std::runtime_error("Unsupported variable type");
        }
        }
        return var;
    }




    template <VarType Type>
    std::shared_ptr<VarBase> createVariableImpl(const std::string& name, std::shared_ptr<VarBase> value) {
        if constexpr (Type == VarType::Void) {
            throw std::runtime_error("void has no data");
        }
        else {
            using ClassName = typename Type2ClassMap<Type>::ClassName;
            return std::make_shared<ClassName>(name, value);
        }
    }

    
    std::shared_ptr<VarBase> CreateVariable(VarType type, const std::string& name, std::shared_ptr<VarBase> value) {
        // 创建非指针和引用的基本类型
        if (!IsBasicType(type))
            throw std::runtime_error("Unsupported variable type; only support basic type; not ptr; not ref");

        switch (type) {
        case VarType::Void:    return createVariableImpl<VarType::Void>(name, value);
        case VarType::Int:     return createVariableImpl<VarType::Int>(name, value);
        case VarType::Double:  return createVariableImpl<VarType::Double>(name, value);
        case VarType::Char:    return createVariableImpl<VarType::Char>(name, value);
        default: throw std::runtime_error("Unsupported variable type");
        }
    }

    template <VarType Type>
    std::shared_ptr<VarBase> createVariableImpl(const std::string& name, const std::string& initialValue) {
        if constexpr (Type == VarType::Void) {
            if (!initialValue.empty()) {
                throw std::runtime_error("void has no data");
            }
            return std::make_shared<VarVoid>(name);
        }
        else if constexpr (Type == VarType::Char) {
            if (!initialValue.empty()) {
                if (initialValue.size() != 1) {
                    throw std::runtime_error("Char must be exactly 1 character");
                }
                return std::make_shared<VarChar>(name, initialValue[0]);
            }
            else {
                return std::make_shared<VarChar>(name);
            }
        }
        else {  // else for int and double
            using ClassName = typename Type2ClassMap<Type>::ClassName;
            if (!initialValue.empty()) {
                return std::make_shared<ClassName>(name, initialValue);
            }
            else {
                return std::make_shared<ClassName>(name);
            }
        }
    }
    std::shared_ptr<VarBase> CreateVariable(VarType type, const std::string& name, const std::string& initialValue) {
        if (!IsBasicType(type))
            throw std::runtime_error("Unsupported variable type; only support basic type; not ptr; not ref");

        switch (type) {
        case VarType::Void:    return createVariableImpl<VarType::Void>(name, initialValue);
        case VarType::Int:     return createVariableImpl<VarType::Int>(name, initialValue);
        case VarType::Double:  return createVariableImpl<VarType::Double>(name, initialValue);
        case VarType::Char:    return createVariableImpl<VarType::Char>(name, initialValue);
        default: throw std::runtime_error("Unsupported variable type");
        }
    }



    
    template <VarType Type>
    std::shared_ptr<VarBase> createVariablePtrImpl(const std::string& name, std::shared_ptr<VarBase> source, bool shareData) {
        using PtrType = typename Type2ClassMap<Type>::ClassName;

        if (source != nullptr && !IsValidPointerSource(Type,source->get_type())) {
            throw std::runtime_error("type not match");
        }

        if (source) {
            return std::make_shared<PtrType>(name, source, shareData);
        }
        else {
            return std::make_shared<PtrType>(name);
        }
    }
    std::shared_ptr<VarBase> CreateVariablePtr(VarType type, const std::string& name, std::shared_ptr<VarBase> source, bool shareData) {
        switch (type) {
        case VarType::IntPtr:    return createVariablePtrImpl<VarType::IntPtr>(name, source, shareData);
        case VarType::CharPtr:   return createVariablePtrImpl<VarType::CharPtr>(name, source, shareData);
        case VarType::DoublePtr: return createVariablePtrImpl<VarType::DoublePtr>(name, source, shareData);
        default: throw std::runtime_error("Unsupported variable type");
        }
    }

  
    template <VarType Type>
    std::shared_ptr<VarBase> createVariableRefImpl(const std::string& name, std::shared_ptr<VarBase> source) {
        using RefType = typename Type2ClassMap<Type>::ClassName;

        if (source == nullptr) {
            if (name != "return_var") { // 支持返回值为引用（特殊处理）预先创建一个空指针
                throw std::runtime_error("ref must initialize when declare");
            }
            return std::make_shared<RefType>(name);
        }

        if (!IsValidRefSource(Type, source->get_type())) {
            throw std::runtime_error("type not match");
        }
        return std::make_shared<RefType>(name, source);
    }
    std::shared_ptr<VarBase> CreateVariableRef(VarType type, const std::string& name, std::shared_ptr<VarBase> source) {
        switch (type) {
        case VarType::IntRef:    return createVariableRefImpl<VarType::IntRef>(name, source);
        case VarType::CharRef:   return createVariableRefImpl<VarType::CharRef>(name, source);
        case VarType::DoubleRef: return createVariableRefImpl<VarType::DoubleRef>(name, source);
        default: throw std::runtime_error("Unsupported variable type");
        }
    }



    // 临时值创建参 如 fun(1,2)
    std::shared_ptr<VarBase> CreateVarByTemp(VarType type, std::string paramName, std::string argValue) {
        std::shared_ptr<VarBase> ret;
        if (!IsBasicType(type))
            throw std::runtime_error("Unsupported variable type; only support basic type; not ptr; not ref");

        if (type == VarType::Char) {
            char c = ParseCharLiteral(argValue);            // 对于char类型，需要特殊处理
            ret = std::make_shared<VarChar>(paramName, c);
        }
        else {
            // 对于int和double类型，直接使用字符串表示
            ret = CreateVariable(type, paramName, argValue);
        }
        return ret;

    }

    
    std::shared_ptr<VarBase> CreateVarByVar(VarType type, std::string name, std::shared_ptr<VarBase> rightVar,
        bool createPtr , bool createRef , bool getAddr , bool deRef ) {
        if (createPtr && createRef)
            throw std::runtime_error("wrong type");
        if (getAddr && deRef)
            throw std::runtime_error("wrong right stmt");
        std::shared_ptr<VarBase> var;

        /*
         *   合法语法：
         *   int& r = a ;int* p = &a
         *   int& r= *p;
         *   int* p= p1;
         *   int a=*p; int a=b;
         *   非法语法：
         *   int& r = &a, int* p=*p1; int a=&r;
         */
        if (createRef && !getAddr && !deRef) { // int& r = a
            var = CreateVariableRef(type, name, rightVar);
        }
        else if (createRef && deRef) {  // int& r=*p
            std::shared_ptr<VarBase> deRefVar = DeReference(rightVar);
            var = CreateVariableRef(type, name, deRefVar);
        }
        else if (createPtr && getAddr) { //int* p = &a  此时p指向a 
            // 特别的 当a是引用时 因为a底层是指针，此时表现为指针共享
            var = CreateVariablePtr(type, name, rightVar, IsRef(rightVar->get_type()));
        }
        else if (createPtr && !getAddr && !deRef) {  //int* p= p1  此时p和p1指向同一变量
            if (!IsPtr(rightVar->get_type()))
                throw std::runtime_error("int* p= p1 ,p1 must be a pointer");
            var = CreateVariablePtr(type, name, rightVar, true);
        }
        else if (!createPtr && !createRef && deRef) { //  int a=*p
            std::shared_ptr<VarBase> deRefVar = DeReference(rightVar);
            var = CreateVariable(type, name, deRefVar);
        }
        else if (!createPtr && !createRef && !getAddr && !deRef) {  //int a=b
            var = CreateVariable(type, name, rightVar);
        }
        else {
            LOG_DEBUG("%s %s", type2Str(type), rightVar);
            throw std::runtime_error("invalid stmt");
        };
        return var;
    }

    std::shared_ptr<VarBase> CreateVarByVarMap(VarType type, std::string name, std::string argValue, VarMap varMap) {
        bool createRef = IsRef(type), createPtr = IsPtr(type);
        bool getAddr = false, deRef = false;     // 取值和解引用判断

        std::string realArgValue = argValue;
        if (argValue[0] == '*') {
            deRef = true;
            realArgValue = realArgValue.substr(1);
        }
        else if (argValue[0] == '&') {
            getAddr = true;
            realArgValue = realArgValue.substr(1);
        }

        std::shared_ptr<VarBase> var;
        auto it = varMap.find(realArgValue);
        if (it != varMap.end()) {
            std::shared_ptr<VarBase> argVar = it->second;
            var = CreateVarByVar(type, name, argVar, createPtr, createRef, getAddr, deRef);
        }
        else {
            if (createRef)
                throw std::runtime_error("ref must initialize by another var");
            var = CreateVarByTemp(type, name, argValue);
        }
        return var;
    };




}