#include "redblack.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////* local structs *//////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


typedef enum RELATION{LEFTLEFT,LEFTRIGHT,RIGHTLEFT,RIGHTRIGHT}relation;
//typedef enum DELMODE{DELETE,FIX}del_mode;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////* local (global) variables *//////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


static int (*compare)(void* X1,void* X2) = NULL;
static void (*free_data)(void* X) = NULL;
static void (*format_data)(char* S,void* X) = NULL;
static size_t data_size_in_bytes = 0;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////* private functions */////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static char* RBTformat(rbNode* node){
	const int BUFFSIZE = 64;
	if (format_data==NULL){
		printf("Error in function %s: no format function given -> abort\n",__func__);
		exit(EXIT_FAILURE);
	}
	if (node!=NULL){
		char* Buffer = (char*)malloc(BUFFSIZE*sizeof(char));
		char* f = format_helper(node->data);
		if (node->color==black) sprintf(Buffer,"(%s,black)",f); else sprintf(Buffer,"(%s,red)",f);
		if (f!=NULL) free(f);
		int size = strlen(Buffer);
		Buffer = (char*)realloc(Buffer,(size+1)*sizeof(char));
		return Buffer;
	}
	else return NULL;
}

/*static void RBTsubTreePrint(rbNode* node,char** Canvas,int X,int Y,int depth,int sizeX,int sizeY){	
	if (node!=NULL && Y<sizeY){
		int i;
		int offset = (int)round((double)sizeX*exp(-depth*log(2.))/4.)+1;
		char* label = RBTformat(node);
		int l = strlen(label);
		for (i=X-l/2;i<X+l-l/2;i++) if (i>=0 && i<sizeX) Canvas[Y][i] = label[i-X+l/2];
		free(label);
		RBTsubTreePrint(node->left,Canvas,X-l/2-offset,Y+2,depth+1,sizeX,sizeY);
		RBTsubTreePrint(node->right,Canvas,X+l/2+offset,Y+2,depth+1,sizeX,sizeY);
	}
	if(node==NULL) printf("empty tree: nothing to print\n");
}*/

static int rbtOK(rbNode* node,int blacks,int** blacklist,int* size){
	
	if (node==NULL || node->color==black) blacks++;
	
	if (node==NULL){
		(*size)++;
		*blacklist = (int*)realloc(*blacklist,(*size)*sizeof(int));
		(*blacklist)[(*size)-1] = blacks;
		return 1;
	}
	else{
		if (node->color==red){
			if (node->left!=NULL && node->left->color==red) goto REDREDERROR;
			if (node->right!=NULL && node->right->color==red) goto REDREDERROR;
		}
		return (rbtOK(node->left,blacks,blacklist,size) | rbtOK(node->right,blacks,blacklist,size));
	}
	
REDREDERROR:
	printf("Adjacent Reds at node %s (parent: %s)\n",RBTformat(node),RBTformat(node->parent));
	return 0;
}

static rbNode* RBTcreateNode(rbNode* parent,void* data){
	rbNode* node = (rbNode*)malloc(sizeof(rbNode));
	node->parent = parent;
	node->left = NULL;
	node->right = NULL;
	if (parent==NULL) node->color = black; else node->color = red;
	node->data = data;
	return node;
}

void RBTfree(rbNode* root){
	if (root!=NULL){
		if (free_data!=NULL && root->data!=NULL) (*free_data)(root->data);
		RBTfree(root->left);
		RBTfree(root->right);
		free(root);
	}
}

rbNode* RBTgetNode(rbNode* root,void* data){
	if (root!=NULL){
		int cmp = (*compare)(data,root->data);
		if (cmp==0) return root;
		else if (cmp<0) return RBTgetNode(root->left,data);
		else return RBTgetNode(root->right,data);
	}
	else return NULL;
}

static rbNode* getUncle(rbNode* node){
	rbNode* P = node->parent;
	rbNode* G = P->parent;	
	if (G->left==P) return G->right;else return G->left;
}

