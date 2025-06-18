#ifndef CPP_UTILITIES_CONTAINERS_SEQUENCIALMAP_HPP
#define CPP_UTILITIES_CONTAINERS_SEQUENCIALMAP_HPP

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

/**
 * \defgroup Containers Containers
 * \brief Classes and functions for some convenient containers.
 * \details
 *   Provide classes and functions for some convenient containers:
 *     - Container::SequencialMap : Key-value container behaves like std::map,
 *       but extended with random-access operations and traverses in the
 *       sequence order of value appends like `std::vector`.
 * @{
 */

UTILITIES_NAMESPACE_BEGIN

/**
 * \namespace Container
 * \brief Namespace for all classes and functions of convenient containers.
 *        See \ref Containers for more instrucion.
 * @{
 */
namespace Container {
/**
 * \brief Key-value container behaves like std::map, but extended with
 *        random-access operations and traverses in the sequence order of value
 *        appends like `std::vector`.
 * \tparam  Key       Key type of input maps.
 * \tparam  T         Value type of input maps.
 * \tparam  Compare   Comparison function object to use for all comparisons of
 *                    keys.
 * \tparam  Allocator Allocator to use for all memory allocations of this
 *                    container.
 * \details
 *   Same API as `std::map`, but add more APIs from `std::vector` to extend
 *   random-access operations.\n
 *   All iterators and random-access operations traverse the map in the sequence
 *   of value appends like `std::vector`.\n
 *   \n
 *   **Iterator and Reference Invalidation**\n
 *   Iterator invalidation of modify operations behave like `std::vector`.\n
 *   Reference invalidation of modify operations behave like `std::map`.\n
 *   \n
 *   **Algorithmic Complexity**\n
 *   Most operations of SequencialMap have the same algorithmic complexity as
 *   `std::map`, while random-access operations share the same complexity as
 *   `std::vector`.\n
 *     - Key lookup: O(log _n_)
 *     - Index lookup: O(1)
 *     - Insertion/Erase: O(_n_) (Much faster than raw `std::vector`, because
 *       moved values are `std::map::iterator`, not acture `T` node.)
 *     - Appending: O(log _n_)
 */
template<typename Key,
         typename T,
         typename Compare = std::less<Key>,
         typename Allocator = std::allocator<std::pair<const Key, T>>>
class SequencialMap
{
public:
    /**
     * \brief Provide same member type of `std::map`.
     */
    using allocator_type = Allocator;
    /**
     * \brief Underlying map type for map APIs.
     */
    using map_type = std::map<Key, T, Compare, Allocator>;
    /**
     * \brief Underlying map type for random-access operations and sequencial
     *        traversal.
     */
    using vector_type = std::vector<typename map_type::iterator>;

    /**
     * \brief Provide same member type of `std::map`.
     */
    using key_type = typename map_type::key_type;
    /**
     * \brief Provide same member type of `std::map`.
     */
    using mapped_type = typename map_type::mapped_type;
    /**
     * \brief Provide same member type of `std::map`.
     */
    using key_compare = typename map_type::key_compare;
    /**
     * \brief Provide same member type of `std::map`.
     */
    using value_compare = typename map_type::value_compare;

    /**
     * \brief Provide same member type of `std::map`.
     */
    using value_type = typename map_type::value_type;
    /**
     * \brief Provide same member type of `std::map`.
     */
    using pointer = typename map_type::pointer;
    /**
     * \brief Provide same member type of `std::map`.
     */
    using const_pointer = typename map_type::const_pointer;
    /**
     * \brief Provide same member type of `std::map`.
     */
    using reference = typename map_type::reference;
    /**
     * \brief Provide same member type of `std::map`.
     */
    using const_reference = typename map_type::const_reference;
    /**
     * \brief Provide same member type of `std::map`.
     */
    using size_type = typename map_type::size_type;
    /**
     * \brief Provide same member type of `std::map`.
     */
    using difference_type = typename map_type::difference_type;

    // Forward declaration
    template<bool constant> struct iterator_base;
    /**
     * \brief Mutable iterator type for `LegacyRandomAccessIterator`.
     */
    using iterator = iterator_base<false>;
    /**
     * \brief Immutable iterator type for constant `LegacyRandomAccessIterator`.
     */
    using const_iterator = iterator_base<true>;
    /**
     * \brief Mutable reverse iterator type.
     */
    using reverse_iterator = std::reverse_iterator<iterator>;
    /**
     * \brief Immutable reverse iterator type.
     */
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    // Forward declaration
    struct key_iterator;
    /**
     * \brief Reverse iterator to traverse keys.
     */
    using reverse_key_iterator = std::reverse_iterator<key_iterator>;

    /**
     * \brief Default constructor, constructs an empty container.
     */
    SequencialMap()
    {
//        static_assert(!std::is_arithmetic<Key>::value, "Key type cannot not be arithmetic!");
    }

    /**
     * \brief Constructs an empty container with given comparator and allocator.
     * \param comp  Comparison function object given for this container.
     * \param alloc Allocator given for this container.
     */
    explicit SequencialMap(const Compare& comp, const Allocator& alloc = Allocator())
        : m(comp, alloc)
    {
//        static_assert(!std::is_arithmetic<Key>::value, "Key type cannot not be arithmetic!");
    }

    /**
     * \brief Constructs the container with the contents of the range
     *        `[first, last)`. If multiple elements in the range have keys that
     *        compare equivalent, only the first element is inserted.
     * \param first Iterator to the first element to copy from.
     * \param last  Iterator after the last element to copy from.
     * \param comp  Comparison function object given for this container.
     * \param alloc Allocator given for this container.
     */
    template<typename InputIt>
    SequencialMap(InputIt first, InputIt last, const Compare& comp = Compare(), const Allocator& alloc = Allocator())
        : m(comp, alloc)
    {
//        static_assert(!std::is_arithmetic<Key>::value, "Key type cannot not be arithmetic!");
        push_back(first, last);
    }

    /**
     * \brief Constructs the container with the contents of the range
     *        `[first, last)`. If multiple elements in the range have keys that
     *        compare equivalent, only the first element is inserted.
     * \param first Iterator to the first element to copy from.
     * \param last  Iterator after the last element to copy from.
     * \param alloc Allocator given for this container.
     */
    template<typename InputIt>
    SequencialMap(InputIt first, InputIt last, const Allocator& alloc)
        : m(Compare(), alloc)
    {
//        static_assert(!std::is_arithmetic<Key>::value, "Key type cannot not be arithmetic!");
        push_back(first, last);
    }

    /**
     * \brief Copy constructor. Constructs the container with the copy of the
     *        contents of `other`. If `alloc` is not provided, allocator is
     *        obtained by calling
     *        ```cpp
     *        std::allocator_traits<allocator_type>::select_on_container_copy_construction(other.get_allocator())
     *        ```
     * \param other Another container to be used as source to initialize the
     *              elements of the container with.
     * \param alloc Allocator given for this container.
     */
    SequencialMap(const SequencialMap& other, const Allocator& alloc = Allocator())
        : m(Compare(), alloc)
    { push_back(other.begin(), other.end()); }

    /**
     * \brief Move constructor. Constructs the container with the contents of
     *        `other` using move semantics. If alloc is not provided, allocator
     *        is obtained by move-construction from the allocator belonging to
     *        `other`.
     * \param other Another container to be used as source to initialize the
     *              elements of the container with.
     * \param alloc Allocator given for this container.
     */
    SequencialMap(SequencialMap&& other, const Allocator& alloc = Allocator())
        : v(std::forward<vector_type>(other.v), alloc), m(std::forward<map_type>(other.m))
    {
    }

