#ifndef __SBM_INCLUDED__
#define __SBM_INCLUDED__

#include "Node.h" 
#include <map>
#include <set>
#include <memory>
#include <numeric>
#include <utility> 


//=================================
// What this file declares
//=================================
class SBM;
struct Trans_Probs;
typedef std::shared_ptr<Node> NodePtr;

// A map keyed by ID of nodes that live on a given level of the SBM
typedef std::map<string, NodePtr> NodeLevel;

typedef std::shared_ptr<NodeLevel> LevelPtr;

// A map keyed by level integer of each level of nodes 
typedef std::map<int, LevelPtr> LevelMap;

typedef std::map<std::pair<string, string>, int> EdgeCounts;

using std::string;
using std::vector;
using std::map;

//=================================
// Main node class declaration
//=================================
class SBM {
  public:
    SBM(); 
    // ==========================================
    // Attributes
    LevelMap       nodes;               // A map keyed by level integer of each level of nodes 
    std::set<int>  unique_node_types;   // Vector storing all the unique types of nodes seen. Used to make sure the correct move proposals are made

    // ==========================================
    // Methods
    void           add_level(int);                            // Setup a new Node level
    LevelPtr       get_level(int); 
    void           check_level_has_nodes(const LevelPtr);     // Validates that a given level has nodes and throws error if it doesn;t
    list<NodePtr>  get_nodes_from_level(int, int, bool);      // Return nodes of a desired type from level can be switched from matching or not ma
    list<NodePtr>  get_nodes_of_type_at_level(int, int);      // Return nodes of a desired type from level can be switched from matching or not ma
    list<NodePtr>  get_nodes_not_of_type_at_level(int, int);  // Return nodes node of a specified type from level
    NodePtr        create_group_node(int, int);               // Creates a new group node and adds it to its neccesary level
    NodePtr        get_node_by_id(string);                    // Grabs and returns node of specified id, if node doesn't exist, node is created first
    NodePtr        add_node(string, int, int);                // Adds a node of specified id of a type and at a level
    NodePtr        add_node(string, int);                     // Adds a node of specified id of a type and at level 0
    void           add_connection(string, string);            // Adds a connection between two nodes based on their ids
    void           add_connection(NodePtr, NodePtr);          // Adds a connection between two nodes based on their pointers
    void           give_every_node_a_group_at_level(int);     // Builds and assigns a group node for every node in a given level
    NodePtr        get_node_from_level(int);                  // Grabs the first node found at a given level, used in testing.
    Trans_Probs    get_transition_probs_for_groups(NodePtr);  // Calculates probabilities for joining a given new group based on current SBM state
    int            clean_empty_groups();                      // Scan through levels and remove all group nodes that have no children. Returns # removed
    EdgeCounts     gather_edge_counts(int);                   // Builds a id-id paired map of edge counts between nodes of the same level
    
    static void    update_edge_counts(EdgeCounts&, int, NodePtr, NodePtr, NodePtr); // Update an EdgeCount map after moving a node around to avoid rescanning
    static string  build_group_id(int, int, int);             // Builds a group id from a scaffold for generated new groups
      
};


// Two equal-sized vectors, one containing probabilities of a node joining a
// group and the other containing the group which each probability is associated
struct Trans_Probs {
  vector<double>  probability;
  vector<NodePtr> group;
  Trans_Probs(vector<double> p, vector<NodePtr> g):
    probability(p),
    group(g){};
};

// Helper function that insures order doesn't matter for EdgeCount key pair
std::pair<string, string> id_pair(string, string);


#endif