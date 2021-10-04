#pragma once
#include "array_ptr.h"
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
    explicit SimpleVector(size_t size):
        items_(size),
        size_(size),
        capacity_(size) {
        std::fill(begin(), end(), 0);
    }

    // Создаёт вектор из size элементов, инициализированных значением value
    SimpleVector(size_t size, const Type& value):
        items_(size),
        size_(size),
        capacity_(size) {
        std::fill(begin(), end(), value);
    }

    // Создаёт вектор из std::initializer_list 
    SimpleVector(std::initializer_list<Type> init)
    : items_(init.size()),
        size_(init.size()),
        capacity_(init.size()) {
        std::move(init.begin(), init.end(), begin());
    }

    // Создаёт пустой вектор и резервирует необходимую память
    explicit SimpleVector(ReserveProxyObj capacity)
        : items_(capacity.capacity_),
        size_(0),
        capacity_(capacity.capacity_) {
    }

    //Конструктор копирования и оператор присваивания
    SimpleVector(const SimpleVector& other) {
        SimpleVector tmp(other.size_);
        std::copy(other.begin(), other.end(), tmp.begin());
        this->swap(tmp);
    }

    SimpleVector& operator=(const SimpleVector& rhs) {
        if (items_.Get() != rhs.items_.Get()) {
            SimpleVector(rhs);
        }
        return *this;
    }

    //Перемещающий конструктор и оператор присваивания
    SimpleVector(SimpleVector&& other) noexcept :
        items_(std::move(other.items_)) {
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }

    SimpleVector& operator=(SimpleVector&& rhs) noexcept{
        if (items_.Get() != rhs.items_.Get()) {
            Resize(rhs.size_);
            std::move(rhs.begin(), rhs.end(), begin());
        }
        return *this;
    }

    ~SimpleVector() {
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
        return items_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    const Type& operator[](size_t index) const noexcept {
        return items_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    Type& At(size_t index) {
        if (index >= size_) {
            throw std::out_of_range("Index is out of range");
        }
        return items_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    const Type& At(size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("Index is out of range");
        }
        return items_[index];
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
        return items_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator end() noexcept {
        return items_.Get() + size_;
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator begin() const noexcept {
        return items_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator end() const noexcept {
        return items_.Get() + size_;
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cbegin() const noexcept {
        return items_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cend() const noexcept {
        return items_.Get() + size_;
    }


    // Добавляет элемент в конец вектора
    // При нехватке места увеличивает вдвое вместимость вектора
    void PushBack(const Type& item) {
        SimpleVector tmp(*this);
        tmp.Resize(tmp.size_ + 1);
        auto it = tmp.items_.Get() + tmp.size_ - 1;
        *it = item;
        this->swap(tmp);
    }

    void PushBack(Type&& item) {
        Resize(size_ + 1);
        auto it = items_.Get() + size_ - 1;
        *it = std::move(item);
    }

    // Вставляет значение value в позицию pos.
    // Возвращает итератор на вставленное значение
    // Если перед вставкой значения вектор был заполнен полностью,
    // вместимость вектора должна увеличиться вдвое, а для вектора вместимостью 0 стать равной 1
    Iterator Insert(ConstIterator pos, const Type& value) {
        auto distance = pos - cbegin();
        auto old_size = size_;
        Resize(size_ + 1);
        Iterator pos_ = items_.Get() + distance;
        std::copy_backward(pos_, items_.Get() + old_size, end());
        *pos_ = value;
        return pos_;
    }

    Iterator Insert(ConstIterator pos, Type&& value) {
        auto distance = pos - cbegin();
        auto old_size = size_;
        Resize(size_ + 1);
        Iterator pos_ = items_.Get() + distance;
        std::move_backward(pos_, items_.Get() + old_size, end());
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
        Iterator pos_ = items_.Get() + distance;
        std::move(pos_ + 1, end(), pos_);
        --size_;
        return pos_;
    }

    // Обменивает значение с другим вектором
    void swap(SimpleVector& other) noexcept {
        items_.swap(other.items_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }

    void Reserve(size_t new_capacity) {
        if (new_capacity > capacity_) {
            SimpleVector new_vector(new_capacity);
            std::move(begin(), end(), new_vector.begin());
            items_.swap(new_vector.items_);
            std::exchange(capacity_, new_capacity);
        }
        else return;
    }

private:
    ArrayPtr<Type> items_{};
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