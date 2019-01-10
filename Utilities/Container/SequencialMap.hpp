#pragma once

#include <cassert>
#include <utility>
#include <algorithm>
#include <functional>
#include <map>
#include <vector>
#include <memory>
#include <iterator>
#include <type_traits>
#include <initializer_list>

namespace Container {
template<typename Key,
         typename T,
         typename Compare = std::less<Key>,
         typename Allocator = std::allocator<std::pair<const Key, T>>>
class SequencialMap
{
public:
    using allocator_type = Allocator;

    using vector_type = std::vector<std::pair<const Key, T>>;
    using value_type = typename vector_type::value_type;
    using pointer = typename vector_type::pointer;
    using const_pointer = typename vector_type::const_pointer;
    using reference = typename vector_type::reference;
    using const_reference = typename vector_type::const_reference;
    using size_type = typename vector_type::size_type;
    using difference_type = typename vector_type::difference_type;

    using map_type = std::map<Key, size_type>;
    using key_type = Key;
    using mapped_type = T;
    using key_compare = Compare;
    using value_compare = std::function<bool(const value_type&, const value_type&)>;

    template<bool constant> struct iterator_base;
    using iterator = typename vector_type::iterator;
    using const_iterator = typename vector_type::const_iterator;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    struct key_iterator;
    using reverse_key_iterator = std::reverse_iterator<key_iterator>;

    SequencialMap()
    {
        static_assert(!std::is_arithmetic<Key>::value, "Key type cannot not be arithmetic!");
    }

    explicit SequencialMap(const Allocator& alloc)
        : v(alloc)
    {
        static_assert(!std::is_arithmetic<Key>::value, "Key type cannot not be arithmetic!");
    }

    template<typename InputIt>
    SequencialMap(InputIt first, InputIt last, const Compare& comp = Compare(), const Allocator& alloc = Allocator())
        : v(alloc), m(comp)
    {
        static_assert(!std::is_arithmetic<Key>::value, "Key type cannot not be arithmetic!");
        push_back(first, last);
    }

    template<typename InputIt>
    SequencialMap(InputIt first, InputIt last, const Allocator& alloc)
        : v(alloc)
    {
        static_assert(!std::is_arithmetic<Key>::value, "Key type cannot not be arithmetic!");
        push_back(first, last);
    }

    SequencialMap(const SequencialMap& other, const Allocator& alloc = Allocator())
        : v(other.v, alloc), m(other.m)
    {
    }

    SequencialMap(SequencialMap&& other, const Allocator& alloc = Allocator())
        : v(std::forward<vector_type>(other.v), alloc),
          m(std::forward<vector_type>(other.m))
    {
    }

    SequencialMap(std::initializer_list<value_type> init,
                  const Compare& comp = Compare(),
                  const Allocator& alloc = Allocator())
        : v(alloc), m(comp)
    {
        static_assert(!std::is_arithmetic<Key>::value, "Key type cannot not be arithmetic!");
        push_back(init);
    }

    ~SequencialMap() = default;

    [[nodiscard]] bool empty() const noexcept
    { return v.empty(); }

    size_type size() const noexcept
    { return v.size(); }

     size_type max_size() const noexcept
    { return v.max_size(); }

    void reserve(size_type new_cap)
    { v.reserve(new_cap); }

    size_type capacity() const noexcept
    { return v.capacity(); }

    void clear() noexcept
    { v.clear(); m.clear(); }

    bool contains(const key_type& key) const
    { return find(key) != cend(); }

    iterator find(const key_type& key)
    {
        auto it = m.find(key);
        if (it == m.end()) return end();
        else return begin() + it->second;
    }

    const_iterator find(const key_type& key) const
    {
        auto it = m.find(key);
        if (it == m.cend()) return cend();
        else return cbegin() + it->second;
    }

    template<template<typename> typename Container>
    Container<Key> keys() const
    {
        Container<Key> ret;
        for (auto it = cbegin(); it != cend(); ++it)
        { ret.push_back(it->first); }
        return ret;
    }

    const key_type key(const T& value, const key_type& defaultKey = key_type()) const
    {
        for (auto it = cbegin(); it != cend(); ++it)
        { if (it->second == value) return it->first; }
        return defaultKey;
    }

    const T& value(size_type pos, const_reference defaultValue = value_type()) const
    {
        if (pos >= size()) return defaultValue;
        else return v[pos].second;
    }

    const T& value(const key_type& key, const_reference defaultValue = value_type()) const
    {
        auto it = find(key);
        if (it == cend()) return defaultValue;
        else return it->second;

    }

    reference at(size_type pos)
    { return v.at(pos); }

    const_reference at(size_type pos) const
    { return v.at(pos); }

    T& operator[](const key_type& key)
    {
        auto pair = m.insert(std::make_pair(key, size()));
        if (pair.first) v.push_back(std::make_pair(key, T()));
        return v[size() - 1].second;
    }

    const T& operator[](key_type&& key)
    {
        key_type k = key;
        auto pair = m.insert(std::make_pair(k, size()));
        if (pair.first) v.push_back(std::make_pair(k, T()));
        return v[size() - 1].second;
    }

