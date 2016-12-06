/* A (reverse) trie with fine-grained (per node) locks. 
 *
 * Hint: We recommend using a hand-over-hand protocol to order your locks,
 * while permitting some concurrency on different subtrees.
 * Additions to boiler plate made by Maxwell Daum. Honor Code: I did not give or recieve any unauthorized help during this assignment.
 */
#include <pthread.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "trie.h"

struct trie_node {
  struct trie_node *next;  /* parent list */
  unsigned int strlen; /* Length of the key */
  int32_t ip4_address; /* 4 octets */
  struct trie_node *children; /* Sorted list of children */
  char key[64]; /* Up to 64 chars */
  pthread_mutex_t node_lock;
};

static struct trie_node * root = NULL;
 int node_count = 0;
static int max_count = 100;  //Try to stay at no more than 100 nodes

pthread_mutex_t trie_mutex;
pthread_cond_t isFull;
pthread_mutex_t node_count_mutex;//used for node_count
pthread_mutex_t rootex;//used for null root

struct trie_node * new_leaf (const char *string, size_t strlen, int32_t ip4_address) { //only time node is ever created....
  struct trie_node *new_node = malloc(sizeof(struct trie_node));
  assert(pthread_mutex_lock(&node_count_mutex)==0); //lock count
  node_count++;
  assert(pthread_mutex_unlock(&node_count_mutex)==0);//unlock count
  if (!new_node) {
    printf ("WARNING: Node memory allocation failed.  Results may be bogus.\n");
    return NULL;
  }
  assert(strlen < 64);
  assert(strlen > 0);
  new_node->next = NULL;
  new_node->strlen = strlen;
  strncpy(new_node->key, string, strlen);
  new_node->key[strlen] = '\0';
  new_node->ip4_address = ip4_address;
  new_node->children = NULL;
  assert(pthread_mutex_init(&new_node->node_lock,NULL)==0);//instantiate lock for node

  return new_node;
}

// Compare strings backward.  Unlike strncmp, we assume
// that we will not stop at a null termination, only after
// n chars (or a difference).  Base code borrowed from musl
int reverse_strncmp(const char *left, const char *right, size_t n)
{
    const unsigned char *l= (const unsigned char *) &left[n-1];
    const unsigned char *r= (const unsigned char *) &right[n-1];
    if (!n--) return 0;
    for (; *l && *r && n && *l == *r ; l--, r--, n--);
    return *l - *r;
}


int compare_keys (const char *string1, int len1, const char *string2, int len2, int *pKeylen) {
    int keylen, offset;
    char scratch[64];
    assert (len1 > 0);
    assert (len2 > 0);
    // Take the max of the two keys, treating the front as if it were 
    // filled with spaces, just to ensure a total order on keys.
    if (len1 < len2) {
      keylen = len2;
      offset = keylen - len1;
      memset(scratch, ' ', offset);
      memcpy(&scratch[offset], string1, len1);
      string1 = scratch;
    } else if (len2 < len1) {
      keylen = len1;
      offset = keylen - len2;
      memset(scratch, ' ', offset);
      memcpy(&scratch[offset], string2, len2);
      string2 = scratch;
    } else
      keylen = len1; // == len2
      
    assert (keylen > 0);
    if (pKeylen)
      *pKeylen = keylen;
    return reverse_strncmp(string1, string2, keylen);
}

int compare_keys_substring (const char *string1, int len1, const char *string2, int len2, int *pKeylen) {
  int keylen, offset1, offset2;
  keylen = len1 < len2 ? len1 : len2;
  offset1 = len1 - keylen;
  offset2 = len2 - keylen;
  assert (keylen > 0);
  if (pKeylen)
    *pKeylen = keylen;
  return reverse_strncmp(&string1[offset1], &string2[offset2], keylen);
}

void init(int numthreads) {
  if (numthreads == 1)
    printf("WARNING: This is meant to be used with multiple threads!!!  You have %d!!!\n", numthreads);
	assert(pthread_mutex_init(&trie_mutex, NULL)==0);//make sure mutex is made
	assert(pthread_cond_init(&isFull, NULL)==0);//make sure cond var is made
	assert(pthread_mutex_init(&node_count_mutex, NULL)==0);//make sure mutex is made
	assert(pthread_mutex_init(&rootex, NULL)==0);//make sure mutex is made
  root = NULL;
}

