#pragma once

// STL
#include <map>        /* std::map */
#include <iostream>   /* std::cout, std::endl */
#include <string>     /* std::string */
#include <tuple>      /* std::tuple, std::get */
#include <vector>     /* std::vector */
#include <typeinfo>   /* typeid */
#include <functional> /* function */
#include <iterator>   /* std::iterator_traits */

// HANA
// HANA will be part of boost in the future !
#include <graybat/utils/hana/boost/hana.hpp>
namespace hana = boost::hana;

namespace utils {
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

	template <typename T_Map, typename T_ValuesCT, typename T_KeysCT, typename T_Tuple>
	void operator()(T_Map &map, T_ValuesCT & values, T_KeysCT& keys, T_Tuple & tuple){
	    auto begin = map.begin();
	    auto end   = map.end();
	    for(auto it = begin; it != end; ++it){
		auto nextTuple = hana::concat(tuple, hana::make_tuple(it->first));
		SubTreeValues<Left-1>()(it->second, values, keys, nextTuple);
	    }
        
	}

	template <typename T_Map, typename T_ValuesCT, typename T_Keys>
	void operator()(T_Map &map, T_ValuesCT &values, T_Keys& keys){
	    auto begin = map.begin();
	    auto end   = map.end();
	    for(auto it = begin; it != end; ++it){
		auto t = hana::make_tuple(it->first);
		SubTreeValues<Left-1>()(it->second, values, keys, t);
	    }
        
	}  
    
    };

    template <>
    struct SubTreeValues <0> {
    
	template <typename T_Value, typename T_ValuesCT, typename T_KeysCT, typename T_Tuple>    
	void operator()(T_Value &value, T_ValuesCT& values, T_KeysCT& keys, T_Tuple &tuple){
	    keys.push_back(tuple);
	    values.push_back(value);
	}
    
    };


    auto at = [] (auto &map,  const auto &key) mutable -> auto& {
	return map[key];
    };


    template <typename T_Map, typename T_Keys>
    auto& traverse(T_Map& map, T_Keys const& keys){
	return hana::fold_left(keys, map, at);
    }



    /**
     * @brief A map with multiple keys, implemented by cascading several 
     *        std::maps.
     *
     */
    template <typename T_Value, typename... T_Keys>
    struct MultiKeyMap {

	typename MultiKeyMapType<T_Keys..., T_Value>::type multiKeyMap;

	MultiKeyMap() {

	}

	T_Value& operator()(T_Keys... keys){
	    auto  keysTuple = hana::make_tuple(keys...);
	    return traverse(multiKeyMap, keysTuple);
	}

	T_Value& at(T_Keys... keys){
	    auto  keysTuple = hana::make_tuple(keys...);

	    auto firstKeysSize = hana::int_c<hana::minus(hana::int_c<hana::size(keysTuple)>, 1)>;
	    auto firstKeys     = hana::take_c<firstKeysSize>(keysTuple);
	    auto lastKey       = hana::back(keysTuple);

	    using namespace std::placeholders;
             
	    return traverse(multiKeyMap, firstKeys).at(lastKey);
	}

	auto test(T_Keys... keys){
	    auto  keysTuple = hana::make_tuple(keys...);

	    auto firstKeysSize = hana::int_c<hana::minus(hana::int_c<hana::size(keysTuple)>, 1)>;
	    auto firstKeys     = hana::take_c<firstKeysSize>(keysTuple);
	    auto lastKey       = hana::back(keysTuple);
        
	    auto it  = traverse(multiKeyMap, firstKeys).find(lastKey);
	    auto end = traverse(multiKeyMap, firstKeys).end();
	    if(it == end) return false;
	    else          return true;
	}

	bool erase(T_Keys ...keys){
	    auto  keysTuple = hana::make_tuple(keys...);

	    auto firstKeysSize = hana::int_c<hana::minus(hana::int_c<hana::size(keysTuple)>, 1)>;
	    auto firstKeys     = hana::take_c<firstKeysSize>(keysTuple);
	    auto lastKey       = hana::back(keysTuple);

	    auto it  = traverse(multiKeyMap, firstKeys).find(lastKey);
	    auto end = traverse(multiKeyMap, firstKeys).end();

	    if(it == end) return false;
	    else {
		traverse(multiKeyMap, firstKeys).erase(it);
		return true;
	    }

	}

	template <typename T_KeysTPL>
	bool erase(T_KeysTPL const keys){
	    auto firstKeysSize = hana::int_c<hana::minus(hana::int_c<hana::size(keys)>, 1)>;
	    auto firstKeys     = hana::take_c<firstKeysSize>(keys);
	    auto lastKey       = hana::back(keys);

	    auto it  = traverse(multiKeyMap, firstKeys).find(lastKey);
	    auto end = traverse(multiKeyMap, firstKeys).end();

	    if(it == end) return false;
	    else {
		traverse(multiKeyMap, firstKeys).erase(it);
		return true;
	    }

	}

    

	template <typename T_ValuesCT, typename T_KeysCT>
	void values(T_Value &values, T_KeysCT& keys){
	    constexpr size_t keysTupleSize = std::tuple_size<std::tuple<T_Keys...>>::value;
	    SubTreeValues<keysTupleSize>()(multiKeyMap, values, keys);
	}
    
	template <typename T_ValuesCT, typename T_KeysCT, typename ...T_Sub_Keys>
	void values(T_ValuesCT &values, T_KeysCT& keys, T_Sub_Keys... subKeys){
	    constexpr size_t subKeysTupleSize = std::tuple_size<std::tuple<T_Sub_Keys...>>::value;
	    constexpr size_t keysTupleSize    = std::tuple_size<std::tuple<T_Keys...>>::value;
	    constexpr size_t subTreeSize      = keysTupleSize - subKeysTupleSize;
	    auto tuple = hana::make_tuple(subKeys...);
	    auto t = hana::make_tuple(subKeys...);
	    SubTreeValues<subTreeSize>()(traverse(multiKeyMap, t), values, keys, tuple);
    
	}

    };
    
} /* utils */