    /**
     * \brief Constructs the container with the contents of the initializer list
     *        `init`. If multiple elements in the range have keys that compare
     *        equivalent, only the first element is inserted.
     * \param init  Initializer list with `value_type` elements.
     * \param comp  Comparison function object given for this container.
     * \param alloc Allocator given for this container.
     */
    SequencialMap(std::initializer_list<value_type> init,
                  const Compare& comp = Compare(),
                  const Allocator& alloc = Allocator())
        : v(alloc), m(comp)
    {
//        static_assert(!std::is_arithmetic<Key>::value, "Key type cannot not be arithmetic!");
        push_back(init);
    }

    /**
     * \brief Destructs the container. The destructors of the elements are
     *        called and the used storage is deallocated. Note, that if the
     *        elements are pointers, the pointed-to objects are not destroyed.
     */
    ~SequencialMap() = default;

    /**
     * \brief Returns the allocator associated with the container.
     * \return The associated allocator.
     */
    allocator_type get_allocator() const
    { return m.get_allocator(); }

    /**
     * \brief Checks if the container has no elements, i.e. whether
     *        `begin() == end()`.
     * \return `true` if the container is empty, `false` otherwise.
     * \details
     *  **Complexity**\n
     *  Constant.
     */
    bool empty() const noexcept
    { return m.empty(); }

    /**
     * \brief Returns the number of elements in the container, i.e.
     *        `std::distance(begin(), end())`.
     * \return The number of elements in the container.
     * \details
     *   **Complexity**\n
     *   Constant.
     */
    size_type size() const noexcept
    { return m.size(); }

    /**
     * \brief Returns the maximum number of elements the container is able to
     *        hold due to system or library implementation limitations, i.e.
     *        `std::distance(begin(), end())` for the largest container.
     * \return Maximum number of elements.
     * \details
     *   This value typically reflects the theoretical limit on the size of the
     *   container, at most `std::numeric_limits<difference_type>::max()`. At
     *   runtime, the size of the container may be limited to a value smaller
     *   than `max_size()` by the amount of RAM available.\n
     *   **Complexity**\n
     *   Constant.
     */
     size_type max_size() const noexcept
    { return m.max_size(); }

     /**
     * \@brief Erases all elements from the container. After this call, `size()`
     *         returns zero.
     * \details
     *   Invalidates any references, pointers, or iterators referring to
     *   contained elements. Any past-the-end iterator remains valid.
     * \details
     *   **Complexity**\n
     *   Linear in the size of the container, i.e., the number of elements.
     */
    void clear() noexcept
    { v.clear(); m.clear(); }

    /**
     * \brief Checks if there is an element with key equivalent to key in the
     *        container.
     * \param key Key value of the element to search for.
     * \return `true` if there is such an element, otherwise `false`.
     * \details
     *   **Complexity**\n
     *   Logarithmic in the size of the container.
     */
    bool contains(const key_type& key) const
    { return find(key) != cend(); }

    /**
     * \brief Finds an element with key equivalent to key.
     * \param key Key value of the element to search for.
     * \return Iterator to an element with key equivalent to `key`. If no such
     *         element is found, past-the-end (see `end()`) iterator is returned.
     * \details
     *   **Complexity**\n
     *   Logarithmic in the size of the container.
     */
    iterator find(const key_type& key)
    {
        return std::find_if(begin(), end(), [&key](const value_type& value){
            return value.first == key;
        });
    }

    /**
     * \brief Finds an element with key equivalent to key.
     * \param key Key value of the element to search for.
     * \return Iterator to an element with key equivalent to `key`. If no such
     *         element is found, past-the-end (see `end()`) iterator is returned.
     * \details
     *   **Complexity**\n
     *   Logarithmic in the size of the container.
     */
    const_iterator find(const key_type& key) const
    {
        return std::find_if(cbegin(), cend(), [&key](const value_type& value){
            return value.first == key;
        });
    }

    /**
     * \brief Returns a list containing all the keys in the map in the sequence
     *        order of value appends.
     * \tparam Container Vector-like container to contain return keys.
     * \return List containing all the keys in the map in the sequence order of
     *         value appends.
     * \details
     *   The order is guaranteed to be the same as that used by `values()`.
     * \details
     *   **Complexity**\n
     *   Linear in the size of the container, i.e., the number of elements.
     * \sa values, key
     */
    template<typename Container = std::vector<key_type>>
    Container keys() const
    {
        Container ret;
        for (auto it = key_begin(); it != key_end(); ++it)
        { ret.push_back(*it); }
        return ret;
    }

    /**
     * \brief Returns the key with value `value`, or `defaultKey` if the map
     *        contains no item with value `value`.
     * \param value      Value to find key from.
     * \param defaultKey Default return if the map contains no item with value
     *                   `value`.
     * \return The key with value `value`, or `defaultKey` if the map contains no
     *         item with value `value`.
     * \details
     *   This function can be slow(linear time), because its internal data
     *   structure is optimized for fast lookup by key, not by value.\n
     *   **Complexity**\n
     *   Linear in the size of the container, i.e., the number of elements.
     */
    const key_type key(const T& value, const key_type& defaultKey = key_type()) const
    {
        auto it = std::find_if(cbegin(), cend(), [&value](const value_type& v){
            return v.second == value;
        });
        if (it == cend()) return defaultKey;
        else return it->first;
    }

    /**
     * \brief Returns a list containing all the values in the map in the
     *        sequence order of value appends.
     * \tparam Container Vector-like container to contain return values.
     * \return List containing all the values in the map in the sequence order
     *         of value appends.
     * \details
     *   The order is guaranteed to be the same as that used by `values()`.
     * \details
     *   **Complexity**\n
     *   Linear in the size of the container, i.e., the number of elements.
     * \sa keys, value
     */
    template<typename Container = std::vector<T>>
    Container values() const
    {
        Container ret;
        for (auto it = cbegin(); it != cend(); ++it)
        { ret.push_back(it->second); }
        return ret;
    }

    /**
     * \brief Returns the value associated with the key `key`.
     * \param key          Key to find value from.
     * \param defaultValue Default return value if the map contains no item with
     *                     key `key`.
     * \return The key with value `value`, or `defaultKey` if the map contains no
     *         item with value `value`.
     * \details
     *   If the map contains no item with key `key`, the function returns
     *   `defaultValue`.
     * \details
     *   **Complexity**\n
     *   Logarithmic in the size of the container.
     */
    const T& value(const key_type& key, const T& defaultValue = T()) const
    {
        auto it = find(key);
        if (it == cend()) return defaultValue;
        else return it->second;
    }

    /**
     * \brief Returns a reference to the element at specified location `pos`,
     *        with bounds checking.
     * \param pos Position of the element to return
     * \return Reference to the requested element.
     * \exception std::out_of_range
     *   If pos is not within the range of the container, i.e.
     *   `if !(pos < size())`, an exception of type `std::out_of_range` is thrown.
     * \details
     *   **Complexity**\n
     *   Constant.
     */
    reference at(size_type pos)
    { return *v.at(pos); }

    /**
     * \brief Returns a const reference to the element at specified location
     *        `pos`, with bounds checking.
     * \param pos Position of the element to return
     * \return Const reference to the requested element.
     * \exception std::out_of_range
     *   If pos is not within the range of the container, i.e.
     *   `if !(pos < size())`, an exception of type `std::out_of_range` is thrown.
     * \details
     *   **Complexity**\n
     *   Constant.
     */
    const_reference at(size_type pos) const
    { return *v.at(pos); }

    /**
     * \brief Returns a reference to the value that is mapped to a key
     *        equivalent to `key`, performing an insertion if such key does not
     *        already exist.
     * \param key The key of the element to find.
     * \return Reference to the mapped value of the new element if no element
     *         with key `key` existed. Otherwise a reference to the mapped value
     *         of the existing element whose key is equivalent to `key`.
     * \details
     *   **Complexity**\n
     *   Logarithmic in the size of the container.
     */
    T& operator[](const key_type& key)
    {
        auto pair = m.insert(std::make_pair(key, T()));
        if (pair.second) v.push_back(pair.first);
        return pair.first->second;
    }

