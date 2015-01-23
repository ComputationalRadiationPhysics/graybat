/*******************************************************************************
 *
 * GRAPH AUXILARY
 *
 *******************************************************************************/

namespace topology {

    template<typename T_Vertex>
    std::vector<T_Vertex> generateVertices(const size_t numVertices){
	std::vector<T_Vertex> vertices;
	for(unsigned i = 0; i < numVertices; ++i){
	    vertices.push_back(T_Vertex(i));
	}
	return vertices;
    }

    template<typename T_Graph>
    std::vector<typename T_Graph::EdgeDescriptor> fullyConnected(const unsigned verticesCount, std::vector<typename T_Graph::Vertex> &vertices){
	typedef typename T_Graph::Vertex Vertex;
	typedef typename T_Graph::Edge Edge;
	typedef typename T_Graph::EdgeDescriptor EdgeDescriptor;

	if(vertices.empty()){
	    vertices = generateVertices<Vertex>(verticesCount);

	}

	assert(vertices.size() == verticesCount);

	//std::cout << "Create fully connected with " << vertices.size() << " cells" << std::endl;

	unsigned edgeCount = 0;    
	std::vector<EdgeDescriptor> edges;

	for(unsigned i = 0; i < vertices.size(); ++i){
	    vertices[i].id = i;
	    for(unsigned j = 0; j < vertices.size(); ++j){
		if(vertices[i].id == vertices[j].id){
		    continue;
		} 
		else {
		    edges.push_back(std::make_tuple(vertices[i], vertices[j], Edge(edgeCount++)));
		}
	    }

	}

	return edges;
    }

    template<typename T_Graph>
    std::vector<typename T_Graph::EdgeDescriptor> star(const unsigned verticesCount, std::vector<typename T_Graph::Vertex> &vertices){
	typedef typename T_Graph::Vertex Vertex;
	typedef typename T_Graph::Edge Edge;
	typedef typename T_Graph::EdgeDescriptor EdgeDescriptor;

	vertices = generateVertices<Vertex>(verticesCount);
    
	unsigned edgeCount = 0;    
	std::vector<EdgeDescriptor> edges;

	for(unsigned i = 0; i < vertices.size(); ++i){
	    if(i != 0){
		edges.push_back(std::make_tuple(vertices[i], vertices[0], Edge(edgeCount++)));
	    }
		
	}

	return edges;
    }

    unsigned hammingDistance(unsigned a, unsigned b){
	unsigned abXor = a xor b;
	return (unsigned) __builtin_popcount(abXor);
    }

    template<typename T_Graph>
    std::vector<typename T_Graph::EdgeDescriptor> hyperCube(const unsigned dimension, std::vector<typename T_Graph::Vertex> &vertices){
	typedef typename T_Graph::Vertex Vertex;
	typedef typename T_Graph::Edge Edge;
	typedef typename T_Graph::EdgeDescriptor EdgeDescriptor;
	assert(dimension >= 1);
	std::vector<EdgeDescriptor> edges;

	unsigned verticesCount = pow(2, dimension);
	unsigned edgeCount = 0;
	vertices  = generateVertices<Vertex>(verticesCount);

	for(Vertex v1 : vertices){
	    for(Vertex v2 : vertices){
		if(hammingDistance(v1.id, v2.id) == 1){
		    edges.push_back(std::make_tuple(v1, v2, Edge(edgeCount++)));
		}

	    }
	}
    
	return edges;
    }

