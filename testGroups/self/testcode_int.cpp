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
    int& a1 = IntAddFourRef(a); // a1=4

    int v1 = 1;
    int v2 = 1;
    int* p1 = &v1;
    int* p2 = &v2;
    int a2 = Sum(p1,p2);  // a2=2

    int v3 = 1;
    // a3 = 2 + 1 + 2 + 30 + 55 + 10
    int a3 = TestCala(v3, 3, 2, 1) + 1 + Sum(p1,p2) + ForAddThirty(0) + Accumulate(1) + LoopBlockCallFunc(1); 
    int* b1 = IntAddTwoPtr(b); // b1=2
    addOne(b1, a1);  // b1++ a1++
    int c1 = a1 + *b1 + a2 + a3;  
    int* ret = &c1;
    return ret;
}

// arg1=0, arg2=0
int mainFunc(int arg1, int arg2)
{
    int& a = arg1;
    int* b;
    b = &arg2;
    int* temp_b;
    temp_b = b;
    int* b1 = FunCall(a, temp_b);
    int ret = *b1;
    return ret;
}