void shutdown_delete_thread() {
  pthread_mutex_lock(&trie_mutex);
  pthread_cond_signal(&isFull);//should allow it to terminate gracefully
  pthread_mutex_unlock(&trie_mutex);
  return;
}

void handle_delete_thread(){ //called from main delete_thread
	pthread_mutex_lock(&trie_mutex); //must first acquire lock
	while(node_count<=100)pthread_cond_wait(&isFull,&trie_mutex); 
    check_max_nodes_delThread();
	pthread_mutex_unlock(&trie_mutex);
	return;
}


/* Recursive helper function.
 * Returns a pointer to the node if found.
 * Stores an optional pointer to the 
 * parent, or what should be the parent if not found.
 * 
 */
struct trie_node * 
_search (struct trie_node *node, const char *string, size_t strlen) { //root is locked prior to entering this method from search...
  int keylen, cmp;

  // First things first, check if we are NULL 
  if (node == NULL) return NULL; //will never try to acquire lock on null node so no worries here
  assert(node->strlen < 64);

  // See if this key is a substring of the string passed in
  cmp = compare_keys_substring(node->key, node->strlen, string, strlen, &keylen);
  if (cmp == 0) {
    // Yes, either quit, or recur on the children

    // If this key is longer than our search string, the key isn't here
    if (node->strlen > keylen) {
	  assert(pthread_mutex_unlock(&node->node_lock)==0); //release node lock before returning
      return NULL;
    } else if (strlen > keylen) {
      // Recur on children list
	  struct trie_node * child = node->children;//capture this before unlocking node
	  if(child!=NULL)assert(pthread_mutex_lock(&child->node_lock)==0);//lock the child before releasing parent
	  assert(pthread_mutex_unlock(&node->node_lock)==0); //release node lock before returning
      return _search(child, string, strlen - keylen);
    } else {
      assert (strlen == keylen);
	 assert(pthread_mutex_unlock(&node->node_lock)==0); //release node lock before returning
      return node;
    }

  } else {
    cmp = compare_keys(node->key, node->strlen, string, strlen, &keylen);
    if (cmp < 0) {
      // No, look right (the node's key is "less" than the search key)
	  struct trie_node * n = node->next;//captrue this before unlocking node
	  if(n!=NULL)assert(pthread_mutex_lock(&n->node_lock)==0);//lock the child before releasing parent
	  assert(pthread_mutex_unlock(&node->node_lock)==0); //release node lock before returning
      return _search(n, string, strlen);
    } else {
      // Quit early
	assert(pthread_mutex_unlock(&node->node_lock)==0); //release node lock before returning
      return 0;
    }
  }
}


int search  (const char *string, size_t strlen, int32_t *ip4_address) { //INTERFACE
  struct trie_node *found;
  // Skip strings of length 0

    if (strlen == 0){
		return 0;
  }
  if (root==NULL)return 0;
  assert(pthread_mutex_lock(&rootex)==0);//lock root pointer
  assert(pthread_mutex_lock(&root->node_lock)==0);//first lock root outside of recursive function if exists
  assert(pthread_mutex_unlock(&rootex)==0);//unlock root pointer
  found = _search(root, string, strlen); 
  
  if (found && ip4_address)
    *ip4_address = found->ip4_address;


  return (found != NULL);
}

