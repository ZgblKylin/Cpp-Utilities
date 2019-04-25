#include <gtest/gtest.h>
#include <string>
#include <sstream>
#include <fstream>
#include <list>
#define private public
#include <Containers/SequencialMap.hpp>

UTILITIES_USING_NAMESPACE
using Container::SequencialMap;

static const SequencialMap<std::string, int> Map = {
    { "c", 1 }, { "a", 2 }, { "b", 3 }
};
#define K1 "d"
static const std::string k1 = K1;
#define V1 4
static const int v1 = V1;
static const auto value1 = std::make_pair(k1, v1);
#define K2 "a"
static const std::string k2 = K2;
#define V2 5
static const int v2 = V2;
static const auto value2 = std::make_pair(k2, v2);

TEST(SequencialMap, Constructor)
{
    std::map<std::string, int> m = {
        { "c", 1 }, { "a", 2 }, { "b", 3 }
    };

    SequencialMap<std::string, int> map;
    EXPECT_TRUE(map.empty());
    EXPECT_EQ(map.size(), 0);

    SequencialMap<std::string, int> map2((std::less<std::string>()));
    EXPECT_TRUE(map2.empty());
    EXPECT_EQ(map2.size(), 0);

    SequencialMap<std::string, int> map3(m.begin(), m.end());
    EXPECT_EQ(map3.m, m);
    EXPECT_EQ(map3["c"], 1);
    EXPECT_EQ(map3["a"], 2);
    EXPECT_EQ(map3["b"], 3);

    SequencialMap<std::string, int> map4(m.begin(), m.end(), std::allocator<std::pair<std::string, int>>());
    EXPECT_EQ(map4.m, m);
    EXPECT_EQ(map4["c"], 1);
    EXPECT_EQ(map4["a"], 2);
    EXPECT_EQ(map4["b"], 3);

    SequencialMap<std::string, int> map5(map4);
    EXPECT_EQ(map5.m, m);

    SequencialMap<std::string, int> map6(std::move(map5));
    EXPECT_EQ(map6.m, m);
    EXPECT_TRUE(map5.empty());

    static bool deleted = false;
    struct RAII
    {
        ~RAII() { deleted = true; }
    };
    auto map7 = new SequencialMap<std::string, RAII>{ { "a", RAII() } };
    delete map7;
    EXPECT_TRUE(deleted);
}

TEST(SequencialMap, Capacity)
{
    SequencialMap<std::string, int> map;
    EXPECT_TRUE(map.empty());
    EXPECT_EQ(map.size(), 0);

    SequencialMap<std::string, int> map2 = {
        { "c", 1 }, { "a", 2 }, { "b", 3 }
    };
    EXPECT_FALSE(map2.empty());
    EXPECT_EQ(map2.size(), 3);
    EXPECT_GE(map2.max_size(), map2.size());
}

TEST(SequencialMap, find)
{
    auto map = Map;
    auto it = map.find("j");
    EXPECT_EQ(it, map.end());
    it = map.find("a");
    ASSERT_NE(it, map.end());
    EXPECT_EQ(it->first, "a");
    EXPECT_EQ(it->second, 2);
    EXPECT_TRUE(std::is_const<decltype(it->first)>::value);
    ASSERT_FALSE(std::is_const<decltype(it->second)>::value);

    const SequencialMap<std::string, int>& cmap = map;
    it->second = 4;
    auto it2 = cmap.find("a");
    ASSERT_NE(it2, cmap.cend());
    EXPECT_EQ(it2->first, "a");
    EXPECT_EQ(it2->second, 4);
    it2 = cmap.find("j");
    EXPECT_EQ(it2, cmap.cend());
}

