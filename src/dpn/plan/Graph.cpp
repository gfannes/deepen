#include <dpn/plan/Graph.hpp>
#include <dpn/meta/Effort.hpp>

#include <gubg/naft/Document.hpp>
#include <gubg/map.hpp>

namespace dpn { namespace plan { 

	std::ostream &operator<<(std::ostream &os, const Graph &graph)
	{
		gubg::naft::Document doc{os};
		auto graph_n = doc.node("Graph");
		graph_n.attr("size", graph.vertices.size());
		for (auto id: graph.vertices)
		{
			// if (auto depth = gubg::get(graph.depth, id))
			// {
			// 	if (*depth > 7)
			// 		continue;
			// }

			auto vertex_n = graph_n.node("Vertex");
			vertex_n.attr("id", id);
			if (auto depth = gubg::get(graph.depth, id))
				vertex_n.attr("depth", *depth);
			if (auto parent = gubg::get(graph.parent, id))
				vertex_n.attr("parent", *parent);
			if (auto agg_effort = gubg::get(graph.agg_effort, id))
				vertex_n.attr("effort", meta::Effort::to_dsl(*agg_effort));
			if (auto text = gubg::get(graph.text, id))
				vertex_n.attr("text", *text);
		}
		return os;
	}

} } 