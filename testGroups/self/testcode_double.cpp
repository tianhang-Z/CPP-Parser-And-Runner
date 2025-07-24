double* IntAddTwoPtr(double* a)
{
    *a = *a + 1;
    double* b = a;
    *b = *a + 1;
    return b;
}

// 支持pass by value , ref ptr
// 支持return by value, ref , ptr
// 支持从变量创建引用，从引用创建引用
// 支持从变量创建指针，从引用创建指针， 从指针创建指针
// 支持指针重新赋值
// 支持利用指针和引用改变变量

double& IntAddFourRef(double& a)
{
    a = a + 1;
    double& b = a;
    b = a + 1;
    double* c = &b;
    *c = *c + 1;
    double d = *c + 1;
    a = d;
    return a;
}

void addOne(double* arg1, double& arg2) {
    *arg1 = *arg1 + 1;
    arg2 = arg2 + 1;
}

double TestCala(double arg1, double arg2, double arg3, double arg4) {
    return arg1 + (arg2 - arg3) / arg4;
}

double Sum(double* a, double* b) {
    return *a + *b;
}


double ForAddTen(double a) {
    for (double i = 0; i < 10; i++) {
        a = a + 1;
    };      // 必须以;结尾
    return a;
}

double* FunCall(double a, double* b)
{
    double& a1 = IntAddFourRef(a); // a1=4

    double v1 = 1;
    double v2 = 1;
    double* p1 = &v1;
    double* p2 = &v2;
    double a2 = Sum(p1, p2);  // a2=2

    double v3 = 1;
    double a3 = TestCala(v3, 3, 2, 1) + 1 + Sum(p1, p2) + ForAddTen(0); // a3 = 2+1+2
    double* b1 = IntAddTwoPtr(b); // b1=2
    addOne(b1, a1);  // b1++ a1++
    double c1 = a1 + *b1 + a2 + a3;
    double* ret = &c1;
    return ret;
}

// arg1=0, arg2=0
double mainFunc(double arg1, double arg2)
{
    double& a = arg1;
    double* b;
    b = &arg2;
    double* temp_b;
    temp_b = b;
    double* b1 = FunCall(a, temp_b);
    double ret = *b1;
    return ret;
}