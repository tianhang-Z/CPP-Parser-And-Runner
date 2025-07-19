double doNothing(double a)
{
    return a;
}

double sum(double a, double b)
{
    return doNothing(a) + b;
}

double mainFunc(double arg1, double arg2, double arg3)
{
    double a = 0;
    a = sum(arg1, arg2);
    double b = sum(a, arg3);
    double c = b;

    return c;
}