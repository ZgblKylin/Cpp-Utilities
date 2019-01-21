#pragma once

#include <cassert>
#include <utility>
#include <algorithm>
#include <map>
#include <vector>
#include <memory>
#include <iterator>
#include <type_traits>
#include <initializer_list>
#include <functional>
#include "../Common.h"

UTILITIES_NAMESPACE_BEGIN

namespace Container {
template<typename Key,
         typename T,
         typename Compare = std::less<Key>,
         typename Allocator = std::allocator<std::pair<const Key, T>>>
class SequencialMap
{
public:
    using allocator_type = Allocator;
    using map_type = std::map<Key, T, Compare, Allocator>;
    using vector_type = std::vector<typename map_type::iterator>;

    using key_type = typename map_type::key_type;
    using mapped_type = typename map_type::mapped_type;
    using key_compare = typename map_type::key_compare;
    using value_compare = typename map_type::value_compare;

    using value_type = typename map_type::value_type;
    using pointer = typename map_type::pointer;
    using const_pointer = typename map_type::const_pointer;
    using reference = typename map_type::reference;
    using const_reference = typename map_type::const_reference;
    using size_type = typename map_type::size_type;
    using difference_type = typename map_type::difference_type;

    template<bool constant> struct iterator_base;
    using iterator = iterator_base<false>;
    using const_iterator = iterator_base<true>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    struct key_iterator;
    using reverse_key_iterator = std::reverse_iterator<key_iterator>;

    SequencialMap()
    {
//        static_assert(!std::is_arithmetic<Key>::value, "Key type cannot not be arithmetic!");
    }

    explicit SequencialMap(const Compare& comp, const Allocator& alloc = Allocator())
        : m(comp, alloc)
    {
//        static_assert(!std::is_arithmetic<Key>::value, "Key type cannot not be arithmetic!");
    }

    template<typename InputIt>
    SequencialMap(InputIt first, InputIt last, const Compare& comp = Compare(), const Allocator& alloc = Allocator())
        : v(alloc), m(comp)
    {
//        static_assert(!std::is_arithmetic<Key>::value, "Key type cannot not be arithmetic!");
        push_back(first, last);
    }

    template<typename InputIt>
    SequencialMap(InputIt first, InputIt last, const Allocator& alloc)
        : v(alloc)
    {
//        static_assert(!std::is_arithmetic<Key>::value, "Key type cannot not be arithmetic!");
        push_back(first, last);
    }

    SequencialMap(const SequencialMap& other, const Allocator& alloc = Allocator())
        : v(alloc)
    { push_back(other.begin(), other.end()); }

    SequencialMap(SequencialMap&& other, const Allocator& alloc = Allocator())
        : v(std::move(other.v), alloc), m(std::move(other.m))
    {
    }

    SequencialMap(std::initializer_list<value_type> init,
                  const Compare& comp = Compare(),
                  const Allocator& alloc = Allocator())
        : v(alloc), m(comp)
    {
//        static_assert(!std::is_arithmetic<Key>::value, "Key type cannot not be arithmetic!");
        push_back(init);
    }

    ~SequencialMap() = default;

    [[nodiscard]] bool empty() const noexcept
    { return m.empty(); }

    size_type size() const noexcept
    { return m.size(); }

     size_type max_size() const noexcept
    { return m.max_size(); }

    void clear() noexcept
    { v.clear(); m.clear(); }

    bool contains(const key_type& key) const
    { return find(key) != cend(); }

    iterator find(const key_type& key)
    {
        return std::find_if(begin(), end(), [&key](const value_type& value){
            return value.first == key;
        });
    }

    const_iterator find(const key_type& key) const
    {
        return std::find_if(cbegin(), cend(), [&key](const value_type& value){
            return value.first == key;
        });
    }

    template<typename Container = std::vector<std::string>>
    Container keys() const
    {
        Container ret;
        for (auto it = key_begin(); it != key_end(); ++it)
        { ret.push_back(*it); }
        return ret;
    }

    const key_type key(const T& value, const key_type& defaultKey = key_type()) const
    {
        auto it = std::find_if(cbegin(), cend(), [&value](const value_type& v){
            return v.second == value;
        });
        if (it == cend()) return defaultKey;
        else return it->first;
    }

    const T& value(const key_type& key, const T& defaultValue = T()) const
    {
        auto it = find(key);
        if (it == cend()) return defaultValue;
        else return it->second;

    }

    reference at(size_type pos)
    { return *v.at(pos); }

    const_reference at(size_type pos) const
    { return *v.at(pos); }

    T& operator[](const key_type& key)
    {
        auto pair = m.insert(std::make_pair(key, T()));
        if (pair.second) v.push_back(pair.first);
        return pair.first->second;
    }

