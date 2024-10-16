# include <bits/stdc++.h>

// 泛型函数
template <typename T>

class Vector {
private:
    T *elements; // 指针
    size_t capacity; // 容量
    size_t size; // 大小

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
    // 构造函数
    Vector() : elements(nullptr), capacity(0), size(0) {}

    // 析构函数 ：在生命周期结束时调用，释放动态分配的内存
    ~Vector(){
        delete[] elements; // 将指针所指内存清空
    }

    // 拷贝构造函数, Vector new_vec = new Vector(other);
    Vector(const Vector &other) : capacity(other.capacity), size(other.size) {
        elements = new T[capacity];
        std::copy(other.elements, other.elements + size, elements);
    }

    // 拷贝赋值操作符, Vector vec = other
    Vector& operator = (const Vector& other) {
        if(this == &other) return *this; // 自复制
        delete[] elements;
        capacity = other.capacity;
        size = other.size;
        elements = new T[capacity];
        std::copy(other.elements, other.elements + size, elements);
        return *this;
    }

    // get 函数
    size_t getSize() const {
        return size;
    }

    size_t getCapacity() const {
        return capacity;
    }

    // 分为const/nonconst版本，对应定义时的Vector是否不变
    // 注意返回的是引用，可以直接修改Vector中的值
    T& operator [] (size_t index) {
        if (index >= size) {
            throw std::out_of_range("Index out of range");
        }
        return elements[index];
    }

    const T& operator [] (size_t index) const {
        if (index >= size) {
            throw std::out_of_range("Index out of range");
        }
        return elements[index];
    }

    // 尾部添加元素 Vector.push_back(0);
    void push_back(const T& value) {
        if(size == capacity) {
            if(!capacity) reserve(1);
            else reserve(2 * capacity);
        }
        elements[size++] = value;
    }

    // 插入元素 Vector.insert(12, 'A')
    void insert(size_t index, const T& value) {
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

    void pop_back() {
        if(size) {
            size--;
        }
    }

    void clear() {
        size = 0;
    }

    // 迭代器相关函数
    T* begin() {
        return elements;
    }

    T* end() {
        return elements + size;
    }

    const T* begin() const {
        return elements;
    }

    const T* end() const {
        return elements + size;
    }

    void printElements() const {
        for (size_t i=0; i<size; i++) {
            std::cout<<elements[i]<<" ";
        }
        std::cout<<std::endl;
    }
};



// Test
int main(){
    // 创建一个 Vector 对象
    Vector<int> myVector;

    int N;
    std::cin >> N;
    // 读走回车
    getchar();

    std::string line;
    for (int i = 0; i < N; i++)
    {
        // 读取整行
        std::getline(std::cin, line);
        std::istringstream iss(line);
        std::string command;
        iss >> command;

        if (command == "push")
        {
            int value;
            iss >> value;
            myVector.push_back(value);
        }
        else if (command == "print")
        {
            if (myVector.getSize() == 0) {
                std::cout << "empty" << std::endl;
                continue;
            }
            myVector.printElements();
        }
        else if (command == "size")
        {
            std::cout << myVector.getSize() << std::endl;
        }
        else if (command == "get")
        {
            int index;
            iss >> index;
            std::cout << myVector[index] << std::endl;
        }
        else if (command == "insert")
        {
            int index, value;
            iss >> index >> value;
            myVector.insert(index, value);
        }
        else if (command == "pop")
        {
            myVector.pop_back();
        }
        else if (command == "iterator")
        {
            if (myVector.getSize() == 0)
            {
                std::cout << "empty" << std::endl;
                continue;
            }
            for (auto it = myVector.begin(); it != myVector.end(); ++it)
            {
                std::cout << *it << " ";
            }
            std::cout << std::endl;
        }
        else if (command == "foreach")
        {
            if (myVector.getSize() == 0)
            {
                std::cout << "empty" << std::endl;
                continue;
            }
            for (const auto &element : myVector)
            {
                std::cout << element << " ";
            }
            std::cout << std::endl;
        }
        else if (command == "clear")
        {
            myVector.clear();
        }
    }
    return 0;
}

