#include <BasicVar.h>
#include <FileTools.h>
#include <logger.h>

namespace thz {

    bool isBasicVar(const std::string& type) {
        static const std::set<std::string> typeMap = { "int","char","double" ,
            "int*","int&" ,"char*","char*","double*","double&","void"};
        return typeMap.count(type);
    }

    bool isPtr(VarType type) {
        if (type == VarType::INT_PTR ||type == VarType::DOUBLE_PTR||type == VarType::CHAR_PTR)
            return true;
        else return false;
    }

    bool isRef(VarType type) {
        if (type == VarType::INT_REF||type==VarType::DOUBLE_REF||type==VarType::CHAR_REF)
            return true;
        else return false;
    }

    struct TypeRelation {
        VarType base;      // 基础类型 (如 INT)
        VarType ref;       // 引用类型 (如 INT_REF)
        VarType ptr;       // 指针类型 (如 INT_PTR)
    };

    // 支持的变量类型关系表
    static const std::vector<TypeRelation> supportedTypes = {
        {VarType::INT,    VarType::INT_REF,    VarType::INT_PTR},
        {VarType::CHAR,   VarType::CHAR_REF,   VarType::CHAR_PTR},
        {VarType::DOUBLE, VarType::DOUBLE_REF, VarType::DOUBLE_PTR}
    };

    // 根据基础类型获取对应的引用类型
    VarType getReferenceType(VarType baseType) {
        for (const auto& tr : supportedTypes) {
            if (baseType == tr.base) {
                return tr.ref;
            }
        }
        return VarType::NOTYPE;
    }

    // 根据基础类型获取对应的指针类型
    VarType getPointerType(VarType baseType) {
        for (const auto& tr : supportedTypes) {
            if (baseType == tr.base) {
                return tr.ptr;
            }
        }
        return VarType::NOTYPE;
    }

    // 引用可以从基本类型和引用创建
    bool isValidRefSource(VarType tar, VarType source) {
        for (const auto& group : supportedTypes) {
            if (tar == group.ref) {
                return source == group.base || source == group.ref;
            }
        }
        return false;
    }

    // 指针可以从指针、引用和基本变量创建
    bool isValidPointerSource(VarType tar, VarType source) {
        for (const auto& group : supportedTypes) {
            if (tar == group.ptr) {
                return source == group.base || source == group.ref || source == group.ptr;
            }
        }
        return false;
    }

    std::shared_ptr<VarBase> deReference(std::shared_ptr<VarBase> var) {
        // 获取var指向的VarBase
        VarType type = var->getType();
        if (!isPtr(type)&&!isRef(type))
            throw std::runtime_error("only pointer can dereference");

        switch (var->getType()) {
        case VarType::INT_PTR: {
            std::shared_ptr<VarIntPtr> ret = std::dynamic_pointer_cast<VarIntPtr>(var);
            return ret->m_data;
        }
        case VarType::INT_REF: {
            std::shared_ptr<VarIntRef> ret = std::dynamic_pointer_cast<VarIntRef>(var);
            return ret->m_data;
        }
        case VarType::DOUBLE_PTR: {
            std::shared_ptr<VarDoublePtr> ret = std::dynamic_pointer_cast<VarDoublePtr>(var);
            return ret->m_data;
        }
        case VarType::DOUBLE_REF: {
            std::shared_ptr<VarDoubleRef> ret = std::dynamic_pointer_cast<VarDoubleRef>(var);
            return ret->m_data;
        }
        default: {
            throw std::runtime_error("Unsupported variable type");
        }
        }
    }

