
/*************************** Start of AHUFF.C *************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "bitio.h"
#include "errhand.h"
    char *CompressionName = "Adaptive Huffman coding, with escape codes";
    char *Usage = "infile outfile";
#define END_OF_STREAM 256
#define ESCAPE 257
#define SYMBOL_COUNT 258
#define NODE_TABLE_COUNT ( ( SYMBOL_COUNT * 2 ) - 1 )
#define ROOT_NODE 0
#define MAX_WEIGHT 0X8000
#define TRUE 1
#define FALSE 0
/*
* This data structure is all that is needed to maintain an adaptive
* Huffman tree for both encoding and decoding. The leaf array is a
* set of indices into the nodes that indicate which node is the
* parent of a symbol. For example, to encode 'A', we would find the
* leaf node by way of leaf[ 'A' ]. The next_free_node index is used
* to tell which node is the next one in the array that can be used.
* Since nodes are allocated when characters are read in for the first
* time, this pointer keeps track of where we are in the node array.
* Finally, the array of nodes is the actual Huffman tree. The child
* index is either an index pointing to a pair of children, or an
* actual symbol value, depending on whether 'child_is_leaf' is true
* or false.
*/
typedef struct tree {
    int leaf[ SYMBOL_COUNT ];
    int next_free_node;
    struct node {
    unsigned int weight;
    int parent;
    int child_is_leaf;
    int child;
    } nodes [ NODE_TABLE_COUNT ];
} TREE;
/*
* The Tree used in this program is a global structure. Under other
* circumstances it could just as well be a dynamically allocated
* structure built when needed, since all routines here take a TREE
* pointer as an argument.
*/
TREE Tree;

/*
* Function prototypes for both ANSI C compilers and their K&R breth-
* ren.
*/
#ifdef __STDC__
    void CompressFile( FILE *input, BIT_FILE *output, int argc, char *argv[] );
    void ExpandFile( BIT_FILE *input, FILE *output, int argc, char *argv[] );
    void InitializeTree( TREE *tree );
    void EncodeSymbol( TREE *tree, unsigned int c, BIT_FILE *output );
    int DecodeSymbol( TREE *tree, BIT_FILE *input );
    void UpdateModel( TREE *tree, int c );
    void RebuildTree( TREE *tree );
    void swap_nodes( TREE *tree, int i, int j );
    void add_new_node( TREE *tree, int c );
    void PrintTree( TREE *tree );
    void print_codes( TREE *tree );
    void print_code( TREE *tree, int c );
    void calculate_rows( TREE *tree, int node, int level );
    int calculate_columns( TREE *tree, int node, int starting_guess );
    int find_minimum_column( TREE *tree, int node, int max_row );
    void rescale_columns( int factor );
#else
    void CompressFile();
    void ExpandFile();
    void InitializeTree();
    void EncodeSymbol();
    int DecodeSymbol();
    void UpdateModel();
    void RebuildTree();
    void swap_nodes();
    void add_new_node();
#endif
    /*
    * The high level view of the compression routine is very simple.
    * First, we initialize the Huffman tree, with just the ESCAPE and
    * END_OF_STREAM symbols. Then, we sit in a loop, encoding symbols,
    * and adding them to the model. When there are no more characters
    * to send, the special END_OF_STREAM symbol is encoded. The decoder
    * will later be able to use this symbol to know when to quit.
    */
void CompressFile(FILE *input,BIT_FILE *output,int argc,char *argv[])
{
    int c;
    InitializeTree( &Tree );
    while ( ( c = getc( input ) ) != EOF ) {
        EncodeSymbol( &Tree, c, output );
        UpdateModel( &Tree, c );
    }
    EncodeSymbol( &Tree, END_OF_STREAM, output );
    while ( argc-- > 0 ) {
        printf( "Unused argument: %s\n", *argv ); argv++;
    }
}
/*
* The Expansion routine looks very much like the compression routine.
* It first initializes the Huffman tree, using the same routine as
* the compressor did. It then sits in a loop, decoding characters and
* updating the model until it reads in an END_OF_STREAM symbol. At
* that point, it is time to quit.
*/
void ExpandFile(BIT_FILE *input,FILE *output,int argc,char *argv[])
{
    int c;
    while ( argc > 0 )
    {
        printf( "Unused argument: %s\n", *argv++ );
        argc --;
    }

    InitializeTree( &Tree );
    while ( ( c = DecodeSymbol( &Tree, input ) ) != EOF) {
        if ( putc( c, output ) == EOF )
            fatal_error( "Error writing character" );
        UpdateModel( & Tree, c );
    }
}
/*
* When performing adaptive compression, the Huffman tree starts out
* very nearly empty. The only two symbols present initially are the
* ESCAPE symbol and the END_OF_STREAM symbol. The ESCAPE symbol has to
* be included so we can tell the expansion program that we are
* transmitting a previously unseen symbol. The END_OF_STREAM symbol
* is here because it is greater than eight bits, and our ESCAPE
* sequence only allows for eight bit symbols following the ESCAPE
* code.
*
* In addition to setting up the root node and its two children, this
* routine also initializes the leaf array. The ESCAPE and
* END_OF_STREAM leaves are the only ones initially defined, the rest
* of the leaf elements are set to -1 to show that they aren't present
* in the Huffman tree yet.
*/
void InitializeTree(TREE *tree)
{
    int i;
    tree->nodes[ ROOT_NODE ].child = ROOT_NODE + 1;
    tree->nodes[ ROOT_NODE ].child_is_leaf = FALSE;
    tree->nodes[ ROOT_NODE ].weight = 2;
    tree->nodes[ ROOT_NODE ].parent = -1;
    tree->nodes[ ROOT_NODE + 1 ].child = END_OF_STREAM;
    tree->nodes[ ROOT_NODE + 1 ].child_is_leaf = TRUE;
    tree->nodes[ ROOT_NODE + 1 ].weight = 1;
    tree->nodes[ ROOT_NODE + 1 ].parent = ROOT_NODE;
    tree->leaf[ END_OF_STREAM ] = ROOT_NODE + 1;
    tree->nodes[ ROOT_NODE + 2 ].child = ESCAPE;
    tree->nodes[ ROOT_NODE + 2 ].child_is_leaf = TRUE;
    tree->nodes[ ROOT_NODE + 2 ].weight = 1;
    tree->nodes[ ROOT_NODE + 2 ].parent = ROOT_NODE;
    tree->leaf[ ESCAPE ] = ROOT_NODE + 2;
    tree->next_free_node = ROOT_NODE + 3;
    for ( i = 0 ; i < END_OF_STREAM ; i++ )
        tree->leaf[ i ] = -1; }