    T& operator[](key_type&& key)
    {
        auto pair = m.insert(std::make_pair(key, T()));
        if (pair.second) v.push_back(pair.first);
        return pair.first->second;
    }

    const T operator[](const key_type& key) const
    {
        auto it = find(key);
        if (it != cend()) return it->second;
        else return T();
    }

    const T operator[](key_type&& key) const
    {
        auto it = find(key);
        if (it != cend()) return it->second;
        else return T();
    }

    reference front()
    { return *begin(); }

    const_reference front() const
    { return *cbegin(); }

    reference back()
    { return *(end() - 1); }

    const_reference back() const
    { return *(cend() - 1); }

    SequencialMap mid(size_type pos) const
    { return mid(pos, size() - pos); }

    SequencialMap mid(size_type pos, size_type length) const
    {
        SequencialMap ret;
        if (pos >= size()) return ret;
        length = std::min(length, size() - pos);
        ret.insert(0, begin() + pos, begin() + pos + length);
        return ret;
    }

    std::pair<iterator, bool> push_back(const_reference value)
    {
        auto it = find(value.first);
        if (it != end()) return std::make_pair(it, false);
        auto pair = m.insert(value);
        v.push_back(pair.first);
        return std::make_pair(end() - 1, true);
    }

    std::pair<iterator, bool> push_back(value_type&& value)
    {
        value_type temp = value;
        auto it = find(temp.first);
        if (it != end()) return std::make_pair(it, false);
        auto pair = m.insert(std::move(temp));
        v.push_back(pair.first);
        return std::make_pair(end() - 1, true);
    }

    std::pair<iterator, bool> push_back(const key_type& key, const T& value)
    { return push_back(std::make_pair(key, value)); }

    std::pair<iterator, bool> push_back(const key_type& key, T&& value)
    { return push_back(std::make_pair(key, value)); }

    void push_back(const SequencialMap& other)
    { push_back(other.cbegin(), other.cend()); }

    void push_back(std::initializer_list<value_type> ilist)
    { push_back(ilist.begin(), ilist.end()); }

    template<typename InputIt>
    void push_back(InputIt first, InputIt last)
    {
        for (auto it = first; it != last; ++it)
        { push_back(*it); }
    }

    std::pair<iterator, bool> insert(const_reference value)
    { return push_back(value); }

    std::pair<iterator, bool> insert(value_type&& value)
    { return push_back(std::forward<value_type>(value)); }

    template<typename... Args>
    std::pair<iterator, bool> emplace_back(key_type&& key, Args&&... args)
    {
        return emplace_at(size(), std::forward<key_type>(key), std::forward<Args>(args)...);
    }

    SequencialMap operator+(const SequencialMap& other) const
    { auto ret = *this; ret.push_back(other.begin(), other.end()); return ret; }

    SequencialMap operator+(SequencialMap&& other) const
    {
        SequencialMap ret = *this;
        for (auto& value : other) { ret.push_back(std::move(value)); }
        return ret;
    }

    SequencialMap& operator+=(const SequencialMap& other)
    { push_back(other.begin(), other.end()); return *this; }

    SequencialMap& operator+=(SequencialMap&& other)
    {
        for (auto& value : other) { push_back(std::move(value)); }
        return *this;
    }

    iterator insert(size_t pos, const_reference value)
    {
        auto it = find(value.first);
        if (it != end()) return it;
        auto pair = m.insert(value);
        v.insert(v.begin() + pos, pair.first);
        return begin() + pos;
    }

    iterator insert(size_t pos, value_type&& value)
    {
        value_type temp = value;
        auto it = find(temp.first);
        if (it != end()) return it;
        auto pair = m.insert(std::move(temp));
        v.insert(v.begin() + pos, pair.first);
        return begin() + pos;
    }

    iterator insert(size_t pos, const key_type& key, const T& value)
    { return insert(pos, std::make_pair(key, value)); }

    iterator insert(size_t pos, const key_type& key, T&& value)
    { return insert(pos, std::make_pair(key, value)); }

    iterator insert(iterator pos, const_reference value)
    { return insert(pos - begin(), value); }

    iterator insert(iterator pos, value_type&& value)
    { return insert(pos - begin(), std::forward<value_type>(value)); }

    iterator insert(iterator pos, const key_type& key, const T& value)
    { return insert(pos, std::make_pair(key, value)); }

    iterator insert(iterator pos, const key_type& key, T&& value)
    { return insert(pos, std::make_pair(key, value)); }

    template<typename InputIt>
    void insert(size_t pos, InputIt first, InputIt last)
    { insert(begin() + pos, first, last); }

    void insert(size_t pos, std::initializer_list<value_type> ilist)
    { insert(begin() + pos, ilist); }

