int* IntAddTwoPtr(int* a)
{
    *a = *a + 1;
    int* b = a;
    *b = *a + 1;
    return b;
}

// 支持pass by value , ref ptr
// 支持return by value, ref , ptr
// 支持从变量创建引用，从引用创建引用
// 支持从变量创建指针，从引用创建指针， 从指针创建指针
// 支持指针重新赋值
// 支持利用指针和引用改变变量

int& IntAddFourRef(int& a)
{
    a = a + 1;
    int& b = a;
    b = a + 1;
    int* c = &b;
    *c = *c + 1;
    int d = *c + 1;
    a = d;
    return a;
}

void addOne(int* arg1, int& arg2) {
    *arg1 = *arg1 + 1;
    arg2 = arg2 + 1;
}

int TestCala(int arg1, int arg2, int arg3, int arg4) {
    return arg1 + (arg2 - arg3) / arg4;
}

int Sum(int* a,int* b) {
    return *a + *b;
}

int SimpleAddOne(int a) {
    return a + 1;
}

class Test{
    int a;
    int b;
    int add_ten(int arg) {
        for (int i = 0; i <= 10; i++) {
            arg = arg + 1;
        }
        return arg;
    }
    int& ref_add_ten(int arg&) {
        for (int i = 0; i <= 10; i++) {
            arg = arg + 1;
        }
        return arg;
    }
    int* ptr_add_ten(int* arg) {
        for (int i = 0; i <= 10; i++) {
            *arg = *arg + a;
        }
        return arg;
    }
}

int& TestIf(int& a,bool check) {
    int left = 10;
    int right = 0;
    if (true) {
        for (int i = 0; i < 10; i++) {
            if (true) {
                a = a + 1;
            }
            else {
                a = a + 2;
            }
        }
    }
    else if (check) {
        a = SimpleAddOne(a);
    }
    else if (left > right) {
        a = ForAddThirty(0);
    }
    else {
        // a += 20
        int b = 1;
        int c = 1;
        for (int i = 0; i <= 9; i++) {
            int b = 1; 
            a = a + b + c;
        }
    }
    return a;
}

int ForAddThirty(int a) {
    int b = 1;
    int c = 1;
    for (int i = 0; i <= 9; i++) {
        int b = 1; // 允许重名
        b = b + 1;
        a = a + b + c;
    }
    return a;
}

// return 100
int& NestedLoop(int& sum) {
    sum = 0;
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            sum = sum + 1;
        }
    }
    return sum;
}

int LoopBlockCallFunc(int a) {
    int sum = 0;
    for ( ; a <= 10; ) {
        sum = sum + SimpleAddOne(0);
        a = a + 1;
    }
    return sum;
}

// a=1, return  55
int Accumulate(int a) {
    int sum = 0;
    for (; a <= 10; a++) {
        sum = sum + a;
    }
    return sum;
}

int* FunCall(int a,int* b)
{
    int iVal = 0;
    int& iRet = iVal;
    iRet = TestIf(iRet, false);

    int& a1 = IntAddFourRef(a); // a1=4

    int v1 = 1;
    int v2 = 1;
    int* p1 = &v1;
    int* p2 = &v2;
    int a2 = Sum(p1,p2);  // a2=2

    int v3 = 1;
    // a3 = 2 + 1 + 2 + 30 + 55 + 10
    int& v4 = v3;
    int a3 = TestCala(v3, 3, 2, 1) + 1 + Sum(p1,p2) + ForAddThirty(0) + Accumulate(1) + LoopBlockCallFunc(1) + NestedLoop(v4) + iRet; 
    int* b1 = IntAddTwoPtr(b); // b1=2
    addOne(b1, a1);  // b1++ a1++
    int c1 = a1 + *b1 + a2 + a3;  
    int* ret = &c1;
    return ret;
}

// arg1=0, arg2=0
int mainFunc(int arg1, int arg2)
{
    Test a_cls;
    a_cls.a = 10;
    //a_cls.b = 10;
    //int& a = arg1;
    //int* b;
    //b = &arg2;
    //int* temp_b;
    //temp_b = b;
    //int* b1 = FunCall(a, temp_b);
    //int ret = *b1;
    //return ret;
    //int b3 = a_cls.get_a();
    //return b3;
    int a = a_cls.a;
    int* a1 = &a;
    int* ret = a_cls.ptr_add_ten(a1);
    return *ret;
}