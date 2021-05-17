
#ifndef HUFFMAN_TREE
#define HUFFMAN_TREE

template <typename P> class huffman_tree_factory
{
private:
  std::map<P, unsigned> symbol_frequency;

public:
  class huffman_tree
  {
  private:
    struct node
    {
      P symbol;
      unsigned frequency;
      node *left;
      node *right;

      bool is_leaf() const
      {
        return (left == NULL) && (right == NULL);
      }

      node(P symbol_, unsigned frequency_) : node(symbol_, frequency_, NULL, NULL)
      {
      }

      node(unsigned frequency_, node *left_, node *right_) : node(P(), frequency_, left_, right_)
      {
      }

      node(P symbol_, unsigned frequency_, node *left_, node *right_)
        : symbol(symbol_), frequency(frequency_), left(left_), right(right_)
      {
      }
    };

    node *root;

    /* P get_symbol(unsigned &code, node *n) const */
    /* { */
    /*   if (n->is_leaf()) */
    /*   { */
    /*     return n->symbol; */
    /*   } */

    /*   unsigned direction = code & 1; */
    /*   code >>= 1; */
    /*   if (direction) */
    /*   { */
    /*     if (n->left) */
    /*     { */
    /*       return get_symbol(code, n->left); */
    /*     } */
    /*   } */
    /*   else */
    /*   { */
    /*     if (n->right) */
    /*     { */
    /*       return get_symbol(code, n->right); */
    /*     } */
    /*   } */
			
			/* return 0; */
			/* /1* throw std::out_of_range ("blah"); *1/ */
    /* } */

		/* unsigned get_code(P symbol, unsigned &code, node* n) const */
		/* { */
    /*   if (n->is_leaf()) */
    /*   { */
    /*     if ( symbol == n->symbol) */
				/* { */
					/* return code; */
				/* } */
				/* else */
				/* { */
					/* /1* throw error; *1/ */
				/* } */
    /*   } */
		/* } */

  public:
    /* P get_symbol(unsigned code) const */
    /* { */
    /*   return get_symbol(code, root); */
    /* } */

		/* unsigned get_code(P symbol) const */
		/* { */
			/* unsigned code = 0; */
			/* return get_code(symbol, node); */
		/* } */

    huffman_tree(std::map<P, unsigned> symbol_frequency)
    {
      std::multimap<unsigned, node *> nodes;
      for (auto &p : symbol_frequency)
      {
        nodes.insert(std::pair<unsigned, node *>(p.second, new node(p.first, p.second)));
      }

      while (nodes.size() > 1)
      {
        auto it = nodes.begin();
        node *l = it->second;
        nodes.erase(it);
        it = nodes.begin();
        node *r = it->second;
        nodes.erase(it);

        node *n = new node(l->frequency + r->frequency, l, r);
        nodes.insert(std::pair<unsigned, node *>(n->frequency, n));
      }
      root = nodes.begin()->second;
    }

    ~huffman_tree()
    {
      // TODO
    }

    friend std::ostream &operator<<(std::ostream &os, const huffman_tree &ht)
    {
      /* return os << *(ht.root); */
    }
  };

  void inc_frequency(P symbol)
  {
    symbol_frequency[symbol]++;
  }

  huffman_tree *create()
  {
    return create(symbol_frequency);
  }

  huffman_tree *create(std::map<P, unsigned> symbol_frequency)
  {
    return new huffman_tree(symbol_frequency);
  }
};

#endif