    /**
     * \brief Returns a reference to the value that is mapped to a key
     *        equivalent to `key`, performing an insertion if such key does not
     *        already exist.
     * \param key The key of the element to find.
     * \return Reference to the mapped value of the new element if no element
     *         with key `key` existed. Otherwise a reference to the mapped value
     *         of the existing element whose key is equivalent to `key`.
     * \details
     *   **Complexity**\n
     *   Logarithmic in the size of the container.
     */
    T& operator[](key_type&& key)
    {
        auto pair = m.insert(std::make_pair(key, T()));
        if (pair.second) v.push_back(pair.first);
        return pair.first->second;
    }

    /**
     * \brief Returns a copy to the value that is mapped to a key equivalent to
     *        `key`, return a default constructed value if such key does not
     *        already exist.
     * \param key The key of the element to find.
     * \return Copy to the mapped value of if element with key `key` existed.
     *         Otherwise a default constructed value is returned.
     * \details
     *   **Complexity**\n
     *   Logarithmic in the size of the container.
     */
    const T operator[](const key_type& key) const
    {
        auto it = find(key);
        if (it != cend()) return it->second;
        else return T();
    }

    /**
     * \brief Returns a copy to the value that is mapped to a key equivalent to
     *        `key`, return a default constructed value if such key does not
     *        already exist.
     * \param key The key of the element to find.
     * \return Copy to the mapped value of if element with key `key` existed.
     *         Otherwise a default constructed value is returned.
     * \details
     *   **Complexity**\n
     *   Logarithmic in the size of the container.
     */
    const T operator[](key_type&& key) const
    {
        auto it = find(key);
        if (it != cend()) return it->second;
        else return T();
    }

    /**
     * \brief Returns a reference to the first element in the container.
     * \return Reference to the first element.
     * \warning Calling front on an empty container is undefined.
     * \details
     *   **Complexity**\n
     *   Constant.
     */
    reference front()
    { return *begin(); }

    /**
     * \brief Returns a const reference to the first element in the container.
     * \return Const reference to the first element.
     * \warning Calling front on an empty container is undefined.
     * \details
     *   **Complexity**\n
     *   Constant.
     */
    const_reference front() const
    { return *cbegin(); }

    /**
     * \brief Returns a reference to the last element in the container.
     * \return Reference to the last element.
     * \warning Calling front on an empty container is undefined.
     * \details
     *   **Complexity**\n
     *   Constant.
     */
    reference back()
    { return *(end() - 1); }

    /**
     * \brief Returns a const reference to the last element in the container.
     * \return Const reference to the last element.
     * \warning Calling front on an empty container is undefined.
     * \details
     *   **Complexity**\n
     *   Constant.
     */
    const_reference back() const
    { return *(cend() - 1); }

    /**
     * \brief Returns a sub-map which contains elements from this map, starting
     *        at position `pos` to the end.
     * \param pos First element position.
     * \return Sub-map contains all elements starting from position `pos`.
     * \details
     *   **Complexity**\n
     *   Linear in the size of the return container, i.e., the number of
     *   elements starting from `pos`.
     */
    SequencialMap mid(size_type pos) const
    { return mid(pos, size() - pos); }

    /**
     * \brief Returns a sub-map which contains elements from this map, starting
     *        at position `pos`, with `length` elements (or all remaining elements
     *        if there are less than `length` elements) are included.
     * \param pos    First element position.
     * \param length Elements numbers from `pos` to be included.
     * \return Sub-map contains `length` elements starting at position `pos`.
     * \details
     *   **Complexity**\n
     *   Linear in the size of the return container, i.e., the number of
     *   `length`.
     */
    SequencialMap mid(size_type pos, size_type length) const
    {
        SequencialMap ret;
        if (pos >= size()) return ret;
        length = std::min(length, size() - pos);
        ret.insert(0, begin() + pos, begin() + pos + length);
        return ret;
    }

    /**
     * \brief Appends the given element value to the end of the container, if
     *        the container doesn't already contain an element with an
     *        equivalent key.
     * \param value The element to append.
     * \return Returns a pair consisting of an iterator to the inserted element
     *         (or to the element that prevented the insertion) and a `bool`
     *         denoting whether the insertion took place.
     * \details
     *   **Complexity**\n
     *   Logarithmic in the size of the container.
     */
    std::pair<iterator, bool> push_back(const_reference value)
    {
        auto it = find(value.first);
        if (it != end()) return std::make_pair(it, false);
        auto pair = m.insert(value);
        v.push_back(pair.first);
        return std::make_pair(end() - 1, true);
    }

    /**
     * \brief Appends the given element value to the end of the container, if
     *        the container doesn't already contain an element with an
     *        equivalent key.
     * \param value The element to append.
     * \return Returns a pair consisting of an iterator to the inserted element
     *         (or to the element that prevented the insertion) and a `bool`
     *         denoting whether the insertion took place.
     * \details
     *   **Complexity**\n
     *   Logarithmic in the size of the container.
     */
    std::pair<iterator, bool> push_back(value_type&& value)
    {
        value_type temp(std::forward<value_type>(value));
        auto it = find(temp.first);
        if (it != end()) return std::make_pair(it, false);
        auto pair = m.insert(std::move(temp));
        v.push_back(pair.first);
        return std::make_pair(end() - 1, true);
    }

    /**
     * \brief Appends the given element value to the end of the container, if
     *        the container doesn't already contain an element with an
     *        equivalent key.
     * \param key   The key of the element to append.
     * \param value The value of the element to append.
     * \return Returns a pair consisting of an iterator to the inserted element
     *         (or to the element that prevented the insertion) and a `bool`
     *         denoting whether the insertion took place.
     * \details
     *   **Complexity**\n
     *   Logarithmic in the size of the container.
     */
    std::pair<iterator, bool> push_back(const key_type& key, const T& value)
    { return push_back(std::make_pair(key, value)); }

    /**
     * \brief Appends the given element value to the end of the container, if
     *        the container doesn't already contain an element with an
     *        equivalent key.
     * \param key   The key of the element to append.
     * \param value The value of the element to append.
     * \return Returns a pair consisting of an iterator to the inserted element
     *         (or to the element that prevented the insertion) and a `bool`
     *         denoting whether the insertion took place.
     * \details
     *   **Complexity**\n
     *   Logarithmic in the size of the container.
     */
    std::pair<iterator, bool> push_back(const key_type& key, T&& value)
    { return push_back(std::make_pair(key, std::forward<T>(value))); }

    /**
     * \brief Appends all elements from given container `other` to the end of the
     *        container, ignores all values with keys already exists in the
     *        container.
     * \param other Another container to append all elements from.
     * \details
     *   **Complexity**\n
     *   `O(N*log(size() + N))`, where N is the number of elements to insert.
     */
    void push_back(const SequencialMap& other)
    { push_back(other.cbegin(), other.cend()); }

    /**
     * \brief Appends all elements from initializer list `ilist` to the end of
     *        the container, ignores all values with keys already exists in the
     *        container.
     * \param other Initializer list to append all elements from.
     * \details
     *   **Complexity**\n
     *   `O(N*log(size() + N))`, where N is the number of elements to insert.
     */
    void push_back(std::initializer_list<value_type> ilist)
    { push_back(ilist.begin(), ilist.end()); }

    /**
     * \brief Appends all elements from from range `[first, last)` to the end of
     *        the container, ignores all values with keys already exists in the
     *        container.
     * \param first Iterator to the first element to append from.
     * \param last  Iterator after the last element to append from.
     * \details
     *   **Complexity**\n
     *   `O(N*log(size() + N))`, where N is the number of elements to insert.
     */
    template<typename InputIt>
    void push_back(InputIt first, InputIt last)
    {
        for (auto it = first; it != last; ++it)
        { push_back(*it); }
    }

