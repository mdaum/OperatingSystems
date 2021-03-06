#ifndef __TRIE_H__
#define __TRIE_H__

#include <stdint.h>
#include <assert.h>

/* A simple (reverse) trie interface 
 * Additions to boiler plate made by Maxwell Daum. Honor Code: I did not give or recieve any unauthorized help during this assignment.
*/

/* Optional init routine.  May not be required. */
void init (int numthreads);

/* Return 1 on success, 0 on failure */
int insert (const char *string, size_t strlen, int32_t ip4_address);

/* Return 1 if the key is found, 0 if not. 
 * If ip4_address is not NULL, store the IP 
 * here.  
 */
int search  (const char *string, size_t strlen, int32_t *ip4_address);


/* Return 1 if the key is found and deleted, 0 if not. */
int delete  (const char *string, size_t strlen);

/* Check the maximum node count.
 * If we have exceeded it, drop some nodes
 * to lower the count.
 */
void check_max_nodes ();

void check_max_nodes_delThread();//used from delThread...since you do not lock in this one....could of added an int param to 
//original method but did not want to modify anything already given in the interface...
void shutdown_delete_thread(); //need here so main can call it

void handle_delete_thread();

void checkReachable (); //check for reachable //not Thread-Safe! used for checking tree after done w/ simulation

int depth();//checks depth of tree //not Thread-Safe! used for checking tree after done w/ simulation


/* Print the structure of the tree.  Mostly useful for debugging. */
void print (); 

/* Determines whether to allow blocking until 
 * a name is available.
 */
extern int allow_squatting;


#endif /* __TRIE_H__ */ 