TEST(SequencialMap, key)
{
    std::vector<std::string> keys = { "c", "a", "b" };
    EXPECT_EQ(Map.keys(), keys);

    std::list<std::string> lkeys = { "c", "a", "b" };
    EXPECT_EQ(Map.keys<std::list<std::string>>(), lkeys);

    struct MyString : public std::string
    {
        using std::string::string;
        using std::string::operator=;
        MyString(const std::string& s) : std::string(s) {}
        MyString& operator=(const std::string& s)
        {
            static_cast<std::string>(*this) = s;
            return *this;
        }
        bool operator==(const MyString& other) const
        { return static_cast<std::string>(*this) == static_cast<std::string>(other); }
    };
    std::list<MyString> myKeys = { "c", "a", "b" };
    EXPECT_EQ(Map.keys<std::list<MyString>>(), myKeys);

    EXPECT_EQ(Map.key(2), "a");
    EXPECT_EQ(Map.key(5, "invalid_key"), "invalid_key");
}

TEST(SequencialMap, value)
{
    std::vector<int> values = { 1, 2, 3 };
    EXPECT_EQ(Map.values(), values);

    auto map = Map;
    const SequencialMap<std::string, int>& cmap = map;

    EXPECT_EQ(map.value("a"), 2);
    EXPECT_EQ(map.value("j", -1), -1);

    map.at(1).second = 5;
    EXPECT_EQ(map.at(1).second, 5);
    EXPECT_EQ(cmap.at(1).second, 5);
    EXPECT_EQ(map["a"], 5);

    map["a"] = 2;
    EXPECT_EQ(map["a"], 2);
    EXPECT_EQ(cmap["a"], 2);
    std::string key = "a";
    map[key] = 3;
    EXPECT_EQ(map[key], 3);
    EXPECT_EQ(cmap[key], 3);

    std::string k = "z";
    EXPECT_EQ(cmap[k], int());
    EXPECT_EQ(cmap["z"], int());

    std::pair<const std::string, int> front = { "c", 1 };
    EXPECT_EQ(map.front(), front);
    EXPECT_EQ(cmap.front(), front);

    std::pair<const std::string, int> back = { "b", 3 };
    EXPECT_EQ(map.back(), back);
    EXPECT_EQ(cmap.back(), back);
}

TEST(SequencialMap, mid)
{
    SequencialMap<std::string, int> mid1 = {
        { "a", 2 }, { "b", 3 }
    };
    EXPECT_EQ(Map.mid(1), mid1);

    SequencialMap<std::string, int> mid2 = { { "a", 2 } };
    EXPECT_EQ(Map.mid(1, 1), mid2);

    EXPECT_TRUE(Map.mid(1, 0).empty());
}