    /**
     * \brief Appends a new element to the end of the container.
     * \tparam Args Arguments to forward to the constructor of the element.
     * \param key  The key of the element to append.
     * \param args Arguments to forward to the constructor of value.
     * \return Returns a pair consisting of an iterator to the inserted element
     *         (or to the element that prevented the insertion) and a `bool`
     *         denoting whether the insertion took place.
     * \details
     *   The element is constructed through `std::allocator_traits::construct`,
     *   which typically uses placement-new to construct the element in-place at
     *   the location provided by the container. The arguments `args...` are
     *   forwarded to the constructor as `std::forward<Args>(args)...`.\n
     * \details
     *   **Complexity**\n
     *   Logarithmic in the size of the container.
     */
    template<typename... Args>
    std::pair<iterator, bool> emplace_back(const key_type& key, Args&&... args)
    {
        return emplace_at(size(), key, std::forward<Args>(args)...);
    }

    /**
     * \brief Appends a new element to the end of the container.
     * \tparam Args Arguments to forward to the constructor of the element.
     * \param key  The key of the element to append.
     * \param args Arguments to forward to the constructor of value.
     * \return Returns a pair consisting of an iterator to the inserted element
     *         (or to the element that prevented the insertion) and a `bool`
     *         denoting whether the insertion took place.
     * \details
     *   The element is constructed through `std::allocator_traits::construct`,
     *   which typically uses placement-new to construct the element in-place at
     *   the location provided by the container. The arguments `args...` are
     *   forwarded to the constructor as `std::forward<Args>(args)...`.\n
     * \details
     *   **Complexity**\n
     *   Logarithmic in the size of the container.
     */
    template<typename... Args>
    std::pair<iterator, bool> emplace_back(key_type&& key, Args&&... args)
    {
        return emplace_at(size(), std::forward<key_type>(key), std::forward<Args>(args)...);
    }

    /**
     * \brief Same as push_back, appends all elements from given container
     *        `other` to the end of the container, ignores all values with keys
     *        already exists in the container.
     * \param other Another container to append all elements from.
     * \details
     *   **Complexity**\n
     *   `O(N*log(size() + N))`, where N is the number of elements to insert.
     */
    SequencialMap operator+(const SequencialMap& other) const
    { auto ret = *this; ret.push_back(other.begin(), other.end()); return ret; }

    /**
     * \brief Same as push_back, appends all elements from given container
     *        `other` to the end of the container, ignores all values with keys
     *        already exists in the container.
     * \param other Another container to append all elements from.
     * \details
     *   **Complexity**\n
     *   `O(N*log(size() + N))`, where N is the number of elements to insert.
     */
    SequencialMap operator+(SequencialMap&& other) const
    {
        SequencialMap ret = *this;
        for (auto&& value : other) {
            ret.push_back(std::forward<value_type>(value));
        }
        return ret;
    }

    /**
     * \brief Same as push_back, appends all elements from given container
     *        `other` to the end of the container and return `*this`, ignores all
     *        values with keys already exists in the container.
     * \param other Another container to append all elements from.
     * \return `*this` after appends.
     * \details
     *   **Complexity**\n
     *   `O(N*log(size() + N))`, where N is the number of elements to insert.
     */
    SequencialMap& operator+=(const SequencialMap& other)
    { push_back(other.begin(), other.end()); return *this; }

    /**
     * \brief Same as push_back, appends all elements from given container
     *        `other` to the end of the container and return `*this`, ignores all
     *        values with keys already exists in the container.
     * \param other Another container to append all elements from.
     * \return `*this` after appends.
     * \details
     *   **Complexity**\n
     *   `O(N*log(size() + N))`, where N is the number of elements to insert.
     */
    SequencialMap& operator+=(SequencialMap&& other)
    {
        for (auto&& value : other) {
            push_back(std::forward<value_type>(value));
        }
        return *this;
    }

    /**
     * \brief Inserts element into the container, if the container doesn't
     *        already contain an element with an equivalent key.
     * \param pos   Index to the position before which the new element will be
     *              inserted.
     * \param value Element to insert.
     * \return An iterator to the inserted element, or to the element that
     *         prevented the insertion.
     * \details
     *   **Complexity**\n
     *   Linear in the size of the container, i.e., the number of elements.
     */
    iterator insert(size_t pos, const_reference value)
    {
        auto it = find(value.first);
        if (it != end()) return it;
        auto pair = m.insert(value);
        v.insert(v.begin() + pos, pair.first);
        return begin() + pos;
    }

    /**
     * \brief Inserts element into the container, if the container doesn't
     *        already contain an element with an equivalent key.
     * \param pos   Index to the position before which the new element will be
     *              inserted.
     * \param value Element to insert.
     * \return An iterator to the inserted element, or to the element that
     *         prevented the insertion.
     * \details
     *   **Complexity**\n
     *   Linear in the size of the container, i.e., the number of elements.\n
     *   Much faster than raw `std::vector`, because moved values are
     *   `std::map::iterator`, not acture `T` node.
     */
    iterator insert(size_t pos, value_type&& value)
    {
        value_type temp(std::forward<value_type>(value));
        auto it = find(temp.first);
        if (it != end()) return it;
        auto pair = m.insert(std::move(temp));
        v.insert(v.begin() + pos, pair.first);
        return begin() + pos;
    }

    /**
     * \brief Inserts element into the container, if the container doesn't
     *        already contain an element with an equivalent key.
     * \param pos   Index to the position before which the new element will be
     *              inserted.
     * \param key   Key of element to insert.
     * \param value Value of element to insert.
     * \return An iterator to the inserted element, or to the element that
     *         prevented the insertion.
     * \details
     *   **Complexity**\n
     *   Linear in the size of the container, i.e., the number of elements.\n
     *   Much faster than raw `std::vector`, because moved values are
     *   `std::map::iterator`, not acture `T` node.
     */
    iterator insert(size_t pos, const key_type& key, const T& value)
    { return insert(pos, std::make_pair(key, value)); }

    /**
     * \brief Inserts element into the container, if the container doesn't
     *        already contain an element with an equivalent key.
     * \param pos   Index to the position before which the new element will be
     *              inserted.
     * \param key   Key of element to insert.
     * \param value Value of element to insert.
     * \return An iterator to the inserted element, or to the element that
     *         prevented the insertion.
     * \details
     *   **Complexity**\n
     *   Linear in the size of the container, i.e., the number of elements.\n
     *   Much faster than raw `std::vector`, because moved values are
     *   `std::map::iterator`, not acture `T` node.
     */
    iterator insert(size_t pos, const key_type& key, T&& value)
    { return insert(pos, std::make_pair(key, std::forward<T>(value))); }

    /**
     * \brief Inserts element into the container, if the container doesn't
     *        already contain an element with an equivalent key.
     * \param pos   Iterator to the position before which the new element will
     *              be inserted.
     * \param value Element to insert.
     * \return An iterator to the inserted element, or to the element that
     *         prevented the insertion.
     * \details
     *   **Complexity**\n
     *   Linear in the size of the container, i.e., the number of elements.\n
     *   Much faster than raw `std::vector`, because moved values are
     *   `std::map::iterator`, not acture `T` node.
     */
    iterator insert(iterator pos, const_reference value)
    { return insert(pos - begin(), value); }

