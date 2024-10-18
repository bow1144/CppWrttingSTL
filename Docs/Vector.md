## 1. 头部
```
template <typename T>
```
泛型函数，可以用T代表一系列数据类型，如`int`,`vector<int>`等

```
    std::mutex vec_mutex;
```
互斥锁：保护成员变量的访问，避免一个函数被多个线程同时访问

## 2. 类内部成员变量

```
    T *elements; // 指针
    size_t capacity; // 容量
    size_t size{}; // 大小
```

1. 元素`elements`是指向数组起始元素的指针
2. 长度`size`是数组**已经填入数据部分的大小**
3. 容量`capacity`是数组开辟的堆内存大小

## 3. 内存扩展
```
    void reserve(size_t newCapacity) {
        // 因为存储元素的空间是堆，所以空间向上新建
        if(newCapacity > capacity) {
            T *newElements = new T[newCapacity]; // 开辟新内存空间
            std::copy(elements, elements + size, newElements); // 复制旧值到新内存空间
            delete[] elements; // 删除旧内存空间
            elements = newElements; // 更新
            capacity = newCapacity;
        }
    }

```
1. `Vector`中，数组内元素储存在堆中，类内元素存储在栈中
2. 堆的内存扩展并不是直接向下扩展，需要将已经填满的内存整体向下复制，下那个上部分作为新开辟的空间
3. `std::copy`输入三个参数：起点，终点，目标起点
4. `delete[] elements`的作用是销毁数组中的对象并释放内存

## 4. 构造函数

### 4.1 default构造函数
```
    Vector() : elements(nullptr), capacity(0), size(0) {}
```
容量、大小均设置为0，不设置数组起始指针

### 4.2 构造函数 [begin, end)
```
    template <typename InputIterator>
    Vector(InputIterator begin, InputIterator end) {
        std::lock_guard<std::mutex> lock(vec_mutex);
        size = std::distance(begin, end);
        capacity = size;
        elements = new T[size];
        std::copy(begin, end, elements);
    }
```
1. `template <typename InputIterator>`是一个模板声明，表示函数可以输入任何输入迭代器
2. `size = std::distance(begin, end);`中`distance`获得了两个迭代器的距离
3. `elements = new T[size];`开辟了一个大小为`size`的空间

### 4.3 构造函数(size, value)
```
    Vector(size_t size, const T& allElements) {
        this->size = size;
        capacity = size;
        elements = new T[size];
        for(size_t i=0; i<size; i++) {
            elements[i] = allElements;
        }
    }
```
1. `this->size = size;`将对象的`size`设置为输入的`size`
