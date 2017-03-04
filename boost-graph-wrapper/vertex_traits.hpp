
#pragma once

#include <memory>
#include <set>
#include "boost-graph-wrapper_export.h"

#include "traits.hpp"
#include "edge_traits.hpp"

namespace core { namespace graph { namespace detail {

/*************
*   BEHAVIOURS
*************/
namespace _impl
{
	template <class EdgeType>
	class vertex_connected
	{
		public:
			typedef typename std::set<EdgeType*> _t_edges;
			virtual ~vertex_connected() {};

		protected:
			virtual void erase_incoming(EdgeType* ptr) = 0;
			virtual void erase_outgoing(EdgeType* ptr) = 0;

			virtual void append_incoming(EdgeType* ptr) = 0;
			virtual void append_outgoing(EdgeType* ptr) = 0;

		protected:
			//static const int IsVertexConnected;
	};

}

template <class EdgeType, int Behaviour>
class vertex_connected;

template <class EdgeType>
class vertex_connected<EdgeType, UNDIRECTED> : public _impl::vertex_connected<EdgeType>
{
		template<class V, class E> friend class _impl::edge_connected; // C++11 standard compliant
	public:
		static const int IsVertexConnected = UNDIRECTED;

	public:
		virtual ~vertex_connected()
		{
			/*! \todo TODO: weird memory problem
			while(_edges.size())
			{
				*_edges.begin())->disconnect();
			}
			*/
		};

		const _t_edges& get_edges() const {return _edges;};

	protected:
		virtual void erase_incoming(EdgeType* ptr)
		{
			_edges.erase(ptr);
		};

		virtual void erase_outgoing(EdgeType* ptr)
		{
			_edges.erase(ptr);
		};

		virtual void append_incoming(EdgeType* ptr)
		{
			_edges.insert(ptr);
		};

		virtual void append_outgoing(EdgeType* ptr)
		{
			_edges.insert(ptr);
		};

	protected:
		_t_edges _edges;
};


template <class EdgeType>
class vertex_connected<EdgeType, DIRECTED> : public _impl::vertex_connected<EdgeType>
{
		template<class V, class E> friend class _impl::edge_connected; // C++11 standard compliant
	public:
		static const int IsVertexConnected = DIRECTED;

	public:
		virtual ~vertex_connected()
		{
			/*! \todo TODO: weird memory problem
			while(_out_edges.size())
			{
				(*_out_edges.begin())->disconnect();
			}
			*/
		};

		const _t_edges& get_outgoing() const {return _out_edges;};

	protected:
		virtual void erase_incoming(EdgeType* ptr) { /* not for DIRECTED graphs */ };
		virtual void erase_outgoing(EdgeType* ptr)
		{
			_out_edges.erase(ptr);
		};

		virtual void append_incoming(EdgeType* ptr) { /* not for DIRECTED graphs */ };
		virtual void append_outgoing(EdgeType* ptr)
		{
			_out_edges.insert(ptr);
		};

	protected:
		_t_edges _out_edges;
};


template <class EdgeType>
class vertex_connected<EdgeType, BIDIRECTIONAL> : public _impl::vertex_connected<EdgeType>
{
		template<class V, class E> friend class _impl::edge_connected; // C++11 standard compliant
	public:
		static const int IsVertexConnected = BIDIRECTIONAL;

	public:
		virtual ~vertex_connected()
		{
			/*! \todo TODO: weird memory problem
			while(_out_edges.size())
			{
				(*_out_edges.begin())->disconnect();
			}
			while(_in_edges.size())
			{
				(*_in_edges.begin())->disconnect();
			}
			*/
		};

		const _t_edges& get_incoming() const {return _in_edges;};
		const _t_edges& get_outgoing() const {return _out_edges;};

	protected:
		virtual void erase_incoming(EdgeType* ptr) { _in_edges.erase(ptr);};
		virtual void erase_outgoing(EdgeType* ptr) { _out_edges.erase(ptr);};

		virtual void append_incoming(EdgeType* ptr) { _in_edges.insert(ptr);};
		virtual void append_outgoing(EdgeType* ptr) { _out_edges.insert(ptr);};

