### 支持代码注释
* 支持单行注释和行末注释 //

### 支持基本变量
* 支持int double char bool类型及其引用和指针。
* 引用的创建方式有，从变量创建和从引用创建。
* 可以利用引用修改原址，支持pass by ref和return by ref。
``` cpp
int& IntRef(int& arg)
{
    int& ref = arg;
    int& ref1 = ref;
    ref1 = ref + 1;
    return ref1;
}
```
* 指针的创建方式有，从变量、引用、指针创建。
* 可以修改指向，可以解引用，支持pass by ptr和return by ptr。
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
### 支持control block
#### 支持for 
* 支持简单的for循环，支持for内定义与外界重名变量，重名时进行覆盖。
* 支持外界变量传入block，
* 支持 <  ,<= ,> ,>=, ++ ,--
* 计数变量支持外部定义和内部定义
* 支持for condition 语句缺省
* 支持block内部函数调用

``` cpp
int ForAddThirty(int a) {
    int b = 1;
    int c = 1;
    for (int i = 0; i < 10; i++) {
        int b = 1; // 允许重名
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

// 语句缺省和函数调用
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
#### 支持if, else if, else