/* Recursive helper function */
int _insert (const char *string, size_t strlen, int32_t ip4_address, 
	     struct trie_node *node, struct trie_node *parent, struct trie_node *left) {
  int cmp, keylen;

  // First things first, check if we are NULL 
  assert (node != NULL);
  assert (node->strlen < 64);

  // Take the minimum of the two lengths
  cmp = compare_keys_substring (node->key, node->strlen, string, strlen, &keylen);
  if (cmp == 0) {
    // Yes, either quit, or recur on the children

    // If this key is longer than our search string, we need to insert
    // "above" this node
    if (node->strlen > keylen) {
      struct trie_node *new_node;

      assert(keylen == strlen);
      assert((!parent) || parent->children == node);

      new_node = new_leaf (string, strlen, ip4_address);
      node->strlen -= keylen;
      new_node->children = node;
	  new_node->next = node->next;
	  node->next=NULL;

      assert ((!parent) || (!left));

      if (parent) {
	parent->children = new_node;
      } else if (left) {
	left->next = new_node;
      } else if ((!parent) || (!left)) {
	root = new_node;
      }
	  if(parent)assert(pthread_mutex_unlock(&parent->node_lock)==0);
	  if(left)assert(pthread_mutex_unlock(&left->node_lock)==0);
	  assert(pthread_mutex_unlock(&node->node_lock)==0);
      return 1;

    } else if (strlen > keylen) {
      
      if (node->children == NULL) {
	// Insert leaf here
	struct trie_node *new_node = new_leaf (string, strlen - keylen, ip4_address);
	node->children = new_node;
	if(parent)assert(pthread_mutex_unlock(&parent->node_lock)==0);
	if(left)assert(pthread_mutex_unlock(&left->node_lock)==0);
	assert(pthread_mutex_unlock(&node->node_lock)==0);
	return 1;
      } else {
	// Recur on children list, store "parent" (loosely defined)
	  assert(pthread_mutex_lock(&node->children->node_lock)==0);
	if(parent)assert(pthread_mutex_unlock(&parent->node_lock)==0);
	if(left)assert(pthread_mutex_unlock(&left->node_lock)==0);
      return _insert(string, strlen - keylen, ip4_address,
		     node->children, node, NULL);
      }
    } else {
      assert (strlen == keylen);
      if (node->ip4_address == 0) {
	node->ip4_address = ip4_address;
    if(parent)assert(pthread_mutex_unlock(&parent->node_lock)==0);
	if(left)assert(pthread_mutex_unlock(&left->node_lock)==0);
	assert(pthread_mutex_unlock(&node->node_lock)==0);
	return 1;
      } else {
    if(parent)assert(pthread_mutex_unlock(&parent->node_lock)==0);
	if(left)assert(pthread_mutex_unlock(&left->node_lock)==0);
	assert(pthread_mutex_unlock(&node->node_lock)==0);
	return 0;
      }
    }

  } else {
    /* Is there any common substring? */
    int i, cmp2, keylen2, overlap = 0;
    for (i = 1; i < keylen; i++) {
      cmp2 = compare_keys_substring (&node->key[i], node->strlen - i, 
				     &string[i], strlen - i, &keylen2);
      assert (keylen2 > 0);
      if (cmp2 == 0) {
	overlap = 1;
	break;
      }
    }

    if (overlap) {
      // Insert a common parent, recur
      int offset = strlen - keylen2;
      struct trie_node *new_node = new_leaf (&string[offset], keylen2, 0);
	  assert(pthread_mutex_lock(&new_node->node_lock)==0);
      assert ((node->strlen - keylen2) > 0);
      node->strlen -= keylen2;
      new_node->children = node;
      new_node->next = node->next;
      node->next = NULL;
      assert ((!parent) || (!left));

      if (node == root) {
	root = new_node;
      } else if (parent) {
	assert(parent->children == node);
	parent->children = new_node;
      } else if (left) {
	left->next = new_node;
      } else if ((!parent) && (!left)) {
	root = new_node;
      }
    if(parent)assert(pthread_mutex_unlock(&parent->node_lock)==0);
	if(left)assert(pthread_mutex_unlock(&left->node_lock)==0);
      return _insert(string, offset, ip4_address,
		     node, new_node, NULL);
    } else {
      cmp = compare_keys (node->key, node->strlen, string, strlen, &keylen);
      if (cmp < 0) {
	// No, recur right (the node's key is "less" than  the search key)
	if (node->next){
	  pthread_mutex_trylock(&node->next->node_lock);
  	if(parent)assert(pthread_mutex_unlock(&parent->node_lock)==0);
	if(left)assert(pthread_mutex_unlock(&left->node_lock)==0);
	  return _insert(string, strlen, ip4_address, node->next, NULL, node);
	}
	else {
	  // Insert here
	  struct trie_node *new_node = new_leaf (string, strlen, ip4_address);
	  node->next = new_node;
	if(parent)assert(pthread_mutex_unlock(&parent->node_lock)==0);
	if(left)assert(pthread_mutex_unlock(&left->node_lock)==0);
	assert(pthread_mutex_unlock(&node->node_lock)==0);
	  return 1;
	}
      } else {
	// Insert here
	struct trie_node *new_node = new_leaf (string, strlen, ip4_address);
	assert(pthread_mutex_lock(&new_node->node_lock)==0);
	new_node->next = node;
	if (node == root)
	  root = new_node;
	else if (parent && parent->children == node)
	  parent->children = new_node;
	else if (left && left->next == node)
	  left->next = new_node;
  
	assert(pthread_mutex_unlock(&new_node->node_lock)==0);
      }
    }
	if(parent)assert(pthread_mutex_unlock(&parent->node_lock)==0);
	if(left)assert(pthread_mutex_unlock(&left->node_lock)==0);
	assert(pthread_mutex_unlock(&node->node_lock)==0);
    return 1;
  }
}

