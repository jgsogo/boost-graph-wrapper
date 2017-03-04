
#pragma once

#include <memory>
#include <vector>
#include <set>
#include <map>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/connected_components.hpp>
#include <boost/iterator/zip_iterator.hpp>
#include <boost/graph/iteration_macros.hpp>

#include <boost/graph/graphviz.hpp>

#include "chart_traits.hpp"
#include "events.hpp"

namespace core { namespace graph {

namespace detail
{

	/*! We are building just a wrapper around boost::adjacency_list
	    to implement some custom/predefined functionalities.
	*/
	class chart_base
	{ // Just for RTTI
		public:
			virtual ~chart_base() {};
	};

	template <class VertexType, class EdgeType, int Behaviour>
	class chart_impl : public chart_base
	{
		public:
			typedef typename chart_traits<VertexType, EdgeType, Behaviour>::vertex_type vertex_type;
			typedef typename chart_traits<VertexType, EdgeType, Behaviour>::edge_type edge_type;
			typedef typename chart_traits<VertexType, EdgeType, Behaviour>::behaviour behaviour;

			typedef typename std::shared_ptr<vertex_type> vertex_type_ptr;
			typedef typename std::shared_ptr<edge_type> edge_type_ptr;
			typedef std::shared_ptr<chart_impl> chart_ptr;

		protected:
			/* Necesitamos que los vértices y aristas se almacenen en listas (boost::listS) para que no 
			 *   se invaliden los 'vertex_id' y 'edge_id' que estamos devolviendo en las funciones.
			 *
			 *   From boost documentation:
			 *
			 *   > If the VertexList of the graph is vecS, then the graph has a builtin vertex indices
			 *   > accessed via the property map for the vertex_index_t property. The indices fall
			 *   > in the range [0, num_vertices(g)) and are contiguous. When a vertex is removed 
			 *   > the indices are adjusted so that they retain these properties.
			 */
			typedef boost::adjacency_list<boost::listS, boost::listS, behaviour, boost::property<boost::vertex_index_t, size_t, vertex_type_ptr>, edge_type_ptr> _t_graph;
			//                                                                   ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ cuando utilizo listS no se crea un index para los vértices, el cual necesito para calcular los connected components.
			//                                   ^^^^^         ^^^^^ cannot use boost::vecS, see above.

		public:
			typedef typename _t_graph::vertex_descriptor vertex_id;
			typedef typename _t_graph::edge_descriptor edge_id;

			typedef std::map<vertex_type_ptr, vertex_id> _t_vertices;
			typedef std::map<edge_type_ptr, edge_id> _t_edges;

			typedef std::map<vertex_id, typename _t_graph::vertices_size_type> _t_connected_components;

		public:
			chart_impl() {};
			virtual ~chart_impl()
			{
				while (_edges.size())
				{
					this->remove_edge( _edges.begin()->first);
				}

				while(_vertices.size())
				{
					this->remove_vertex( _vertices.begin()->first);
				}
			};

			std::pair<vertex_id, bool> add_vertex(vertex_type_ptr ptr)
			{
				// Is it already added?
				_t_vertices::iterator it; bool inserted;
				std::tie(it, inserted) = _vertices.insert(std::make_pair(ptr, vertex_id()));
				if (inserted)
				{
					it->second = boost::add_vertex(boost::property<boost::vertex_index_t, size_t, vertex_type_ptr>(boost::num_vertices(_graph), ptr), _graph);
					ptr->insert_chart(this);
					events::on_vertex_added_to_chart(*it, *this);
				}
				return std::make_pair(it->second, inserted);
			};

			void remove_vertex(vertex_type_ptr ptr)
			{
				_t_vertices::iterator it = _vertices.find(ptr);
				if (it != _vertices.end())
				{
					/*! Si elimino un vértice se eliminan todos los edges conectados
					  en _graph, pero no se eliminan las conexiones establecidas entre
					  estos elementos. Sólo cuando se destruye un vértice se terminan
					  de eliminar las conexiones (o cuando se llama a la función disconnect
					  de los edges)
					*/
					boost::remove_vertex( it->second, _graph);
					_vertices.erase(it);
					ptr->erase_chart(this);
					events::on_vertex_removed_from_chart(ptr, *this);
				}
			};

