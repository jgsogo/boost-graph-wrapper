
#pragma once

#include <memory>
#include <functional>
#include <assert.h>

#include "core/graph/boost-graph-wrapper_export.h"

#include "traits.h"

namespace core { namespace graph { namespace detail {

/*****************
*   BEHAVIOURS
*****************/
namespace _impl
{
	template <class VertexType, class EdgeType> 
	class edge_connected
	{
		protected:
			edge_connected() : _source(0), _target(0) {};

		public:
			virtual ~edge_connected()
			{
				this->disconnect();
			}

			bool is_connected() const
			{
				return (_source || _target);
			}

			virtual void connect(VertexType* source, VertexType* target)
			{
				if (is_connected())
				{
					throw std::runtime_error("Disconnect before connecting to other vertices.");
				}
				_source = source; source->append_outgoing(dynamic_cast<EdgeType*>(this));
				_target = target; target->append_incoming(dynamic_cast<EdgeType*>(this));
			}

			virtual void disconnect()
			{
				if (is_connected())
				{
					assert(_target!=0 && _source!=0);
					_source->erase_outgoing(dynamic_cast<EdgeType*>(this));
					_source = 0;
					_target->erase_incoming(dynamic_cast<EdgeType*>(this));
					_target = 0;
				}
			}

		protected:
			VertexType* _source;
			VertexType* _target;
			static const int IsEdgeConnected;
	};
}

template <class VertexType, class EdgeType, int Behaviour>
class edge_connected;

template <class VertexType, class EdgeType>
class edge_connected<VertexType, EdgeType, UNDIRECTED> : public _impl::edge_connected<VertexType, EdgeType>
{
	public:
		std::pair<VertexType*, VertexType*> get_connected() const
		{
			return std::make_pair(_target, _source); /* Intentionally return them in "wrong" order,
			                                            cause for UNDIRECTED connections there is no
			                                            source nor target.
			                                         */
		};

	protected:
		static const int IsEdgeConnectedUndirected;
};

template <class VertexType, class EdgeType>
class edge_connected<VertexType, EdgeType, DIRECTED> : public _impl::edge_connected<VertexType, EdgeType> {
	public:
		// On directed graph client code will only be able to traverse forwards.
		VertexType* get_target() const { return _target;};
	protected:
		static const int IsEdgeConnectedDirected;
	};

template <class VertexType, class EdgeType>
class edge_connected<VertexType, EdgeType, BIDIRECTIONAL> : public _impl::edge_connected<VertexType, EdgeType>
{
	public:
		VertexType* get_source() const { return _source;};
		VertexType* get_target() const { return _target;};

	protected:
		static const int IsEdgeConnectedBidirectional;
};

// SFINAE test for edge_connected
template <typename T> struct is_edge_connected
{
	struct Fallback { const int IsEdgeConnected; }; // introduce member name "IsEdgeConnected"
	struct Derived : T, Fallback {};

	template <typename C, C> struct ChT;

	template<typename C> static char (&f(ChT<const int Fallback::*, &C::IsEdgeConnected>*))[1];
	template<typename C> static char (&f(...))[2];

	static bool const value = (sizeof(f<Derived>(0)) == 2);
};



/*************
*   INNER
*************/
template <class Inner>
class edge_inner
{
	public:
		typedef Inner inner_type;
		typedef typename std::shared_ptr<Inner> inner_type_ptr;

	public:
		edge_inner(typename std::shared_ptr<Inner> obj) : _obj(obj) {};

		inner_type_ptr operator->() const
		{
			return _obj;
		};

		const inner_type& get_obj() const
		{
			return *_obj.get();
		};