TEST(SequencialMap, push_back)
{
#define PUSH_BACK_SUCCESS(map, pair) \
EXPECT_TRUE(pair.second); \
EXPECT_EQ(map.size(), 4); \
EXPECT_EQ(pair.first, map.end() - 1); \
EXPECT_EQ(pair.first->first, k1); \
EXPECT_EQ(pair.first->second, v1)

#define PUSH_BACK_FAIL(map, pair) \
EXPECT_FALSE(pair.second); \
EXPECT_EQ(map.size(), 4); \
EXPECT_EQ(pair.first, map.begin() + 1); \
EXPECT_EQ(pair.first->first, k2); \
EXPECT_EQ(pair.first->second, 2)

    // std::pair<iterator, bool> push_back(const_reference value)
    {
        auto map = Map;
        // success
        auto pair = map.push_back(value1);
        PUSH_BACK_SUCCESS(map, pair);
        // fail
        pair = map.push_back(value2);
        PUSH_BACK_FAIL(map, pair);
    }

    // std::pair<iterator, bool> push_back(value_type&& value)
    {
        auto map = Map;
        // success
        auto pair = map.push_back({ k1, v1 });
        PUSH_BACK_SUCCESS(map, pair);
        // fail
        pair = map.push_back({ k2, v2 });
        PUSH_BACK_FAIL(map, pair);
    }

    // std::pair<iterator, bool> push_back(const key_type& key, const T& value)
    {
        auto map = Map;
        // success
        auto pair = map.push_back(k1, v1);
        PUSH_BACK_SUCCESS(map, pair);
        // fail
        pair = map.push_back(k2, v2);
        PUSH_BACK_FAIL(map, pair);
    }

    // std::pair<iterator, bool> push_back(const key_type& key, T&& value)
    {
        auto map = Map;
        // success
        auto pair = map.push_back(k1, V1);
        PUSH_BACK_SUCCESS(map, pair);
        // fail
        pair = map.push_back(k2, V2);
        PUSH_BACK_FAIL(map, pair);
    }

    // void push_back(const SequencialMap& other)
    {
        SequencialMap<std::string, int> other = {
            { "c", 10 }, { "h", 8 }, { "i", 9 }
        };
        auto map = Map;
        map.push_back(other);
        EXPECT_EQ(map["c"], 1);
        EXPECT_EQ(map.at(3).second, 8);
        EXPECT_EQ(map.at(4).second, 9);
    }

    // void push_back(std::initializer_list<value_type> ilist)
    {
        auto map = Map;
        map.push_back({ { k2, v2 }, { "j", 10 }, { "k", 11 } });
        EXPECT_EQ(map.size(), 5);
        EXPECT_EQ(map[k2], 2);
        EXPECT_EQ(map.at(3).second, 10);
        EXPECT_EQ(map.at(4).second, 11);
    }

    // void push_back(InputIt first, InputIt last)
    {
        SequencialMap<std::string, int> otherMap = { { k2, v2 }, { "l", 12 }, { "m", 13 } };
        auto map = Map;
        map.push_back(otherMap);
        EXPECT_EQ(map.size(), 5);
        EXPECT_EQ(map[k2], 2);
        EXPECT_EQ(map.at(3).second, 12);
        EXPECT_EQ(map.at(4).second, 13);
    }

    // template<typename... Args>
    // std::pair<iterator, bool> emplace_back(const key_type& key, Args&&... args)
    {
        auto map = Map;
        // success
        auto pair = map.emplace_back(K1, V1);
        PUSH_BACK_SUCCESS(map, pair);
        // fail
        pair = map.emplace_back(K2, V2);
        PUSH_BACK_FAIL(map, pair);
    }
}

TEST(SequencialMap, plus)
{
    const SequencialMap<std::string, int> other = {
        { "d", 4 }, { "a", 2 }, { "b", 3 }
    };

    // SequencialMap operator+(const SequencialMap& other) const
    auto map = Map;
    auto map2 = map + other;
    EXPECT_EQ(map2.size(), 4);
    EXPECT_EQ(map2["a"], 2);
    EXPECT_EQ(map2.at(3).first, "d");
    EXPECT_EQ(map2.at(3).second, 4);

    // SequencialMap operator+(SequencialMap&& other) const
    auto map3 = Map;
    auto map4 = map3 + SequencialMap<std::string, int>{
        { "d", 4 }, { "a", 2 }, { "b", 3 }
    };
    EXPECT_EQ(map4.size(), 4);
    EXPECT_EQ(map4["a"], 2);
    EXPECT_EQ(map4.at(3).first, "d");
    EXPECT_EQ(map4.at(3).second, 4);

    // SequencialMap& operator+=(const SequencialMap& other)
    auto map5 = Map;
    map5 += map2;
    EXPECT_EQ(map5.size(), 4);
    EXPECT_EQ(map5["a"], 2);
    EXPECT_EQ(map5.at(3).first, "d");
    EXPECT_EQ(map5.at(3).second, 4);

    // SequencialMap& operator+=(SequencialMap&& other)
    auto map6 = Map;
    map6 += SequencialMap<std::string, int>{
        { "d", 4 }, { "a", 2 }, { "b", 3 }
    };
    EXPECT_EQ(map6.size(), 4);
    EXPECT_EQ(map6["a"], 2);
    EXPECT_EQ(map6.at(3).first, "d");
    EXPECT_EQ(map6.at(3).second, 4);
}