static relation getRelationCase(rbNode* child,rbNode* parent,rbNode* grand){
	if (parent==grand->left){
		if (child==parent->left) return LEFTLEFT; else return LEFTRIGHT;
	}
	else{
		if (child==parent->right) return RIGHTRIGHT; else return RIGHTLEFT;
	}
}

static void RBTrotation(rbNode* child,rbNode* parent){
	rbNode* grand = parent->parent;
	if (grand!=NULL){			
		if (grand->left==parent) grand->left = child; else grand->right = child;
	}
	
	if (child==parent->left){				
		if (child->right!=NULL) child->right->parent = parent;			
		parent->left = child->right;
		child->right = parent;		
	}
	else{
		if (child->left!=NULL) child->left->parent = parent;
		parent->right = child->left;
		child->left = parent;
	}
	
	child->parent = grand;
	parent->parent = child;	
}

static void RBTrotationNoColor(rbNode* child,rbNode* parent){
	RBTrotation(child,parent);
	RBcolor c = child->color;
	child->color = parent->color;
	parent->color = c;
}

static void RBfixtree(rbNode* node){
	rbNode* parent = node->parent;
	if (parent==NULL){
		node->color = black;
		return;
	}
	
	if (node->color==red && parent->color==red){
		rbNode* grand = parent->parent;		
		rbNode* uncle = getUncle(node);
		
		if (uncle!=NULL && uncle->color==red){
			parent->color = black;
			uncle->color = black;
			if (grand->parent!=NULL) grand->color = red;
			RBfixtree(grand);			
		}
		else{
			relation rel = getRelationCase(node,parent,grand);
			switch(rel){
				case LEFTLEFT:
					parent->color = black;
					grand->color = red;
					RBTrotation(parent,grand);
					break;
				case LEFTRIGHT:
					node->color = black;
					grand->color = red;
					RBTrotation(node,parent);					
					RBTrotation(node,grand);				
					break;
				case RIGHTLEFT:
					node->color = black;
					grand->color = red;
					RBTrotation(node,parent);					
					RBTrotation(node,grand);
					break;
				case RIGHTRIGHT:
					parent->color = black;
					grand->color = red;
					RBTrotation(parent,grand);
					break;
			}					
		}		
	}
}

static void RBTfindroot(rbNode** root){
	if (*root!=NULL){
		while((*root)->parent!=NULL){
			*root = (*root)->parent;
		}
	}
}

static rbNode* RBTinsert(rbNode** root,rbNode* injector,void* data){
	if (*root==NULL){
		*root = RBTcreateNode(NULL,data);
		return NULL;
	}	
	
	int cmp = (*compare)(data,injector->data);
	if (cmp!=0){
		rbNode* child = (cmp<0) ? injector->left : injector->right;
		if (child==NULL){	
			child = RBTcreateNode(injector,data);		
			if (cmp<0) injector->left = child; else injector->right = child;
			RBfixtree(child);
			RBTfindroot(root);
			return NULL;
		}
		else return RBTinsert(root,child,data);
	}
	else return injector;	
}

static rbNode* RBTupUntilLeftChild(rbNode* node){	
	if (node==NULL) return NULL;
	while(node->parent!=NULL && node!=node->parent->left){
		node = node->parent;
	}
	return node->parent;
}

static rbNode* RBTupUntilRightChild(rbNode* node){
	if (node==NULL) return NULL;
	while(node->parent!=NULL && node!=node->parent->right){
		node = node->parent;
	}
	return node->parent;
}

static rbNode* RBTnearestLargerNode(rbNode* node,void* data){
	if (node!=NULL){
		int cmp = (*compare)(data,node->data);
		if (cmp==0){
			if (node->right==NULL) return RBTupUntilLeftChild(node);
			else return RBTminNode(node->right);
		}
		else if (cmp>0){			
			if (node->right==NULL) return RBTupUntilLeftChild(node);
			else return RBTnearestLargerNode(node->right,data);
		}
		else{
			if (node->left==NULL) return node;
			else return RBTnearestLargerNode(node->left,data);
		}		
	}
	else return NULL;
}

