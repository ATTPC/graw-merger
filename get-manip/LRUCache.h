#ifndef LRUCACHE_H
#define LRUCACHE_H

#include <unordered_map>
#include <list>

template <class K, class V>
class LRUCache
{
public:
    LRUCache(size_t cacheSize) : maxSize(cacheSize) {}

    void put(const K& key, const V& value)
    {
        auto it = item_map.find(key);
        if (it != item_map.end()) {
            // Entry already exists in the cache. Update it.
            it->second = value;
            item_list.
        }
    }

    V get(const K& key)
    {

    }


private:
    std::unordered_map<K, V> item_map;
    std::list<K> item_list;
    size_t maxSize;
};

#endif /* end of include guard: LRUCACHE_H */
