## 一、 头部
```
template <typename T>
```
泛型函数，可以用T代表一系列数据类型，如`int`,`vector<int>`等

```
    std::mutex vec_mutex;
```
互斥锁：保护成员变量的访问，避免一个函数被多个线程同时访问

## 二、 类内部成员变量

```
    T *elements; // 指针
    size_t capacity; // 容量
    size_t size{}; // 大小
```

1. 元素`elements`是指向数组起始元素的指针
2. 长度`size`是数组**已经填入数据部分的大小**
3. 容量`capacity`是数组开辟的堆内存大小

## 三、 内存扩展
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

## 四、 构造函数

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

### 4.4 析构函数
```
    ~Vector(){
        std::lock_guard<std::mutex> lock(vec_mutex);
        delete[] elements; // 将指针所指内存清空
    }
```
1. 作用：在生命周期结束时调用，释放动态分配的内存

### 4.5 拷贝构造函数
```
    Vector(const Vector &other) : capacity(other.capacity), size(other.size) {
        std::lock_guard<std::mutex> lock(vec_mutex);
        elements = new T[capacity];
        std::copy(other.elements, other.elements + size, elements);
    }
```
1. 作用：`Vector new_vec = new Vector(other_vec);`

## 五、 重载操作符
### 5.1 拷贝赋值操作符
```
    Vector& operator = (const Vector& other) {
        std::lock_guard<std::mutex> lock(vec_mutex);
        if(this == &other) return *this; // 自复制
        delete[] elements;
        capacity = other.capacity;
        size = other.size;
        elements = new T[capacity];
        std::copy(other.elements, other.elements + size, elements);
        return *this;
    }
```
1. 作用：新建对象实例，`vec = other_vec`
2. 先清空本`Vector`再赋值

### 5.2 重载操作符==和!=
```
    bool operator == (const Vector& other) {
        if(this->getSize() != other.getSize()) return false;
        size_t i = this.begin(), j = other.begin();
        while(i < this.getSize()) {
            if(this[i++] != other[j++]) return false;
        }
        return true;
    }

    bool operator != (const Vector& other) {
        return !(this == other);
    }
```

### 5.3 重载比较操作符>和<
```
    bool operator < (const Vector& other) {
        size_t min_size = std::min(this->getSize(), other.getSize());
        for(size_t i=0; i<min_size; i++) {
            if(this[i] < other[i]) return true;
            if(this[i] > other[i]) return false;
        }
        return this->getSize() < other.getSize();
    }

    bool operator > (const Vector& other) {
        size_t min_size = std::min(this->getSize(), other.getSize());
        for(size_t i=0; i<min_size; i++) {
            if(this[i] > other[i]) return true;
            if(this[i] < other[i]) return false;
        }
        return this->getSize() > other.getSize();
    }
```
1. `Vector`的比较是字典序比较，从前往后只要有一个元素不等就比较这个元素，如果一直一致就比较两个数组的长度

## 六、状态获取
### 6.1 类实例状态获取
```
    [[nodiscard]] size_t getSize() const {
        return size;
    }

    [[nodiscard]] size_t getCapacity() const {
        return capacity;
    }

    // 打印数组
    void printElements() const {
        for (size_t i=0; i<size; i++) {
            std::cout<<elements[i]<<" ";
        }
        std::cout<<std::endl;
    }
```

### 6.2 元素获取
```
    T& operator [] (size_t index) noexcept{
        if (index >= size) {
            throw std::out_of_range("Index out of range");
        }
        return elements[index];
    }

    T& operator [] (size_t index) const noexcept{
        if (index >= size) {
            throw std::out_of_range("Index out of range");
        }
        return elements[index];
    }

    [[nodiscard]] T& front() {
        if(size) {
            return elements[0];
        }
        else throw std::out_of_range("Index out of range");
    }

    [[nodiscard]] T& back() {
        if(size) {
            return elements[size-1];
        }
        else throw std::out_of_range("Index out of range");
    }
```
1. 分为`const`和`noconst`版本，对于`noconst`版本，可以完成以下操作：`vec[10] = 12`
2. `noexcept` 是一个声明，表明这个函数 不会抛出任何异常，编译器会基于此优化调用代码。例如，`std::vector` 在 `noexcept` 的情况下可以避免不必要的异常检查，提升性能。
3. `[[nodiscard]]`声明此函数返回值重要，在调用没有返回值时编译器警告

