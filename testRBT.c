#include "redblack.h"
#include <math.h>
#include <time.h>

typedef struct POINT2D{
	double x;
	double y;
}point2D;

double norm(point2D P){
	return sqrt(P.x*P.x+P.y*P.y);
}

int compare_point2D(void* X1,void* X2){
	const double tolerance = 1e-10;
	
	double r1 = norm(*(point2D*)X1);
	double r2 = norm(*(point2D*)X2);	
	if (fabs(r1-r2)<tolerance) return 0;
	else if (r1>r2) return 1;
	else return -1;
}

void free_point2D(void* X){
	point2D* P = (point2D*)X;
	free(P);	
}

void format_point2D(char* S,void* data){
	point2D* P = (point2D*)data;
	sprintf(S,"(%f,%f)",P->x,P->y);	
}

int main(int argc, char* argv[]){
	
	char* s;
	int n;
	
	if (argc>1) n = atoi(argv[1]);
	else{
		n = 10;
		printf("\nno list size given -> use default value %d\n\n",n);
	}
	
	// initiate the tree
	RBTree_set_compare(&compare_point2D);
	RBTree_set_free(&free_point2D);
	RBTree_set_format(&format_point2D);
	RBTree_set_data_size(sizeof(point2D));
	rbNode* root = NULL;
	
	// create <n> random points within the square [-1,1]x[-1,1] and insert in red-black tree
	int i;
	point2D* P;
	srand(time(NULL));
	for (i=0;i<n;i++){
		P = (point2D*)malloc(sizeof(point2D));
		P->x = (double)(2.*rand()/RAND_MAX-1.);
		P->y = (double)(2.*rand()/RAND_MAX-1.);
		RBTinsertElement(&root,P);
	}
	
	// check for red-black-rules
	RBTcheck(root,0);
	
	// create an ordered array from tree and print the elements
	int array_size = 0;
	point2D* array = NULL;
	RBTtoArray(root,(void**)&array,&array_size);	
	printf("\nThe tree has %d vertices\n",array_size);
	for (i=0;i<array_size;i++){
		s = format_helper(&(array[i]));
		printf("element # %d: data: %s, norm: %f\n",i,s,norm(array[i]));
		free(s);
	}
	free(array);
	
	// get the nodes that are the smallest larger than 0 and 0.5 respectively and delete these
	point2D Q = {.x=0,.y=0};
	P = (point2D*)RBTnearestLarger(root,&Q);
	s = format_helper(P);
	printf("\nnearest larger node to r=0: %s -> delete\n",s);
	free(s);
	RBTdeleteElement(&root,P);
	Q.y = 0.5;
	P = (point2D*)RBTnearestLarger(root,&Q);
	s = format_helper(P);
	printf("nearest larger node to r=0.5: %s -> delete\n\n",s);
	free(s);
	RBTdeleteElement(&root,P);
	
	// check for red-black-rules
	RBTcheck(root,0);
	
	// print reduced tree
	array = NULL;
	RBTtoArray(root,(void**)&array,&array_size);	
	printf("\nThe reduced tree has %d vertices\n",array_size);
	for (i=0;i<array_size;i++){
		s = format_helper(&(array[i]));
		printf("element # %d: data: %s, norm: %f\n",i,s,norm(array[i]));
		free(s);
	}
	free(array);
	
	RBTorderedPrint(root,stdout);
	
	// free red-black tree	
	RBTfree(root);
	
	printf("\ntest successful !\n");	
	return 0;
}

