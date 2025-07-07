#include "AST.h"
#include "graph.h"

void add_nodes_to_graphviz_file(AST_node* node, FILE* f, int depth)
{
    //depth++;
    if (depth > 10) {
        return;
    }
    for (int i = 0; i < node->children->count; i++) {
        
        char* str1 = malloc(sizeof(char)*strlen(node->token->data));
        strcpy(str1, node->token->data);
        if (str1[0] == '"') {
            str1[0] = ';';
            str1[strlen(str1) - 1] = ';';
        }
        char* str2 = malloc(sizeof(char)*strlen(((AST_node**)(node->children->data))[i]->token->data));
        strcpy(str2, ((AST_node**)(node->children->data))[i]->token->data);
        if (str2[0] == '"') {
            str2[0] = ';';
            str2[strlen(str2) - 1] = ';';
        }
        
        fprintf(f, "\"%s, %i, %i\" -> \"%s, %i, %i\"\n",
                str1,
                node->token->pos_in_file,
                node->node_type,
                str2,
                ((AST_node**)(node->children->data))[i]->token->pos_in_file,
                ((AST_node**)(node->children->data))[i]->node_type
                );

        free(str1);
        free(str2);
        
        add_nodes_to_graphviz_file(((AST_node**)(node->children->data))[i], f, i);
    }
}



void generate_graphviz_from_AST_node(AST_node* node, char* file_name) {
    FILE* graphviz_file = fopen(file_name, "w");
    fprintf(graphviz_file, "digraph G {\n");
    add_nodes_to_graphviz_file(node, graphviz_file, 0);
    fprintf(graphviz_file, "}");
    fclose(graphviz_file);
}


