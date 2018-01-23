#ifndef EASY_RED_BLACK
#define EASY_RED_BLACK

#include <stdio.h>
#include <math.h>
#include <stdlib.h> 
#include <string.h>

///////////////////////////////////////// public structs ////////////////////////////////////////////////////////////////////////

/**
 * Color type for the red-black nodes
 */
typedef enum RBCOLOR{red,black}RBcolor;

/**
 * Default types for the data represented by the tree
 */
typedef enum RBDATA{INT,DOUBLE}RBdata;

/**
 * Red-Black-Node type
 * 
 *see  https://en.wikipedia.org/wiki/Red%E2%80%93black_tree
 */
typedef struct RBNODE{
	struct RBNODE* parent; 	/**< pointer to parent node, if node is root then it contains NULL */
	struct RBNODE* left;	/**< pointer to left child */
	struct RBNODE* right;	/**< pointer to right child */
	RBcolor color;			/**< color of the node (either red or black) */
	void* data;				/**< pointer to the data represented by the node */
}rbNode;

/**
 * Iterator type
 * 
 * Create an iterator to easily access consecutive elements of the tree 
 */
typedef struct RBTITERATOR{
	rbNode* root;			/**< pointer to the root of the tree the iterator refers to */
	rbNode* position;		/**< pointer to the actual position of the iterator within the tree */
}rbIterator;

//////////////////////////////////////// public functions ///////////////////////////////////////////////////////////////////////

void RBTree_set_compare(int (*Compare)(void* X1,void* X2));
void RBTree_set_free(void (*Free_data)(void* X));
void RBTree_set_format(void (*Format_data)(char* string,void* data));
void RBTree_set_data_size(size_t data_size);
void RBTsetDefaultData(RBdata type);

rbIterator RBTinitIterator(rbNode* root);

rbNode* RBTminNode(rbNode* root);
rbNode* RBTmaxNode(rbNode* root);
rbNode* RBTpredecessor(rbNode* node);
rbNode* RBTsuccessor(rbNode* node);
rbNode* RBTinsertElement(rbNode** root,void* data);
rbNode* RBTcopy(rbNode* root);
rbNode* RBTnearestSmallerOrEqualNode(rbNode* node,void* data);
rbNode* RBTnearestLargerOrEqualNode(rbNode* node,void* data);
rbNode* RBTgetNode(rbNode* root,void* data);

void RBTdeleteNode(rbNode** root,rbNode* node);
void RBTdeleteElement(rbNode** root,void* data);
void* RBTmin(rbNode* root);
void* RBTmax(rbNode* root);
void* RBTnearestLarger(rbNode* node,void* data);
void* RBTnearestSmaller(rbNode* node,void* data);
void* RBTiteraterNext(rbIterator* iterator);
void* RBTiteraterPrevious(rbIterator* iterator);

char* format_helper(void* data);

int RBTcontains(rbNode* root,void* data);
int RBTnodeCount(rbNode* root);
int RBTcheck(rbNode* root,int quiet);

void RBTfree(rbNode* root);
void RBTtoArray(rbNode* root,void** Array,int* size);
void RBTtoIntArray(rbNode* root,int** Array,int* size);
void RBTorderedPrint(rbNode* root,FILE* file);
void RBTtoIncArray(rbNode* root,void** Array,int* size);
void int_tree(int n);

#endif