    /**
     * \brief Inserts element into the container, if the container doesn't
     *        already contain an element with an equivalent key.
     * \param pos   Iterator to the position before which the new element will
     *              be inserted.
     * \param value Element to insert.
     * \return An iterator to the inserted element, or to the element that
     *         prevented the insertion.
     * \details
     *   **Complexity**\n
     *   Linear in the size of the container, i.e., the number of elements.\n
     *   Much faster than raw `std::vector`, because moved values are
     *   `std::map::iterator`, not acture `T` node.
     */
    iterator insert(iterator pos, value_type&& value)
    { return insert(pos - begin(), std::forward<value_type>(value)); }

    /**
     * \brief Inserts element into the container, if the container doesn't
     *        already contain an element with an equivalent key.
     * \param pos   Iterator to the position before which the new element will
     *              be inserted.
     * \param key   Key of element to insert.
     * \param value Value of element to insert.
     * \return An iterator to the inserted element, or to the element that
     *         prevented the insertion.
     * \details
     *   **Complexity**\n
     *   Linear in the size of the container, i.e., the number of elements.\n
     *   Much faster than raw `std::vector`, because moved values are
     *   `std::map::iterator`, not acture `T` node.
     */
    iterator insert(iterator pos, const key_type& key, const T& value)
    { return insert(pos, std::make_pair(key, value)); }

    /**
     * \brief Inserts element into the container, if the container doesn't
     *        already contain an element with an equivalent key.
     * \param pos   Iterator to the position before which the new element will
     *              be inserted.
     * \param key   Key of element to insert.
     * \param value Value of element to insert.
     * \return An iterator to the inserted element, or to the element that
     *         prevented the insertion.
     * \details
     *   **Complexity**\n
     *   Linear in the size of the container, i.e., the number of elements.\n
     *   Much faster than raw `std::vector`, because moved values are
     *   `std::map::iterator`, not acture `T` node.
     */
    iterator insert(iterator pos, const key_type& key, T&& value)
    { return insert(pos, std::make_pair(key, std::forward<T>(value))); }

    /**
     * \brief Inserts elements into the container, if the container doesn't
     *        already contain an element with an equivalent key.
     * \tparam InputIt Must meet the requirements of LegacyInputIterator.
     * \param pos   Index to the position before which the new element will be
     *              inserted.
     * \param first Iterator to the first element to insert.
     * \param last  Iterator after the last element to insert.
     * \details
     *   **Complexity**\n
     *   `O(N*log(size() + N))`, where N is the number of elements to insert.\n
     *   Much faster than raw `std::vector`, because moved values are
     *   `std::map::iterator`, not acture `T` node.
     */
    template<typename InputIt>
    void insert(size_t pos, InputIt first, InputIt last)
    { insert(begin() + pos, first, last); }

    /**
     * \brief Inserts elements into the container, if the container doesn't
     *        already contain an element with an equivalent key.
     * \param pos   Index to the position before which the new element will be
     *              inserted.
     * \param ilist Initializer list to insert the values from.
     * \details
     *   **Complexity**\n
     *   `O(N*log(size() + N))`, where N is the number of elements to insert.\n
     *   Much faster than raw `std::vector`, because moved values are
     *   `std::map::iterator`, not acture `T` node.
     */
    void insert(size_t pos, std::initializer_list<value_type> ilist)
    { insert(begin() + pos, ilist); }

    /**
     * \brief Inserts elements into the container, if the container doesn't
     *        already contain an element with an equivalent key.
     * \tparam InputIt Must meet the requirements of LegacyInputIterator.
     * \param pos   Iterator to the position before which the new element will
     *              be inserted.
     * \param first Iterator to the first element to insert.
     * \param last  Iterator after the last element to insert.
     * \details
     *   **Complexity**\n
     *   `O(N*log(size() + N))`, where N is the number of elements to insert.\n
     *   Much faster than raw `std::vector`, because moved values are
     *   `std::map::iterator`, not acture `T` node.
     */
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

    /**
     * \brief Inserts elements into the container, if the container doesn't
     *        already contain an element with an equivalent key.
     * \param pos   Iterator to the position before which the new element will
     *              be inserted.
     * \param ilist Initializer list to insert the values from.
     * \details
     *   **Complexity**\n
     *   `O(N*log(size() + N))`, where N is the number of elements to insert.\n
     *   Much faster than raw `std::vector`, because moved values are
     *   `std::map::iterator`, not acture `T` node.
     */
    void insert(iterator pos, std::initializer_list<value_type> ilist)
    { insert(pos, ilist.begin(), ilist.end()); }

    /**
     * \brief Inserts a new element to the container as close as possible to
     *        the position just before hint. The element is constructed
     *        in-place, i.e. no copy or move operations are performed.
     * \tparam Args
     * \param pos  Index to the position before which the new element will be
     *             inserted.
     * \param key  Key of element to insert.
     * \param args Arguments to forward to the constructor of the value.
     * \return Returns an iterator to the newly inserted element. \n
     *         If the insertion failed because the element already exists,
     *         returns an iterator to the already existing element with the
     *         equivalent key.
     * \details
     *   The constructor of the value type (`T`) is called with exactly the
     *   same arguments as supplied to the function, forwarded with
     *   `std::forward<Args>(args)...`.\n
     *   Invalidates iterators **after** `pos`.\n
     *   No references are invalidated.
     * \details
     *   **Complexity**\n
     *   Linear in the size of the container, i.e., the number of elements.\n
     *   Much faster than raw `std::vector`, because moved values are
     *   `std::map::iterator`, not acture `T` node.
     */
    template<typename... Args>
    std::pair<iterator, bool> emplace_at(size_t pos, const key_type& key, Args&&... args)
    {
        key_type k = key;
        auto it = find(k);
        if (it != end()) return std::make_pair(it, false);
        auto pair = m.emplace(std::move(k), std::forward<Args>(args)...);
        v.insert(v.begin() + pos, pair.first);
        return std::make_pair(begin() + pos, true);
    }

    /**
     * \brief Inserts a new element to the container as close as possible to
     *        the position just before hint. The element is constructed
     *        in-place, i.e. no copy or move operations are performed.
     * \tparam Args
     * \param pos  Index to the position before which the new element will be
     *             inserted.
     * \param key  Key of element to insert.
     * \param args Arguments to forward to the constructor of the value.
     * \return Returns an iterator to the newly inserted element. \n
     *         If the insertion failed because the element already exists,
     *         returns an iterator to the already existing element with the
     *         equivalent key.
     * \details
     *   The constructor of the value type (`T`) is called with exactly the
     *   same arguments as supplied to the function, forwarded with
     *   `std::forward<Args>(args)...`.\n
     *   Invalidates iterators **after** `pos`.\n
     *   No references are invalidated.
     * \details
     *   **Complexity**\n
     *   Linear in the size of the container, i.e., the number of elements.\n
     *   Much faster than raw `std::vector`, because moved values are
     *   `std::map::iterator`, not acture `T` node.
     */
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

    /**
     * \brief Inserts a new element to the container as close as possible to
     *        the position just before hint. The element is constructed
     *        in-place, i.e. no copy or move operations are performed.
     * \tparam Args
     * \param pos  Iterator to the position before which the new element will
     *             be inserted.
     * \param key  Key of element to insert.
     * \param args Arguments to forward to the constructor of the value.
     * \return Returns an iterator to the newly inserted element. \n
     *         If the insertion failed because the element already exists,
     *         returns an iterator to the already existing element with the
     *         equivalent key.
     * \details
     *   The constructor of the value type (`T`) is called with exactly the
     *   same arguments as supplied to the function, forwarded with
     *   `std::forward<Args>(args)...`.\n
     *   Invalidates iterators **after** `hint`.\n
     *   No references are invalidated.
     * \details
     *   **Complexity**\n
     *   Linear in the size of the container, i.e., the number of elements.\n
     *   Much faster than raw `std::vector`, because moved values are
     *   `std::map::iterator`, not acture `T` node.
     */
    template<typename... Args>
    iterator emplace_hint(const_iterator hint, key_type&& key, Args&&... args)
    {
        return emplace_at(hint - cbegin(),
                          std::forward<key_type>(key),
                          std::forward<Args>(args)...)
                .first;
    }