## 七、插入元素
### 7.1 尾部插入元素push_back()
```
    void push_back(const T& value) {
        std::lock_guard<std::mutex> lock(vec_mutex);
        if(size == capacity) {
            if(!capacity) reserve(8);
            else reserve(2 * capacity);
        }
        elements[size++] = value;
    }
```
1. 若`size`到达`capacity`的上限，则扩充容量，`reserve`设置为8的原因是初次扩充时即获得一定容量，防止反复调用`reverse`浪费性能

### 7.2 尾部插入元素emplace_back()
```
    template <typename... Args>
    void emplace_back(Args&&... args) {
        std::lock_guard<std::mutex> lock(vec_mutex);
        if(size == capacity) {
            if(!capacity) reserve(1);
            else reserve(2 * capacity);
        }
        elements[size++] = T(std::forward<Args>(args)...);
    }
```
1. `Args&&...` 是一个**转发引用**，它可以同时处理左值引用和右值引用。这是为了支持完美转发，将其左值右值属性完全载入
2. `void emplace_back(Args&&... args)`可以实现处理任意数量和类型的参数
3. 相比于`push_back`，`emplace_back`
   * `push_back`：需要一个已经构造好的对象作为参数。这个方法会将传入的对象拷贝或移动到容器中
   * `emplace_back`：则可以直接在容器的末尾“原地构造”对象。它会根据传递的参数，直接调用构造函数在容器的内存空间中构造对象，而不会进行多余的拷贝或移动操作
4. `emplace_back` 更灵活高效，允许直接原地构造对象，避免拷贝或移动的开销，特别是当构造对象需要多个参数时。

### 7.3 插入元素insert和emplace
```
    void insert(size_t index, const T& value) {
        std::lock_guard<std::mutex> lock(vec_mutex);
        if(index >= size) {
            throw std::out_of_range("Index out of range");
        }
        if (size == capacity) {
            if(!capacity) reserve(1);
            else reserve(2 * capacity);
        }
        for(size_t i=size; i>index; i--) {
            elements[i] = elements[i-1];
        }
        elements[index] = value;
        size++;
    }
```

```
    template <typename... Args>
    void emplace(size_t index, Args&&... args) {
        std::lock_guard<std::mutex> lock(vec_mutex);
        if(index >= size) {
            throw std::out_of_range("Index out of range");
        }
        if (size == capacity) {
            if(!capacity) reserve(1);
            else reserve(2 * capacity);
        }
        for(size_t i=size; i>index; i--) {
            elements[i] = elements[i-1];
        }
        elements[index] = T(std::forward<Args>(args)...);
        size++;
    }
```

## 八、删除元素

### 8.1 删除单一元素
```
    void erase(size_t index) {
        if(index >= size) {
            throw std::out_of_range("Index out of range");
        }
        for(size_t i=index; i<size-1; i++) {
            elements[i] = elements[i+1];
        }
        size--;
    }
```
1. 提供了`throw`错误抛出
2. 通过对`elements`的整体迁移实现元素删除
3. 注意更新`size`

### 8.2 删除范围元素
```
    template<typename InputIterator>
    void erase(InputIterator begin, InputIterator end) {
        if(begin<elements || end>elements+size || begin>end) {
            throw std::out_of_range("Invalid iterator range");
        }
        size_t len = std::distance(begin, end);
        for(InputIterator it=begin; it!=end; ++it) {
            *it = *(it + len);
        }
        size -= len;
    }
```
1. 输入内容是迭代器
2. `*it = *(it + len);`中的`*`代表指针，由迭代器指向内容

### 8.3 pop_back
```
    void pop_back() {
        if(size) {
            size--;
        }
    }
```

### 8.4 clear
```
    void clear() {
        size = 0;
    }
```

## 九、迭代器相关函数
```
    [[nodiscard]] T* begin() {
        return elements;
    }

    [[nodiscard]] T* begin() const {
        return elements;
    }

    [[nodiscard]] T* end() {
        return elements + size;
    }

    [[nodiscard]] T* end() const {
        return elements + size;
    }
```

1. 返回`*T`代表返回指针（迭代器），而`T&`代表返回内容
2. 返回值与是否`const`无关

## 十、覆盖函数
### 10.1 覆盖为指定长度同一值
```
    void fill_init(size_t count, const T& value) {
        if(count > capacity) {
            reserve(count);
        }
        for(size_t i=0; i<count; i++) {
            elements[i] = value;
        }
        size = count;
    }
```