int insert (const char *string, size_t strlen, int32_t ip4_address) { //INTERFACE	
	//puts("in insert");
  // Skip strings of length 0
  if (strlen == 0){
	  
	if(node_count>100){ 
 	  assert(pthread_mutex_lock(&trie_mutex)==0);//lock start mutex sections 
	  pthread_cond_signal(&isFull); //wake up delete_thread
	  assert(pthread_mutex_unlock(&trie_mutex)==0); //potential unlock
	}
    return 0;
  }

  /* Edge case: root is null */
  assert(pthread_mutex_lock(&rootex)==0);
  if (root == NULL) {
    root = new_leaf (string, strlen, ip4_address);
	assert(pthread_mutex_unlock(&rootex)==0);
    return 1;
  }
  assert(pthread_mutex_lock(&root->node_lock)==0);//first lock root outside of recursive function if exists...it should otherwise failed assertion in _insert
  int ret= _insert (string, strlen, ip4_address, root, NULL, NULL); //uses hand-over hand fine-grained locking
   assert(pthread_mutex_unlock(&rootex)==0);
	if(node_count>100){ 
 	  assert(pthread_mutex_lock(&trie_mutex)==0);//lock start mutex sections 
	  pthread_cond_signal(&isFull); //wake up delete_thread
	  assert(pthread_mutex_unlock(&trie_mutex)==0); //potential unlock
	}
	return ret;
}

/* Recursive helper function.
 * Returns a pointer to the node if found.
 * Stores an optional pointer to the 
 * parent, or what should be the parent if not found.
 * 
 */
struct trie_node * 
_delete (struct trie_node *node, const char *string, 
	 size_t strlen) { //not doing hand over hand....locking the root here as suggested in piazza
  int keylen, cmp;

  // First things first, check if we are NULL 
  if (node == NULL) return NULL;
  assert(node->strlen < 64);

  // See if this key is a substring of the string passed in
  cmp = compare_keys_substring (node->key, node->strlen, string, strlen, &keylen);
  if (cmp == 0) {
    // Yes, either quit, or recur on the children

    // If this key is longer than our search string, the key isn't here
    if (node->strlen > keylen) {
      return NULL;
    } else if (strlen > keylen) {
      struct trie_node *found =  _delete(node->children, string, strlen - keylen);
      if (found) {
	/* If the node doesn't have children, delete it.
	 * Otherwise, keep it around to find the kids */
	if (found->children == NULL && found->ip4_address == 0) {
	  assert(node->children == found);
	  node->children = found->next;
	  free(found);
	  pthread_mutex_lock(&node_count_mutex);
	  node_count--;
	  pthread_mutex_unlock(&node_count_mutex);
	}
	
	/* Delete the root node if we empty the tree */
	if (node == root && node->children == NULL && node->ip4_address == 0) {
	  root = node->next;
	  free(node);
	  pthread_mutex_lock(&node_count_mutex);
	  node_count--;
	  pthread_mutex_unlock(&node_count_mutex);
	}
	
	return node; /* Recursively delete needless interior nodes */
      } else 
	return NULL;
    } else {
      assert (strlen == keylen);

      /* We found it! Clear the ip4 address and return. */
      if (node->ip4_address) {
	node->ip4_address = 0;

	/* Delete the root node if we empty the tree */
	if (node == root && node->children == NULL && node->ip4_address == 0) {
	  root = node->next;
	  free(node);
	  pthread_mutex_lock(&node_count_mutex);
	  node_count--;
	  pthread_mutex_unlock(&node_count_mutex);
	  return (struct trie_node *) 0x100100; /* XXX: Don't use this pointer for anything except 
						 * comparison with NULL, since the memory is freed.
						 * Return a "poison" pointer that will probably 
						 * segfault if used.
						 */
	}
	return node;
      } else {
	/* Just an interior node with no value */
	return NULL;
      }
    }

  } else {
    cmp = compare_keys (node->key, node->strlen, string, strlen, &keylen);
    if (cmp < 0) {
      // No, look right (the node's key is "less" than  the search key)
      struct trie_node *found = _delete(node->next, string, strlen);
      if (found) {
	/* If the node doesn't have children, delete it.
	 * Otherwise, keep it around to find the kids */
	if (found->children == NULL && found->ip4_address == 0) {
	  assert(node->next == found);
	  node->next = found->next;
	  free(found);
	  pthread_mutex_lock(&node_count_mutex);
	  node_count--;
	  pthread_mutex_unlock(&node_count_mutex);
	}       
	
	return node; /* Recursively delete needless interior nodes */
      }
      return NULL;
    } else {
      // Quit early
      return NULL;
    }
  }
}