TEST(SequencialMap, insert)
{
#define INSERT_SUCCESS(map, it) \
EXPECT_EQ(map.size(), 4); \
EXPECT_EQ(it, map.begin() + 1); \
EXPECT_EQ(it->first, k1); \
EXPECT_EQ(it->second, v1)

#define INSERT_FAIL(map, it) \
EXPECT_EQ(map.size(), 4); \
EXPECT_EQ(it, map.begin() + 2); \
EXPECT_EQ(it->first, k2); \
EXPECT_EQ(it->second, 2)

    SequencialMap<std::string, int> map = {
        { "c", 1 }, { "a", 2 }, { "b", 3 }
    };

    // iterator insert(size_t pos, const_reference value)
    {
        auto map = Map;
        // success
        auto it = map.insert(1, value1);
        INSERT_SUCCESS(map, it);
        // fail
        it = map.insert(1, value2);
        INSERT_FAIL(map, it);
    }

    // iterator insert(size_t pos, value_type&& value)
    {
        auto map = Map;
        // success
        auto it = map.insert(1, std::make_pair(std::string("d"), 4));
        INSERT_SUCCESS(map, it);
        // fail
        it = map.insert(1, std::make_pair(std::string("a"), 5));
        INSERT_FAIL(map, it);
    }

    // iterator insert(size_t pos, const key_type& key, const T& value)
    {
        auto map = Map;
        // success
        auto it = map.insert(1, k1, v1);
        INSERT_SUCCESS(map, it);
        // fail
        it = map.insert(1, k2, v2);
        INSERT_FAIL(map, it);
    }

    // iterator insert(size_t pos, const key_type& key, T&& value)
    {
        auto map = Map;
        // success
        auto it = map.insert(1, k1, 4);
        INSERT_SUCCESS(map, it);
        // fail
        it = map.insert(1, k2, 5);
        INSERT_FAIL(map, it);
    }

    // iterator insert(iterator pos, const_reference value)
    {
        auto map = Map;
        // success
        auto it = map.insert(map.begin() + 1, value1);
        INSERT_SUCCESS(map, it);
        // fail
        it = map.insert(map.begin() + 1, value2);
        INSERT_FAIL(map, it);
    }

    // iterator insert(iterator pos, value_type&& value)
    {
        auto map = Map;
        // success
        auto it = map.insert(map.begin() + 1, std::make_pair(k1, v1));
        INSERT_SUCCESS(map, it);
        // fail
        it = map.insert(map.begin() + 1, std::make_pair(k2, v2));
        INSERT_FAIL(map, it);
    }

    // iterator insert(iterator pos, const key_type& key, const T& value)
    {
        auto map = Map;
        // success
        auto it = map.insert(map.begin() + 1, k1, v1);
        INSERT_SUCCESS(map, it);
        // fail
        it = map.insert(map.begin() + 1, k2, v2);
        INSERT_FAIL(map, it);
    }

    // iterator insert(iterator pos, const key_type& key, T&& value)
    {
        auto map = Map;
        // success
        auto it = map.insert(map.begin() + 1, k1, V1);
        INSERT_SUCCESS(map, it);
        // fail
        it = map.insert(map.begin() + 1, k2, V2);
        INSERT_FAIL(map, it);
    }

    // template<typename InputIt>
    // void insert(size_t pos, InputIt first, InputIt last)
    {
        auto map = Map;
        std::vector<std::pair<const std::string, int>> container = {
            { K1, V1}, { K2, V2 }, { "e", 6 }
        };
        map.insert(1, container.begin(), container.end());
        EXPECT_EQ(map.size(), 5);
        EXPECT_EQ(map.at(1).first, k1);
        EXPECT_EQ(map.at(1).second, v1);
        EXPECT_EQ(map.at(2).first, "e");
        EXPECT_EQ(map.at(2).second, 6);
    }

    // template<typename InputIt>
    // void insert(size_t pos, std::initializer_list<value_type> ilist)
    {
        auto map = Map;
        map.insert(1u, {
                       { k1, v1}, { k2, v2 }, { "e", 6 }
                   });
        EXPECT_EQ(map.size(), 5);
        EXPECT_EQ(map.at(1).first, k1);
        EXPECT_EQ(map.at(1).second, v1);
        EXPECT_EQ(map.at(2).first, "e");
        EXPECT_EQ(map.at(2).second, 6);
    }

    // template<typename InputIt>
    // void insert(iterator pos, InputIt first, InputIt last)
    {
        auto map = Map;
        std::vector<std::pair<const std::string, int>> container = {
            { K1, V1}, { K2, V2 }, { "e", 6 }
        };
        map.insert(map.begin() + 1, container.begin(), container.end());
        EXPECT_EQ(map.size(), 5);
        EXPECT_EQ(map.at(1).first, k1);
        EXPECT_EQ(map.at(1).second, v1);
        EXPECT_EQ(map.at(2).first, "e");
        EXPECT_EQ(map.at(2).second, 6);
    }
    // template<typename InputIt>
    // void insert(const_iterator pos, std::initializer_list<value_type> ilist)
    {
        auto map = Map;
        map.insert(map.begin() + 1, {
                       { k1, v1}, { k2, v2 }, { "e", 6 }
                   });
        EXPECT_EQ(map.size(), 5);
        EXPECT_EQ(map.at(1).first, k1);
        EXPECT_EQ(map.at(1).second, v1);
        EXPECT_EQ(map.at(2).first, "e");
        EXPECT_EQ(map.at(2).second, 6);
    }

    // template<typename... Args>
    // std::pair<iterator, bool> emplace_at(size_t pos, const key_type& key, Args&&... args)
    {
        auto map = Map;
        // success
        auto pair = map.emplace_at(1, K1, V1);
        EXPECT_TRUE(pair.second);
        INSERT_SUCCESS(map, pair.first);
        // fail
        pair = map.emplace_at(1, K2, V2);
        EXPECT_FALSE(pair.second);
        INSERT_FAIL(map, pair.first);
    }

    // template<typename... Args>
    // iterator emplace_hint(const_iterator hint, key_type&& key, Args&&... args)
    {
        auto map = Map;
        // success
        auto it = map.emplace_hint(map.begin() + 1, K1, V1);
        INSERT_SUCCESS(map, it);
        // fail
        it = map.emplace_hint(map.begin() + 1, K2, V2);
        INSERT_FAIL(map, it);
    }
}