			void remove_vertex(vertex_id v_id)
			{
				vertex_type_ptr ptr = _graph[v_id];
				this->remove_vertex(ptr);
			};

			std::pair<edge_id, bool> add_edge(edge_type_ptr ptr, const vertex_id& source, const vertex_id& target)
			{
				/*! Adds edge to chart and connect selected vertices, so:
				 * vertices must exists
				 * edge must not be connected
				 */
				if (ptr->is_connected())
				{
					throw std::runtime_error("edge is already connected");
				}

				vertex_type_ptr source_ptr = this->get_vertex(source);
				vertex_type_ptr target_ptr = this->get_vertex(target);
				_t_edges::iterator it; bool inserted;
				std::tie(it, inserted) = _edges.insert(std::make_pair(ptr, edge_id()));
				if (inserted)
				{
					std::tie(it->second, inserted) = boost::add_edge(source, target, ptr, _graph);
					/* boost::add_edge: Adds edge (u,v) to the graph and returns the edge descriptor for the new edge. 
					    For graphs that do not allow parallel edges, if the edge is already
					    in the graph then a duplicate will not be added and the bool flag
					    will be false. When the flag is false, the returned edge descriptor points
					    to the already existing edge.
					*/
					if (inserted)
					{
						// Create connection
						ptr->connect(source_ptr.get(), target_ptr.get());
						ptr->set_chart(this, [this, it](){this->remove_edge(it->second);} );
						events::on_edge_added_to_chart(*it, *this);
					}
				}
				return std::make_pair(it->second, inserted);
			};

			void remove_edge(edge_type_ptr ptr)
			{
				_t_edges::iterator it = _edges.find(ptr);
				if (it != _edges.end())
				{
					boost::remove_edge(it->second, _graph);
					_edges.erase(it);
					ptr->remove_chart(this);
					events::on_edge_removed_from_chart(ptr, *this);
				}
			};

			void remove_edge(edge_id id)
			{
				edge_type_ptr ptr = _graph[id];
				this->remove_edge(ptr);
			};

			const _t_vertices& get_vertices() const { return _vertices;};
			const _t_edges& get_edges() const { return _edges;};

			vertex_type_ptr get_vertex(const vertex_id& id) const { return _graph[id];};
			edge_type_ptr get_edge(const edge_id& id) const { return _graph[id];};

			vertex_id get_vertex_id(const vertex_type_ptr& ptr) const { return _vertices.find(ptr)->second;};
			edge_id get_edge_id(const edge_type_ptr& ptr) const { return _edges.find(ptr)->second;};

			size_t connected_components(_t_connected_components& vertex_at_cmp) const
			{
				/* Refundido de: http://stackoverflow.com/questions/7935417/how-provide-a-vertex-index-property-for-my-graph
				                 http://stackoverflow.com/questions/3183186/perform-connected-components-with-boost-adjacency-list-where-vertexlist-lists
				                 http://lists.boost.org/boost-users/2007/08/30612.php
				                 http://www.boost.org/doc/libs/1_35_0/libs/graph/doc/faq.html
				*/

				boost::associative_property_map< _t_connected_components > component_map(vertex_at_cmp);

				size_t index = 0;
				BGL_FORALL_VERTICES(v, _graph, _t_graph)
				{
					boost::put(component_map, v, index++);
				}

				return boost::connected_components(_graph, component_map);
			}

		protected:
			_t_graph _graph;
			_t_vertices _vertices;
			_t_edges _edges;
	};


	/****************
	*   CHART with VERTEX_INNER
	****************/
	template <class VertexType, class EdgeType, int Behaviour, typename Enable=void>
	class chart_vertex_inner
	{
		static_assert(true, "NON");
	};