    void shareRef(std::shared_ptr<VarBase> var, std::shared_ptr<VarBase> target) {
        if (!isRef(var->getType()))
            throw std::runtime_error("only pointer can rePtr");

        switch (var->getType()) {
        case VarType::INT_REF: {
            std::shared_ptr<VarIntRef> temp_var = std::dynamic_pointer_cast<VarIntRef>(var);
            std::shared_ptr<VarIntRef> temp_target = std::dynamic_pointer_cast<VarIntRef>(target);
            temp_var->m_data = temp_target->m_data;
            break;
        }
        case VarType::DOUBLE_REF: {
            std::shared_ptr<VarDoubleRef> temp_var = std::dynamic_pointer_cast<VarDoubleRef>(var);
            std::shared_ptr<VarDoubleRef> temp_target = std::dynamic_pointer_cast<VarDoubleRef>(target);
            temp_var->m_data = temp_target->m_data;
            break;
        }
        default: {
            throw std::runtime_error("Unsupported variable type");
        }
        }
    }

    void sharePtr(std::shared_ptr<VarBase> var, std::shared_ptr<VarBase> target) {
        if (!isPtr(var->getType()))
            throw std::runtime_error("only pointer can rePtr");

        switch (var->getType()) {
        case VarType::INT_PTR: {
            std::shared_ptr<VarIntPtr> temp_var = std::dynamic_pointer_cast<VarIntPtr>(var);
            std::shared_ptr<VarIntPtr> temp_target = std::dynamic_pointer_cast<VarIntPtr>(target);
            temp_var->m_data = temp_target->m_data;
            break;
        }
        case VarType::DOUBLE_PTR: {
            std::shared_ptr<VarDoublePtr> temp_var = std::dynamic_pointer_cast<VarDoublePtr>(var);
            std::shared_ptr<VarDoublePtr> temp_target = std::dynamic_pointer_cast<VarDoublePtr>(target);
            temp_var->m_data = temp_target->m_data;
            break;
        }
        default: {
            throw std::runtime_error("Unsupported variable type");
        }
        }
    }
    // 修改指针指向 让var的m_data指向target
    void rePtr(std::shared_ptr<VarBase> var, std::shared_ptr<VarBase> target) {
        if (!isPtr(var->getType()))
            throw std::runtime_error("only pointer can rePtr");

        switch (var->getType()) {
        case VarType::INT_PTR: {
            std::shared_ptr<VarIntPtr> temp = std::dynamic_pointer_cast<VarIntPtr>(var);
            temp->m_data = target;
            break;
        }
        case VarType::DOUBLE_PTR: {
            std::shared_ptr<VarDoublePtr> temp = std::dynamic_pointer_cast<VarDoublePtr>(var);
            temp->m_data = target;
            break;
        }
        default: {
            throw std::runtime_error("Unsupported variable type");
        }
        }
    }

    void displayVar(std::shared_ptr<VarBase> var) {
        std::cout << "name:" << var->getName() << std::endl
            << "type:" << type2Str(var->getType()) << std::endl
            << "data:" << var->getData() << std::endl;
    }


    void setBasicVar(std::shared_ptr<VarBase> target, double value) {
        switch (target->getType()) {
        case VarType::DOUBLE:
        case VarType::DOUBLE_REF:
        case VarType::DOUBLE_PTR:
            target->setData(std::to_string(value));
            break;
        case VarType::CHAR:
        case VarType::CHAR_REF:
        case VarType::CHAR_PTR:
            target->setData(std::string(1, static_cast<char>(value)));
            break;
        case VarType::INT:
        case VarType::INT_REF:
        case VarType::INT_PTR:
            target->setData(std::to_string(static_cast<int>(value)));
            break;
        default:
            throw std::runtime_error("Unsupported target type");
        }
    }


