
#ifndef GRAPH_H
#define GRAPH_H

#include <stdio.h>

void add_nodes_to_graphviz_file(AST_node* node, FILE* f, int depth);

void generate_graphviz_from_AST_node(AST_node* node, char* file_name);

#endif // GRAPH_H