    T& operator[](size_type pos)
    { return v[pos].second; }

    const T& operator[](size_type pos) const
    { return v[pos].second; }

    reference front()
    { return v.front(); }

    const_reference front() const
    { return v.front(); }

    reference back()
    { return v.back(); }

    const_reference back() const
    { return v.back(); }

    SequencialMap mid(size_type pos) const
    { return mid(pos, size() - pos); }

    SequencialMap mid(size_type pos, size_type length) const
    {
        SequencialMap ret;
        if (pos >= size()) return ret;
        length = std::max(length, size() - pos);
        ret.insert(cbegin() + pos, cbegin() + pos + length);
        return ret;
    }

    std::pair<iterator,bool> push_back(const_reference value)
    {
        auto it = find(value.first);
        if (it != end()) return std::make_pair(it, false);
        m.insert(std::make_pair(value.first, size()));
        v.push_back(value);
        return std::make_pair(end() - 1, true);
    }

    std::pair<iterator,bool> push_back(value_type&& value)
    {
        value_type temp = value;
        auto it = find(temp.first);
        if (it != end()) return std::make_pair(it, false);
        m.insert(std::make_pair(temp.first, size()));
        v.push_back(std::move(temp));
        return std::make_pair(end() - 1, true);
    }

    std::pair<iterator, bool> push_back(const key_type& key, const T& value)
    { return push_back(std::make_pair(key, value)); }

    std::pair<iterator, bool> push_back(const key_type& key, T&& value)
    { return push_back(std::make_pair(key, value)); }

    size_type push_back(const SequencialMap& other)
    { return push_back(other.cbegin(), other.cend()); }

    size_type push_back(std::initializer_list<value_type> ilist)
    { return push_back(ilist.begin(), ilist.end()); }

    template<typename InputIt>
    size_type push_back(InputIt first, InputIt last)
    {
        size_type count = 0;
        for (auto it = first; it != last; ++it)
        { if (push_back(*it).second) ++count; }
        return count;
    }

    template<typename... Args>
    std::pair<iterator, bool> emplace_back(const key_type& key, Args&&... args)
    {
        T value;
        new(&value) T(std::forward<Args>(args)...);
        return push_back(key, std::move(value));
    }

    SequencialMap operator+(const SequencialMap& other) const
    { SequencialMap ret = *this; ret.push_back(other); return ret; }

    SequencialMap operator+(SequencialMap&& other) const
    { SequencialMap ret = *this; ret.push_back(other); return ret; }

    SequencialMap& operator+=(const SequencialMap& other)
    { push_back(other); return *this; }

    SequencialMap& operator+=(SequencialMap&& other)
    { push_back(other); return *this; }

    iterator insert(size_t pos, const_reference value)
    { return insert(cbegin() + pos, value); }

    iterator insert(size_t pos, value_type&& value)
    { return insert(cbegin() + pos, value); }

    iterator insert(size_t pos, const key_type& key, const T& value)
    { return insert(pos, std::make_pair(key, value)); }

    iterator insert(size_t pos, const key_type& key, T&& value)
    { return insert(pos, std::make_pair(key, value)); }

    iterator insert(const_iterator pos, const_reference value)
    {
        auto it = find(value.first);
        if (it != end()) return it;
        m.insert(std::make_pair(value.first), 0);
        v.insert(v.begin() + pos.p, value);
        refresh_index(pos.p);
        return begin() + pos.p;
    }

    iterator insert(const_iterator pos, value_type&& value)
    {
        value_type temp = value;
        auto it = find(temp.first);
        if (it != end()) return it;
        m.insert(std::make_pair(temp.first), 0);
        v.insert(v.begin() + pos.p, std::move(temp));
        refresh_index(pos.p);
        return begin() + pos.p;
    }

    iterator insert(const_iterator pos, const key_type& key, const T& value)
    { return insert(pos, std::make_pair(key, value)); }

    iterator insert(const_iterator pos, const key_type& key, T&& value)
    { return insert(pos, std::make_pair(key, value)); }

    template<typename InputIt>
    void insert(size_t pos, InputIt first, InputIt last)
    { return insert(cbegin() + pos, first, last); }

    template<typename InputIt>
    void insert(size_t pos, std::initializer_list<value_type> ilist)
    { return insert(cbegin() + pos, ilist); }

    template<typename InputIt>
    void insert(const_iterator pos, InputIt first, InputIt last)
    {
        auto hint = v.cbegin() + pos.p;
        size_t min_index = size() - 1;
        for (auto it = first; it != last; ++it)
        {
            if (contains(it->first)) continue;
            pos = v.insert(pos, *it);
            min_index = std::min(pos.p, min_index);
            ++pos;
        }
        refresh_index(min_index);
    }

    template<typename InputIt>
    void insert(const_iterator pos, std::initializer_list<value_type> ilist)
    { insert(pos, ilist.begin(), ilist.end()); }

    template<typename... Args>
    iterator emplace_at(size_t pos, const key_type& key, Args&&... args)
    {
        T value;
        new(&value) T(std::forward<Args>(args)...);
        return insert(pos, key, std::move(value)).first;
    }

