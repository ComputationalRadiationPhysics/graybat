// Communication
#include <Graph.hpp>                         /* Graph */
#include <CommunicationAbstractionLayer.hpp> /* CommunicationAbstractionLayer */
#include <MPI.hpp>                           /* CommunicationPolicy::MPI*/
#include <GraphBasedVirtualOverlayNetwork.hpp>         /* GraphBasedVirtualOverlayNetwork */

// Helpers
#include <distribution.hpp> /* roundRobin */
#include <topology.hpp>     /* meshDiagonal */
#include <evals.hpp>        /* variance, median, avg */

// STL
#include <iostream>   /* std::cout */
#include <tuple>      /* std::pair */
#include <vector>     /* std::vector */
#include <array>      /* std::array */
#include <algorithm>  /* std::copy */
#include <cmath>      /* sqrt, pow */
#include <cstdlib>    /* atoi */
#include <assert.h>   /* assert */
#include <cstdlib>    /* rand */
#include <chrono>     /* std::chrono::high_resolution_clock */ 

// Boost uBlas
#include <boost/numeric/ublas/vector.hpp>

// Body definition
struct Body : public SimpleProperty{
	
    Body() : SimpleProperty(0), m{{1}}, r{{0,0}}, v{{0,0}} { }
    Body(ID id) : SimpleProperty(id), m{{1}}, r{{0,0}}, v{{0,0}} { }

       
    Body(ID id, 
	 std::array<double, 1> m, 
	 std::array<double, 2> r, 
	 std::array<double, 2> v ) : SimpleProperty(id), m(m), r(r), v(v){
	    
    }
	
    std::array<double, 1> m;
    std::array<double, 2> r;	
    std::array<double, 2> v;

};

template <class T_Body>
void updateBody(T_Body &body, const boost::numeric::ublas::vector<double> F, const double dt){
    
    // Convert STL container to uBlas container
    double m = body.m[0];
    boost::numeric::ublas::vector<double> r(2);
    boost::numeric::ublas::vector<double> v(2);
    boost::numeric::ublas::vector<double> a(2);

    std::copy(body.r.begin(), body.r.end(), r.begin());
    std::copy(body.v.begin(), body.v.end(), v.begin());

    assert(m > 0);

    // Calculate value changes
    a = F / m;
    r = (a/2) * pow(dt,2) + v * dt + r;
    v = a * dt + v;

    // Put changes back into body
    std::copy(r.begin(), r.end(), body.r.begin());
    std::copy(v.begin(), v.end(), body.v.begin());

}

template <class T_Body>
boost::numeric::ublas::vector<double> twoBodyForce(T_Body body1, T_Body body2){
    const double G = 6.67384e-11;
    
    // Convert STL container to uBlas container
    double m1 = body1.m[0];
    boost::numeric::ublas::vector<double> r1(2);
    boost::numeric::ublas::vector<double> v1(2);
    double m2 = body2.m[0];
    boost::numeric::ublas::vector<double> r2(2);
    boost::numeric::ublas::vector<double> v2(2);
    boost::numeric::ublas::vector<double> F(2);

    std::copy(body1.r.begin(), body1.r.end(), r1.begin());
    std::copy(body1.v.begin(), body1.v.end(), v1.begin());
    std::copy(body2.r.begin(), body2.r.end(), r2.begin());
    std::copy(body2.v.begin(), body2.v.end(), v2.begin());


    // Calculate force, norm_2 == eucledeon norm
    F = (G * m1 * m2)  * ((r2 - r1) / pow(boost::numeric::ublas::norm_2(r2 - r1), 3));
    
    return F;
    
}

template <class T_Body>
void printBody(T_Body body){
    std::cout << "[" << body.id << "] ";
    std::cout << " m = " << body.m[0];
    std::cout << " r = [" << body.r[0] << "," << body.r[1] << "]";
    std::cout << " v = [" << body.v[0] << "," << body.v[1] << "]";
    std::cout << std::endl;

}