    template<typename InputIt>
    void insert(iterator pos, InputIt first, InputIt last)
    {
        difference_type index = pos - begin();
        for (auto it = first; it != last; ++it)
        {
            auto temp = find(it->first);
            if (temp != cend()) { continue; }
            else { insert(index, it->first, it->second); ++index; }
        }
    }

    void insert(iterator pos, std::initializer_list<value_type> ilist)
    { insert(pos, ilist.begin(), ilist.end()); }

    template<typename... Args>
    std::pair<iterator, bool> emplace_at(size_t pos, key_type&& key, Args&&... args)
    {
        key_type k = key;
        auto it = find(k);
        if (it != end()) return std::make_pair(it, false);
        auto pair = m.emplace(std::move(k), std::forward<Args>(args)...);
        v.insert(v.begin() + pos, pair.first);
        return std::make_pair(begin() + pos, true);
    }

    template<typename... Args>
    iterator emplace_hint(const_iterator hint, key_type&& key, Args&&... args)
    {
        return emplace_at(hint - cbegin(), std::forward<key_type>(key), std::forward<Args>(args)...).first;
    }

    void pop_back()
    {
        auto it = v.back();
        v.pop_back();
        m.erase(it);
    }

    void erase(const key_type& key)
    {
        auto it = find(key);
        if (it == cend()) return;
        else erase(it);
    }

    void erase(size_type pos, size_type count = 1)
    {
        erase(cbegin() + pos, cbegin() + pos + count);
    }

    iterator erase(const_iterator pos)
    {
        difference_type index = pos - cbegin();
        m.erase(*(pos.n));
        v.erase(v.begin() + (pos.n - v.data()));
        return begin() + index;
    }

    iterator erase(const_iterator first, const_iterator last)
    {
        iterator ret;
        for (auto it = const_reverse_iterator(last); it != const_reverse_iterator(first); ++it)
        { ret = erase(it.base() - 1); }
        return ret;
    }

    iterator begin()
    { return iterator(v.data()); }

    const_iterator begin() const
    { return cbegin(); }

    iterator end()
    { return iterator(v.data() + size()); }

    const_iterator end() const
    { return cend(); }

    const_iterator cbegin() const
    { return const_iterator(v.data()); }

    const_iterator cend() const
    { return const_iterator(v.data() + size()); }

    reverse_iterator rbegin()
    { return reverse_iterator(end()); }

    const_reverse_iterator rbegin() const
    { return crbegin(); }

    reverse_iterator rend()
    { return reverse_iterator(begin()); }

    const_reverse_iterator rend() const
    { return crend(); }

    const_reverse_iterator crbegin() const
    { return const_reverse_iterator(cend()); }

    const_reverse_iterator crend() const
    { return const_reverse_iterator(cbegin()); }

    key_iterator key_begin() const
    { return key_iterator(v.data()); }

    key_iterator key_end() const
    { return key_iterator(v.data() + size()); }

    reverse_key_iterator key_rbegin() const
    { return reverse_key_iterator(key_end()); }

    reverse_key_iterator key_rend() const
    { return reverse_key_iterator(key_begin()); }

    SequencialMap& operator=(const SequencialMap& other)
    {
        if (this == &other) return *this;
        clear(); push_back(other.begin(), other.end()); return *this;
    }

    SequencialMap& operator=(SequencialMap&& other)
    { other.swap(*this); return *this; }

    SequencialMap& operator=(std::initializer_list<value_type> ilist)
    { clear(); insert(ilist); return *this; }

    bool operator==(const SequencialMap& other) const
    { return m == other.m; }

    bool operator!=(const SequencialMap& other) const
    { return *this != other; }

    bool operator<(const SequencialMap& other) const
    { return m < other.m; }

    bool operator<=(const SequencialMap& other) const
    { return m <= other.n; }

    bool operator>(const SequencialMap& other) const
    { return m > other.m; }

    bool operator>=(const SequencialMap& other) const
    { return m >= other.m; }

    void swap(SequencialMap& other)
    {
        v.swap(other.v);
        m.swap(other.m);
    }

    key_compare key_comp() const
    {
        return m.key_comp();
    }

    value_compare value_comp() const
    {
        return m.value_comp();
    }

    template<typename Stream>
    friend Stream& operator<<(Stream& out, const SequencialMap& map)
    {
        // SequencialMap(("a",0),("b",1),("c",2),("d",3),("e",4),
        // ("f",5),("g",6),("h",7),("i",8),("k",9),...)
        size_t count = std::min(size_t(10u), map.size());
        out << "SequencialMap(";
        for (auto it = map.cbegin(); it != map.cbegin() + count; ++it)
        {
            out << '(' << it->first << ',' <<  it->second << ')';
            if (it != map.cbegin() + count - 1) out << ',';
        }
        if (count < map.size()) out << ",...";
        out << ')';
        return out;
    }

