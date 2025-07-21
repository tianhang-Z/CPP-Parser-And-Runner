double* doNothing(double* a)
{
    return a;
}

void sum(double& a, double& b, double& out)
{
    double* c = doNothing(&a);
    double& c1 = *c;
    out = c1 + b;
}

double mainFunc(double arg1, double arg2, double arg3)
{
    double a = 0;
    sum(arg1, arg2, a);
    double b = 0;
    sum(a, arg3, b);
    double c = b;

    return c;
}