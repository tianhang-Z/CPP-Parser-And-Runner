int* doNothing(int* a)
{   
    return a;
}

void sum(int& a, int& b, int& out)
{
    int* c = doNothing(&a);
    int& c1 = *c;
    out = c1 + b;
}

int mainFunc(int arg1, int arg2, int arg3)
{
    int a = 0;
    sum(arg1, arg2, a);
    int b = 0;
    sum(a, arg3, b);
    int c = b;

    return c;
}