#pragma once

// GRAYBAT
#include <graybat/communicationPolicy/Traits.hpp>

namespace graybat {
    
    namespace communicationPolicy {

        namespace bmpi {

            template <typename T_CP>
            struct VAddrIterator {

                using VAddr = typename graybat::communicationPolicy::VAddr<T_CP>;
                using CP    = T_CP;
                
                VAddr current;

                VAddrIterator(VAddr current) :
                    current(current){
                }

                auto operator++()
                    -> VAddrIterator<CP>& {
                    current++;
                    return *this;
                }

                auto operator--()
                    -> VAddrIterator<CP>& {
                    current--;
                    return *this;
                }

                auto operator==(VAddrIterator<CP> &otherIter) const 
                    -> bool {
                    return otherIter.current == current;
                }

                auto operator!=(VAddrIterator<CP> &otherIter) const 
                    -> bool {
                    return otherIter.current != current;
                }

                auto operator*() const
                -> VAddr {
                    return current;
                }
                
            };

        } // namespace bmpi

    } // namespace communicationPolicy

} // namespace graybat