	protected:
		inner_type_ptr _obj;
		static const int IsEdgeInner;
};

template<>
class edge_inner<void>
{
};

// SFINAE test for edge_inner
template <typename T> struct is_edge_inner
{
	struct Fallback { int IsEdgeInner; }; // introduce member name "IsEdgeInner"
	struct Derived : T, Fallback {};

	template <typename C, C> struct ChT;

	template<typename C> static char (&f(ChT<int Fallback::*, &C::IsEdgeInner>*))[1];
	template<typename C> static char (&f(...))[2];

	static bool const value = (sizeof(f<Derived>(0)) == 2);
};



/*************
*   CHART Belonging
*************/
class chart_base;

template <class Chart, class EdgeType>
class edge_chart
{
	//typedef typename Chart::vertex_type_ptr vertex_type_ptr;
	//typedef typename Chart::edge_id edge_id;
		template <class VertexType, class EdgeType, int Behaviour> friend class chart_impl;

	public:
		Chart* get_chart() const { return _chart;};

	protected:
		edge_chart() : _chart(0) {};

		virtual void disconnect()
		{
			if (_chart)
			{
				//_chart->remove_edge(static_cast<EdgeType*>(this));
				_on_disconnect();
				//_chart->remove_edge(_e_id);
			}
		};

		void set_chart(chart_base* chart, std::function<void ()> on_disconnect)
		{
			_chart = dynamic_cast<Chart*>(chart);
			_on_disconnect = on_disconnect;
		};

		void remove_chart(chart_base* chart)
		{
			/*assert(_chart == dynamic_cast<Chart*>(chart)); /*! \todo TODO: 'dynamic_cast<Chart*>(chart)' 
			                                                           is evaluated to 0! when called from
			                                                           'chart' destructor...
			                                                 */
			_chart = (Chart*)0;
		};

	protected:
		Chart* _chart;
		std::function<void ()> _on_disconnect;
		static int IsChart;
};

template <class EdgeType>
class edge_chart<void, EdgeType>
{
	public:
		virtual void disconnect() {};
};

// SFINAE test for edge_chart
template <typename T> struct is_edge_chart
{
	struct Fallback { int IsChart; }; // introduce member name "IsChart"
	struct Derived : T, Fallback {};

	template <typename C, C> struct ChT;

	template<typename C> static char (&f(ChT<int Fallback::*, &C::IsChart>*))[1];
	template<typename C> static char (&f(...))[2];

	static bool const value = (sizeof(f<Derived>(0)) == 2);
};



/*************
*   VISITOR
*************/
class edge_visitable
{
	//template <class VertexType, class EdgeType> friend class search_algorithm;
	public:
		enum _e_visitor_labels {NON_TREE_EDGE, TREE_EDGE, BACK_EDGE, FORWARD_OR_CROSS_EDGE};

	public:
		edge_visitable() : _label(NON_TREE_EDGE) {};

		const _e_visitor_labels& get_label() const { return _label;};

	//protected:
		_e_visitor_labels _label;
};



/*************
*   EDGE
*************/
template <class VertexType, class EdgeType, int Behaviour, class Graph=void, class Inner=void>
class edge : public detail::edge_connected<VertexType, EdgeType, Behaviour>,
             public detail::edge_chart<Graph, EdgeType>,
             public detail::edge_inner<Inner>,
             public detail::edge_visitable
{
	public:
		edge(inner_type_ptr obj) : detail::edge_inner<Inner>(obj) {};

		virtual ~edge() {};

		virtual void disconnect()
		{
			detail::edge_connected<VertexType, EdgeType, Behaviour>::disconnect();
			detail::edge_chart<Graph, EdgeType>::disconnect();
		};
};

// Specialization not requesting 'inner' instance on constructor
template <class VertexType, class EdgeType, int Behaviour, class Graph>
class edge<VertexType, EdgeType, Behaviour, Graph, void> :  public edge_connected<VertexType, EdgeType, Behaviour>, 
                                                            public detail::edge_chart<Graph, EdgeType>,
                                                            public detail::edge_inner<void>,
                                                            public detail::edge_visitable
{
	public:
		edge() {};

		virtual ~edge() {};

		virtual void disconnect()
		{
			detail::edge_connected<VertexType, EdgeType, Behaviour>::disconnect();
			detail::edge_chart<Graph, EdgeType>::disconnect();
		};

};

} } }