    struct SerializeManipulator;

    SerializeManipulator serialize() const
    { return SerializeManipulator{const_cast<SequencialMap&>(*this)}; }

    SerializeManipulator deserialize()
    { return SerializeManipulator{*this}; }

    struct SerializeManipulator
    {
        template<typename Stream>
        friend Stream& operator<<(Stream& out, SerializeManipulator&& manip)
        {
            out << manip.map.size();
            for (const value_type& value : manip.map)
            { out << value.first << value.second; }
            return out;
        }

        template<typename Stream>
        friend Stream& operator>>(Stream& in, SerializeManipulator&& manip)
        {
            manip.map.clear();
            size_t size;
            in >> size;
            for (size_t i = 0; i < size; ++i)
            {
                Key key;
                T value;
                in >> key >> value;
                manip.map.push_back(key, value);
            }
            return in;
        }

    private:
        SequencialMap& map;
    };

    template<bool constant>
    struct iterator_base
    {
        using iterator_category = std::random_access_iterator_tag;
        using difference_type = typename SequencialMap::difference_type;
        using node_type = typename SequencialMap::vector_type::value_type;
        using value_type = typename SequencialMap::value_type;
        using pointer = typename std::conditional<constant, const value_type*, value_type*>::type;
        using reference = typename std::conditional<constant, const value_type&, value_type&>::type;

        inline iterator_base() = default;

        template<bool OtherConstant>
        inline iterator_base(const iterator_base<OtherConstant>& other)
            : n(other.n)
        {
        }

        inline reference operator*() const
        { return n->operator*(); }

        inline pointer operator->() const
        { return n->operator->(); }

        template<bool OtherConstant>
        inline iterator_base& operator=(const iterator_base<OtherConstant>& other)
        { n = other.n; return *this; }

        template<bool otherConstant>
        inline bool operator==(const iterator_base<otherConstant>& other) const
        { return (n == other.n); }

        template<bool otherConstant>
        inline bool operator!=(const iterator_base<otherConstant>& other) const
        { return n != other.n; }

        template<bool otherConstant>
        inline bool operator<(const iterator_base<otherConstant>& other) const
        { return n < other.n; }

        template<bool otherConstant>
        inline bool operator<=(const iterator_base<otherConstant>& other) const
        { return n <= other.n; }

        template<bool otherConstant>
        inline bool operator>(const iterator_base<otherConstant>& other) const
        { return n > other.n; }

        template<bool otherConstant>
        inline bool operator>=(const iterator_base<otherConstant>& other) const
        { return n >= other.n; }

        inline iterator_base& operator++()
        { ++n; return *this; }

        inline iterator_base operator++(int)
        { node_type* node = n; ++n; return node; }

        inline iterator_base& operator--()
        { --n; return *this; }

        inline iterator_base operator--(int)
        { node_type* node = n; --n; return node; }

        inline iterator_base& operator+=(difference_type j)
        { n += j; return *this; }

        inline iterator_base& operator-=(difference_type j)
        { n -= j; return *this; }

        inline iterator_base operator+(difference_type j) const
        { return iterator_base(n + j); }

        friend inline iterator_base operator+(difference_type j, iterator_base& it)
        { return it + j; }

        inline iterator_base operator-(difference_type j) const
        { return iterator_base(n - j); }

        inline difference_type operator-(iterator_base j) const
        { return difference_type(n - j.n); }

    protected:
        inline iterator_base(const node_type* node)
            : n(const_cast<node_type*>(node))
        {
        }

        mutable node_type* n = nullptr;
        friend class SequencialMap;
        friend struct iterator_base<!constant>;
    };

    struct key_iterator : public iterator_base<true>
    {
        using value_type = Key;
        using reference = const value_type&;
        using pointer = const value_type*;
        using iterator_base<true>::iterator_base;
        using iterator_base<true>::operator=;
        friend class SequencialMap;

        reference operator*() const
        { return const_iterator::operator*().first; }

        pointer operator->() const
        { return &const_iterator::operator->()->first; }
    };

private:
    vector_type v;
    map_type m;
};
} // namespace Container

UTILITIES_NAMESPACE_END

namespace std {
template<typename Key,
         typename T,
         typename Compare = std::less<Key>,
         typename Allocator = std::allocator<std::pair<const Key, T>>>
inline void swap(Container::SequencialMap<Key, T, Compare, Allocator>& lhs, Container::SequencialMap<Key, T, Compare, Allocator>& rhs) noexcept
{ lhs.swap(rhs); }
} // namespace std