    /**
     * \brief Removes the last element of the container.
     * \details
     *   Calling pop_back on an empty container is undefined.\n
     *   No iterators or references except for back() and end() are
     *   invalidated.
     * \details
     *   **Complexity**\n
     *   Amortized constant.
     */
    void pop_back()
    {
        auto it = v.back();
        v.pop_back();
        m.erase(it);
    }

    /**
     * \brief Removes specified element from the container.
     * \param key Key of element to erase.
     * \details
     *   Invalidates reference to the erased element.\n
     *   Invalidates iterators at or **after** the point of the erase.\n
     *   Other references and iterators are not affected.
     * \details
     *   **Complexity**\n
     *   Linear: the number of calls to the destructor of T is the same as the
     *   number of elements erased, the assignment operator of T is called the
     *   number of times equal to the number of elements in the vector after
     *   the erased elements.\n
     *   Much faster than raw `std::vector`, because moved values are
     *   `std::map::iterator`, not acture `T` node.
     */
    void erase(const key_type& key)
    {
        auto it = find(key);
        if (it == cend()) return;
        else erase(it);
    }

    /**
     * \brief Removes specified elements from the container.
     * \param pos   Index to the position of the first element to erase.
     * \param count Elements count to erase.
     * \details
     *   Invalidates references to the erased element.\n
     *   Invalidates iterators at or **after** the eraseed elements.\n
     *   Other references and iterators are not affected.
     * \details
     *   **Complexity**\n
     *   Linear: the number of calls to the destructor of T is the same as the
     *   number of elements erased, the assignment operator of T is called the
     *   number of times equal to the number of elements in the vector after
     *   the erased elements.\n
     *   Much faster than raw `std::vector`, because moved values are
     *   `std::map::iterator`, not acture `T` node.
     */
    void erase(size_type pos, size_type count = 1)
    {
        erase(cbegin() + pos, cbegin() + pos + count);
    }

    /**
     * \brief Removes specified elements from the container.
     * \param key Key of element to erase.
     * \return Iterator following the last removed element. If the iterator pos
     *         refers to the last element, the end() iterator is returned.
     * \details
     *   Invalidates reference to the erased element.\n
     *   Invalidates iterators at or **after** the point of the erase.\n
     *   Other references and iterators are not affected.\n
     * \details
     *   **Complexity**\n
     *   Linear: the number of calls to the destructor of T is the same as the
     *   number of elements erased, the assignment operator of T is called the
     *   number of times equal to the number of elements in the vector after
     *   the erased elements.\n
     *   Much faster than raw `std::vector`, because moved values are
     *   `std::map::iterator`, not acture `T` node.
     */
    iterator erase(const_iterator pos)
    {
        difference_type index = pos - cbegin();
        m.erase(*(pos.n));
        v.erase(v.begin() + (pos.n - v.data()));
        return begin() + index;
    }

    /**
     * \brief Removes specified elements from the container.
     * \param first Iterator to the first element to erase.
     * \param last  Iterator after the last element to erase.
     * \return Iterator following the last removed element. If the iterator pos
     *         refers to the last element, the end() iterator is returned.
     * \details
     *   Invalidates references to the erased element.\n
     *   Invalidates iterators at or **after** the eraseed elements.\n
     *   Other references and iterators are not affected.
     * \details
     *   **Complexity**\n
     *   Linear: the number of calls to the destructor of T is the same as the
     *   number of elements erased, the assignment operator of T is called the
     *   number of times equal to the number of elements in the vector after
     *   the erased elements.\n
     *   Much faster than raw `std::vector`, because moved values are
     *   `std::map::iterator`, not acture `T` node.
     */
    iterator erase(const_iterator first, const_iterator last)
    {
        iterator ret;
        for (auto it = const_reverse_iterator(last); it != const_reverse_iterator(first); ++it)
        { ret = erase(it.base() - 1); }
        return ret;
    }

    /**
     * \brief
     *   Returns an iterator to the first element of the container.\n
     *   If the container is empty, the returned iterator will be equal to
     *   `end()`.
     * \return Iterator to the first element.
     * \details
     *   **Complexity**\n
     *   Constant.
     */
    iterator begin()
    { return iterator(v.data()); }

    /**
     * \brief
     *   Returns an iterator to the first element of the container.\n
     *   If the container is empty, the returned iterator will be equal to
     *   `end()`.
     * \return Iterator to the first element.
     * \details
     *   **Complexity**\n
     *   Constant.
     */
    const_iterator begin() const
    { return cbegin(); }

    /**
     * \brief
     *   Returns an iterator to the first element of the container.\n
     *   If the container is empty, the returned iterator will be equal to
     *   `end()`.
     * \return Iterator to the first element.
     * \details
     *   **Complexity**\n
     *   Constant.
     */
    const_iterator cbegin() const
    { return const_iterator(v.data()); }

    /**
     * \brief
     *   Returns an iterator to the element following the last element of the
     *   container.\n
     *   This element acts as a placeholder; attempting to access it results in
     *   undefined behavior.
     * \return Iterator to the element following the last element.
     * \details
     *   **Complexity**\n
     *   Constant.
     */
    iterator end()
    { return iterator(v.data() + size()); }

    /**
     * \brief
     *   Returns an iterator to the element following the last element of the
     *   container.\n
     *   This element acts as a placeholder; attempting to access it results in
     *   undefined behavior.
     * \return Iterator to the element following the last element.
     * \details
     *   **Complexity**\n
     *   Constant.
     */
    const_iterator end() const
    { return cend(); }

    /**
     * \brief
     *   Returns an iterator to the element following the last element of the
     *   container.\n
     *   This element acts as a placeholder; attempting to access it results in
     *   undefined behavior.
     * \return Iterator to the element following the last element.
     * \details
     *   **Complexity**\n
     *   Constant.
     */
    const_iterator cend() const
    { return const_iterator(v.data() + size()); }

    /**
     * \brief
     *   Returns a reverse iterator to the first element of the reversed
     *   container. It corresponds to the last element of the non-reversed
     *   container. If the container is empty, the returned iterator is equal
     *   to `rend()`.
     * \return Reverse iterator to the first element.
     * \details
     *   **Complexity**\n
     *   Constant.
     */
    reverse_iterator rbegin()
    { return reverse_iterator(end()); }

    /**
     * \brief
     *   Returns a reverse iterator to the first element of the reversed
     *   container. It corresponds to the last element of the non-reversed
     *   container. If the container is empty, the returned iterator is equal
     *   to `rend()`.
     * \return Reverse iterator to the first element.
     * \details
     *   **Complexity**\n
     *   Constant.
     */
    const_reverse_iterator rbegin() const
    { return crbegin(); }

    /**
     * \brief
     *   Returns a reverse iterator to the first element of the reversed
     *   container. It corresponds to the last element of the non-reversed
     *   container. If the container is empty, the returned iterator is equal
     *   to `rend()`.
     * \return Reverse iterator to the first element.
     * \details
     *   **Complexity**\n
     *   Constant.
     */
    const_reverse_iterator crbegin() const
    { return const_reverse_iterator(cend()); }

    /**
     * \brief
     *   Returns a reverse iterator to the element following the last element
     *   of the reversed container. It corresponds to the element preceding
     *   the first element of the non-reversed container. This element acts as
     *   a placeholder, attempting to access it results in undefined behavior.
     * \return Reverse iterator to the element following the last element.
     * \details
     *   **Complexity**\n
     *   Constant.
     */
    reverse_iterator rend()
    { return reverse_iterator(begin()); }

