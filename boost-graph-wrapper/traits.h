
#pragma once

namespace core { namespace graph {

    /*! Behavoiour policies define the way vertex and edges can be assembled:
        1) BIDIRECTIONAL: source and target concepts are clear for vertices
            and edges, in such a way that the client code will be able to
            traverse the graph forwards and backwards.
        2) UNDIRECTED: there is no distinction between source and target, there
            is no incoming neither outgoing. Graph will only talk about "neighbours"
            and connections.
        3) DIRECTED: client code will only be able to traverse the graph on one
            direction, all "backwards" connections are hidden.
    */
    enum _e_behaviour { BIDIRECTIONAL, UNDIRECTED, DIRECTED };

    /*! Direction policies define visiting order for search (if direction makes sense according to behaviour policy):
        1) FORWARDS: visitor moves forwards, from incoming edges to outgoing.
        2) BACKWARDS: from outgoing to incoming
    */
    enum _e_direction { FORWARDS, BACKWARDS };

}}

