#pragma once
#include <cstdint>
#include <cstddef>
#include <string>
#include <string_view>
#include <cstring>
#include <type_traits>
#include "conceptf.hpp"
#include "fflerror.hpp"

template<conceptf::TriviallyCopyable T, size_t Capacity> struct StaticVector
{
    T data[Capacity];
    uint32_t size;

    bool empty() const
    {
        return size == 0;
    }

    void clear()
    {
        size = 0;
    }

    constexpr size_t capacity() const
    {
        return Capacity;
    }

    /**/  T &front()       { if(empty()){ throw fflerror("empty vector"); } return data[0]; }
    const T &front() const { if(empty()){ throw fflerror("empty vector"); } return data[0]; }
    /**/  T & back()       { if(empty()){ throw fflerror("empty vector"); } return data[size - 1]; }
    const T & back() const { if(empty()){ throw fflerror("empty vector"); } return data[size - 1]; }

    /**/  T *begin()       noexcept { return data; }
    const T *begin() const noexcept { return data; }
    /**/  T *  end()       noexcept { return data + size; }
    const T *  end() const noexcept { return data + size; }

    void push(const T &t)
    {
        if(size >= capacity()){
            throw fflerror("capacity exceeds");
        }
        else{
            data[size++] = t;
        }
    }

    void pop()
    {
        if(size > 0){
            size--;
        }
        else{
            throw fflerror("emtpy vector");
        }
    }
};

static_assert(std::is_trivially_copyable_v<StaticVector<char, 128>>);