int delete  (const char *string, size_t strlen) { //INTERFACE not doing hand-over-hand here...locking whole path as suggested in piazza
  // Skip strings of length 0
  if (strlen == 0){
    return 0;
  }
  pthread_mutex_lock(&rootex);
  if(root!=NULL)assert(pthread_mutex_lock(&root->node_lock)==0);//lock root before entering recursive method
  int ret =(NULL != _delete(root, string, strlen));
    if(root!=NULL)assert(pthread_mutex_unlock(&root->node_lock)==0);//unlock root after
  pthread_mutex_unlock(&rootex);
	return ret;
}

//Recursively checks number of reachable nodes using DFS
int numReachable(struct trie_node *node){ //not Thread-Safe! used for checking tree after done w/ simulation
	int count=1;
	if(!node)return 0;
	if(node->children)
		count+=numReachable(node->children);
	if(node->next)
		count+=numReachable(node->next);
	return count;
}

//INTERFACE
void checkReachable (){ //not Thread-Safe! used for checking tree after done w/ simulation
	 assert(numReachable(root)==node_count); //adding in assertion for sequential trie
}

int max(int a, int b){
	if(a>b)return a;
	return b;
}


int _depth(struct trie_node *node){ //not Thread-Safe! used for checking tree after done w/ simulation
	int count=1;
	if(!node)return 0;
	count+=_depth(node->children);
	return max(count,_depth(node->next));
}
//INTERFACE
int depth(){ //not Thread-Safe! used for checking tree after done w/ simulation
	return _depth(root);
}



/* Find one node to remove from the tree. 
 * Use any policy you like to select the node.
 */
 //INTERFACE
int drop_one_node  () { //finding first leaf and killing it...hand over hand locking (except holding on 2 parent and child locks)
//puts("in drop node");
  assert(node_count > max_count);
	struct trie_node *b4Temp = NULL;
    pthread_mutex_lock(&rootex);	
	assert(pthread_mutex_lock(&root->node_lock)==0);//lock root
	struct trie_node *temp = root;
	while(temp->children!=NULL){
		if(b4Temp)assert(pthread_mutex_unlock(&b4Temp->node_lock)==0);//unlock parent of temp
		b4Temp=temp;
		assert(pthread_mutex_lock(&temp->children->node_lock)==0);//acquire child of temp
		temp=temp->children;
	}
	if(temp==root){ //edge case: root has no children
		root=temp->next;
		assert(b4Temp==NULL);
	}
	else{ //root has children
		assert(b4Temp->children == temp);
		assert(temp->children==NULL);
		b4Temp->children = temp->next;
	}
	temp->ip4_address=0;
	if(b4Temp)assert(pthread_mutex_unlock(&b4Temp->node_lock)==0);//unlock b4Temp if exists
	assert(pthread_mutex_unlock(&temp->node_lock)==0);//unlock temp
	free(temp);
	pthread_mutex_lock(&node_count_mutex);
	node_count--;
	pthread_mutex_unlock(&node_count_mutex);
	pthread_mutex_unlock(&rootex);
  return 1;
}

/* Check the total node count; see if we have exceeded a the max.
 */
 //INTERFACE
void check_max_nodes  () {
	pthread_mutex_lock(&trie_mutex);
  while (node_count > max_count) {
	  drop_one_node();
  }
	pthread_mutex_unlock(&trie_mutex);
}
//INTERFACE
void check_max_nodes_delThread  () {
  while (node_count > max_count) {
	  drop_one_node();
  }
}

void _print (struct trie_node *node) {
  printf ("Node at %p.  Key %.*s, IP %d.  Next %p, Children %p\n", 
	  node, node->strlen, node->key, node->ip4_address, node->next, node->children);
  if (node->children)
    _print(node->children);
  if (node->next)
    _print(node->next);
}

void print() {
  printf ("Root is at %p\n", root);
  /* Do a simple depth-first search */
  if (root)
    _print(root);

 printf("Num Nodes:%d\nNum Reachable:%d\n",node_count,numReachable(root)); //view numNodes at hend
	  int d = depth();
  printf("Depth of tree is %d\n",d);
 }
