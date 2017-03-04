
#pragma once

#include <boost/graph/adjacency_list.hpp>

#include "vertex_traits.hpp"
#include "edge_traits.hpp"

namespace core { namespace graph { namespace detail {

	///***********
	//*   VERTEX
	//***********/
	//template <class VertexType, class EdgeType, int Behaviour>
	//struct vertex_traits {
	//    typedef VertexType vertex_type;
	//    };

	///***********
	//*   EDGE
	//***********/
	//template <class VertexType, class EdgeType, int Behaviour>
	//struct edge_traits {
	//    typedef EdgeType edge_type;
	//    };

	//template <class VertexType, class EdgeType>
	//struct edge_traits<VertexType, EdgeType, BIDIRECTIONAL> {
	//    static_assert( std::is_base_of<edge_connected<VertexType, EdgeType, BIDIRECTIONAL>, EdgeType>::value, "EdgeType template argument must inherit form core::graph::detail::edge_connected<VertexType, BIDIRECTIONAL>" );
	//    typedef EdgeType edge_type;
	//    };

	/***********
	*   BEHAVIOUR
	***********/
	template <class VertexType, class EdgeType, int Behaviour>
	struct behaviour_traits;

	template <class VertexType, class EdgeType>
	struct behaviour_traits<VertexType, EdgeType, UNDIRECTED>
	{
		typedef boost::undirectedS behaviour;
	};

	template <class VertexType, class EdgeType>
	struct behaviour_traits<VertexType, EdgeType, DIRECTED>
	{
		typedef boost::directedS behaviour;
	};

	template <class VertexType, class EdgeType>
	struct behaviour_traits<VertexType, EdgeType, BIDIRECTIONAL>
	{
		typedef boost::bidirectionalS behaviour;
	};
}


template <class VertexType, class EdgeType, int Behaviour>
struct chart_traits
{
	// vertex type check
	static_assert( detail::is_vertex_connected<VertexType>::value, "VertexType template argument must inherit form core::graph::detail::vertex_connected<EdgeType, Behaviour>" );
	static_assert( detail::is_vertex_multichart<VertexType>::value, "VertexType template argument must inherit form core::graph::detail::vertex_multichart<Graph>" );

	// edge type check
	static_assert( detail::is_edge_connected<EdgeType>::value, "EdgeType template argument must inherit form core::graph::detail::edge_connected<VertexType, Behaviour>" );
	static_assert( detail::is_edge_chart<EdgeType>::value, "EdgeType template argument must inherit form core::graph::detail::edge_chart<Graph>" );

	typedef typename detail::behaviour_traits<VertexType, EdgeType, Behaviour>::behaviour behaviour;
	typedef VertexType vertex_type;
	typedef EdgeType edge_type;
};

}}