TEST(SequencialMap, erase)
{
    // void pop_back()
    {
        auto map = Map;
        EXPECT_NE(map.find("b"), map.end());
        map.pop_back();
        EXPECT_EQ(map.size(), 2);
        EXPECT_EQ(map.find("b"), map.end());
    }

    // void erase(const key_type& key)
    {
        auto map = Map;
        // success
        EXPECT_NE(map.find(k2), map.end());
        map.erase(k2);
        EXPECT_EQ(map.size(), 2);
        EXPECT_EQ(map.find(k2), map.end());
        // fail
        map.erase(k1);
        EXPECT_EQ(map.size(), 2);
    }

    // void erase(size_type pos, size_type count = 1)
    {
        auto map = Map;
        map.erase(1, 2);
        EXPECT_EQ(map.size(), 1);
        EXPECT_EQ(map["c"], 1);
    }

    // iterator erase(const_iterator pos)
    {
        auto map = Map;
        EXPECT_NE(map.find(k2), map.end());
        auto it = map.erase(map.begin() + 1);
        EXPECT_EQ(map.size(), 2);
        EXPECT_EQ(map.find(k2), map.end());
        EXPECT_EQ(it->first, "b");
        EXPECT_EQ(it->second, 3);
    }

    // iterator erase(const_iterator first, const_iterator last)
    {
        auto map = Map;
        auto it = map.erase(map.begin() + 1, map.end());
        EXPECT_EQ(map.size(), 1);
        EXPECT_EQ(it, map.end());
        EXPECT_EQ(map["c"], 1);
    }
}

