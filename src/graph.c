#include "AST.h"

void add_nodes_to_graphviz_file(AST_node* node, FILE* f, int depth)
{
    depth++;
    if (depth > 15) {
        return;
    }
    for (int i = 0; i < node->children->count; i++) {

        fprintf(f, "\"%s, %i\" -> \"%s, %i\"\n",
                node->token->data,
                node->token->pos_in_file,
                ((AST_node**)(node->children->data))[i]->token->data,
                ((AST_node**)(node->children->data))[i]->token->pos_in_file
                );

        add_nodes_to_graphviz_file(((AST_node**)(node->children->data))[i], f, depth);
    }
}

void generate_graphviz_from_AST_node(AST_node* node, char* file_name) {
    FILE* graphviz_file = fopen(file_name, "w");
    fprintf(graphviz_file, "digraph G {\n");
    add_nodes_to_graphviz_file(node, graphviz_file, 0);
    fprintf(graphviz_file, "}");
    fclose(graphviz_file);
}
