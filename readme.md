## �򵥵�cpp�ļ���̬������������

### ֧�ִ���ע��
* ֧�ֵ���ע�ͺ���ĩע�� //

### ֧�ֻ�����������
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
* ֧�ּ��������ⲿ������ڲ�����
* ֧��for condition ���ȱʡ
* ֧��block�ڲ���������
* ֧��loopǶ��
* ����: block����ʹ��\{\}

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

// return 100
int& NestedLoop(int& sum) {
    sum = 0;
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            sum = sum + 1;
        }
    }
    return sum;
}
```
#### ֧��if, else if, else
* ֧�ֶ��else if��֧
* ֧���ж�����Ϊtrue�� false�� bool�������Ƚ����
* ֧���ڲ���������
* ֧���ڲ�Ƕ��loop for, Ƕ��if else�� ֧�ֶ��߻���Ƕ��

```
int& TestIf(int& a,bool check) {
    int left = 10;
    int right = 0;
    if (true) {
        for (int i = 0; i < 10; i++) {
            if (true) {
                a = a + 1;
            }
            else {
                a = a + 2;
            }
        }
    }
    else if (check) {
        a = SimpleAddOne(a);
    }
    else if (left > right) {
        a = ForAddThirty(0);
    }
    else {
        // a += 20
        int b = 1;
        int c = 1;
        for (int i = 0; i <= 9; i++) {
            int b = 1; 
            a = a + b + c;
        }
    }
    return a;
}
```

#### ֧��class
* ֧��class��Ա����ʹ��.����
* ֧�ֳ�Ա����
* ����ָ�������
* ����loop for��if
* �ݲ�֧��classָ��

```
class Test{
    int a;
    int b;
    int add_ten(int arg) {
        for (int i = 0; i <= 10; i++) {
            arg = arg + 1;
        }
        return arg;
    }
    int& ref_add_ten(int arg&) {
        for (int i = 0; i <= 10; i++) {
            arg = arg + 1;
        }
        return arg;
    }
    int* ptr_add_ten(int* arg) {
        for (int i = 0; i <= 10; i++) {
            *arg = *arg + 1;
        }
        return arg;
    }
}
```
