/**
 * Copyright 2016 Erik Zenker
 *
 * This file is part of Graybat.
 *
 * Graybat is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Graybat is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Graybat.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

// STL
#include <map>                /* std::map */
#include <functional>         /* std::function */
#include <condition_variable> /* std::condition_variable */
#include <mutex>              /* std::mutex, std::lock_guard, std::unique_lock*/
#include <queue>              /* std::queue */
#include <utility>            /* std::forward */
#include <chrono>             /* std::chrono::milliseconds */
#include <atomic>             /* std::atomic */

// BOOST
#include <boost/optional.hpp>
#include <boost/core/ignore_unused.hpp>

namespace utils {

	/**
	 * @brief Compares two tuple types and calculates the "Difference" of both both tuples
	 * @param[in] std::tuple<T1, ... , Tn>
	 * @param[in] std::tuple<T1, ..., Tn, ..., Tm>
	 * @result ValueType = std::tuple<Tn+1, ..., Tm>
	 *
	 * T1 to Tn has to be the same for both tuples. Otherwise it will not compile.
	 *
	 */

	template <class... Types>
	struct Difference;

	template <
		class T,
		class... Types,
		class... subTypes
	>
	struct Difference<std::tuple<T, Types...>, std::tuple<T, subTypes...>> {
		using ValueType = typename Difference<
			std::tuple<Types...>,
			std::tuple<subTypes...>
		>::ValueType;
	};

	template <class... Types>
	struct Difference;

	template <
		class... Types
	>
	struct Difference<std::tuple<Types...>, std::tuple<>> {
		using ValueType = std::tuple<Types...>;
	};

	/**
	 *
	 * @brief Compares the nth to mth element of two tuples. If all of them are the same it will return true.
	 * @param[in] std::tuple<T1, ... , Tn>
	 * @param[in] std::tuple<T1, ..., Tn, ..., Tm>
	 * @param[in] unsigned int n
	 * @param[in] unsigned int m
	 *
	 */
	template <class T1, class T2, unsigned int pos, unsigned int end>
	struct PrefixMatch;

	template<class T1, class T2, unsigned int pos>
	struct PrefixMatch<T1, T2, pos, pos> {
		bool operator()(const T1& t1, const T2& t2) {
            boost::ignore_unused(t1, t2);
			return true;
		}
	};

	template <class T1, class T2, unsigned int pos, unsigned int end>
	struct PrefixMatch {
		bool operator()(const T1& t1, const T2& t2) {
			return
				std::get<pos>(t1) == std::get<pos>(t2) && // compare nth element of both tuples
				PrefixMatch<T1,T2,pos+1,end>()(t1,t2); // and rest tuples
		}
	};

	template <class... args1, class... args2>
	bool prefixMatch(std::tuple<args1...> t1, std::tuple<args2...> t2) {
		return
			PrefixMatch<
				decltype(t1),
				decltype(t2),
				0,
				std::tuple_size<
					decltype(t1)
				>::value
			>()(t1,t2);
	}
	template <typename T_Value, typename... T_Keys>
	struct MultiKeyMap {

		std::map<std::tuple<T_Keys...>, T_Value> multiKeyMap;

		MultiKeyMap() :
			multiKeyMap()
		{}

		template <typename T_KeysTpl>
		T_Value& operator[](const T_KeysTpl keysTuple){
			return multiKeyMap[keysTuple];
		}

		T_Value& operator()(const T_Keys... keys){
			const auto key = std::make_tuple(keys...);
			return operator[](key);
		}

		template <typename T_KeysTpl>
		T_Value& at(const T_KeysTpl keysTuple){
			return multiKeyMap.at(keysTuple);
		}


		T_Value& at(const T_Keys... keys){
			return multiKeyMap.at(std::make_tuple(keys...));
		}

		/***************************************************************************
		* test
		***************************************************************************/
		bool test(const T_Keys ...keys) {
			return multiKeyMap.count(std::make_tuple(keys...));
		}

		template <typename T_KeysTpl>
		bool test(const T_KeysTpl keysTuple) {
			return multiKeyMap.test(keysTuple);
		}

		/***************************************************************************
		* erase
		***************************************************************************/
		bool erase(const T_Keys ...keys) {
			return multiKeyMap.erase(std::make_tuple(keys...));
		}

		template <typename T_KeysTPL>
		bool erase(const T_KeysTPL keysTuple) {
			return multiKeyMap.erase(keysTuple);
		}

		/***************************************************************************
		* values
		***************************************************************************/

		/*
		// This implementation is a special case of values with subKeys == []>, but not needed
		template <typename T_ValuesCT, typename T_KeysCT>
		void values(T_Value &value, T_KeysCT& keys){
			for(auto& pair : multiKeyMap) {
				values.push_back(pair.second);
				keys.push_back(pair.first);
			}
		}
		*/

		/* MKM searches for all subkeys and saves all matching vales in values and the corresponding keys in keys */
		template <typename T_ValuesCT, typename T_KeysCT, typename... T_Sub_Keys>
		void values(T_ValuesCT &values, T_KeysCT& keys, const T_Sub_Keys... subKeys){
			auto subKeysTuple = std::make_tuple(subKeys...);
			for(auto& pair : multiKeyMap) {
				const bool match = prefixMatch(subKeysTuple, pair.first);
				if(match) {
					values.push_back(pair.second);
					keys.push_back(pair.first);
				}
			}
		}

	};