    /**
     * \brief
     *   Returns a reverse iterator to the element following the last element
     *   of the reversed container. It corresponds to the element preceding
     *   the first element of the non-reversed container. This element acts as
     *   a placeholder, attempting to access it results in undefined behavior.
     * \return Reverse iterator to the element following the last element.
     * \details
     *   **Complexity**\n
     *   Constant.
     */
    const_reverse_iterator rend() const
    { return crend(); }

    /**
     * \brief
     *   Returns a reverse iterator to the element following the last element
     *   of the reversed container. It corresponds to the element preceding
     *   the first element of the non-reversed container. This element acts as
     *   a placeholder, attempting to access it results in undefined behavior.
     * \return Reverse iterator to the element following the last element.
     * \details
     *   **Complexity**\n
     *   Constant.
     */
    const_reverse_iterator crend() const
    { return const_reverse_iterator(cbegin()); }

    /**
     * \brief
     *   Returns an iterator to the first key of the container.\n
     *   If the container is empty, the returned iterator will be equal to
     *   `end()`.
     * \return Iterator to the first key.
     * \details
     *   **Complexity**\n
     *   Constant.
     */
    key_iterator key_begin() const
    { return key_iterator(v.data()); }

    /**
     * \brief
     *   Returns an iterator to the key following the last key of the
     *   container.\n
     *   This key acts as a placeholder; attempting to access it results in
     *   undefined behavior.
     * \return Iterator to the key following the last key.
     * \details
     *   **Complexity**\n
     *   Constant.
     */
    key_iterator key_end() const
    { return key_iterator(v.data() + size()); }

    /**
     * \brief
     *   Returns a reverse iterator to the first key of the reversed
     *   container. It corresponds to the last key of the non-reversed
     *   container. If the container is empty, the returned iterator is equal
     *   to `rend()`.
     * \return Reverse iterator to the first key.
     * \details
     *   **Complexity**\n
     *   Constant.
     */
    reverse_key_iterator key_rbegin() const
    { return reverse_key_iterator(key_end()); }

    /**
     * \brief
     *   Returns a reverse iterator to the key following the last key of the
     *   reversed container. It corresponds to the key preceding the first key
     *   of the non-reversed container. This key acts as a placeholder,
     *   attempting to access it results in undefined behavior.
     * \return Reverse iterator to the key following the last key.
     * \details
     *   **Complexity**\n
     *   Constant.
     */
    reverse_key_iterator key_rend() const
    { return reverse_key_iterator(key_begin()); }

    /**
     * \brief Replaces the contents of the input container.
     * \param other Another container to use as data source
     * \return `*this`.
     * \details
     *   **Complexity**\n
     *   Linear in the size of `*this` and `other`.
     */
    SequencialMap& operator=(const SequencialMap& other)
    {
        if (this == &other) return *this;
        clear(); push_back(other.begin(), other.end()); return *this;
    }

    /**
     * \brief Replaces the contents of the input container.
     * \param other Another container to use as data source
     * \return `*this`.
     * \details
     *   **Complexity**\n
     *   Linear in the size of `*this` unless the allocators do not compare
     *   equal and do not propagate, in which case linear in the size of
     *   `*this` and `other`.
     */
    SequencialMap& operator=(SequencialMap&& other)
    { other.swap(*this); return *this; }

    /**
     * \brief Replaces the contents of the input container.
     * \param Ilist Initializer list to use as data source.
     * \return `*this`.
     * \details
     *   **Complexity**\n
     *   `O(NlogN)` in general, where `N` is `size() + ilist.size()`. Linear if
     *   `ilist` is sorted with respect to `value_comp()`.
     */
    SequencialMap& operator=(std::initializer_list<value_type> ilist)
    { clear(); insert(ilist); return *this; }

    /**
     * \brief Checks if the contents of two containers are not equal.
     * \param other Another container whose contents to compare.
     * \return `true` if the contents of the containers are `equal`, `false`
     *         otherwise.
     * \details
     *   `Equal` means that they have the same number of elements and each
     *   element in `this` compares equal with the element in `other` at the
     *   same position.\n
     *   **Complexity**\n
     *   Constant if containers are of different size, otherwise linear in the
     *   size of the container.
     */
    bool operator==(const SequencialMap& other) const
    { return m == other.m; }

    /**
     * \brief Checks if the contents of two containers are equal, that is,
     * \param other Another container whose contents to compare.
     * \return `true` if the contents of the containers are `not equal`, `false`
     *         otherwise.
     * \details
     *   `Equal` means that they have the same number of elements and each
     *   element in `this` compares equal with the element in `other` at the
     *   same position.\n
     *   **Complexity**\n
     *   Constant if containers are of different size, otherwise linear in the
     *   size of the container.
     */
    bool operator!=(const SequencialMap& other) const
    { return *this != other; }

    /**
     * \brief Compares the contents of two containers lexicographically.
     * \param other Another container whose contents to compare.
     * \return `true` if the contents of `this` are lexicographically `less`
     *         than the contents of `other`, `false` otherwise.
     * \details
     *   The comparison is performed by a function equivalent to
     *   `std::lexicographical_compare`. This comparison ignores the
     *   container's ordering Compare.\n
     *   **Complexity**\n
     *   Linear in the size of the container.
     */
    bool operator<(const SequencialMap& other) const
    { return m < other.m; }

    /**
     * \brief Compares the contents of two containers lexicographically.
     * \param other Another container whose contents to compare.
     * \return `true` if the contents of `this` are lexicographically `less`
     *         than or `equal` the contents of `other`, `false` otherwise.
     * \details
     *   The comparison is performed by a function equivalent to
     *   `std::lexicographical_compare`. This comparison ignores the
     *   container's ordering Compare.\n
     *   **Complexity**\n
     *   Linear in the size of the container.
     */
    bool operator<=(const SequencialMap& other) const
    { return m <= other.m; }

    /**
     * \brief Compares the contents of two containers lexicographically.
     * \param other Another container whose contents to compare.
     * \return `true` if the contents of `this` are lexicographically
     *         `greater` than the contents of `other`, `false` otherwise.
     * \details
     *   The comparison is performed by a function equivalent to
     *   `std::lexicographical_compare`. This comparison ignores the
     *   container's ordering Compare.\n
     *   **Complexity**\n
     *   Linear in the size of the container.
     */
    bool operator>(const SequencialMap& other) const
    { return m > other.m; }

    /**
     * \brief Compares the contents of two containers lexicographically.
     * \param other Another container whose contents to compare.
     * \return `true` if the contents of `this` are lexicographically
     *         `greater` than or `equal` the contents of `other`, `false`
     *         otherwise.
     * \details
     *   The comparison is performed by a function equivalent to
     *   `std::lexicographical_compare`. This comparison ignores the
     *   container's ordering Compare.\n
     *   **Complexity**\n
     *   Linear in the size of the container.
     */
    bool operator>=(const SequencialMap& other) const
    { return m >= other.m; }

    /**
     * \brief Exchanges the contents of the container with those of other. Does
     *        not invoke any move, copy, or swap operations on individual
     *        elements.
     * \param other Container to exchange the contents with.
     * \details
     *   All iterators and references remain valid. The past-the-end iterator
     *   is invalidated.\n
     *   The Pred objects must be Swappable, and they are exchanged using
     *   unqualified call to non-member swap.\n
     *   **Complexity**\n
     *   Constant.
     */
    void swap(SequencialMap& other)
    {
        v.swap(other.v);
        m.swap(other.m);
    }

    /**
     * \brief Returns the function object that compares the keys, which is a
     *        copy of this container's constructor argument comp.
     * \return The key comparison function object.
     * \details
     *   **Complexity**\n
     *   Constant.
     */
    key_compare key_comp() const
    {
        return m.key_comp();
    }

