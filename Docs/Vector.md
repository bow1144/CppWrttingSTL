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