	protected:
		_t_edges _in_edges, _out_edges;
};

// SFINAE test for vertex_connected
template <typename T> struct is_vertex_connected
{
	struct Fallback { const int IsVertexConnected; }; // introduce member name "IsVertexConnected"
	struct Derived : T, Fallback {};

	template <typename C, C> struct ChT;

	template<typename C> static char (&f(ChT<const int Fallback::*, &C::IsVertexConnected>*))[1];
	template<typename C> static char (&f(...))[2];

	static bool const value = (sizeof(f<Derived>(0)) == 2);
};

template <typename T, int TypeConnection>
struct test_type_connection
{
	static bool const value = (T::IsVertexConnected == TypeConnection);
};



/*************
*   MULTICHART
*************/
class chart_base;

template <class Chart>
class vertex_multichart
{
		template <class VertexType, class EdgeType, int Behaviour> friend class chart_impl;
	public:
		typedef typename std::set<Chart*> _t_charts;
		const _t_charts& get_charts() const { return _charts;};

	protected:
		void insert_chart(chart_base* chart)
		{
			_charts.insert(dynamic_cast<Chart*>(chart));
		};

		void erase_chart(chart_base* chart)
		{
			_charts.erase(dynamic_cast<Chart*>(chart));
		};

	protected:
		_t_charts _charts;
		static int IsMultichart;
};

template <>
class vertex_multichart<void> {};

// SFINAE test for vertex_multichart
template <typename T> struct is_vertex_multichart
{
	struct Fallback { int IsMultichart; }; // introduce member name "IsMultichart"
	struct Derived : T, Fallback {};

	template <typename C, C> struct ChT;

	template<typename C> static char (&f(ChT<int Fallback::*, &C::IsMultichart>*))[1];
	template<typename C> static char (&f(...))[2];

	static bool const value = (sizeof(f<Derived>(0)) == 2);
};

/*! \todo TODO: to check with c++11 (consider implementing also for edge_connected)
template <class VertexType, typename Enable=void>
struct is_base_vertex_multichart : std::false_type {};

template <class VertexType>
struct is_base_vertex_multichart<VertexType, typename std::enable_if<!std::is_same<decltype(std::declval<VertexType>().IsMultichart), void>::value, bool>::type> : std::true_type {};
*/


/*************
*   INNER
*************/
template <class Inner>
class vertex_inner
{
	public:
		typedef Inner inner_type;
		typedef typename std::shared_ptr<Inner> inner_type_ptr;

	public:
		vertex_inner(inner_type_ptr obj) : _obj(obj) {};

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
		static int IsVertexInner;
};

template <>
class vertex_inner<void>
{
	static_assert(true, "!!!!VERTEX_INNER!!!");
};

// SFINAE test for vertex_inner
template <typename T> struct is_vertex_inner
{
	struct Fallback { int IsVertexInner; }; // introduce member name "IsVertexInner"
	struct Derived : T, Fallback {};

	template <typename C, C> struct ChT;

	template<typename C> static char (&f(ChT<int Fallback::*, &C::IsVertexInner>*))[1];
	template<typename C> static char (&f(...))[2];

	static bool const value = (sizeof(f<Derived>(0)) == 2);
};


/*************
*   VISITOR
*************/
class vertex_visitable
{
		//template <class VertexType, class EdgeType> friend class search_algorithm;
	public:
		enum _e_visitor_labels {UNDISCOVERED, DISCOVERED, EXPLORED};                

	public:
		vertex_visitable() : _label(UNDISCOVERED) {};

		const _e_visitor_labels& get_label() const { return _label;};

	//protected:
		_e_visitor_labels _label;
};


/*************
*   VERTEX
*************/            
template <class EdgeType, int Behaviour, class Graph=void, class Inner=void>
class vertex : public detail::vertex_connected<EdgeType, Behaviour>,
               public detail::vertex_multichart<Graph>,
               public detail::vertex_inner<Inner>,
               public detail::vertex_visitable
{
	public:
		vertex(inner_type_ptr obj) : detail::vertex_inner<Inner>(obj) {};
		virtual ~vertex() {};

};

// Specialization not requesting 'inner' instance on constructor
template <class EdgeType, int Behaviour, class Graph>
class vertex<EdgeType, Behaviour, Graph, void> : public detail::vertex_connected<EdgeType, Behaviour>,
                                                 public detail::vertex_multichart<Graph>,
                                                 public detail::vertex_inner<void>, // void impl
                                                 public detail::vertex_visitable
{
	public:
		vertex() {};
		virtual ~vertex() {};
};

}}}

