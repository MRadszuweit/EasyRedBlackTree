# EasyRedBlackTree
This little library can be used for creation of a red-black search tree (see 
https://en.wikipedia.org/wiki/Red%E2%80%93black_tree for more information) and search queries. It is well 
suited for large data structures, since search, insert and delete actions have a complexity of log(n) if n is the 
number of nodes in the tree. It is very easy to use. Just copy the .c and.h file in your project folder. 

The data content can be of arbitrary type. The user has the give

1. a compare function for two elements:     int compare(void* X1,void* X2)

  returning 1 if X1>X2, 0 if X1=X2, and -1 if X1<X2
  
  pass it to:  void RBTree_set_compare(int (*Compare)(void* X1,void* X2))
  
2. a function to free the data:  void free_data(void* X) 

  pass it to: void RBTree_set_free(void (*Free_data)(void* X))

3. give the memory size of a data element in bytes: data_size_in_bytes

  This is only necessary if one want to convert a tree to an array of tha data
  
  pass it to: void RBTree_set_data_size(size_t data_size)

4. For debugging on can print out the content of en element. 

  One has to give a format function:  void format_data(char* S,void* X) 
  that determines how an element X is written to a string S
  
  pass this function to: void RBTree_set_format(void (*Format_data)(char* string,void* data))
  
  


