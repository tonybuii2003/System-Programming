#include <stdlib.h>

#include "global.h"
#include "debug.h"
size_t getLen(char *s)
{
    int i = 0;
    size_t length = 0;
    while (*s != 0)
    {
        length += 1;
        s++;
    }
    return length;
}
char *str_copy(char *str1, char *str2, int length)
{
    for (int i = 0; i < length; i++)
    {
        *str1 = *str2;
        str1++;
        str2++;
    }
    *str1 = 0;
    return str1;
}
int equals(char *s1, char *s2)
{
    int length = getLen(s2);
    for (int i = 0; i < length; i++)
    {
        if (*s1 != *s2)
        {
            return 0;
        }
        s1++;
        s2++;
    }
    return 1;
}
double string_to_float(char *s)
{
    char *ptr = s;
    double f = 0.1;
    double answer = 0;
    while (*ptr != 0)
    {
        if (*ptr >= '0' && *ptr <= '9')
        {
            answer = answer * 10.0 + (*ptr - '0');
            ptr++;
        }
        else if (*ptr == '.')
        {
            ptr++;
            while (*ptr >= '0' && *ptr <= '9')
            {
                answer += (*ptr - '0') * f;
                f *= 0.1;
                ptr++;
            }
        }
        else
        {
            return -1.0;
        }
    }
    return answer;
}
void update_row_sum()
{
    double ans = 0;
    double *row_sum_ptr = row_sums;
    double *distance_ptr = (double *)distances;
    for (int i = 0; i < num_active_nodes; i++)
    {
        int actual_i = *(active_node_map + i);
        ans = 0;
        for (int j = 0; j < num_active_nodes; j++)
        {
            int actual_j = *(active_node_map + j);
            ans += *(distance_ptr + actual_i * MAX_NODES + actual_j);
            // printf("%lf +", *(distance_ptr + actual_i * MAX_NODES + actual_j));
        }
        // printf("\n");
        *(row_sum_ptr + actual_i) = ans;
    }
}

/**
 * @brief  Read genetic distance data and initialize data structures.
 * @details  This function reads genetic distance data from a specified
 * input stream, parses and validates it, and initializes internal data
 * structures.
 *
 * The input format is a simplified version of Comma Separated Values
 * (CSV).  Each line consists of text characters, terminated by a newline.
 * Lines that start with '#' are considered comments and are ignored.
 * Each non-comment line consists of a nonempty sequence of data fields;
 * each field iis terminated either by ',' or else newline for the last
 * field on a line.  The constant INPUT_MAX specifies the maximum number
 * of data characters that may be in an input field; fields with more than
 * that many characters are regarded as invalid input and cause an error
 * return.  The first field of the first data line is empty;
 * the subsequent fields on that line specify names of "taxa", which comprise
 * the leaf nodes of a phylogenetic tree.  The total number N of taxa is
 * equal to the number of fields on the first data line, minus one (for the
 * blank first field).  Following the first data line are N additional lines.
 * Each of these lines has N+1 fields.  The first field is a taxon name,
 * which must match the name in the corresponding column of the first line.
 * The subsequent fields are numeric fields that specify N "distances"
 * between this taxon and the others.  Any additional lines of input following
 * the last data line are ignored.  The distance data must form a symmetric
 * matrix (i.e. D[i][j] == D[j][i]) with zeroes on the main diagonal
 * (i.e. D[i][i] == 0).
 *
 * If 0 is returned, indicating data successfully read, then upon return
 * the following global variables and data structures have been set:
 *   num_taxa - set to the number N of taxa, determined from the first data line
 *   num_all_nodes - initialized to be equal to num_taxa
 *   num_active_nodes - initialized to be equal to num_taxa
 *   node_names - the first N entries contain the N taxa names, as C strings
 *   distances - initialized to an NxN matrix of distance values, where each
 *     row of the matrix contains the distance data from one of the data lines
 *   nodes - the "name" fields of the first N entries have been initialized
 *     with pointers to the corresponding taxa names stored in the node_names
 *     array.
 *   active_node_map - initialized to the identity mapping on [0..N);
 *     that is, active_node_map[i] == i for 0 <= i < N.
 *
 * @param in  The input stream from which to read the data.
 * @return 0 in case the data was successfully read, otherwise -1
 * if there was any error.  Premature termination of the input data,
 * failure of each line to have the same number of fields, and distance
 * fields that are not in numeric format should cause a one-line error
 * message to be printed to stderr and -1 to be returned.
 */