### 10.2 覆盖为另一数组的范围
```
    template<typename InputIterator>
    void range_init(InputIterator begin, InputIterator end) {
        size_t count = std::distance(begin, end);
        if(count > capacity) {
            reserve(count);
        }
        size_t i = 0;
        for(InputIterator it=begin; it!=end; ++it) {
            elements[i] = *it;
        }
        size = count;
    }
```

## 十一、内存释放函数
### 11.1 放弃多余容量 shrink_to_fit
```
    void shrink_to_fit() {
        if(size < capacity) {
            T* newElement = new T[size];
            std::copy(elements, elements+size, newElement);
            delete[] elements;
            elements = newElement;
            capacity = size;
        }
    }
```
1. 将已经填满的部分转移，重置`capacity`

### 11.2 销毁并重新覆盖
```
    void distroy_and_recover(size_t new_capacity = 0) {
        if(elements) {
            delete[] elements;
            elements = nullptr;
        }
        size = 0;
        if(new_capacity) {
            elements = new T[new_capacity];
            capacity = new_capacity;
        }
        else {
            capacity = 0;
        }
    }
```


## 附整体代码
```
//
// Created by 30252 on 2024/10/17.
//

#ifndef VECTOR_H
#define VECTOR_H

#include <bits/stdc++.h>

// 泛型函数
template <typename T>

class Vector {
private:
    // 互斥锁：保护成员变量的访问，避免多个线程同时访问
    std::mutex vec_mutex;

    T *elements; // 指针
    size_t capacity; // 容量
    size_t size{}; // 大小

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

public:
    // default 构造函数
    Vector() : elements(nullptr), capacity(0), size(0) {}

    // 构造函数 [begin, end)
    template <typename InputIterator>
    Vector(InputIterator begin, InputIterator end) {
        std::lock_guard<std::mutex> lock(vec_mutex);
        size = std::distance(begin, end);
        capacity = size;
        elements = new T[size];
        std::copy(begin, end, elements);
    }

    // 构造函数 (size, value)
    Vector(size_t size, const T& allElements) {
        this->size = size;
        capacity = size;
        elements = new T[size];
        for(size_t i=0; i<size; i++) {
            elements[i] = allElements;
        }
    }

    // 析构函数 ：在生命周期结束时调用，释放动态分配的内存
    ~Vector(){
        std::lock_guard<std::mutex> lock(vec_mutex);
        delete[] elements; // 将指针所指内存清空
    }

    // 拷贝构造函数, Vector new_vec = new Vector(other_vec);
    Vector(const Vector &other) : capacity(other.capacity), size(other.size) {
        std::lock_guard<std::mutex> lock(vec_mutex);
        elements = new T[capacity];
        std::copy(other.elements, other.elements + size, elements);
    }

    // 拷贝赋值操作符, vec = other
    Vector& operator = (const Vector& other) {
        std::lock_guard<std::mutex> lock(vec_mutex);
        if(this == &other) return *this; // 自复制
        delete[] elements;
        capacity = other.capacity;
        size = other.size;
        elements = new T[capacity];
        std::copy(other.elements, other.elements + size, elements);
        return *this;
    }

    // 重载比较操作符
    bool operator == (const Vector& other) {
        if(this->getSize() != other.getSize()) return false;
        size_t i = this.begin(), j = other.begin();
        while(i < this.getSize()) {
            if(this[i++] != other[j++]) return false;
        }
        return true;
    }

    bool operator != (const Vector& other) {
        return !(this == other);
    }

    bool operator < (const Vector& other) {
        size_t min_size = std::min(this->getSize(), other.getSize());
        for(size_t i=0; i<min_size; i++) {
            if(this[i] < other[i]) return true;
            if(this[i] > other[i]) return false;
        }
        return this->getSize() < other.getSize();
    }

    bool operator > (const Vector& other) {
        size_t min_size = std::min(this->getSize(), other.getSize());
        for(size_t i=0; i<min_size; i++) {
            if(this[i] > other[i]) return true;
            if(this[i] < other[i]) return false;
        }
        return this->getSize() > other.getSize();
    }

    bool empty(const Vector& v) {
        return size == 0;
    }

    // get 函数
    [[nodiscard]] size_t getSize() const {
        return size;
    }

    [[nodiscard]] size_t getCapacity() const {
        return capacity;
    }

    // 打印数组
    void printElements() const {
        for (size_t i=0; i<size; i++) {
            std::cout<<elements[i]<<" ";
        }
        std::cout<<std::endl;
    }


    // 分为const/nonconst版本，对应定义时的Vector是否不变
    // 注意返回的是引用，可以直接修改Vector中的值

    T& operator [] (size_t index) noexcept{
        if (index >= size) {
            throw std::out_of_range("Index out of range");
        }
        return elements[index];
    }

    T& operator [] (size_t index) const noexcept{
        if (index >= size) {
            throw std::out_of_range("Index out of range");
        }
        return elements[index];
    }

    [[nodiscard]] T& front() {
        if(size) {
            return elements[0];
        }
        else throw std::out_of_range("Index out of range");
    }

    [[nodiscard]] T& back() {
        if(size) {
            return elements[size-1];
        }
        else throw std::out_of_range("Index out of range");
    }

    // 尾部添加元素 Vector.push_back(0);
    void push_back(const T& value) {
        std::lock_guard<std::mutex> lock(vec_mutex);
        if(size == capacity) {
            if(!capacity) reserve(8);
            else reserve(2 * capacity);
        }
        elements[size++] = value;
    }

    // 插入元素 Vector.insert(12, 'A')
    void insert(size_t index, const T& value) {
        std::lock_guard<std::mutex> lock(vec_mutex);
        if(index >= size) {
            throw std::out_of_range("Index out of range");
        }
        if (size == capacity) {
            if(!capacity) reserve(1);
            else reserve(2 * capacity);
        }
        for(size_t i=size; i>index; i--) {
            elements[i] = elements[i-1];
        }
        elements[index] = value;
        size++;
    }

    template <typename... Args>
    void emplace(size_t index, Args&&... args) {
        std::lock_guard<std::mutex> lock(vec_mutex);
        if(index >= size) {
            throw std::out_of_range("Index out of range");
        }
        if (size == capacity) {
            if(!capacity) reserve(1);
            else reserve(2 * capacity);
        }
        for(size_t i=size; i>index; i--) {
            elements[i] = elements[i-1];
        }
        elements[index] = T(std::forward<Args>(args)...);
        size++;
    }

    template <typename... Args>
    void emplace_back(Args&&... args) {
        std::lock_guard<std::mutex> lock(vec_mutex);
        if(size == capacity) {
            if(!capacity) reserve(1);
            else reserve(2 * capacity);
        }
        elements[size++] = T(std::forward<Args>(args)...);
    }

    // 删除元素
    void erase(size_t index) {
        if(index >= size) {
            throw std::out_of_range("Index out of range");
        }
        for(size_t i=index; i<size-1; i++) {
            elements[i] = elements[i+1];
        }
        size--;
    }

    template<typename InputIterator>
    void erase(InputIterator begin, InputIterator end) {
        if(begin<elements || end>elements+size || begin>end) {
            throw std::out_of_range("Invalid iterator range");
        }
        size_t len = std::distance(begin, end);
        for(InputIterator it=begin; it!=end; ++it) {
            *it = *(it + len);
        }
        size -= len;
    }

    void pop_back() {
        if(size) {
            size--;
        }
    }

    void clear() {
        size = 0;
    }

    // 迭代器相关函数
    [[nodiscard]] T* begin() {
        return elements;
    }

    [[nodiscard]] T* begin() const {
        return elements;
    }

    [[nodiscard]] T* end() {
        return elements + size;
    }

    [[nodiscard]] T* end() const {
        return elements + size;
    }

    // 构造：fill_init,初始化为指定数量的同一元素
    void fill_init(size_t count, const T& value) {
        if(count > capacity) {
            reserve(count);
        }
        for(size_t i=0; i<count; i++) {
            elements[i] = value;
        }
        size = count;
    }

    // 构造：range_init,用范围迭代器构造数组
    template<typename InputIterator>
    void range_init(InputIterator begin, InputIterator end) {
        size_t count = std::distance(begin, end);
        if(count > capacity) {
            reserve(count);
        }
        size_t i = 0;
        for(InputIterator it=begin; it!=end; ++it) {
            elements[i] = *it;
        }
        size = count;
    }

    // 放弃多余容量
    void shrink_to_fit() {
        if(size < capacity) {
            T* newElement = new T[size];
            std::copy(elements, elements+size, newElement);
            delete[] elements;
            elements = newElement;
            capacity = size;
        }
    }

    // distroy_and_recover 函数
    void distroy_and_recover(size_t new_capacity = 0) {
        if(elements) {
            delete[] elements;
            elements = nullptr;
        }
        size = 0;
        if(new_capacity) {
            elements = new T[new_capacity];
            capacity = new_capacity;
        }
        else {
            capacity = 0;
        }
    }
};

#endif //VECTOR_H
```