rbNode* RBTnearestLargerOrEqualNode(rbNode* node,void* data){
	if (node!=NULL){
		int cmp = (*compare)(data,node->data);
		if (cmp==0) return node;
		else if (cmp>0){			
			if (node->right==NULL) return RBTupUntilLeftChild(node);
			else return RBTnearestLargerNode(node->right,data);
		}
		else{
			if (node->left==NULL) return node;
			else return RBTnearestLargerOrEqualNode(node->left,data);
		}		
	}
	else return NULL;
}

static rbNode* RBTnearestSmallerNode(rbNode* node,void* data){
	if (node!=NULL){
		int cmp = (*compare)(data,node->data);
		if (cmp==0){
			if (node->left==NULL) return RBTupUntilRightChild(node);
			else return RBTmaxNode(node->left);
		}
		else if (cmp>0){	
			if (node->right==NULL) return node;
			else return RBTnearestSmallerNode(node->right,data);								
		}
		else{
			if (node->left==NULL) return RBTupUntilRightChild(node);
			else return RBTnearestSmallerNode(node->left,data);
		}		
	}
	else return NULL;
}

rbNode* RBTnearestSmallerOrEqualNode(rbNode* node,void* data){
	if (node!=NULL){
		int cmp = (*compare)(data,node->data);
		if (cmp==0) return node;
		else if (cmp>0){	
			if (node->right==NULL) return node;
			else return RBTnearestSmallerNode(node->right,data);								
		}
		else{
			if (node->left==NULL) return RBTupUntilRightChild(node);
			else return RBTnearestSmallerOrEqualNode(node->left,data);
		}		
	}
	else return NULL;
}


static rbNode* RBTgetSibling(rbNode* node){
	if (node!=NULL){
		rbNode* parent = node->parent;
		if (parent!=NULL){
			if (parent->left==node) return parent->right; else return parent->left;
		}
		return NULL;		
	}
	else return NULL;
}

static RBcolor RBTgetColor(rbNode* node){
	if (node==NULL) return black;
	else return node->color;
}

static int RBThasRedChild(rbNode* node){
	if (node!=NULL){
		if (RBTgetColor(node->left)==red) return 1;
		if (RBTgetColor(node->right)==red) return 1;
		return 0;
	}
	else return 0;
}

static relation getSiblingCase(rbNode* sibling){
	if (sibling==sibling->parent->left){
		if (RBTgetColor(sibling->left)==red) return LEFTLEFT; else return LEFTRIGHT;
	}
	else{
		if (RBTgetColor(sibling->right)==red) return RIGHTRIGHT; else return RIGHTLEFT;
	}
}

static void RBTfixDelete(rbNode** root,rbNode* parent,rbNode* sibling){
	if(sibling->color==black){
		if (RBThasRedChild(sibling)){	// case a):  If sibling is black and at least one of sibling’s children is red												
			switch(getSiblingCase(sibling)){
				case LEFTLEFT:														
					sibling->left->color = black;
					RBTrotationNoColor(sibling,parent);					
					break;
				case LEFTRIGHT:						
					sibling->right->color = black;
					RBTrotationNoColor(sibling->right,sibling);
					RBTrotationNoColor(parent->left,parent);
					break;
				case RIGHTLEFT:						
					sibling->left->color = black;
					RBTrotationNoColor(sibling->left,sibling);
					RBTrotationNoColor(parent->right,parent);
					break;
				case RIGHTRIGHT:										
					sibling->right->color = black;
					RBTrotationNoColor(sibling,parent);													
					break;
			}
			RBTfindroot(root);
		}
		else{							// case b):   If sibling is black and its both children are black,
			sibling->color = red;
			if (parent->color==black){				
				rbNode* uncle = RBTgetSibling(parent);
				rbNode* grand = parent->parent;
				if (grand!=NULL) RBTfixDelete(root,grand,uncle);
			}
			else parent->color = black;		
			RBTfindroot(root);		
		}		
	}
	else{								// case c):   If sibling is red
		rbNode* grandchild = (sibling==parent->right) ? sibling->left : sibling->right;						
		RBTrotationNoColor(sibling,parent);		
		RBTfixDelete(root,parent,grandchild);		
	}	
}

