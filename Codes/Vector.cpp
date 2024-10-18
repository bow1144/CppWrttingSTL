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

    // 拷贝构造函数, Vector new_vec = new Vector(other);
    Vector(const Vector &other) : capacity(other.capacity), size(other.size) {
        std::lock_guard<std::mutex> lock(vec_mutex);
        elements = new T[capacity];
        std::copy(other.elements, other.elements + size, elements);
    }

    // 拷贝赋值操作符, Vector vec = other
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
            if(!capacity) reserve(1);
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

    // 打印数组
    void printElements() const {
        for (size_t i=0; i<size; i++) {
            std::cout<<elements[i]<<" ";
        }
        std::cout<<std::endl;
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