int read_distance_data(FILE *in)
{
    if (!in)
    {
        return -1;
    }
    int c;
    int skip = 0;
    int count_length = 0;
    int taxa_count = 0;
    int count_line = 0;
    int array_index = 0;
    int input_buffer_index = 0;
    int current_row = 0;
    char prev = 0;
    char *input = input_buffer;
    char *current_name = (char *)node_names;
    double *current_float = (double *)distances;
    while ((c = fgetc(in)) != EOF)
    {
        // printf("countline: %d\n", count_line);
        // printf("current char: %c \n", c);
        if (c == '#')
        {
            skip = 1;
            while ((c != '\n' && c != EOF) || c == 0)
            {
                c = fgetc(in);
            }
            // printf("don't\n");
        }
        if (skip == 1)
        {
            skip = 0;
            continue;
        }
        if (c == EOF)
            break;
        // printf("char: %c hex: %x\n", c, c);
        if (count_line == 0)
        {
            if (array_index > INPUT_MAX || input_buffer_index > INPUT_MAX + 1)
                return -1;
            if (c == ',')
            {
                if (num_taxa != 0)
                {
                    // printf("Yes\n");
                    *input = 0;
                    // printf("current buffer: %s\n", input_buffer);
                    str_copy(*(node_names + array_index), input_buffer, getLen(input_buffer));
                    // printf("current name: %s\n", *(node_names + array_index));
                    // printf("array index: %d\n", array_index);
                    array_index++;
                    input = input_buffer;
                    input_buffer_index = 0;
                }
                num_taxa += 1;
            }
            if (c != ',' && c != '\n' && num_taxa != 0)
            {

                *input = c;
                // printf("current buffer: %s\n", input_buffer);
                input_buffer_index += 1;
                input += 1;
            }
            if (c == '\n')
            {
                if (num_taxa != 0)
                {
                    // printf("Yes\n");
                    *input = 0;
                    // printf("current buffer: %s\n", input_buffer);
                    str_copy(*(node_names + array_index), input_buffer, getLen(input_buffer));
                    // printf("current name: %s\n", *(node_names + array_index));
                    // printf("array index: %d\n", array_index);
                    array_index++;
                    input = input_buffer;
                    input_buffer_index = 0;
                }
            }
        }

        if (count_line >= 1)
        {
            if (count_line > num_taxa)
            {

                fprintf(stderr, "The distance matrix is not a square matrix\n");
                return -1;
            }
            input = input_buffer;
            if (prev == '\n')
            {
                while (c != ',')
                {
                    if (c == '\n')
                        return -1;
                    *input = c;

                    input_buffer_index += 1;
                    input += 1;
                    c = fgetc(in);
                }
                *input = 0;
                // printf("Got the name!\n");
                // printf("current buffer: %s\n", input_buffer);
                // printf("current node name: %s\n", *(node_names + (count_line - 1)));
                if (!equals(input_buffer, *(node_names + (count_line - 1))))
                {
                    fprintf(stderr, "Invalid names with row and column\n");
                    return -1;
                }
            }
            if (prev == ',')
            {
                while (c != ',' && c != '\n')
                {
                    *input = c;

                    input_buffer_index += 1;
                    input += 1;
                    c = fgetc(in);
                }
                *input = 0;

                // printf("matrix address %p\n", &current_float);
                // printf("current buffer: %s\n", input_buffer);
                // printf("After converted: %lf\n", string_to_float(input_buffer));
                if (string_to_float(input_buffer) < 0)
                {
                    fprintf(stderr, "Invalid distance input\n");
                    return -1;
                }
                *current_float = string_to_float(input_buffer);
                current_float += 1;
                if (c == '\n')
                {
                    // printf("current row: %d\n", current_row);
                    current_row += 1;
                    current_float = (double *)(distances + current_row);
                }
            }
        }
        if (c == '\n')
        {
            count_line += 1;
        }
        prev = c;
    }
    current_float = (double *)distances;

    // printf("count_taxa: %d\n", num_taxa);
    num_active_nodes = num_taxa;
    num_all_nodes = num_taxa;
    // printf("\n");
    for (int i = 0; i < num_taxa; i++)
    {
        for (int j = 0; j < num_taxa; j++)
        {
            if (*(current_float + i * MAX_NODES + j) != *(current_float + j * MAX_NODES + i))
            {
                fprintf(stderr, "The distance matrix is not Symmetric\n");
                return -1;
            }
        }
    }

    return 0;
}

