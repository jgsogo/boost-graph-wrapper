
#pragma once

#include <typeinfo>
#include "vertex_traits.h"

namespace core { namespace graph { namespace detail {

struct events {

	/***********
	*   VERTICES
	***********/
	// events::on_vertex_added_to_chart
	template <class Chart>
	static void on_vertex_added_to_chart(const std::pair<typename Chart::vertex_type_ptr, typename Chart::vertex_id>&, Chart& chart)
	{
		// Called whenever a vertex is added to a chart
	};

	// events::on_vertex_removed_from_chart
	template <class Chart>
	static void on_vertex_removed_from_chart(typename Chart::vertex_type_ptr, Chart& chart)
	{
		// Called whenever a vertex is removed from a chart
	};

	/***********
	*   EDGES
	***********/
	// events::on_edge_added_to_chart
	template <class Chart>
	static void on_edge_added_to_chart(const typename std::pair<typename Chart::edge_type_ptr, typename Chart::edge_id>&, Chart& chart)
	{
	};

	// events::on_edge_removed_from_chart
	template <class Chart>
	static void on_edge_removed_from_chart(typename Chart::edge_type_ptr, Chart& chart)
	{
	};

};

}}}

