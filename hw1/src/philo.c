#include <stdlib.h>

#include "global.h"
#include "debug.h"
size_t getLen(char *s)
{
    long i = 0;
    size_t length = 0;
    while (*s != 0)
    {
        length += 1;
        s++;
    }
    return length;
}
int reverseNumber(int num)
{
    int reversed = 0;

    while (num != 0)
    {
        int digit = num % 10;
        reversed = reversed * 10 + digit;
        num /= 10;
    }

    return reversed;
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
    long length = getLen(s2);
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
/**
 * @brief  Converts string to float
 * @details  This function reads the given string
 * and convert the string to float return -1.0 if not convertable
 */
double string_to_float(char *s)
{
    char *ptr = s;
    double f = 0;
    double d = 1;
    double num = 0;
    double floating = 0;
    int is_floating = 0;

    while (*ptr != 0)
    {
        if (is_floating == 0)
        {
            if (*ptr >= '0' && *ptr <= '9')
            {
                num = num * 10 + (*ptr - '0');
                ptr++;
            }
            else if (*ptr == '.')
            {
                is_floating = 1;
                ptr++;
                continue;
            }
            else
            {

                return -1.0;
            }
        }
        else
        {
            if (*ptr >= '0' && *ptr <= '9')
            {
                floating = floating * 10 + (*ptr - '0');
                d *= 10;
                ptr++;
            }
            else
            {

                return -1.0;
            }
        }
    }

    return (num + (floating / d));
}

/**
 * @brief  Update the sumrow
 * @details  This function loop over distances by row and get the
 * sum to that row and put it in row_sums
 */
void update_row_sum()
{
    double ans = 0;
    double *row_sum_ptr = row_sums;
    double *distance_ptr = (double *)distances;
    for (long i = 0; i < num_active_nodes; i++)
    {
        long actual_i = *(active_node_map + i);
        ans = 0;
        for (long j = 0; j < num_active_nodes; j++)
        {
            long actual_j = *(active_node_map + j);
            ans += *(distance_ptr + actual_i * MAX_NODES + actual_j);
        }
        *(row_sum_ptr + actual_i) = ans;
    }
}
/**
 * @brief  find the index of node_names
 * @details  This function reads the given name and find the
 * name index and return the index, return -1 if not found
 */
long find_name_index(char *name)
{
    for (long i = 0; i < num_all_nodes; i++)
    {
        if (equals(*(node_names + i), name))
        {
            return i;
        }
    }
    return -1;
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
        if (c == '#')
        {
            skip = 1;
            while ((c != '\n' && c != EOF) || c == 0)
            {
                c = fgetc(in);
            }
        }
        if (skip == 1)
        {
            skip = 0;
            continue;
        }
        if (c == EOF)
        {

            break;
        }

        if (count_line == 0)
        {
            if (array_index > INPUT_MAX || input_buffer_index > INPUT_MAX + 1)
            {
                fprintf(stderr, "Stack overflow\n");
                return -1;
            }

            if (c == ',')
            {
                if (num_taxa != 0)
                {
                    *input = 0;
                    str_copy(*(node_names + array_index), input_buffer, getLen(input_buffer));
                    array_index++;
                    input = input_buffer;
                    input_buffer_index = 0;
                }
                num_taxa += 1;
            }
            if (c != ',' && c != '\n' && num_taxa != 0)
            {

                *input = c;
                input_buffer_index += 1;
                input += 1;
            }
            if (c == '\n')
            {
                if (num_taxa != 0)
                {
                    *input = 0;
                    str_copy(*(node_names + array_index), input_buffer, getLen(input_buffer));
                    array_index++;
                    input = input_buffer;
                    input_buffer_index = 0;
                }
            }
        }

        if (count_line >= 1)
        {
            if (prev == '\n' && c == '\n')
            {
                if (count_line < num_taxa)
                {
                    fprintf(stderr, "There is empty line or there are missing data\n");
                }

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
                if (string_to_float(input_buffer) < 0)
                {
                    fprintf(stderr, "Invalid distance input\n");
                    return -1;
                }
                *current_float = string_to_float(input_buffer);
                current_float += 1;
                if (c == '\n')
                {
                    current_row += 1;
                    current_float = (double *)(distances + current_row);
                }
            }
        }
        if (c == '\n')
        {
            count_line += 1;
            if (count_line - 1 == num_taxa)
                break;
        }
        prev = c;
    }
    current_float = (double *)distances;

    num_active_nodes = num_taxa;
    num_all_nodes = num_taxa;
    if (count_line - 1 < num_taxa)
    {

        fprintf(stderr, "The distance matrix is not a square matrix\n");
        return -1;
    }
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
    for (int i = 0; i < num_active_nodes; i++)
    {
        *(active_node_map + i) = i;
        (nodes + i)->name = *(node_names + i);
        (*((nodes + i)->neighbors + 0)) = 0;
        (*((nodes + i)->neighbors + 1)) = 0;
        (*((nodes + i)->neighbors + 2)) = 0;
    }

    return 0;
}
long get_size(NODE *nei)
{
    long count = 0;
    for (int i = 0; i < 3; i++)
    {
        if ((nei + i) != NULL)
        {
            count += 1;
        }
    }
    return count;
}