TEST(SequencialMap, ArithmeticKey)
{
    {
        SequencialMap<int, std::string> map = {
            { v1, k1 }, { v2, k2 }
        };
        EXPECT_EQ(map.size(), 2);
        EXPECT_EQ(map.at(0).first, v1);
        EXPECT_EQ(map.at(0).second, k1);
        EXPECT_EQ(map[V1], k1);
        EXPECT_EQ(map.at(1).first, v2);
        EXPECT_EQ(map.at(1).second, k2);
        EXPECT_EQ(map[V2], k2);
    }

    {
        SequencialMap<double, std::string> map = {
            { 11.11, k1 }, { 3.14, k2 }
        };
        EXPECT_EQ(map.size(), 2);
        EXPECT_EQ(map.at(0).first, 11.11);
        EXPECT_EQ(map.at(0).second, k1);
        EXPECT_EQ(map[11.11], k1);
        EXPECT_EQ(map.at(1).first, 3.14);
        EXPECT_EQ(map.at(1).second, k2);
        EXPECT_EQ(map[3.14], k2);
    }
}

TEST(SequencialMap, iterators)
{
#define VALUE_FOR_COMPARE
#define IT2_KEY it2->first
#define IT2_VALUE it2->second
#define MAP_KEY .first
#define MAP_VALUE .second

#define ITERATOR_TEST(it2_ptr, index_0, index_1, index_2) \
auto it2(it); \
EXPECT_EQ(*it2, map.at(index_0)VALUE_FOR_COMPARE); \
EXPECT_EQ(IT2_KEY, map.at(index_0)MAP_KEY); \
EXPECT_EQ(IT2_VALUE, map.at(index_0)MAP_VALUE); \
 \
it2 = it; \
EXPECT_EQ(*it2, map.at(index_0)VALUE_FOR_COMPARE); \
EXPECT_EQ(IT2_KEY, map.at(index_0)MAP_KEY); \
EXPECT_EQ(IT2_VALUE, map.at(index_0)MAP_VALUE); \
 \
EXPECT_TRUE(it == it2); \
EXPECT_FALSE(it != it2); \
EXPECT_TRUE(it >= it2); \
EXPECT_TRUE(it <= it2); \
EXPECT_FALSE(it > it2); \
EXPECT_FALSE(it < it2); \
 \
++it2; \
EXPECT_EQ(*it2, map.at(index_1)VALUE_FOR_COMPARE); \
EXPECT_EQ(IT2_KEY, map.at(index_1)MAP_KEY); \
EXPECT_EQ(IT2_VALUE, map.at(index_1)MAP_VALUE); \
EXPECT_FALSE(it == it2); \
EXPECT_TRUE(it != it2); \
EXPECT_TRUE(it < it2); \
EXPECT_TRUE(it <= it2); \
EXPECT_FALSE(it > it2); \
EXPECT_FALSE(it >= it2); \
EXPECT_EQ(it2 - it, 1); \
EXPECT_EQ(it - it2, -1); \
 \
it2++; \
EXPECT_EQ(*it2, map.at(index_2)VALUE_FOR_COMPARE); \
EXPECT_EQ(IT2_KEY, map.at(index_2)MAP_KEY); \
EXPECT_EQ(IT2_VALUE, map.at(index_2)MAP_VALUE); \
 \
--it2; \
EXPECT_EQ(*it2, map.at(index_1)VALUE_FOR_COMPARE); \
EXPECT_EQ(IT2_KEY, map.at(index_1)MAP_KEY); \
EXPECT_EQ(IT2_VALUE, map.at(index_1)MAP_VALUE); \
 \
it2--; \
EXPECT_EQ(*it2, map.at(index_0)VALUE_FOR_COMPARE); \
EXPECT_EQ(IT2_KEY, map.at(index_0)MAP_KEY); \
EXPECT_EQ(IT2_VALUE, map.at(index_0)MAP_VALUE); \
 \
it2 += 1; \
EXPECT_EQ(*it2, map.at(index_1)VALUE_FOR_COMPARE); \
EXPECT_EQ(IT2_KEY, map.at(index_1)MAP_KEY); \
EXPECT_EQ(IT2_VALUE, map.at(index_1)MAP_VALUE); \
 \
it2 = it2 + 1; \
EXPECT_EQ(*it2, map.at(index_2)VALUE_FOR_COMPARE); \
EXPECT_EQ(IT2_KEY, map.at(index_2)MAP_KEY); \
EXPECT_EQ(IT2_VALUE, map.at(index_2)MAP_VALUE); \
 \
it2 -= 1; \
EXPECT_EQ(*it2, map.at(index_1)VALUE_FOR_COMPARE); \
EXPECT_EQ(IT2_KEY, map.at(index_1)MAP_KEY); \
EXPECT_EQ(IT2_VALUE, map.at(index_1)MAP_VALUE); \
 \
it2 = it2 - 1; \
EXPECT_EQ(*it2, map.at(index_0)VALUE_FOR_COMPARE); \
EXPECT_EQ(IT2_KEY, map.at(index_0)MAP_KEY); \
EXPECT_EQ(IT2_VALUE, map.at(index_0)MAP_VALUE)

    // begin
    {
        auto map = Map;
        auto it = map.begin();
        it->second = 10;
        EXPECT_EQ(map.at(0).second, 10);
        it->second = 1;
        ITERATOR_TEST(it2, 0, 1, 2);
    }

    // end
    {
        auto map = Map;
        auto it = map.end() - int(map.size());
        it->second = 10;
        EXPECT_EQ(map.at(0).second, 10);
        it->second = 1;
        ITERATOR_TEST(it2, 0, 1, 2);
    }

    // cbegin
    {
        auto map = Map;
        auto temp = Map.begin();
        auto it = map.cbegin();
        EXPECT_EQ(typeid(decltype(temp)), typeid(decltype(it)));
        ITERATOR_TEST(it2, 0, 1, 2);
    }

    // cend
    {
        auto map = Map;
        auto temp = Map.begin();
        auto it = map.cend() - int(map.size());
        EXPECT_EQ(typeid(decltype(temp)), typeid(decltype(it)));
        ITERATOR_TEST(it2, 0, 1, 2);
    }

    // rbegin
    {
        auto map = Map;
        auto it = map.rbegin();
        it->second = 10;
        EXPECT_EQ(map.at(2).second, 10);
        it->second = 1;
        ITERATOR_TEST(it2, 2, 1, 0);
    }

    // rend
    {
        auto map = Map;
        auto it = map.rend() - int(map.size());
        it->second = 10;
        EXPECT_EQ(map.at(2).second, 10);
        it->second = 1;
        ITERATOR_TEST(it2, 2, 1, 0);
    }

    // crbegin
    {
        auto map = Map;
        auto temp = Map.rbegin();
        auto it = map.crbegin();
        EXPECT_EQ(typeid(decltype(temp)), typeid(decltype(it)));
        ITERATOR_TEST(it2, 2, 1, 0);
    }

    // crend
    {
        auto map = Map;
        auto temp = Map.rbegin();
        auto it = map.crend() - int(map.size());
        EXPECT_EQ(typeid(decltype(temp)), typeid(decltype(it)));
        ITERATOR_TEST(it2, 2, 1, 0);
    }

#undef VALUE_FOR_COMPARE
#define VALUE_FOR_COMPARE .first
#undef IT2_KEY
#define IT2_KEY *it2
#undef IT2_VALUE
#define IT2_VALUE *it2
#undef MAP_VALUE
#define MAP_VALUE .first

    // key_begin
    {
        auto map = Map;
        auto it = map.key_begin();
        ITERATOR_TEST(*it2, 0, 1, 2);
    }

    // key_end
    {
        auto map = Map;
        auto it = map.key_end() - int(map.size());
        ITERATOR_TEST(*it2, 0, 1, 2);
    }

    // key_rbegin
    {
        auto map = Map;
        auto it = map.key_rbegin();
        ITERATOR_TEST(*it2, 2, 1, 0);
    }

    // key_rend
    {
        auto map = Map;
        auto it = map.key_rend() - int(map.size());
        ITERATOR_TEST(*it2, 2, 1, 0);
    }
}