static rbNode** BSTdelete(rbNode** root,rbNode* node){
	rbNode *parent,*child;
	
	if (node!=NULL){
		parent = node->parent;
		if (node->left!=NULL && node->right!=NULL){
			rbNode* successor = RBTsuccessor(node);
			if (successor==NULL){
				printf("Error in function %s: something is wrong with the tree -> abort\n",__func__);
				exit(EXIT_FAILURE);
			}
			if (node->data!=NULL) (*free_data)(node->data);
			node->data = successor->data;
			successor->data = NULL;
			return BSTdelete(root,successor);						
		}
		else{			
			rbNode** res;
			if (node->left!=NULL){		
				if (node->color==red || node->left->color==red){
					node->color = black;
					res = NULL;
				}
				else{
					res = (rbNode**)malloc(2*sizeof(rbNode*));
					res[0] = parent;
					res[1] = RBTgetSibling(node);
				}						
				child = node->left;			 
				if (node->data!=NULL) (*free_data)(node->data);
				node->data = child->data;
				child->data = NULL;
				node->left = child->left;
				if (child->left!=NULL) child->left->parent = node;			 
				node->right = child->right;
				if (child->right!=NULL) child->right->parent = node;	
				free(child);		 			 				
			}
			else if (node->right!=NULL){
				if (node->color==red || node->right->color==red){
					node->color = black;
					res = NULL;
				}
				else{
					res = (rbNode**)malloc(2*sizeof(rbNode*));
					res[0] = parent;
					res[1] = RBTgetSibling(node);
				}				 
				child = node->right;			
				if (node->data!=NULL) (*free_data)(node->data);
				node->data = child->data;
				child->data = NULL;
				node->left = child->left;
				if (child->left!=NULL) child->left->parent = node;			 
				node->right = child->right;
				if (child->right!=NULL) child->right->parent = node;			
				free(child);			
			}
			else{
				if (node->color==red) res = NULL;
				else{
					res = (rbNode**)malloc(2*sizeof(rbNode*));
					res[0] = parent;
					res[1] = RBTgetSibling(node);
				}	
				if (parent!=NULL){					
					if (node==parent->left) parent->left = NULL; else parent->right = NULL;				
				}				
				if (node->data!=NULL) (*free_data)(node->data);
				free(node);		
				if (parent==NULL) *root = NULL;		
			}
			RBTfindroot(root);
			return res;
		}		
	}
	else return NULL;
}

void RBTdeleteNode(rbNode** root,rbNode* node){
	if ((*root)!=NULL && node!=NULL){															
		rbNode** toFix = BSTdelete(root,node);									// ret1: parent, ret2: sibling
		if (toFix!=NULL){
			if (*root!=NULL){
				rbNode* parent = toFix[0];
				rbNode* sibling = toFix[1];
				if (parent==NULL){
					(*root)->color = black;
				}
				else if (sibling!=NULL) RBTfixDelete(root,parent,sibling);					
			}
			free(toFix);	
		}				
	}
}

static inline void inc_ptr(void** Iterator,const size_t element_memsize){
	const size_t CHARSIZE = sizeof(unsigned char);
	
	int d = element_memsize / CHARSIZE;
	unsigned char *C_Iter = (unsigned char*)*Iterator;
	C_Iter += d;
	*Iterator = C_Iter;
}

static void RBTtoArray_helper(rbNode* root,void** Iterator){
	if (root!=NULL){
		RBTtoArray_helper(root->left,Iterator);
		memcpy(*Iterator,root->data,data_size_in_bytes);		
		inc_ptr(Iterator,data_size_in_bytes);
		RBTtoArray_helper(root->right,Iterator);
	}
}

static void RBTtoIntArray_helper(rbNode* root,int** Iterator){
	if (root!=NULL){
		RBTtoIntArray_helper(root->left,Iterator);
		**Iterator = *(int*)root->data;
		(*Iterator)++;
		RBTtoIntArray_helper(root->right,Iterator);
	}
}

