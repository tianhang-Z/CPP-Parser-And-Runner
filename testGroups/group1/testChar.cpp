char doNothing(char a)
{
    return a;
}

char sum(char a, int b)
{
    return doNothing(a) + b;
}

char mainFunc(char arg1, int arg2, int arg3)
{
    char a = 0;
    a = sum(arg1, arg2);
    char b = sum(a, arg3);
    char c = b;

    return c;
}