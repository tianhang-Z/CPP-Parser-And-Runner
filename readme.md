### ֧�ִ���ע��
* ֧�ֵ���ע�ͺ���ĩע�� //

### ֧�ֻ�������
* ֧��int double char bool���ͼ������ú�ָ�롣
* ���õĴ�����ʽ�У��ӱ��������ʹ����ô�����
* �������������޸�ԭַ��֧��pass by ref��return by ref��
``` cpp
int& IntRef(int& arg)
{
    int& ref = arg;
    int& ref1 = ref;
    ref1 = ref + 1;
    return ref1;
}
```
* ָ��Ĵ�����ʽ�У��ӱ��������á�ָ�봴����
* �����޸�ָ�򣬿��Խ����ã�֧��pass by ptr��return by ptr��
``` cpp
int* IntAPtr(int* a)
{
    *a = *a + 1;
    int* b ;
    b = a;
    *b = *a + 1;
    return b;
}

```
### ֧��control block
#### ֧��for 
* ֧�ּ򵥵�forѭ����֧��for�ڶ����������������������ʱ���и��ǡ�
* ֧������������block��
* ֧�� <  ,<= ,> ,>=, ++ ,--
* ��������֧���ⲿ������ڲ�����
* ֧��for condition ���ȱʡ
* ֧��block�ڲ���������

``` cpp
int ForAddThirty(int a) {
    int b = 1;
    int c = 1;
    for (int i = 0; i < 10; i++) {
        int b = 1; // ��������
        b = b + 1;
        a = a + b + c;
    }
    return a;
}

// a=0, return  55
int Accumulate(int a) {
    int sum = 0;
    for ( ; a <= 10; a++) {
        sum = sum + a;
    }
    return sum;
}

// ���ȱʡ�ͺ�������
// a = 1 , return 10
int LoopBlockCallFunc(int a) {
    int sum = 0;
    for ( ; a <= 10; ) {
        sum = sum + SimpleAddOne(0);
        a = a + 1;
    }
    return sum;
}
```
#### ֧��if, else if, else