static rbNode* RBTcopy_helper(rbNode* node,rbNode* parent){
	if (node!=NULL){
		rbNode* newnode = (rbNode*)malloc(sizeof(rbNode));
		newnode->data = (void*)malloc(data_size_in_bytes);
		memcpy(newnode->data,node->data,data_size_in_bytes);
		newnode->parent = parent;
		newnode->color = node->color;
		newnode->left = RBTcopy_helper(node->left,newnode);
		newnode->right = RBTcopy_helper(node->right,newnode);
		return newnode;
	}
	else return NULL;
}

static int INT_compare(void* X1,void* X2){
	int i1 = *(int*)X1;
	int i2 = *(int*)X2;
	if (i1<i2) return -1;
	else if (i1>i2) return 1;
	else return 0;
}

static int DOUBLE_compare(void* X1,void* X2){
	const double tol = 1e-10;
	double d1 = *(double*)X1;
	double d2 = *(double*)X2;
	if (fabs(d1-d2)<tol) return 0;
	else if(d1<d2) return -1;
	else return 1;	
}

static void INT_free(void* X){
	int* I = (int*)X;
	free(I);
}

static void DOUBLE_free(void* X){
	double* D = (double*)X;
	free(D);
}

static void INT_format(char* S,void* data){
	sprintf(S,"%d",*(int*)data);	
}

