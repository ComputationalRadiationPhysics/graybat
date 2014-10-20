#pragma once

#include <numeric>    /* std::accumulate */
#include <vector>     /* std::vector */
#include <cmath>      /* sqrt, pow */
#include <algorithm>  /* std::sort */

template <typename T>
T avg(std::vector<T> values){
    return std::accumulate(values.begin(), values.end(), 0.0, std::plus<T>()) / (T) values.size(); 


}
template <typename T>
T variance(const std::vector<T> values, const T avgValue){

    T sum = 0;
    for(T value : values){
	sum += pow(avgValue - value, 2);
    }

    return sum / values.size();

}

template <typename T>
T median(std::vector<T> values){
    
    std::sort(values.begin(), values.end());
    
    unsigned mid_i = values.size() / 2;

    return values[mid_i];


}