	template <class VertexType, class EdgeType, int Behaviour>
	class chart_vertex_inner<VertexType, EdgeType, Behaviour, typename std::enable_if<detail::is_vertex_inner<VertexType>::value>::type> 
	: public virtual chart_impl<VertexType, EdgeType, Behaviour>
	{
			static_assert(true, "YES");
			// Add some functions to 'chart' when vertex class implements vertex_inner behaviour
		public:
			std::pair<vertex_id, bool> create_vertex(typename VertexType::inner_type_ptr inner)
			{
				vertex_type_ptr ptr(new vertex_type(inner));
				return this->add_vertex(ptr);
			};
	};

	/****************
	*   CHART with EDGE_INNER
	****************/
	template <class VertexType, class EdgeType, int Behaviour, typename Enable=void>
	class chart_edge_inner : public virtual chart_impl<VertexType, EdgeType, Behaviour>
	{
		public:
			std::pair<edge_id, bool> create_edge(const vertex_id& source, const vertex_id& target)
			{
				edge_type_ptr ptr(new edge_type());
				return this->add_edge(ptr, source, target);
			};
	};

	template <class VertexType, class EdgeType, int Behaviour>
	class chart_edge_inner<VertexType, EdgeType, Behaviour, typename std::enable_if<detail::is_edge_inner<EdgeType>::value>::type> 
	: public virtual chart_impl<VertexType, EdgeType, Behaviour>
	{
		// Add some functions to 'chart' when vertex class implements vertex_inner behaviour
		public:
			std::pair<edge_id, bool> create_edge(typename EdgeType::inner_type_ptr inner, const vertex_id& source, const vertex_id& target)
			{
				edge_type_ptr ptr(new edge_type(inner));
				return this->add_edge(ptr, source, target);
			};
	};


	/****************
	*   Behaviour
	****************/
	template <class VertexType, class EdgeType, int Behaviour>
	class chart_behaviour
	{
	};

	template <class VertexType, class EdgeType>
	class chart_behaviour<VertexType, EdgeType, BIDIRECTIONAL> : public virtual chart_impl<VertexType, EdgeType, BIDIRECTIONAL>
	{
		public:
			// All this functions requires "Directional" graphs
			void get_edges_outgoing(const vertex_id& vertex, std::vector<edge_id>& edges) const
			{
				boost::graph_traits<_t_graph>::out_edge_iterator it_begin, it_end;
				boost::tie(it_begin, it_end) = boost::out_edges(vertex, _graph);
				//std::copy(it_begin, it_end, edges.begin()); //! \todo TODO: Why not?
				std::for_each(it_begin, it_end, [&edges](const edge_id& id) {edges.push_back(id);});
			};

			void get_edges_incoming(const vertex_id& vertex, std::vector<edge_id>& edges) const
			{
				//boost::graph_traits<_t_graph>::in_edge_iterator it_begin, it_end;
				_t_graph::in_edge_iterator it_begin, it_end;
				boost::tie(it_begin, it_end) = boost::in_edges(vertex, _graph);
				//std::copy(it_begin, it_end, edges.begin()); //! \todo TODO: Why not?
				std::for_each(it_begin, it_end, [&edges](const edge_id& id) {edges.push_back(id);});
			};

			vertex_id get_source(const edge_id& edge) const
			{
				return boost::source(edge, _graph);
			};

			vertex_id get_target(const edge_id& edge) const
			{
				return boost::target(edge, _graph);
			};
	};
	
	template <class VertexType, class EdgeType>
	class chart_behaviour<VertexType, EdgeType, UNDIRECTED> : public virtual chart_impl<VertexType, EdgeType, UNDIRECTED>
	{
		public:
			std::pair<vertex_id, vertex_id> get_connected(const edge_id& edge) const
			{
				return std::make_pair(boost::source(edge, _graph), boost::target(edge, _graph));
			};
	};
}


// Join all policies
template <class VertexType, class EdgeType, int Behaviour>
class chart : public virtual detail::chart_impl<VertexType, EdgeType, Behaviour>,
              public virtual detail::chart_vertex_inner<VertexType, EdgeType, Behaviour>,
              public virtual detail::chart_edge_inner<VertexType, EdgeType, Behaviour>,
              public virtual detail::chart_behaviour<VertexType, EdgeType, Behaviour>
{
	public:
		virtual ~chart() {};
};


}}

