#ifndef LRUCACHE_H
#define LRUCACHE_H

#include <list>
#include <unordered_map>
#include <functional>
#include <utility>

template <class Key, class T>
class LRUCache
{
public:
    LRUCache(const size_t maxSize, const std::function<void(T)>& beforeDeleteCallback)
        : maxSize(maxSize), beforeDeleteCallback(beforeDeleteCallback) {}

    void insert(const Key& key, T&& value)
    {
        itemList.emplace_front(key, std::forward<T>(value));
        iterMap.emplace(key, itemList.begin());

        if (size() > maxSize) {
            evictOldestElement();
        }
    }

    T* get(const Key& key)
    {
        typename decltype(iterMap)::mapped_type& iterPair = iterMap.at(key);

        // Move this item to the front of the list
        itemList.splice(itemList.begin(), itemList, iterPair);

        std::pair<Key, T>& item = itemList.front();

        return &(item.second);
    }

    T extract(const Key& key)
    {
        auto iter = iterMap.at(key);
        T item = iter->second;
        itemList.erase(iter);
        iterMap.erase(key);
        return item;
    }

    void evictOldestElement()
    {
        auto oldest = itemList.back();
        beforeDeleteCallback(std::move(oldest.second));

        itemList.pop_back();
        iterMap.erase(oldest.first);
    }

    size_t size() const
    {
        return itemList.size();
    }

private:
    std::list<std::pair<Key, T>> itemList;
    std::unordered_map<Key, typename std::list<std::pair<Key, T>>::iterator> iterMap;

    size_t maxSize;
    std::function<void(T&&)> beforeDeleteCallback;
};

#endif /* end of include guard: LRUCACHE_H */