/**
 * @brief  Emit a representation of the phylogenetic tree in Newick
 * format to a specified output stream.
 * @details  This function emits a representation in Newick format
 * of a synthesized phylogenetic tree to a specified output stream.
 * See (https://en.wikipedia.org/wiki/Newick_format) for a description
 * of Newick format.  The tree that is output will include for each
 * node the name of that node and the edge distance from that node
 * its parent.  Note that Newick format basically is only applicable
 * to rooted trees, whereas the trees constructed by the neighbor
 * joining method are unrooted.  In order to turn an unrooted tree
 * into a rooted one, a root will be identified according by the
 * following method: one of the original leaf nodes will be designated
 * as the "outlier" and the unique node adjacent to the outlier
 * will serve as the root of the tree.  Then for any other two nodes
 * adjacent in the tree, the node closer to the root will be regarded
 * as the "parent" and the node farther from the root as a "child".
 * The outlier node itself will not be included as part of the rooted
 * tree that is output.  The node to be used as the outlier will be
 * determined as follows:  If the global variable "outlier_name" is
 * non-NULL, then the leaf node having that name will be used as
 * the outlier.  If the value of "outlier_name" is NULL, then the
 * leaf node having the greatest total distance to the other leaves
 * will be used as the outlier.
 *
 * @param out  Stream to which to output a rooted tree represented in
 * Newick format.
 * @return 0 in case the output is successfully emitted, otherwise -1
 * if any error occurred.  If the global variable "outlier_name" is
 * non-NULL, then it is an error if no leaf node with that name exists
 * in the tree.
 */
int emit_newick_format(FILE *out)
{
    // TO BE IMPLEMENTED
    abort();
}

/**
 * @brief  Emit the synthesized distance matrix as CSV.
 * @details  This function emits to a specified output stream a representation
 * of the synthesized distance matrix resulting from the neighbor joining
 * algorithm.  The output is in the same CSV form as the program input.
 * The number of rows and columns of the matrix is equal to the value
 * of num_all_nodes at the end of execution of the algorithm.
 * The submatrix that consists of the first num_leaves rows and columns
 * is identical to the matrix given as input.  The remaining rows and columns
 * contain estimated distances to internal nodes that were synthesized during
 * the execution of the algorithm.
 *
 * @param out  Stream to which to output a CSV representation of the
 * synthesized distance matrix.
 * @return 0 in case the output is successfully emitted, otherwise -1
 * if any error occurred.
 */
int emit_distance_matrix(FILE *out)
{
    // TO BE IMPLEMENTED
    abort();
}

/**
 * @brief  Build a phylogenetic tree using the distance data read by
 * a prior successful invocation of read_distance_data().
 * @details  This function assumes that global variables and data
 * structures have been initialized by a prior successful call to
 * read_distance_data(), in accordance with the specification for
 * that function.  The "neighbor joining" method is used to reconstruct
 * phylogenetic tree from the distance data.  The resulting tree is
 * an unrooted binary tree having the N taxa from the original input
 * as its leaf nodes, and if (N > 2) having in addition N-2 synthesized
 * internal nodes, each of which is adjacent to exactly three other
 * nodes (leaf or internal) in the tree.  As each internal node is
 * synthesized, information about the edges connecting it to other
 * nodes is output.  Each line of output describes one edge and
 * consists of three comma-separated fields.  The first two fields
 * give the names of the nodes that are connected by the edge.
 * The third field gives the distance that has been estimated for
 * this edge by the neighbor-joining method.  After N-2 internal
 * nodes have been synthesized and 2*(N-2) corresponding edges have
 * been output, one final edge is output that connects the two
 * internal nodes that still have only two neighbors at the end of
 * the algorithm.  In the degenerate case of N=1 leaf, the tree
 * consists of a single leaf node and no edges are output.  In the
 * case of N=2 leaves, then no internal nodes are synthesized and
 * just one edge is output that connects the two leaves.
 *
 * Besides emitting edge data (unless it has been suppressed),
 * as the tree is built a representation of it is constructed using
 * the NODE structures in the nodes array.  By the time this function
 * returns, the "neighbors" array for each node will have been
 * initialized with pointers to the NODE structure(s) for each of
 * its adjacent nodes.  Entries with indices less than N correspond
 * to leaf nodes and for these only the neighbors[0] entry will be
 * non-NULL.  Entries with indices greater than or equal to N
 * correspond to internal nodes and each of these will have non-NULL
 * pointers in all three entries of its neighbors array.
 * In addition, the "name" field each NODE structure will contain a
 * pointer to the name of that node (which is stored in the corresponding
 * entry of the node_names array).
 *
 * @param out  If non-NULL, an output stream to which to emit the edge data.
 * If NULL, then no edge data is output.
 * @return 0 in case the output is successfully emitted, otherwise -1
 * if any error occurred.
 */
