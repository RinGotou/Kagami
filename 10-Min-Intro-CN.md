# 10分钟介绍Kagami Script Language

## Kagami Project

这是一个出于个人实验目的而诞生的动态类型脚本语言项目，存在着很多设计问题和各个版本之间的不相容性。

目前仍在进行大规模的修改，所以本文档仅供参考。

不建议初学者使用本脚本语言，或者将本套解释器用于高稳定性的用途之上，如前所述，她是“出于个人实验目的而诞生”的。

## 版本

目前testing与master分支的代码使用名为“如月”的IR解释框架。

1.6版本使用“葉月”作为IR解释框架。

## 语法

### 注释

本语言使用'#'字符作为注释标记。

### 作用域

脚本默认包含一个全局作用域，以下的代码结构将会创建新的作用域：

if-elif-end结构

while-end结构

case-when-else-end结构

函数

### 变量

本脚本语言采用基于引用计数的垃圾回收机制，当变量失去所有的绑定时，其本身将会进行析构。

本语言没有var之类的关键字，当变量名称第一次被与变量绑定时即为定义。如：
```
a = string('这是字符串') #将a与string函数返回的变量进行绑定
a = 0                    #原绑定变量失去绑定，被回收，与新值0进行绑定
```

### 变量类型
当前版本已经实现并可以使用的变量类型如下：

RawString (包含所有的数字、字符串、变量名称等都属于该类型，但是你无法对变量名称进行操作)

Null (空，返回值专用)

String (字符串)

WString (宽字符串)

Array (数组)

Instream (文件输入流)

Outstream (文件输出流)

Regex (正则表达式)

Function (函数)

TODO:补充方法列表

### *预置变量
\_\_name\_\_ 当前模块名称（为后续语言版本预留）

\_\_platform\_\_ 当前解释器所运行的平台

\_\_version\_\_ 当前解释器版本

\_\_backend\_\_ 当前解释器后端版本(咕咕咕，长月已经在写了)


### 表达式
```
a = b #绑定变量
a + b 
a - b 
a * b 
a / b 
a && b #逻辑与
a || b #逻辑或
a < b
a > b
a <= b
a >= b
a != b
a == b
!a
~a
```

### 条件表达式
if-elif-else-end
```
if(a == b)
  #如果上表达式为true
elif(a == c)
  #同上
else
  #以上全不满足时
end
```
case-when-else-end
```
case(a)
when(1)
  #blablabla
when(2,3,4)
  #另外的blablabla
else
  #以上均不满足时
end
```
此表达式当前只支持RawString和String类型的判断。

### 循环
由于还未解决迭代器的问题，所以本语言目前不支持for-each循环。

while-end
```
a = 0
while(a <= 10)
  #bla
  a = a + 1
end
```

### 函数

函数也可以作为变量使用，只不过其实质为一个动态包装，你无法对函数本身进行任何修改。

函数由fn关键字定义，以end作为函数结尾。

```
fn hello(name)
  print('hello,' + name)
end

hello('Suzu')     #调用函数
print(hello.id()) #函数自动包装成变量，并调用其方法
```

使用return关键字可以返回特定值，如果return参数为空或者不存在return则返回类型为Null的变量。
```
fn calc(a, b)
  return(a + b)
end

result = calc(1, 2)
```

#### 闭包

你可以在函数内创建闭包。

```
fn Create(numA)
  fn func(numB)
    println(numA + numB)
  end
  
  return(func)
end

a = Create(10)
a.call(1)
b = Create(20)
b.call(2)
```

#### optional关键字

optional关键字指示被修饰的参数可以被忽略。该参数必须在常规参数之后定义。

```
fn Func(a, optional b)
  if (null(b))

  end
end
```

#### variable关键字

variable关键字指示将从被修饰的参数起的后续参数组装成为数组。

```
fn Func(variable b)
  println(b)
end
```

# 待续