static void DOUBLE_format(char* S,void* data){
	sprintf(S,"%f",*(double*)data);	
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////* public functions */////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

rbNode* RBTminNode(rbNode* root){
	if (root==NULL) return NULL;
	while(root->left!=NULL){
		root = root->left;
	}
	return root;
}

rbNode* RBTmaxNode(rbNode* root){
	if (root==NULL) return NULL;
	while(root->right!=NULL){
		root = root->right;	
	}
	return root;
}

char* format_helper(void* data){
	const int BUFFSIZE = 64;
	if (data!=NULL){		
		char* res = (char*)malloc(BUFFSIZE*sizeof(char));
		(*format_data)(res,data);
		int size = strlen(res);
		res = (char*)realloc(res,(size+1)*sizeof(char));
		return res;
	}
	else return NULL;
}

void RBTorderedPrint(rbNode* root,FILE* file){
	printf("\n");
	if (format_data==NULL){
		printf("Error in function %s: no format function given -> abort\n",__func__);
		exit(EXIT_FAILURE);
	}
	if (root!=NULL){	
		void* data;		
		int index = 0;
		if (file==NULL) file = stdout;
		fprintf(file,"tree has %d nodes:\n",RBTnodeCount(root));
		rbIterator iterator = RBTinitIterator(root);
		while((data = RBTiteraterNext(&iterator))!=NULL){
			char* s = format_helper(data);
			fprintf(file,"index #%d: %s\n",index++,s);
			free(s);			
		}		
	}	
	else{
		printf("tree is empty\n");
	}
	printf("\n");
}

int RBTcheck(rbNode* root,int quiet){
	int i;
	
	int OK = 1;
	
	if (root!=NULL){
		if (root->color==red){
			printf("Root is red !\n");
			OK = 0;
		}
		else{	
			int size = 0;
			int* Blacklist = NULL;
			if (rbtOK(root,1,&Blacklist,&size)){						
				for (i=1;i<size;i++) if (Blacklist[i]!=Blacklist[i-1]){
					printf("Black count not equal on different paths !\n");
					OK = 0;
				}
			}
			else OK = 0;
			if (Blacklist!=NULL) free(Blacklist);
		}
	}	
	if (!OK) printf("Sorry, this is not a proper Red-Black Tree !\n");	
	else if (!quiet) printf("Congratulations! Your Red-Black Tree is fine !\n");
	fflush(stdout);
	return OK;
}

void RBTree_set_compare(int (*Compare)(void* X1,void* X2)){
	compare = Compare;	
}

void RBTree_set_free(void (*Free_data)(void* X)){
	free_data = Free_data;	
}
	
void RBTree_set_format(void (*Format_data)(char* string,void* data)){
	format_data = Format_data;	
}

void RBTree_set_data_size(size_t data_size){
	data_size_in_bytes = data_size;
}

rbNode* RBTcopy(rbNode* root){
	if (root!=NULL) return RBTcopy_helper(root,root->parent); else return NULL;
}

rbNode* RBTinsertElement(rbNode** root,void* data){
	return RBTinsert(root,*root,data);
}

void RBTdeleteElement(rbNode** root,void* data){
	rbNode* toDelete = RBTgetNode(*root,data);
	if (toDelete!=NULL) RBTdeleteNode(root,toDelete);
}

int RBTnodeCount(rbNode* root){
	if (root!=NULL){
		return RBTnodeCount(root->left)+RBTnodeCount(root->right)+1;
	}
	else return 0;
}

int RBTcontains(rbNode* root,void* data){
	if (RBTgetNode(root,data)!=NULL) return 1; else return 0;
}

rbNode* RBTpredecessor(rbNode* node){
	if (node!=NULL){
		if (node->left!=NULL) return RBTmaxNode(node->left);
		else return RBTupUntilRightChild(node);				
	}
	else return NULL;
}

rbNode* RBTsuccessor(rbNode* node){
	if (node!=NULL){
		if (node->right!=NULL) return RBTminNode(node->right);
		else return RBTupUntilLeftChild(node);			
	}
	else return NULL;
}

void* RBTmin(rbNode* root){
	rbNode* node = RBTminNode(root);
	if (node!=NULL) return node->data; else return NULL;
}

void* RBTmax(rbNode* root){
	rbNode* node = RBTmaxNode(root);
	if (node!=NULL) return node->data; else return NULL;
}

void* RBTnearestLarger(rbNode* node,void* data){
	rbNode* larger = RBTnearestLargerNode(node,data);
	if (larger!=NULL) return larger->data; else return NULL;
}

void* RBTnearestSmaller(rbNode* node,void* data){
	rbNode* smaller = RBTnearestSmallerNode(node,data);
	if (smaller!=NULL) return smaller->data; else return NULL;
}

void RBTtoIntArray(rbNode* root,int** Array,int* size){
	*size = RBTnodeCount(root);
	*Array = (int*)realloc(*Array,(*size)*sizeof(int));
	int* iterator = *Array;
	RBTtoIntArray_helper(root,&iterator);
}

void RBTtoArray(rbNode* root,void** Array,int* size){
	*size = RBTnodeCount(root);
	if (*size>0){
		*Array = (void*)realloc(*Array,(*size)*data_size_in_bytes);
		void* iterator = *Array;
		RBTtoArray_helper(root,&iterator);
	}
}

void RBTtoIncArray(rbNode* root,void** Array,int* size){	
	*size = RBTnodeCount(root);
	*Array = (void*)realloc(*Array,(*size)*data_size_in_bytes);
	void* ptr = *Array;
	rbNode* it = RBTminNode(root);
	while(it!=NULL){
		memcpy(ptr,it->data,data_size_in_bytes);
		inc_ptr(&ptr,data_size_in_bytes);
		it = RBTsuccessor(it);
	}	
}

rbIterator RBTinitIterator(rbNode* root){
	rbIterator res = {.root = root,.position = NULL};
	return res;
}

void* RBTiteraterNext(rbIterator* iterator){
	if (iterator->root!=NULL){
		if (iterator->position==NULL) iterator->position = RBTminNode(iterator->root); else iterator->position = RBTsuccessor(iterator->position);			
		if (iterator->position!=NULL) return iterator->position->data; else return NULL;
	}
	else return NULL;
} 

void* RBTiteraterPrevious(rbIterator* iterator){
	if (iterator->root!=NULL){
		if (iterator->position==NULL) iterator->position = RBTmaxNode(iterator->root); else iterator->position = RBTpredecessor(iterator->position);			
		if (iterator->position!=NULL) return iterator->position->data; else return NULL;
	}
	else return NULL;
}

void RBTsetDefaultData(RBdata type){
	switch(type){
		case INT:
			compare = &INT_compare;
			free_data = &INT_free;
			format_data = &INT_format;
			data_size_in_bytes = sizeof(int);
			break;
		case DOUBLE:
			compare = &DOUBLE_compare;
			free_data = &DOUBLE_free;
			format_data = &DOUBLE_format;
			data_size_in_bytes = sizeof(double);
			break;		
	}
}
