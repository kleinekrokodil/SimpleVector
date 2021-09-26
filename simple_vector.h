#pragma once

#include <cassert>
#include <initializer_list>
#include <algorithm>
#include <stdexcept>
#include <functional>
#include <utility>

class ReserveProxyObj {
public:
    ReserveProxyObj(size_t capacity) {
        capacity_ = capacity;
    }
    size_t capacity_;
};

ReserveProxyObj Reserve(size_t capacity_to_reserve) {
    return ReserveProxyObj(capacity_to_reserve);
}

template <typename Type>
class SimpleVector {
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default;

    // Создаёт вектор из size элементов, инициализированных значением по умолчанию
    explicit SimpleVector(size_t size) {
        items_ = new Type[size];
        size_ = size;
        capacity_ = size;
        std::fill(begin(), end(), 0);
    }

    // Создаёт вектор из size элементов, инициализированных значением value
    SimpleVector(size_t size, const Type& value) {
        items_ = new Type[size];
        size_ = size;
        capacity_ = size;
        std::fill(begin(), end(), value);

    }

    // Создаёт вектор из std::initializer_list 
    SimpleVector(std::initializer_list<Type> init) {
        size_ = init.size();
        items_ = new Type[size_];
        capacity_ = init.size();
        std::move(init.begin(), init.end(), begin());
    }

    // Создаёт пустой вектор и резервирует необходимую память
    explicit SimpleVector(ReserveProxyObj capacity) {
        size_ = 0;
        items_ = new Type[capacity.capacity_];
        capacity_ = capacity.capacity_;
    }

    //Конструктор копирования и оператор присваивания
    SimpleVector(const SimpleVector& other) {
        size_ = other.size_;
        items_ = new Type[size_];
        capacity_ = other.capacity_;
        std::copy(other.begin(), other.end(), begin());
    }

    SimpleVector& operator=(const SimpleVector& rhs) {
        if (items_ != rhs.items_) {
            Resize(rhs.size_);
            std::copy(rhs.begin(), rhs.end(), begin());
        }
        return *this;
    }

    //Перемещающий конструктор и оператор присваивания
    SimpleVector(SimpleVector&& other) noexcept {
        items_ = new Type[other.size_];
        std::move(other.begin(), other.end(), begin());
        std::swap(capacity_, other.capacity_);
        std::swap(size_, other.size_);
    }

    SimpleVector& operator=(SimpleVector&& rhs) noexcept{
        if (items_ != rhs.items_) {
            Resize(rhs.size_);
            std::move(rhs.begin(), rhs.end(), begin());
        }
        return *this;
    }

    ~SimpleVector() {
        delete[] items_;
        items_ = nullptr;
        size_ = 0;
        capacity_ = 0;
    }

    // Возвращает количество элементов в массиве
    size_t GetSize() const noexcept {
        return size_;
    }

    // Возвращает вместимость массива
    size_t GetCapacity() const noexcept {
        return capacity_;
    }

    // Сообщает, пустой ли массив
    bool IsEmpty() const noexcept {
        return (size_ == 0);
    }

    // Возвращает ссылку на элемент с индексом index
    Type& operator[](size_t index) noexcept {
        return *(items_ + index);
    }

    // Возвращает константную ссылку на элемент с индексом index
    const Type& operator[](size_t index) const noexcept {
        return *(items_ + index);
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    Type& At(size_t index) {
        if (index >= size_) {
            throw std::out_of_range("Index is out of range");
        }
        return *(items_ + index);
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    const Type& At(size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("Index is out of range");
        }
        return *(items_ + index);
    }

    // Обнуляет размер массива, не изменяя его вместимость
    void Clear() noexcept {
        size_ = 0;
    }

    // Изменяет размер массива.
    // При увеличении размера новые элементы получают значение по умолчанию для типа Type
    void Resize(size_t new_size) {
        if (new_size <= size_) {
            size_ = new_size;
        }
        else if (new_size <= capacity_) {
            std::fill(begin() + size_, begin() + new_size, 0);
            size_ = new_size;
        }
        else {
            if (new_size > (capacity_ * 2)) {
                SimpleVector new_vector(new_size);
                std::move(begin(), end(), new_vector.begin());
                swap(new_vector);
                size_ = new_size;
            }
            else if (new_size <= (capacity_ * 2)) {
                SimpleVector new_vector(capacity_ * 2);
                std::move(begin(), end(), new_vector.begin());
                swap(new_vector);
                size_ = new_size;
            }
        }
    }

    // Возвращает итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator begin() noexcept {
        return items_;
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator end() noexcept {
        return items_ + size_;
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator begin() const noexcept {
        return items_;
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator end() const noexcept {
        return items_ + size_;
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cbegin() const noexcept {
        return items_;
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cend() const noexcept {
        return items_ + size_;
    }


    // Добавляет элемент в конец вектора
    // При нехватке места увеличивает вдвое вместимость вектора
    void PushBack(const Type& item) {
        Resize(size_ + 1);
        auto it = items_ + size_ - 1;
        *it = item;
    }

    void PushBack(Type&& item) {
        Resize(size_ + 1);
        auto it = items_ + size_ - 1;
        std::swap(*it, item);
    }

    // Вставляет значение value в позицию pos.
    // Возвращает итератор на вставленное значение
    // Если перед вставкой значения вектор был заполнен полностью,
    // вместимость вектора должна увеличиться вдвое, а для вектора вместимостью 0 стать равной 1
    Iterator Insert(ConstIterator pos, const Type& value) {
        auto distance = pos - cbegin();
        auto old_size = size_;
        Resize(size_ + 1);
        Iterator pos_ = items_ + distance;
        std::copy_backward(pos_, items_ + old_size, end());
        *pos_ = value;
        return pos_;
    }

    Iterator Insert(ConstIterator pos, Type&& value) {
        auto distance = pos - cbegin();
        auto old_size = size_;
        Resize(size_ + 1);
        Iterator pos_ = items_ + distance;
        std::move_backward(pos_, items_ + old_size, end());
        std::swap(*pos_, value);
        return pos_;
    }

    // "Удаляет" последний элемент вектора. Вектор не должен быть пустым
    void PopBack() noexcept {
        if (size_ > 0) {
            --size_;
        }
    }

    // Удаляет элемент вектора в указанной позиции
    Iterator Erase(ConstIterator pos) {
        auto distance = pos - cbegin();
        Iterator pos_ = items_ + distance;
        std::move(pos_ + 1, end(), pos_);
        --size_;
        return pos_;
    }

    // Обменивает значение с другим вектором
    void swap(SimpleVector& other) noexcept {
        std::swap(items_, other.items_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }

    void Reserve(size_t new_capacity) {
        if (new_capacity > capacity_) {
            SimpleVector new_vector(new_capacity);
            std::move(begin(), end(), new_vector.begin());
            delete[] items_;
            std::exchange(items_, new_vector.items_);
            std::exchange(capacity_, new_capacity);
            new_vector.items_ = nullptr;
        }
        else return;
    }

private:
    // Вместо сырого указателя лучше использовать умный указатель, такой как ArrayPtr
    Type* items_ = nullptr;

    size_t size_ = 0;
    size_t capacity_ = 0;
};


template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end(), [](Type x, Type y) {return x == y; });
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs == rhs);
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end(), [](Type x, Type y) {return x < y; });
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs > rhs);
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return rhs < lhs;
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs < rhs);
}