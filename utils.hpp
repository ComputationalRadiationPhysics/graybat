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
     * @brief A map with multiple keys, implemented by cascading several 
     *        std::maps.
     *
     */
    template <typename Value_T, typename... Keys_T>
    struct MultiKeyMap {
    
        typename MultiKeyMapType<Keys_T..., Value_T>::type multiKeyMap;

        Value_T& operator()(Keys_T... keys){
            return traverse(multiKeyMap, keys...)[lastArg(keys...)];
        }

        Value_T& at(Keys_T... keys){
            return traverse(multiKeyMap, keys...).at(lastArg(keys...));
        }

        auto test(Keys_T... keys){
            auto it = traverse(multiKeyMap, keys...).find(lastArg(keys...));
            if(it == traverse(multiKeyMap, keys...).end())
                return false;
            else
                return true;
        }

        bool erase(Keys_T ...keys){
            auto it = traverse(multiKeyMap, keys...).find(lastArg(keys...));
            if(it == traverse(multiKeyMap, keys...).end()){
                return false;
            }
            else {
                traverse(multiKeyMap, keys...).erase(it);
                return true;
            }
        }
        
    private:

        template <typename Map_T, typename T, typename... Ts>
        auto& traverse(Map_T &map, T key, Ts... keys){
            return traverse(map[key], keys...);
        }

        template <typename Map_T, typename T>
        auto& traverse(Map_T& map, T key){
            return map;
	
        }

        template <typename... Ts>
        auto lastArg(Ts... ts){
            const std::tuple<Keys_T...> tsTuple(ts...);
            const size_t N = std::tuple_size<decltype(tsTuple)>::value - 1;
            return std::get<N>(tsTuple);

        }

    
    };

} /* utils */