TEST(SequencialMap, compare)
{
    // operators
    // TODO

    // key_compare key_comp() const
    auto key_comp = Map.key_comp();
    bool result = key_comp(k1, k2);
    EXPECT_EQ(result, k1 < k2);

    // value_compare value_comp() const
    auto value_comp = Map.value_comp();
    result = value_comp(value1, value2);
    EXPECT_EQ(result, k1 < k2);
}

TEST(SequencialMap, utilities)
{
    // void swap(SequencialMap& other)
    {
        auto map = SequencialMap<std::string, int>{ { k1, v1 }, { k2, v2 } };
        auto map1 = SequencialMap<std::string, int>{ { k1, v1 }, { k2, v2 } };
        auto map2 = SequencialMap<std::string, int>{ { k2, v2 }, { k1, v1 } };
        EXPECT_EQ(map1, map);
        EXPECT_EQ(map1.at(0), map2.at(1));
        EXPECT_EQ(map1.at(1), map2.at(0));

        std::swap(map1, map2);
        EXPECT_EQ(map2, map);
        EXPECT_EQ(map1.at(0), map2.at(1));
        EXPECT_EQ(map1.at(1), map2.at(0));
    }

    // template<typename Stream>
    // Stream& to_string(Stream& out)
    {
        SequencialMap<std::string, int> map{
            { "a", 0 }, { "b", 1 }, { "c", 2 }, { "d", 3 }, { "e", 4 },
            { "f", 5 }, { "g", 6 }, { "h", 7 }, { "j", 8 }, { "k", 9 },
            { "l", 10 }
        };
        std::stringstream out1;
        out1 << "SequencialMap(";
        for (size_t i = 0; i < 10; ++i)
        {
            out1 << '(' << map.at(i).first << ','
                << map.at(i).second << ')';
            if (i != 9) out1 << ',';
        }
        out1 << ",...)";
        EXPECT_EQ(map.size(), 11);
        std::stringstream out2;
        out2 << map;
        EXPECT_EQ(out1.str(), out2.str());
    }

    // std::ios_base doesn't support binary mode on some compilers, so use
    // custom stream here for testing.
    struct BinaryStream
    {
        BinaryStream(const std::string& string = {}) : str(string) {}

        BinaryStream& operator<<(size_t val)
        {
            char buf[5];
            memset(buf, '\0', 5);
            sprintf(buf, "%04d", val);
            str += std::string(buf);
            i += 4;
            return *this;
        }

        BinaryStream& operator>>(size_t& val)
        {
            val = size_t(std::stoi(str.substr(i, 4)));
            i += 4;
            return *this;
        }

        BinaryStream& operator<<(int val)
        {
            char buf[5];
            memset(buf, '\0', 5);
            sprintf(buf, "%04d", val);
            str += std::string(buf);
            i += 4;
            return *this;
        }

        BinaryStream& operator>>(int& val)
        {
            val = std::stoi(str.substr(i, 4));
            i += 4;
            return *this;
        }

        BinaryStream& operator<<(const std::string& val)
        {
            *this << val.size();
            str += val;
            i += val.size();
            return *this;
        }

        BinaryStream& operator>>(std::string& val)
        {
            size_t size;
            *this >> size;
            val = str.substr(i, size);
            i += size;
            return *this;
        }

        std::string str;
        size_t i = 0;
    };

    // serialization
    {
        std::string str;

        // serialize
        {
            BinaryStream out;
            out << Map.serialize();
            str = out.str;
        }

        // deserialize
        {
            BinaryStream in(str);
            SequencialMap<std::string, int> map;
            in >> map.deserialize();
            EXPECT_EQ(map, Map);
        }
    }
}
