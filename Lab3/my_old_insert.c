int _insert (const char *string, size_t strlen, int32_t ip4_address, 
	     struct trie_node *node, struct trie_node *parent, struct trie_node *left) { //root is locked before entering first time from insert...
																					//prior to future calls...guaranteed node, parent, left are locked if they exist
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
	  assert(pthread_mutex_lock(&new_node->node_lock)==0);//lock the newly created node
      node->strlen -= keylen;
      new_node->children = node; //will already be locked upon entering method...
      assert ((!parent) || (!left)); //can't have both...be careful with locks here... either should be locked upon entering method

      if (parent) {
	parent->children = new_node;
      } else if (left) {
	left->next = new_node;
      } else if ((!parent) || (!left)) {
		struct trie_node *temp = root;
		int toLock=0;
		if(root!=node &&root!=parent &&root!=left)toLock=1;
		if(toLock==1)assert(pthread_mutex_lock(&root->node_lock)==0);
		root = new_node;
		if(toLock==1)assert(pthread_mutex_unlock(&temp->node_lock)==0);
      }
	  if(parent) assert(pthread_mutex_unlock(&parent->node_lock)==0);//unlock the parent node if exists
	  if(left)assert(pthread_mutex_unlock(&left->node_lock)==0);//unlock the left node if exists
	  assert(pthread_mutex_unlock(&node->node_lock)==0);//unlock node....not referenced anymore
	  assert(pthread_mutex_unlock(&new_node->node_lock)==0);//unlock the new node
      return 1;

    } else if (strlen > keylen) {
      
      if (node->children == NULL) {
	// Insert leaf here
	struct trie_node *new_node = new_leaf (string, strlen - keylen, ip4_address);
	node->children = new_node;
	if(parent)assert(pthread_mutex_unlock(&parent->node_lock)==0);//unlock the parent node if exists
	if(left)assert(pthread_mutex_unlock(&left->node_lock)==0);//unlock the left node if existss
	assert(pthread_mutex_unlock(&node->node_lock)==0);//unlock node
	return 1;
      } else {
	// Recur on children list, store "parent" (loosely defined)
	  if(parent)assert(pthread_mutex_unlock(&parent->node_lock)==0);//unlock the parent node if exists
	  if(left)assert(pthread_mutex_unlock(&left->node_lock)==0);//unlock the left node if exists
	  assert(pthread_mutex_lock(&node->children->node_lock)==0);//lock the child node...node will stay locked as parent in next iteration...
      return _insert(string, strlen - keylen, ip4_address,
		     node->children, node, NULL);//leave node and node->children locked for next iteration...
      }
    } else {
      assert (strlen == keylen);
      if (node->ip4_address == 0) {
	node->ip4_address = ip4_address;
	if(parent)assert(pthread_mutex_unlock(&parent->node_lock)==0);//unlock the parent node if exists
	if(left)assert(pthread_mutex_unlock(&left->node_lock)==0);//unlock the left node if exists
	assert(pthread_mutex_unlock(&node->node_lock)==0);//unlock node
	return 1;
      } else {
			if(parent)assert(pthread_mutex_unlock(&parent->node_lock)==0);//unlock the parent node if exists
			if(left)assert(pthread_mutex_unlock(&left->node_lock)==0);//unlock the left node if exists
			assert(pthread_mutex_unlock(&node->node_lock)==0);//unlock node
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
	  assert(pthread_mutex_lock(&new_node->node_lock)==0);//lock newly created node
      assert ((node->strlen - keylen2) > 0);
      node->strlen -= keylen2;
      new_node->children = node;
      new_node->next = node->next;
      node->next = NULL;
      assert ((!parent) || (!left)); //will already have lock for these if exists

      if (node == root) {
	root = new_node;
      } else if (parent) {
	assert(parent->children == node);
	parent->children = new_node;
      } else if (left) {
	left->next = new_node;
	
      } else if ((!parent) && (!left)) {
		struct trie_node *temp = root;
		int toLock=0; //to avoid infinite blocking if already have lock
		if(root!=node &&root!=parent &&root!=left)toLock=1;
		if(toLock==1)assert(pthread_mutex_lock(&root->node_lock)==0);
		root = new_node;
		if(toLock==1)assert(pthread_mutex_unlock(&temp->node_lock)==0);
      }
	 if(parent)assert(pthread_mutex_unlock(&parent->node_lock)==0);//unlock the parent node if exists
	 if(left) assert(pthread_mutex_unlock(&left->node_lock)==0);//unlock the left node
      return _insert(string, offset, ip4_address,
		     node, new_node, NULL); //keep node and new_node locked for next iteration...
    } else {
      cmp = compare_keys (node->key, node->strlen, string, strlen, &keylen);
      if (cmp < 0) {
	// No, recur right (the node's key is "less" than  the search key)
	if (node->next){ //COME BACK 
	  if(parent)assert(pthread_mutex_unlock(&parent->node_lock)==0);//unlock the parent node if exists
	  if(left)assert(pthread_mutex_unlock(&left->node_lock)==0);//unlock the left node if exists
	  assert(pthread_mutex_lock(&node->next->node_lock)==0);//lock node->next before passing it into recursive call
	  return _insert(string, strlen, ip4_address, node->next, NULL, node); //keeping node and node->next locked for next iteration
	}
	else {
	  // Insert here
	  struct trie_node *new_node = new_leaf (string, strlen, ip4_address);
	  node->next = new_node;
	  if(parent)assert(pthread_mutex_unlock(&parent->node_lock)==0);//unlock the parent node if exists
	  if(left)assert(pthread_mutex_unlock(&left->node_lock)==0);//unlock the left node if exists
	  assert(pthread_mutex_unlock(&node->node_lock)==0);//unlock node
	  return 1;
	}
      } else {
	// Insert here
	struct trie_node *new_node = new_leaf (string, strlen, ip4_address);
	assert(pthread_mutex_lock(&new_node->node_lock)==0);//lock newly made node
	new_node->next = node;
	if (node == root)
	  root = new_node;
	else if (parent && parent->children == node)
	  parent->children = new_node;
	else if (left && left->next == node)
	  left->next = new_node;
  
	assert(pthread_mutex_unlock(&new_node->node_lock)==0);//unlock new node
      }
    }
	if(parent)assert(pthread_mutex_unlock(&parent->node_lock)==0);//unlock the parent node if exists
    if(left)assert(pthread_mutex_unlock(&left->node_lock)==0);//unlock the left node if exists
	assert(pthread_mutex_unlock(&node->node_lock)==0);//unlock node
    return 1;
  }
}