void print_newick(NODE *root, NODE *prev, FILE *out)
{
    double *distance_ptr = (double *)distances;

    if ((*(root->neighbors)) == NULL && (*(root->neighbors + 2)) == NULL)
    {
        long indexi = find_name_index(root->name);
        long indexj = find_name_index(prev->name);
        fprintf(out, "%s:%.2f", root->name, *(distance_ptr + indexi * MAX_NODES + indexj));
        return;
    }
    fprintf(out, "(");
    int num_printed = 0;
    if (prev != *(root->neighbors + 1))
    {
        if (num_printed > 0)
        {
            fprintf(out, ",");
        }
        print_newick(*(root->neighbors + 1), root, out);
        num_printed++;
    }
    if (prev != *(root->neighbors + 0))
    {
        if (num_printed > 0)
        {
            fprintf(out, ",");
        }
        print_newick(*(root->neighbors + 0), root, out);
        num_printed++;
    }

    if (prev != *(root->neighbors + 2))
    {
        if (num_printed > 0)
        {
            fprintf(out, ",");
        }
        print_newick(*(root->neighbors + 2), root, out);
        num_printed++;
    }
    fprintf(out, ")");
    long indexi = find_name_index(root->name);
    long indexj = find_name_index(prev->name);
    fprintf(out, "%s:%.2f", root->name, *(distance_ptr + indexi * MAX_NODES + indexj));
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
    // find the outliner

    double current_max = -1 * 0x7FEFFFFFFFFFFFFF;
    double *distances_ptr = (double *)distances;
    long out_index;
    if (outlier_name == NULL)
    {
        for (long i = 0; i < num_all_nodes; i++)
        {
            for (long j = 0; j < num_all_nodes; j++)
            {
                if (current_max < *(distances_ptr + i * MAX_NODES + j))
                {
                    current_max = *(distances_ptr + i * MAX_NODES + j);
                    outlier_name = *(node_names + i);
                }
            }
        }
    }
    out_index = find_name_index(outlier_name);
    if (out_index == -1)
    {
        fprintf(stderr, "Ouliner not found\n");
        return -1;
    }
    print_newick(*((nodes + out_index)->neighbors + 1), (nodes + out_index), out);
    fprintf(out, "\n");
    return 0;
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
    double *distance_ptr = (double *)distances;
    // TO BE IMPLEMENTED
    for (int i = 0; i < num_all_nodes; i++)
    {
        fprintf(out, ",%s", *(node_names + i));
    }
    fprintf(out, "\n");
    for (int i = 0; i < num_all_nodes; i++)
    {
        fprintf(out, "%s", *(node_names + i));
        for (int j = 0; j < num_all_nodes; j++)
        {
            fprintf(out, ",%.2f", *(distance_ptr + i * MAX_NODES + j));
        }
        fprintf(out, "\n");
    }
    return 0;
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
    int buffer_index = 0;
    double current_q = 0x7FEFFFFFFFFFFFFF;
    double tmp = 0;
    int qindexi = 0;
    int qindexj = 0;
    double *row_sum_ptr = row_sums;
    double *ptr = (double *)(distances);
    int actual_i = 0;
    int actual_j = 0;
    while (num_active_nodes != 2)
    {
        current_q = 0x7FEFFFFFFFFFFFFF;
        current_q = 0;

        update_row_sum();
        row_sum_ptr = row_sums;
        actual_i = 0;
        actual_j = 0;
        qindexi = 0;
        qindexj = 0;

        for (int i = 0; i < num_active_nodes; i++)
        {
            // double *ptr_dm = (double *)(distances + i * MAX_NODES);
            for (int j = 0; j < num_active_nodes; j++)
            {
                actual_i = *(active_node_map + i);
                actual_j = *(active_node_map + j);

                if (i != j)
                {

                    tmp = (num_active_nodes - 2) * *(ptr + actual_i * MAX_NODES + actual_j) - *(row_sum_ptr + actual_i) - *(row_sum_ptr + actual_j);
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
        char *name_ptr = (char *)node_names;
        NODE *new_node = (nodes + num_all_nodes);
        input = input_buffer;
        buffer_index = 0;
        *input = '#';
        input++;
        buffer_index += 1;
        int tmp_all_node = reverseNumber(num_all_nodes);
        while (tmp_all_node != 0)
        {
            if (buffer_index > (INPUT_MAX + 1))
            {
                fprintf(stderr, "Buffer overflow:%s\n", input_buffer);
                return -1;
            }
            *input = ((tmp_all_node % 10) + '0');
            tmp_all_node = tmp_all_node / 10;
            input++;
            buffer_index += 1;
        }
        *input = '\0';
        input = input_buffer;

        str_copy(*(node_names + num_all_nodes), input_buffer, getLen((input_buffer)));
        new_node->name = *(node_names + num_all_nodes);
        (*((nodes + num_all_nodes)->neighbors + 0)) = (nodes + actual_i);
        (*((nodes + num_all_nodes)->neighbors + 1)) = 0;
        (*((nodes + num_all_nodes)->neighbors + 2)) = (nodes + actual_j);

        (*((nodes + actual_i)->neighbors + 1)) = (nodes + num_all_nodes);
        (*((nodes + actual_j)->neighbors + 1)) = (nodes + num_all_nodes);
        ptr = (double *)(distances);
        for (int i = 0; i < num_active_nodes; i++)
        {
            int u = num_all_nodes;
            int k = *(active_node_map + i);
            if (u == k)
            {
                *(ptr + u * MAX_NODES + k) = *(ptr + k * MAX_NODES + u) = 0;
            }
            else if (k == actual_i)
            {

                *(ptr + u * MAX_NODES + k) = *(ptr + k * MAX_NODES + u) = (*(ptr + actual_i * MAX_NODES + actual_j) + (*(row_sum_ptr + actual_i) - *(row_sum_ptr + actual_j)) / (num_active_nodes - 2)) / 2;
                if (global_options == 0)
                {
                    fprintf(out, "%d,%d,%.2f\n", k, u, *(ptr + u * MAX_NODES + k));
                }
            }
            else if (k == actual_j)
            {
                *(ptr + u * MAX_NODES + k) = *(ptr + k * MAX_NODES + u) = (*(ptr + actual_i * MAX_NODES + actual_j) + (*(row_sum_ptr + actual_j) - *(row_sum_ptr + actual_i)) / (num_active_nodes - 2)) / 2;
                if (global_options == 0)
                {
                    fprintf(out, "%d,%d,%.2f\n", k, u, *(ptr + u * MAX_NODES + k));
                }
            }
            else
            {
                *(ptr + u * MAX_NODES + k) = *(ptr + k * MAX_NODES + u) = (*(ptr + actual_i * MAX_NODES + k) + *(ptr + actual_j * MAX_NODES + k) - *(ptr + actual_i * MAX_NODES + actual_j)) / 2;
            }
        }
        // update active num node
        *(active_node_map + qindexi) = num_all_nodes;
        num_all_nodes++;
        if (num_all_nodes == MAX_NODES)
        {
            fprintf(stderr, "Nodes is full\n");
            return -1;
        }
        *(active_node_map + qindexj) = *(active_node_map + (num_active_nodes - 1));

        num_active_nodes--;
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

                tmp = (num_active_nodes - 2) * *(ptr + actual_i * MAX_NODES + actual_j) - *(row_sum_ptr + actual_i) - *(row_sum_ptr + actual_j);
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

    (*((nodes + actual_i)->neighbors + 1)) = (nodes + actual_j);
    (*((nodes + actual_j)->neighbors + 1)) = (nodes + actual_i);
    int k = actual_j;
    int u = actual_i;
    if (global_options == 0)
    {
        fprintf(out, "%d,%d,%.2f\n", k, u, *(ptr + u * MAX_NODES + k));
    }

    return 0;
}