    /**
     * \brief Returns a function object that compares objects of type
     *        `std::map::value_type` (key-value pairs) by using `key_comp` to
     *        compare the first components of the pairs.
     * \return The value comparison function object.
     * \details
     *   **Complexity**\n
     *   Constant.
     */
    value_compare value_comp() const
    {
        return m.value_comp();
    }

    /**
     * \brief Writes the contents of list to output stream.
     * \tparam Stream Needs to support streaming type `Key` and `T`.
     * \param out Output stream.
     * \param map Map to be written to `out`.
     * \return Stream& `out` itself.
     * \details
     *   Output format will be like:
     *   > SequencialMap(("a",0),("b",1),("c",2),("d",3),("e",4),
     *   > ("f",5),("g",6),("h",7),("i",8),("k",9),...)
     *   **Complexity**\n
     *   Linear in the size of the container, i.e., the number of elements.
     */
    template<typename Stream>
    friend Stream& operator<<(Stream& out, const SequencialMap& map)
    {
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

    /**
     * \brief Serialize the contents to output stream.
     * \return Serialization manipulator forwarding to output stream.
     * \details
     *   **Sample Code**
     *   ```cpp
     *   out << map.serialize();
     *   ```
     * \note
     *   The output stream must support serialization of type `Key` and `T`.
     */
    SerializeManipulator serialize() const
    { return SerializeManipulator{const_cast<SequencialMap&>(*this)}; }

    /**
     * \brief Deserialize the contents from input stream.
     * \return Deserialization manipulator forwarding to input stream.
     * \details
     *   **Sample Code**
     *   ```cpp
     *   in >> map.serialize();
     *   ```
     * \note
     *   The input stream must support deserialization of type `Key` and `T`.
     */
    SerializeManipulator deserialize()
    { return SerializeManipulator{*this}; }

    /**
     * \brief Stream manipulator for serialization and deserialization.
     *
     */
    struct SerializeManipulator
    {
        /**
         * \brief Output stream operator for serialization.
         * \tparam Stream Must support serialization of type `Key` and `T`.
         * \param out   Output stream.
         * \param manip Manipulator for SequencialMap to serialize.
         * \return Stream& `out` stream itself.
         * \sa serialize
         */
        template<typename Stream>
        friend Stream& operator<<(Stream& out, const SerializeManipulator& manip)
        {
            out << manip.map.size();
            for (const value_type& value : manip.map)
            { out << value.first << value.second; }
            return out;
        }

        /**
         * \brief Input stream operator for deserialization.
         * \tparam Stream Must support deserialization of type `Key` and `T`.
         * \param in    Input stream.
         * \param manip Manipulator for SequencialMap to deserialize.
         * \return Stream& `in` stream itself.
         * \sa deserialize.
         */
        template<typename Stream>
        friend Stream& operator>>(Stream& in, SerializeManipulator manip)
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

    /**
     * \brief Base type for iterators.
     * \tparam constant Whether the iterator is mutable or constant.
     */
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
        { node_type* node = n; ++n; return iterator_base(node); }

        inline iterator_base& operator--()
        { --n; return *this; }

        inline iterator_base operator--(int)
        { node_type* node = n; --n; return iterator_base(node); }

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
        inline explicit iterator_base(const node_type* node)
            : n(const_cast<node_type*>(node))
        {
        }

        mutable node_type* n = nullptr;
        friend class SequencialMap;
        friend struct iterator_base<!constant>;
    };

    /**
     * \brief Iterator to traverse keys.
     */
    struct key_iterator
    {
        using iterator_category = std::random_access_iterator_tag;
        using difference_type = typename SequencialMap::difference_type;
        using node_type = typename SequencialMap::vector_type::value_type;
        using value_type = Key;
        using reference = const value_type&;
        using pointer = const value_type*;

        inline key_iterator() = default;

        inline key_iterator(const key_iterator& other)
            : n(other.n)
        {
        }

        inline reference operator*() const
        { return n->operator*().first; }

        inline pointer operator->() const
        { return &(n->operator->()->first); }

        inline key_iterator& operator=(const key_iterator& other)
        { n = other.n; return *this; }

        inline bool operator==(const key_iterator& other) const
        { return (n == other.n); }

        inline bool operator!=(const key_iterator& other) const
        { return n != other.n; }

        inline bool operator<(const key_iterator& other) const
        { return n < other.n; }

        inline bool operator<=(const key_iterator& other) const
        { return n <= other.n; }

        inline bool operator>(const key_iterator& other) const
        { return n > other.n; }

        inline bool operator>=(const key_iterator& other) const
        { return n >= other.n; }

        inline key_iterator& operator++()
        { ++n; return *this; }

        inline key_iterator operator++(int)
        { const node_type* node = n; ++n; return key_iterator(node); }

        inline key_iterator& operator--()
        { --n; return *this; }

        inline key_iterator operator--(int)
        { const node_type* node = n; --n; return key_iterator(node); }

        inline key_iterator& operator+=(difference_type j)
        { n += j; return *this; }

        inline key_iterator& operator-=(difference_type j)
        { n -= j; return *this; }

        inline key_iterator operator+(difference_type j) const
        { return key_iterator(n + j); }

        friend inline key_iterator operator+(difference_type j, key_iterator& it)
        { return it + j; }

        inline key_iterator operator-(difference_type j) const
        { return key_iterator(n - j); }

        inline difference_type operator-(key_iterator j) const
        { return difference_type(n - j.n); }

    private:
        inline explicit key_iterator(const node_type* node)
            : n(node)
        {
        }

        const node_type* n = nullptr;
        friend class SequencialMap;
    };

private:
    vector_type v;
    map_type m;
};
} // namespace Container
/** @} end of namespace Container*/

UTILITIES_NAMESPACE_END

namespace std {
/**
 * \relates Container::SequencialMap
 * \brief Specializes the `std::swap` algorithm.
 * \tparam Key       Key type of input maps.
 * \tparam T         Value type of input maps.
 * \tparam Compare   ComparisonfFunction object to use for all comparisons of
 *                   keys.
 * \tparam Allocator Allocator to use for all memory allocations of this
 *                   container.
 * \param  lhs       Map whose contents to swap.
 * \param  rhs       Map whose contents to swap.
 * \details
 *   Specializes the `std::swap` algorithm for Container::SequencialMap. Swaps
 *   the maps of `lhs` and `rhs`. Calls `lhs.swap(rhs)`.
 * \details
 *   **Complexity**\n
 *   Constant.
 */
template<typename Key,
         typename T,
         typename Compare = std::less<Key>,
         typename Allocator = std::allocator<std::pair<const Key, T>>>
inline void swap(Container::SequencialMap<Key, T, Compare, Allocator>& lhs, Container::SequencialMap<Key, T, Compare, Allocator>& rhs) noexcept
{ lhs.swap(rhs); }

/**
 * \relates Container::SequencialMap
 * \brief Erases all elements that satisfy the predicate pred from the
 *        container.
 * \tparam Key     Key type of input maps.
 * \tparam T       Value type of input maps.
 * \tparam Compare ComparisonfFunction object to use for all comparisons of
 *                 keys.
 * \tparam Alloc   Allocator to use for all memory allocations of this
 *                 container.
 * \tparam Pred    Predicate that returns true if the element should be erased.
 * \param  c       Container from which to erase.
 * \param  pred    Predicate that returns true if the element should be erased.
 * \details
 *   **Complexity**\n
 *   Constant.
 */
template<class Key, class T, class Compare, class Alloc, class Pred>
void erase_if(Container::SequencialMap<Key, T, Compare, Alloc>& c, Pred pred)
{
    for (auto i = c.begin(), last = c.end(); i != last; )
    {
        if (pred(*i)) {
            i = c.erase(i);
        } else {
            ++i;
        }
    }
}
} // namespace std

/** @} end of group Container*/

#endif  // CPP_UTILITIES_CONTAINERS_SEQUENCIALMAP_HPP
