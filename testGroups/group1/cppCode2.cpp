int doNothing(int a)
{
    return a;
}

int sum(int a, int b)
{
    return doNothing(a) + b;
}

int mainFunc(int arg1, int arg2, int arg3)
{
    int a = 0;
    a = sum(arg1, arg2);
    int b = sum(a, arg3);
    int c = b;

    return c;
}