	template <typename T_Value, typename... T_Keys>
	struct MessageBox {

		MessageBox(size_t const maxBufferSize) :
				maxBufferSize(maxBufferSize),
				bufferSize(0){
		}

		size_t maxBufferSize;
		std::atomic<size_t> bufferSize;
		std::mutex access;
		std::mutex writeNotify;
		std::mutex readNotify;
		std::condition_variable writeCondition;
		std::condition_variable readCondition;

		using Queue = std::queue<T_Value>;
		MultiKeyMap<Queue, T_Keys...> multiKeyMap;

		auto enqueue(T_Value&& value, const T_Keys... keys) -> void {
			{
				while(maxBufferSize < bufferSize + value.size()){
					std::unique_lock<std::mutex> notifyLock(readNotify);
					readCondition.wait_for(notifyLock, std::chrono::milliseconds(100));
				}

				bufferSize += value.size();
				//std::cout << bufferSize << std::endl;
				//std::cout << "Try to enqueue message." << std::endl;
				std::lock_guard<std::mutex> accessLock(access);
				//std::cout << "MessageBox enqueue" << std::endl;
				multiKeyMap(keys...).push(std::forward<T_Value>(value));
			}
			//std::cout << "notify on condition variable." << std::endl;
			writeCondition.notify_one();
		}

		auto waitDequeue(const T_Keys... keys) -> T_Value {

			bool test = false;

			{
				std::lock_guard<std::mutex> accessLock(access);
				test = !multiKeyMap.test(keys...);
			}

			while(test){
				std::unique_lock<std::mutex> notifyLock(writeNotify);
				//std::cout << "wait for multikeymap enqueue." << std::endl;
				writeCondition.wait_for(notifyLock, std::chrono::milliseconds(100));
				{
					std::lock_guard<std::mutex> accessLock(access);
					test = !multiKeyMap.test(keys...);
				}

			}

			while([&](){
                //The access of "multiKeyMap.at(keys...)" is a critical section
                bool empty = true;
                {
                    std::lock_guard<std::mutex> accessLock(access);
                    empty = multiKeyMap.at(keys...).empty();
                }
                return empty;
            }()){
				std::unique_lock<std::mutex> notifyLock(writeNotify);
				//std::cout << "wait for queue to be filled." << std::endl;
				writeCondition.wait_for(notifyLock, std::chrono::milliseconds(100));

			}

			{
				std::lock_guard<std::mutex> accessLock(access);
				//std::cout << "get message from queue" << std::endl;
				T_Value value = std::move(multiKeyMap.at(keys...).front());
				multiKeyMap.at(keys...).pop();
				bufferSize -= value.size();
				readCondition.notify_one();
				return value;
			}

		}

		template <typename... SubKeys>
		auto waitDequeue(std::tuple<T_Keys...> &allKeys, const SubKeys... someKeys) -> T_Value {

			while(true){

				std::vector<std::reference_wrapper<Queue>> values;
				std::vector<std::tuple<T_Keys...> > keysList;
				while(values.empty()){
					std::unique_lock<std::mutex> notifyLock(writeNotify);
					{
						std::lock_guard<std::mutex> accessLock(access);
						multiKeyMap.values(values, keysList, someKeys...);

					}
					writeCondition.wait_for(notifyLock, std::chrono::milliseconds(100));
				}
				//std::cout << "return on waitDeque" << std::endl;
				{
					std::lock_guard<std::mutex> accessLock(access);
					for(auto &keys : keysList ){
						auto& queue = multiKeyMap.at(keys);
						if(!queue.empty()) {
							allKeys = keys;
							T_Value value = std::move(queue.front());
							queue.pop();
							bufferSize -= value.size();
							readCondition.notify_one();
							//std::cout << "return value" << std::endl;
							return value;
						}

					}
				}
				std::unique_lock<std::mutex> notifyLock(writeNotify);
				writeCondition.wait_for(notifyLock, std::chrono::milliseconds(100));

			}

		}

		auto tryDequeue(bool &result, const T_Keys... keys) -> T_Value {

			{
				std::lock_guard<std::mutex> accessLock(access);
				if(!multiKeyMap.test(keys...)){
					result = false;
					return T_Value();
				}
			}

			if(multiKeyMap.at(keys...).empty()){
				result = false;
				return T_Value();
			}

			// //while(test){
			//     std::unique_lock<std::mutex> notifyLock(writeNotify);
			//     //std::cout << "wait for multikeymap enqueue." << std::endl;
			//     writeCondition.wait_for(notifyLock, std::chrono::milliseconds(100));
			//     {
			//         std::lock_guard<std::mutex> accessLock(access);
			//         test = !multiKeyMap.test(keys...);
			//     }
			//     //}

			{
				std::lock_guard<std::mutex> accessLock(access);
				//std::cout << "get message from queue" << std::endl;
				T_Value value = std::move(multiKeyMap.at(keys...).front());
				multiKeyMap.at(keys...).pop();
				bufferSize -= value.size();
				readCondition.notify_one();
				result = true;
				return value;
			}

		}

	};

} /* utils */