/*
* This routine is responsible for taking a symbol, and converting
* it into the sequence of bits dictated by the Huffman tree. The
* only complication is that we are working our way up from the leaf
* to the root, and hence are getting the bits in reverse order. This
* means we have to rack up the bits in an integer and then send them
* out after they are all accumulated. In this version of the program,
* we keep our codes in a long integer, so the maximum count is set
* to an arbitrary limit of 0x8000. It could be set as high as 65535
* if desired.
*/
void EncodeSymbol(TREE *tree,unsigned int c, BIT_FILE *output)
{
    unsigned long code;
    unsigned long current_bit;
    int code_size;
    int current_node;
    code = 0;
    current_bit = 1;
    code_size = 0;
    current_node = tree->leaf[ c ];
    if ( current_node == -1 )
        current_node = tree->leaf[ ESCAPE ];
    while ( current_node != ROOT_NODE ) {
        if ( ( current_node & 1 ) == 0 )
            code = current_bit;
            //code | = current_bit;
        current_bit <<= 1;
        code_size++;
        current_node = tree->nodes[ current_node ].parent;
    }
    OutputBits( output, code, code_size );
    if ( tree->leaf[ c ] == -1 ) {
        OutputBits( output, (unsigned long) c, 8 );
        add_new_node( tree, c );
    }
}
/*
* Decoding symbols is easy. We start at the root node, then go down
* the tree until we reach a leaf. At each node, we decide which
* child to take based on the next input bit. After getting to the
* leaf, we check to see if we read in the ESCAPE code. If we did,
* it means that the next symbol is going to come through in the next
* eight bits, unencoded. If that is the case, we read it in here,
* and add the new symbol to the table.
*/
int DecodeSymbol(TREE *tree, BIT_FILE *input)
{
    int current_node;
    int c;
    current_node = ROOT_NODE;
    while ( !tree->nodes[ current_node ].child_is_leaf ) {
        current_node = tree->nodes[ current_node ].child;
        current_node += InputBit( input );
    }
    c = tree->nodes[ current_node ].child;
    if ( c == ESCAPE ) {
        c = (int) InputBits( input, 8 );
        add_new_node( tree, c );
    }
    return( c );
}
/*
* UpdateModel is called to increment the count for a given symbol.
* After incrementing the symbol, this code has to work its way up
* through the parent nodes, incrementing each one of them. That is
* the easy part. The hard part is that after incrementing each
* parent node, we have to check to see if it is now out of the proper
* order. If it is, it has to be moved up the tree into its proper
* place.
*/
void UpdateModel(TREE *tree, int c)
{
    int current_node;
    int new_node;
    if ( tree->nodes[ ROOT_NODE].weight == MAX_WEIGHT )
        RebuildTree( tree );
    current_node = tree->leaf[ c ];
    while ( current_node != -1 ) {
        tree->nodes[ current_node ].weight++;
        for ( new_node = current_node ; new_node > ROOT_NODE ;new_node-- )
        {
            if ( tree->nodes[ new_node - 1 ].weight >=tree->nodes[ current_node ].weight )
                break;
        }
        if ( current_node != new_node )
        {
            swap_nodes( tree, current_node, new_node );
            current_node = new_node;
        }
        current_node = tree->nodes[ current_node ].parent;
    }
}
/*
* Rebuilding the tree takes place when the counts have gone too
* high. From a simple point of view, rebuilding the tree just means
* that we divide every count by two. Unfortunately, due to truncation
* effects, this means that the tree's shape might change. Some nodes
* might move up due to cumulative increases, while others may move
* down.
*/
void RebuildTree(TREE *tree)
{
    int i;
    int j;
    int k;
    unsigned int weight;
    /*
    * To start rebuilding the table, I collect all the leaves of the
    * Huffman tree and put them in the end of the tree. While I am doing
    * that, I scale the counts down by a factor of 2.
    */
    printf( "R" );
    j = tree->next_free_node - 1;
    for ( i = j ; i >= ROOT_NODE ; i-- ) {
        if ( tree->nodes[ i ].child_is_leaf ) {
            tree->nodes[ j ] = tree->nodes[ i ];
            tree->nodes[ j ].weight = ( tree->nodes[ j ].weight + 1 ) / 2;
            j--;
        }
    }
/*
* At this point, j points to the first free node. I now have all the
* leaves defined, and need to start building the higher nodes on the
* tree. I will start adding the new internal nodes at j. Every time
* I add a new internal node to the top of the tree, I have to check
* to see where it really belongs in the tree. It might stay at the
* top, but there is a good chance I might have to move it back down.
* If it does have to go down, I use the memmove() function to scoot
* everyone bigger up by one node.
*/
    for ( i = tree->next_free_node - 2 ; j >= ROOT_NODE;i -= 2, j-- ) {
        k = i + 1;
        tree->nodes[ j ].weight = tree->nodes[ i ].weight + tree->nodes[ k ].weight;
        weight = tree->nodes[ j ].weight;
        tree->nodes[ j ].child_is_leaf = FALSE;
        for ( k = j + 1 ; weight < tree->nodes[ k ].weight ; k++ );
        k--;
        memmove( &tree->nodes[ j ], &tree->nodes[ j + 1 ],( k - j ) * sizeof( struct node ) );
        tree->nodes[ k ].weight = weight;
        tree->nodes[ k ].child = i;
        tree->nodes[ k ].child_is_leaf = FALSE;
    }
/*
* The final step in tree reconstruction is to go through and set up
* all of the leaf and parent members. This can be safely done now
* that every node is in its final position in the tree.
*/
    for ( i = tree->next_free_node - 1 ; i >= ROOT_NODE ; i-- ) {
        if ( tree->nodes[ i ].child_is_leaf ) {
            k = tree->nodes[ i ].child;
            tree->leaf[ k ] = i;
        } else {
            k = tree->nodes[ i ].child;
            tree->nodes[ k ].parent = tree->nodes[ k + 1 ].parent = i;
        }
    }
}
/*
* Swapping nodes takes place when a node has grown too big for its
* spot in the tree. When swapping nodes i and j, we rearrange the
* tree by exchanging the children under i with the children under j*/
void swap_nodes(TREE *tree,int i,int j)
{
    struct node temp;
    if ( tree->nodes [ i ].child_is_leaf )
        tree->leaf[ tree->nodes[ i ].child ] = j;
    else {
        tree->nodes[ tree->nodes[ i ].child ].parent = j;
        tree->nodes[ tree->nodes[ i ].child + 1 ].parent = j;
    }
    if ( tree->nodes[ j ].child_is_leaf )
        tree->leaf[ tree->nodes[ j ].child ] = i;
    else {
        tree->nodes[ tree->nodes[ j ].child ].parent = i;
        tree->nodes[ tree->nodes[ j ].child + 1 ].parent = i;
    }
    temp = tree->nodes[ i ];
    tree->nodes[ i ] = tree->nodes[ j ];
    tree->nodes[ i ].parent = temp.parent;
    temp.parent = tree->nodes[ j ].parent;
    tree->nodes[ j ] = temp;
}
/*
* Adding a new node to the tree is pretty simple. It is just a matter
* of splitting the lightest-weight node in the tree, which is the
* highest valued node. We split it off into two new nodes, one of
* which is the one being added to the tree. We assign the new node a
* weight of 0, so the tree doesn't have to be adjusted. It will be
* updated later when the normal update process occurs. Note that this
* code assumes that the lightest node has a leaf as a child. If this
* is not the case, the tree would be broken.
*/
void add_new_node(TREE *tree, int c )
{
    int lightest_node;
    int new_node;
    int zero_weight_node;
    lightest_node = tree->next_free_node - 1;
    new_node = tree->next_free_node;
    zero_weight_node = tree->next_free_node + 1;
    tree->next_free_node += 2;
    tree->nodes[ new_node ] = tree->nodes[ lightest_node ];
    tree->nodes[ new_node ].parent = lightest_node;
    tree->leaf[ tree->nodes[ new_node ].child ] = new_node;
    tree->nodes[ lightest_node ].child = new_node;
    tree->nodes[ lightest_node ].child_is_leaf = FALSE;
    tree->nodes[ zero_weight_node ].child = c;
    tree->nodes[ zero_weight_node ].child_is_leaf = TRUE;
    tree->nodes[ zero_weight_node ].weight = 0;
    tree->nodes[ zero_weight_node ].parent = lightest_node;
}
/************************** End of AHUFF.C *****************************/