template <class T_Body>
std::vector<T_Body> generateBodies(const unsigned N){
    std::vector<T_Body> bodies(N);

    for(unsigned i = 0; i < bodies.size(); ++i){
	double m  = rand() % int(1e12) + 1;
	double r1 = rand() % 100;
	double r2 = rand() % 100;
	double v1 = rand() % 10;
	double v2 = rand() % 10;
	

	bodies[i] = T_Body(i, {{m}}, {{r1, r2}}, {{v1, v2}});

    }
    return bodies;

}

int nBody(const unsigned nBodies, const unsigned timesteps){
    /***************************************************************************
     * Configuration
     ****************************************************************************/

    // Graph
    typedef Graph<Body, SimpleProperty>         NBodyGraph;
    typedef typename NBodyGraph::Vertex         Vertex;
    typedef typename NBodyGraph::Edge           Edge;
    typedef typename NBodyGraph::EdgeDescriptor EdgeDescriptor;

    // Cal
    typedef CommunicationPolicy::MPI            Mpi;
    typedef CommunicationAbstractionLayer<Mpi>  MpiCAL;
    typedef typename MpiCAL::VAddr              VAddr;
    typedef typename MpiCAL::Event              Event;

    // GVON
    typedef GraphBasedVirtualOverlayNetwork<NBodyGraph, MpiCAL>  GVON;


    /***************************************************************************
     * Init Communication
     ****************************************************************************/
    // Create Graph
    std::vector<Vertex> graphVertices = generateBodies<Vertex>(nBodies);
    std::vector<EdgeDescriptor> edges = Topology::fullyConnected<NBodyGraph>(nBodies, graphVertices);
    NBodyGraph graph (edges, graphVertices); 

    // Initiate communication objects
    MpiCAL cal;
    GVON gvon(cal);

    // Distribute work evenly
    VAddr myVAddr  = cal.getGlobalContext().getVAddr();
    unsigned nAddr = cal.getGlobalContext().size();
    std::vector<Vertex> myGraphVertices = Distribute::consecutive(myVAddr, nAddr, graph);

    // Announce vertices
    gvon.announce(graph, myGraphVertices); 

    /***************************************************************************
     * Start Simulation
     ****************************************************************************/

    std::vector<Event> events;   

    // Simulate life forever
    const double dt = 1;
    for(unsigned timestep = 0; timestep < timesteps; ++timestep){

	// Send body information to all other
	for(Vertex v : myGraphVertices){
	    for(std::pair<Vertex, Edge> edge : graph.getOutEdges(v)){
		events.push_back(gvon.asyncSend(graph, edge.first, edge.second, v.m)); 
		events.push_back(gvon.asyncSend(graph, edge.first, edge.second, v.r)); 
		events.push_back(gvon.asyncSend(graph, edge.first, edge.second, v.v));
	    }
	}

	// Recv body information from all other
	for(Vertex &v : myGraphVertices){
	    boost::numeric::ublas::vector<double> F(2,0);
	    for(std::pair<Vertex, Edge> edge : graph.getInEdges(v)){
		gvon.recv(graph, edge.first, edge.second, edge.first.m);
		gvon.recv(graph, edge.first, edge.second, edge.first.r);
		gvon.recv(graph, edge.first, edge.second, edge.first.v);
		F += twoBodyForce(v, edge.first);
	    }
	    updateBody(v, F, dt);

	}

	// Wait to finish events
	for(unsigned i = 0; i < events.size(); ++i){
	    events.back().wait();
	    events.pop_back();
	}



    }

    return 0;
    
}




int main(int argc, char** argv){

    if(argc < 3){
	std::cout << "Usage ./NBody [nBodies] [nTimeSteps]" << std::endl;
	return 0;
    }

    // Parameter
    unsigned nBodies    = atoi(argv[1]);
    unsigned nTimesteps = atoi(argv[2]);

    nBody(nBodies, nTimesteps);
    return 0;

}