    template<typename... Args>
    iterator emplace_hint(const_iterator hint, const key_type& key, Args&&... args)
    {
        T value;
        new(&value) T(std::forward<Args>(args)...);
        return insert(hint, key, std::move(value)).first;
    }

    void pop_back()
    {
        key_type key = v.back().first;
        v.pop_back();
        m.erase(key);
    }

    void erase(const key_type& key)
    {
        auto it = m.find(key);
        if (it == m.cend()) return;
        erase(it);
    }

    void erase(size_type pos, size_type count = 1)
    {
        erase(cbegin() + pos, cbegin() + pos + count);
    }

    iterator erase(const_iterator pos)
    {
        m.erase(pos->first);
        v.erase(v.cbegin() + pos.p);
        refresh_index(pos.p);
    }

    iterator erase(const_iterator first, const_iterator last)
    {
        for (auto it = v.cbegin() + first.p; it != v.cbegin() + last.p; ++it)
        { m.erase(it->first); }
        v.erase(v.cbegin() + first.p, v.cbegin() + last.p);
        refresh_index(first.p);
    }

    void swap(SequencialMap& other)
    {
        v.swap(other.v);
        m.swap(other.m);
    }

    key_compare key_comp() const
    {
        return key_compare();
    }

    value_compare value_comp() const
    {
        return [](const value_type& left, const value_type& right)
               { return key_compare()(left, right); };
    }

    iterator begin()
    { return v.begin(); }

    const_iterator begin() const
    { return cbegin(); }

    iterator end()
    { return v.end(); }

    const_iterator end() const
    { return cend(); }

    const_iterator cbegin() const
    { return v.cbegin(); }

    const_iterator cend() const
    { return v.cend(); }

    reverse_iterator rbegin()
    { return reverse_iterator(end()); }

    reverse_iterator rend()
    { return reverse_iterator(begin()); }

    const_reverse_iterator crbegin() const
    { return const_reverse_iterator(cend()); }

    const_reverse_iterator crend() const
    { return const_reverse_iterator(cbegin()); }

    key_iterator key_begin() const
    { return key_iterator(cbegin()); }

    key_iterator key_end() const
    { return key_iterator(cend()); }

    reverse_key_iterator key_rbegin() const
    { return reverse_key_iterator(key_end()); }

    reverse_key_iterator key_rend() const
    { return reverse_key_iterator(key_begin()); }

    SequencialMap& operator=(const SequencialMap& other)
    {
        if (this == &other) return *this;
        v = other.v; m = other.m; return *this;
    }

    SequencialMap& operator=(SequencialMap&& other)
    { swap(SequencialMap(other)); return *this; }

    SequencialMap& operator=(std::initializer_list<value_type> ilist)
    { clear(); insert(ilist); return *this; }

    bool operator==(const SequencialMap& other) const
    { return v == other.v; }

    bool operator!=(const SequencialMap& other) const
    { return *this != other; }

    bool operator<(const SequencialMap& other) const
    { return v < other.v; }

    bool operator<=(const SequencialMap& other) const
    { return (*this < other) || (*this == other); }

    bool operator>(const SequencialMap& other) const
    { return !(*this <= other); }

    bool operator>=(const SequencialMap& other) const
    { return !(*this < other); }

    struct key_iterator : public const_iterator
    {
        using value_type = Key;
        using reference = const value_type&;
        using pointer = const value_type*;

        reference operator*() const
        { return (*this)->first; }

        pointer operator->() const
        { return &((*this)->first); }
    };

    template<typename Stream>
    Stream& to_string(Stream& out)
    {
        // SequencialMap(("a", 0)("b", 1)("c", 3)("d", 4)("e", 5)
        // ("f", 6)("g", 7)("h", 8)("i", 9)("k", 10)...)
        size_t count = std::min(size_t(10), size());
        out << "SequencialMap(";
        for (auto it = cbegin(); it != cbegin() + count; ++it)
        { out << '(' << it->first << ", " <<  it->second << ')'; }
        if (count < size()) out << "...";
        out << ')';
        return out;
    }

    template<typename Stream>
    Stream& serialize(Stream& out)
    {
        out << size();
        for (const value_type& value : *this)
        { out << value->first << value->second; }
        return out;
    }

    template<typename Stream>
    Stream& deserialize(Stream& in)
    {
        clear();
        size_t size;
        Key key;
        T value;
        in >> size;
        for (size_t i = 0; i < size; ++i)
        {
            in >> key >> value;
            insert(key, value);
        }
        return in;
    }

private:
    void refresh_index(size_t pos = 0)
    {
        pos = std::min(pos, size());
        for (size_t i = pos; i < size(); ++i)
        { m[v[i].first] = i; }
    }

    vector_type v;
    map_type m;
};
} // namespace Container

namespace std {
template<typename Key,
         typename T,
         typename Compare = std::less<Key>,
         typename Allocator = std::allocator<std::pair<const Key, T>>>
inline void swap(const Container::SequencialMap<Key, T, Compare, Allocator> lhs, const Container::SequencialMap<Key, T, Compare, Allocator> rhs) noexcept
{ lhs.swap(rhs); }
} // namespace std
