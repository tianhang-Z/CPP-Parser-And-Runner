char* IntAddTwoPtr(char* a)
{
    *a = *a + 1;
    char* b = a;
    *b = *b + 1;
    return b;
}

// ֧��pass by value , ref ptr
// ֧��return by value, ref , ptr
// ֧�ִӱ����������ã������ô�������
// ֧�ִӱ�������ָ�룬�����ô���ָ�룬 ��ָ�봴��ָ��
// ֧��ָ�����¸�ֵ
// ֧������ָ������øı����

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
    };      // ������;��β
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