int build_taxonomy(FILE *out)
{
    char *input = input_buffer;
    double current_q = 0x7FEFFFFFFFFFFFFF;
    double tmp = 0;
    // int q_i = 0;
    // int q_j = 0;
    int qindexi = 0;
    int qindexj = 0;
    double *row_sum_ptr = row_sums;

    // printf("num active nodes: %d\n", num_active_nodes);
    double *ptr = (double *)(distances);

    for (int i = 0; i < num_active_nodes; i++)
    {
        *(active_node_map + i) = i;
    }
    int actual_i = 0;
    int actual_j = 0;
    while (num_active_nodes != 2)
    {
        current_q = 0x7FEFFFFFFFFFFFFF;
        current_q = 0;
        // //printf("Current active nodes:\n");
        // for (int i = 0; i < num_active_nodes; i++)
        // {
        //     printf("%d, ", *(active_node_map + i));
        // }
        // printf("\n");
        // printf("Current distance matrix:\n");

        // ptr = (double *)(distances);
        // for (int i = 0; i < num_all_nodes; i++)
        // {
        //     // int dis_i = *(active_node_map + i);
        //     // printf("dis_i: %d\n", dis_i);
        //     for (int j = 0; j < num_all_nodes; j++)
        //     {
        //         printf("%lf ", *(ptr + MAX_NODES * i + j));
        //     }
        //     printf("\n");
        // }

        update_row_sum();
        row_sum_ptr = row_sums;
        actual_i = 0;
        actual_j = 0;
        qindexi = 0;
        qindexj = 0;

        for (int i = 0; i < num_active_nodes; i++)
        {
            double *ptr_dm = (double *)(distances + i * MAX_NODES);
            for (int j = 0; j < num_active_nodes; j++)
            {
                actual_i = *(active_node_map + i);
                actual_j = *(active_node_map + j);

                if (i != j)
                {
                    // printf("Q(%s%s): %d * %lf - %lf - %lf = ", *(node_names + actual_i), *(node_names + actual_j), (num_active_nodes - 2), *(ptr + actual_i * MAX_NODES + actual_j), *(row_sum_ptr + actual_i), *(row_sum_ptr + actual_j));
                    tmp = (num_active_nodes - 2) * *(ptr + actual_i * MAX_NODES + actual_j) - *(row_sum_ptr + actual_i) - *(row_sum_ptr + actual_j);
                    // printf("%lf\n", (num_active_nodes - 2) * *(ptr + actual_i * MAX_NODES + actual_j) - *(row_sum_ptr + actual_i) - *(row_sum_ptr + actual_j));
                    if (tmp < current_q)
                    {
                        current_q = tmp;
                        qindexi = i;
                        qindexj = j;
                    }
                }
            }
        }
        actual_i = *(active_node_map + qindexi);
        actual_j = *(active_node_map + qindexj);
        // printf("%s %s \n", *(node_names + actual_i), *(node_names + actual_j));
        char *name_ptr = (char *)node_names;
        // once you got the q create a new node to connect 2 q(i,j)
        NODE *new_node = (nodes + num_all_nodes);
        input = input_buffer;
        *input = '#';
        input++;
        int tmp_all_node = num_all_nodes;
        while (tmp_all_node != 0)
        {
            // printf("cahr: %d\n", (tmp_all_node) + '0');
            *input = ((tmp_all_node % 10) + '0');
            tmp_all_node = tmp_all_node / 10;
            input++;
        }
        *input = 0;
        input = input_buffer;

        str_copy(*(node_names + num_all_nodes), input_buffer, getLen((input_buffer)));
        new_node->name = *(node_names + num_all_nodes);

        int nodes_length = (int)sizeof(new_node->neighbors) / sizeof(NODE);
        NODE *ptr_nei = (NODE *)new_node->neighbors;

        // insert the 2 node
        NODE *first_node = (NODE *)(nodes + actual_i);
        *ptr_nei = *first_node;
        NODE *second_node = (NODE *)(nodes + actual_j);
        *(ptr_nei + 2) = *second_node;
        // update distance matrix
        // printf("Update distance matrix:\n");
        ptr = (double *)(distances);
        for (int i = 0; i < num_active_nodes; i++)
        {
            int u = num_all_nodes;
            int k = *(active_node_map + i);
            // printf("index of node u: %d k: %d\n", u, k);
            if (u == k)
            {
                *(ptr + u * MAX_NODES + k) = *(ptr + k * MAX_NODES + u) = 0;
            }
            else if (k == actual_i)
            {

                // printf("D(%s, %s) = %lf + (%lf - %lf) / (%d - 2))/2 = %lf\n", *(node_names + u), *(node_names + k), *(ptr + actual_i * MAX_NODES + actual_j), *(row_sum_ptr + actual_i), *(row_sum_ptr + actual_j), num_active_nodes, (*(ptr + actual_i * MAX_NODES + actual_j) + (*(row_sum_ptr + actual_i) - *(row_sum_ptr + actual_j)) / (num_active_nodes - 2)) / 2);
                *(ptr + u * MAX_NODES + k) = (*(ptr + actual_i * MAX_NODES + actual_j) + (*(row_sum_ptr + actual_i) - *(row_sum_ptr + actual_j)) / (num_active_nodes - 2)) / 2;
                fprintf(out, "%d,%d,%.2f\n", k, u, *(ptr + u * MAX_NODES + k));
            }
            else if (k == actual_j)
            {
                // printf("D(%s, %s) = %lf + (%lf - %lf) / (%d - 2))/2 = %lf\n", *(node_names + u), *(node_names + k), *(ptr + actual_i * MAX_NODES + actual_j), *(row_sum_ptr + actual_j), *(row_sum_ptr + actual_i), num_active_nodes, (*(ptr + actual_i * MAX_NODES + actual_j) + (*(row_sum_ptr + actual_j) - *(row_sum_ptr + actual_i)) / (num_active_nodes - 2)) / 2);
                *(ptr + u * MAX_NODES + k) = (*(ptr + actual_i * MAX_NODES + actual_j) + (*(row_sum_ptr + actual_j) - *(row_sum_ptr + actual_i)) / (num_active_nodes - 2)) / 2;
                fprintf(out, "%d,%d,%.2f\n", k, u, *(ptr + u * MAX_NODES + k));
            }
            else
            {
                // printf("D(%s, %s) = (%lf + %lf - %lf) / 2 = %lf\n", *(node_names + u), *(node_names + k), *(ptr + actual_i * MAX_NODES + k), *(ptr + actual_j * MAX_NODES + k), *(ptr + actual_i * MAX_NODES + actual_j), (*(ptr + actual_i * MAX_NODES + k) + *(ptr + actual_j * MAX_NODES + k) - *(ptr + actual_i * MAX_NODES + actual_j)) / 2);
                *(ptr + u * MAX_NODES + k) = *(ptr + k * MAX_NODES + u) = (*(ptr + actual_i * MAX_NODES + k) + *(ptr + actual_j * MAX_NODES + k) - *(ptr + actual_i * MAX_NODES + actual_j)) / 2;
            }
        }
        // update active num node
        // printf("current num all node: %d\n", num_all_nodes);
        *(active_node_map + qindexi) = num_all_nodes;
        num_all_nodes++;
        *(active_node_map + qindexj) = *(active_node_map + (num_active_nodes - 1));

        num_active_nodes--;
        // printf("Current active node num: %d\n", num_active_nodes);
    }
    // update the last 2 node

    current_q = 0x7FEFFFFFFFFFFFFF;
    for (int i = 0; i < num_active_nodes; i++)
    {
        double *ptr_dm = (double *)(distances + i * MAX_NODES);
        for (int j = 0; j < num_active_nodes; j++)
        {
            actual_i = *(active_node_map + i);
            actual_j = *(active_node_map + j);

            if (i != j)
            {
                // printf("Q(%s%s): %d * %lf - %lf - %lf = ", *(node_names + actual_i), *(node_names + actual_j), (num_active_nodes - 2), *(ptr + actual_i * MAX_NODES + actual_j), *(row_sum_ptr + actual_i), *(row_sum_ptr + actual_j));
                tmp = (num_active_nodes - 2) * *(ptr + actual_i * MAX_NODES + actual_j) - *(row_sum_ptr + actual_i) - *(row_sum_ptr + actual_j);
                // printf("%lf\n", (num_active_nodes - 2) * *(ptr + actual_i * MAX_NODES + actual_j) - *(row_sum_ptr + actual_i) - *(row_sum_ptr + actual_j));
                if (tmp < current_q)
                {
                    current_q = tmp;
                    qindexi = i;
                    qindexj = j;
                }
            }
        }
    }
    actual_i = *(active_node_map + qindexi);
    actual_j = *(active_node_map + qindexj);
    // printf("%s %s \n", *(node_names + actual_i), *(node_names + actual_j));
    int k = actual_j;
    int u = actual_i;
    fprintf(out, "%d,%d,%.2f\n", k, u, *(ptr + u * MAX_NODES + k));

    return 0;
}