    // createVariable by double value , 运算器的实现是基于double的 , 所以应该支持从double创建基本变量
    std::shared_ptr<VarBase> createVariable(VarType type, const std::string& name, double value) {
        std::shared_ptr<VarBase> var;
        switch (type) {
        case VarType::VOID: {
            throw std::runtime_error("void has no data");
            break;
        }
        case VarType::INT: {
            var = std::make_shared<VarInt>(name, static_cast<int>(value));
            break;
        }
        case VarType::DOUBLE: {
            var = std::make_shared<VarDouble>(name, value);
            break;
        }
        case VarType::CHAR: {
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


    namespace {
        // 特化映射
        template <VarType Type>
        struct Type2ClassMap;

        template <> struct Type2ClassMap<VarType::INT> { using type = VarInt; };
        template <> struct Type2ClassMap<VarType::DOUBLE> { using type = VarDouble; };
        template <> struct Type2ClassMap<VarType::CHAR> { using type = VarChar; };

        template <> struct Type2ClassMap<VarType::INT_PTR> { using type = VarIntPtr; };
        template <> struct Type2ClassMap<VarType::CHAR_PTR> { using type = VarCharPtr; };
        template <> struct Type2ClassMap<VarType::DOUBLE_PTR> { using type = VarDoublePtr; };

        template <> struct Type2ClassMap<VarType::INT_REF> { using type = VarIntRef; };
        template <> struct Type2ClassMap<VarType::CHAR_REF> { using type = VarCharRef; };
        template <> struct Type2ClassMap<VarType::DOUBLE_REF> { using type = VarDoubleRef; };

    }

    template <VarType Type>
    std::shared_ptr<VarBase> createVariableImpl(const std::string& name, std::shared_ptr<VarBase> value) {
        if constexpr (Type == VarType::VOID) {
            throw std::runtime_error("void has no data");
        }
        else {
            using NumericType = typename Type2ClassMap<Type>::type;
            return std::make_shared<NumericType>(name, value);
        }
    }

    // 创建非指针和引用的基本类型
    std::shared_ptr<VarBase> createVariable(VarType type, const std::string& name, std::shared_ptr<VarBase> value) {
        switch (type) {
        case VarType::VOID:    return createVariableImpl<VarType::VOID>(name, value);
        case VarType::INT:     return createVariableImpl<VarType::INT>(name, value);
        case VarType::DOUBLE:  return createVariableImpl<VarType::DOUBLE>(name, value);
        case VarType::CHAR:    return createVariableImpl<VarType::CHAR>(name, value);
        default: throw std::runtime_error("Unsupported variable type");
        }
    }

    template <VarType Type>
    std::shared_ptr<VarBase> createVariableImpl(const std::string& name, const std::string& initialValue) {
        if constexpr (Type == VarType::VOID) {
            if (!initialValue.empty()) {
                throw std::runtime_error("void has no data");
            }
            return std::make_shared<VarVoid>(name);
        }
        else if constexpr (Type == VarType::CHAR) {
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
        else {
            using NumericType = typename Type2ClassMap<Type>::type;
            if (!initialValue.empty()) {
                return std::make_shared<NumericType>(name, initialValue);
            }
            else {
                return std::make_shared<NumericType>(name);
            }
        }
    }
    std::shared_ptr<VarBase> createVariable(VarType type, const std::string& name, const std::string& initialValue) {
        switch (type) {
        case VarType::VOID:    return createVariableImpl<VarType::VOID>(name, initialValue);
        case VarType::INT:     return createVariableImpl<VarType::INT>(name, initialValue);
        case VarType::DOUBLE:  return createVariableImpl<VarType::DOUBLE>(name, initialValue);
        case VarType::CHAR:    return createVariableImpl<VarType::CHAR>(name, initialValue);
        default: throw std::runtime_error("Unsupported variable type");
        }
    }



    
    template <VarType Type>
    std::shared_ptr<VarBase> createVariablePtrImpl(const std::string& name, std::shared_ptr<VarBase> source, bool shareData) {
        using PtrType = typename Type2ClassMap<Type>::type;

        if (source != nullptr && !isValidPointerSource(Type,source->getType())) {
            throw std::runtime_error("type not match");
        }

        if (source) {
            return std::make_shared<PtrType>(name, source, shareData);
        }
        else {
            return std::make_shared<PtrType>(name);
        }
    }
    std::shared_ptr<VarBase> createVariablePtr(VarType type, const std::string& name, std::shared_ptr<VarBase> source, bool shareData) {
        switch (type) {
        case VarType::INT_PTR:    return createVariablePtrImpl<VarType::INT_PTR>(name, source, shareData);
        case VarType::CHAR_PTR:   return createVariablePtrImpl<VarType::CHAR_PTR>(name, source, shareData);
        case VarType::DOUBLE_PTR: return createVariablePtrImpl<VarType::DOUBLE_PTR>(name, source, shareData);
        default: throw std::runtime_error("Unsupported variable type");
        }
    }

  
    template <VarType Type>
    std::shared_ptr<VarBase> createVariableRefImpl(const std::string& name, std::shared_ptr<VarBase> source) {
        using RefType = typename Type2ClassMap<Type>::type;

        if (source == nullptr) {
            if (name != "return_var") { // 支持返回值为引用（特殊处理）预先创建一个空指针
                throw std::runtime_error("ref must initialize when declare");
            }
            return std::make_shared<RefType>(name);
        }

        if (!isValidRefSource(Type, source->getType())) {
            throw std::runtime_error("type not match");
        }
        return std::make_shared<RefType>(name, source);
    }
    std::shared_ptr<VarBase> createVariableRef(VarType type, const std::string& name, std::shared_ptr<VarBase> source) {
        switch (type) {
        case VarType::INT_REF:    return createVariableRefImpl<VarType::INT_REF>(name, source);
        case VarType::CHAR_REF:   return createVariableRefImpl<VarType::CHAR_REF>(name, source);
        case VarType::DOUBLE_REF: return createVariableRefImpl<VarType::DOUBLE_REF>(name, source);
        default: throw std::runtime_error("Unsupported variable type");
        }
    }



    // 临时值创建参 如 fun(1,2)
    std::shared_ptr<VarBase> createVarByTemp(VarType type, std::string paramName, std::string argValue) {
        std::shared_ptr<VarBase> ret;
        if (type == VarType::CHAR) {
            char c = parseCharLiteral(argValue);            // 对于char类型，需要特殊处理
            ret = std::make_shared<VarChar>(paramName, c);
        }
        else {
            // 对于int和double类型，直接使用字符串表示
            ret = createVariable(type, paramName, argValue);
        }
        return ret;

    }

    
    std::shared_ptr<VarBase> createVarByVar(VarType type, std::string name, std::shared_ptr<VarBase> rightVar,
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
            var = createVariableRef(type, name, rightVar);
        }
        else if (createRef && deRef) {  // int& r=*p
            std::shared_ptr<VarBase> deRefVar = deReference(rightVar);
            var = createVariableRef(type, name, deRefVar);
        }
        else if (createPtr && getAddr) { //int* p = &a  此时p指向a 
            // 特别的 当a是引用时 因为a底层是指针，此时表现为指针共享
            var = createVariablePtr(type, name, rightVar, isRef(rightVar->getType()));
        }
        else if (createPtr && !getAddr && !deRef) {  //int* p= p1  此时p和p1指向同一变量
            if (!isPtr(rightVar->getType()))
                throw std::runtime_error("int* p= p1 ,p1 must be a pointer");
            var = createVariablePtr(type, name, rightVar, true);
        }
        else if (!createPtr && !createRef && deRef) { //  int a=*p
            std::shared_ptr<VarBase> deRefVar = deReference(rightVar);
            var = createVariable(type, name, deRefVar);
        }
        else if (!createPtr && !createRef && !getAddr && !deRef) {  //int a=b
            var = createVariable(type, name, rightVar);
        }
        else {
            DEBUG("%s %s", type2Str(type), rightVar);
            throw std::runtime_error("invalid stmt");
        };
        return var;
    }

    std::shared_ptr<VarBase> createVarByVarMap(VarType type, std::string name, std::string argValue, __var_map__ varMap) {
        bool createRef = isRef(type), createPtr = isPtr(type);
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
            var = createVarByVar(type, name, argVar, createPtr, createRef, getAddr, deRef);
        }
        else {
            if (createRef)
                throw std::runtime_error("ref must initialize by another var");
            var = createVarByTemp(type, name, argValue);
        }
        return var;
    };




}