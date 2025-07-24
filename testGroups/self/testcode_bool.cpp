bool* PtrTrue(bool* a)
{
    *a = true;
    return a;;
}


bool& RefFalse(bool& a)
{
    a = false;
    return a;
}


//arg1=true
bool mainFunc(bool arg1)
{
    bool& b1 = arg1;
    b1 = RefFalse(b1);
    bool* b2 = &b1;
    b2 = PtrTrue(b2);
    return *b2;
}