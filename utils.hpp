#pragma once

#include <iterator> /* std::iterator_traits */


namespace utils {


    template <class InputIterator, class OutputIterator>
    void exclusivePrefixSum(InputIterator first, InputIterator last, OutputIterator result){

	typedef typename std::iterator_traits<InputIterator>::value_type IterType;

	IterType value = 0;

	while(first != last){
	    IterType prevValue = value;
	    value   = value + *first;
	    *result = prevValue;
	    result++; first++;
	}
  
    }


    
    // template <typename... Tail> struct MultiKeyMapType;

    // template <typename Key, typename Value>
    // struct MultiKeyMapType<Key, Value> {
    //     typedef std::map<Key, Value> type;
    // };

    // template <typename Key, typename... Tail>
    // struct MultiKeyMapType<Key, Tail...>  {
    //     typedef std::map<Key, typename MultiKeyMapType<Tail...>::type > type;
    // };
    


    // /**
    //  * @brief A map with multiple keys, implemented by cascading several 
    //  *        std::maps.
    //  *
    //  */
    // template <typename Value_T, typename... Keys_T>
    // struct MultiKeyMap {
    
    //     typename MultiKeyMapType<Keys_T..., Value_T>::type multiKeyMap;

    //     Value_T& operator()(Keys_T... keys){
    //         return traverse(multiKeyMap, keys...)[lastArg(keys...)];
    //     }

    //     Value_T& at(Keys_T... keys){
    //         return traverse(multiKeyMap, keys...).at(lastArg(keys...));
    //     }

    //     auto test(Keys_T... keys){
    //         auto it = traverse(multiKeyMap, keys...).find(lastArg(keys...));
    //         if(it == traverse(multiKeyMap, keys...).end())
    //             return false;
    //         else
    //             return true;
    //     }

    //     bool erase(Keys_T ...keys){
    //         auto it = traverse(multiKeyMap, keys...).find(lastArg(keys...));
    //         if(it == traverse(multiKeyMap, keys...).end()){
    //             return false;
    //         }
    //         else {
    //             traverse(multiKeyMap, keys...).erase(it);
    //             return true;
    //         }
    //     }
        
    // private:

    //     template <typename Map_T, typename T, typename... Ts>
    //     auto& traverse(Map_T &map, T key, Ts... keys){
    //         return traverse(map[key], keys...);
    //     }

    //     template <typename Map_T, typename T>
    //     auto& traverse(Map_T& map, T key){
    //         (void)key;
    //         return map;
	
    //     }

    //     template <typename... Ts>
    //     auto lastArg(Ts... ts){
    //         const std::tuple<Keys_T...> tsTuple(ts...);
    //         const size_t N = std::tuple_size<decltype(tsTuple)>::value - 1;
    //         return std::get<N>(tsTuple);

    //     }

    
    // };

template <typename... Tail> struct MultiKeyMapType;

template <typename Key, typename Value>
struct MultiKeyMapType<Key, Value> {
    typedef std::map<Key, Value> type;
};

template <typename Key, typename... Tail>
struct MultiKeyMapType<Key, Tail...>  {
    typedef std::map<Key, typename MultiKeyMapType<Tail...>::type > type;
};
    

/**
 * @brief Collects all values of a map subtree into the values container
 *
 */
template <int Left>
struct SubTreeValues {

    template <typename T_Map, typename T_Container, typename T_Tuple>
    void operator()(T_Map const &map, T_Container & values, T_Tuple & tuple){
        auto begin = map.begin();
        auto end   = map.end();
        for(auto it = begin; it != end; ++it){
            auto nextTuple = std::tuple_cat(tuple, std::make_tuple(it->first));
            SubTreeValues<Left-1>()(it->second, values, nextTuple);
        }
        
    }

    template <typename T_Map, typename T_Container>
    void operator()(T_Map const &map, T_Container & values){
        auto begin = map.begin();
        auto end   = map.end();
        for(auto it = begin; it != end; ++it){
            auto t = std::make_tuple(it->first);
            SubTreeValues<Left-1>()(it->second, values, t);
        }
        
    }  
    
};

template <>
struct SubTreeValues <0> {
    
    template <typename T_Value, typename T_Container, typename T_Tuple>    
    void operator()(T_Value &value, T_Container & values, T_Tuple &tuple){
        //values.push_back(std::make_pair(tuple, value));
        values.push_back(tuple);        
    }
    
};



/**
 * @brief A map with multiple keys, implemented by cascading several 
 *        std::maps.
 *
 */
template <typename T_Value, typename... T_Keys>
struct MultiKeyMap {
    
    typename MultiKeyMapType<T_Keys..., T_Value>::type multiKeyMap;

    T_Value& operator()(T_Keys... keys){
        return traverse(multiKeyMap, keys...)[lastArg(keys...)];
    }

    T_Value& at(T_Keys... keys){
        return traverse(multiKeyMap, keys...).at(lastArg(keys...));
    }

    auto test(T_Keys... keys){
        auto it = traverse(multiKeyMap, keys...).find(lastArg(keys...));
        if(it == traverse(multiKeyMap, keys...).end())
            return false;
        else
            return true;
    }

    bool erase(T_Keys ...keys){
        auto it = traverse(multiKeyMap, keys...).find(lastArg(keys...));
        if(it == traverse(multiKeyMap, keys...).end()){
            return false;
        }
        else {
            traverse(multiKeyMap, keys...).erase(it);
            return true;
        }
    }


    void values(std::vector<std::tuple<T_Keys...> > &values){
        constexpr size_t keysTupleSize = std::tuple_size<std::tuple<T_Keys...>>::value;
        SubTreeValues<keysTupleSize>()(multiKeyMap, values);
    }
    
    template <typename ...T_Sub_Keys>
    void values(std::vector<std::tuple<T_Keys...> > &values, T_Sub_Keys... subKeys){
        constexpr size_t subKeysTupleSize = std::tuple_size<std::tuple<T_Sub_Keys...>>::value;
        constexpr size_t keysTupleSize    = std::tuple_size<std::tuple<T_Keys...>>::value;
        constexpr size_t subTreeSize      = keysTupleSize - subKeysTupleSize;
        std::tuple<T_Sub_Keys...> tuple(subKeys...);
        SubTreeValues<subTreeSize>()(traverse(multiKeyMap, subKeys...).at(lastArg(subKeys...)), values, tuple);
        
    }


        
private:

    template <typename T_Map, typename T, typename... Ts>
    auto& traverse(T_Map &map, T key, Ts... keys){
        return traverse(map[key], keys...);
    }

    // TODO: traverse should return map[key]
    //       implementation above should reduce keys... size
    template <typename T_Map, typename T>
    auto& traverse(T_Map& map, T key){
        (void)key;
        return map;
	
    }

    template <typename... Ts>
    auto lastArg(Ts... ts){
        const std::tuple<Ts...> tsTuple(ts...);
        const size_t N = std::tuple_size<std::tuple<Ts...>>::value - 1;
        return std::get<N>(tsTuple);

    }

    
};

    
} /* utils */