    template<typename T_Graph>
    std::vector<typename T_Graph::EdgeDescriptor> grid(const unsigned height, const unsigned width, std::vector<typename T_Graph::Vertex> &vertices){
	typedef typename T_Graph::Vertex Vertex;
	typedef typename T_Graph::Edge Edge;
	typedef typename T_Graph::EdgeDescriptor EdgeDescriptor;
	const unsigned verticesCount = height * width;
	vertices = generateVertices<Vertex>(verticesCount);
	std::vector<EdgeDescriptor> edges;

	unsigned edgeCount = 0;

	for(Vertex v: vertices){
	    unsigned i    = v.id;

	    if(i >= width){
		unsigned up   = i - width;
		edges.push_back(std::make_tuple(vertices[i], vertices[up], Edge(edgeCount++)));
	    }

	    if(i < (verticesCount - width)){
		unsigned down = i + width;
		edges.push_back(std::make_tuple(vertices[i], vertices[down], Edge(edgeCount++)));
	    }


	    if((i % width) != (width - 1)){
		int right = i + 1;
		edges.push_back(std::make_tuple(vertices[i], vertices[right], Edge(edgeCount++)));
	    }

	    if((i % width) != 0){
		int left = i - 1;
		edges.push_back(std::make_tuple(vertices[i], vertices[left], Edge(edgeCount++)));
	    }
	

	}

	return edges;
    }

    template<typename T_Graph>
    std::vector<typename T_Graph::EdgeDescriptor> gridDiagonal(const unsigned height, const unsigned width, std::vector<typename T_Graph::Vertex> &vertices){
	typedef typename T_Graph::Vertex Vertex;
	typedef typename T_Graph::Edge Edge;
	typedef typename T_Graph::EdgeDescriptor EdgeDescriptor;
	const unsigned verticesCount = height * width;
	vertices = generateVertices<Vertex>(verticesCount);
	std::vector<EdgeDescriptor> edges;

	unsigned edgeCount = 0;

	for(Vertex v: vertices){
	    unsigned i    = v.id;

	    // UP
	    if(i >= width){
		unsigned up   = i - width;
		edges.push_back(std::make_tuple(vertices[i], vertices[up], Edge(edgeCount++)));
	    }

	    // UP LEFT
	    if(i >= width and (i % width) != 0){
		unsigned up_left   = i - width - 1;
		edges.push_back(std::make_tuple(vertices[i], vertices[up_left], Edge(edgeCount++)));
	    }

	    // UP RIGHT
	    if(i >= width and (i % width) != (width - 1)){
		unsigned up_right   = i - width + 1;
		edges.push_back(std::make_tuple(vertices[i], vertices[up_right], Edge(edgeCount++)));
	    }

	    // DOWN
	    if(i < (verticesCount - width)){
		unsigned down = i + width;
		edges.push_back(std::make_tuple(vertices[i], vertices[down], Edge(edgeCount++)));
	    }

	    // DOWN LEFT
	    if(i < (verticesCount - width) and (i % width) != 0){
		unsigned down_left = i + width - 1;
		edges.push_back(std::make_tuple(vertices[i], vertices[down_left], Edge(edgeCount++)));
	    }

	    // DOWN RIGHT
	    if(i < (verticesCount - width) and (i % width) != (width - 1)){
		unsigned down_right = i + width + 1;
		edges.push_back(std::make_tuple(vertices[i], vertices[down_right], Edge(edgeCount++)));
	    }

	    // RIGHT
	    if((i % width) != (width - 1)){
		int right = i + 1;
		edges.push_back(std::make_tuple(vertices[i], vertices[right], Edge(edgeCount++)));
	    }

	    // LEFT
	    if((i % width) != 0){
		int left = i - 1;
		edges.push_back(std::make_tuple(vertices[i], vertices[left], Edge(edgeCount++)));
	    }

	    

	}
    
	for(unsigned i = 0; i < width; ++i){
	
	}
    

	return edges;
    }

    template <typename T_Graph>
    void printVertexDistribution(const std::vector<typename T_Graph::Vertex>& vertices,const T_Graph& graph, const unsigned commID){
	typedef typename T_Graph::Vertex Vertex;
	std::cout << "[" <<  commID << "] " << "Graph: " << graph.id << " " << "Vertices: ";
	for(Vertex v : vertices){
	    std::cout << v.id << " ";
	}
	std::cout << std::endl;

    }

} /* Topology */
