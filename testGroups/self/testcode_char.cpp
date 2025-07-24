char* IntAddTwoPtr(char* a)
{
    *a = *a + 1;
    char* b = a;
    *b = *b + 1;
    return b;
}

// 支持pass by value , ref ptr
// 支持return by value, ref , ptr
// 支持从变量创建引用，从引用创建引用
// 支持从变量创建指针，从引用创建指针， 从指针创建指针
// 支持指针重新赋值
// 支持利用指针和引用改变变量

char& IntAddFourRef(char& a)
{
    a = a + 1;
    char& b = a;
    b = a + 1;
    char* c = &b;
    *c = *c + 1;
    char d = *c + 1;
    a = d;
    return a;
}


char ForAddFive(char a) {
    for (int i = 0; i < 5; i++) {
        a = a + 1;
    };      // 必须以;结尾
    return a;
}


char mainFunc(char arg1)
{
    char c1 = ForAddFive(arg1);
    char& c2 = c1;
    c2 = IntAddFourRef(c2);
    char* c3 = &c2;
    c3 = IntAddTwoPtr(c3);
    char ret = *c3;
    ret = ret + 1;
    return ret;
}