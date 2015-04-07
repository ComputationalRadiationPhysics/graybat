#pragma once

/**
 * @brief This is an umbrella header that provides all necessary 
 *        header to configure the graybat communication library.
 *
 */

#include <BGL.hpp>     /* graybat::graphPolicy::BGL */
//#include <MPI.hpp>     /* graybat::communicationPolicy::MPI */
#include <BMPI.hpp>    /* graybat::communicationPolicy::BMPI */
#include <Cave.hpp>    /* graybat::Cave */
#include <mapping/mapping.hpp> /* graybat::mapping::Consecutive etc. */
//#include <mapping/GraphPartition.hpp> /* graybat::mappping::GraphPartition*/
#include <pattern.hpp> /* graybat::pattern::GridDiagonal */


