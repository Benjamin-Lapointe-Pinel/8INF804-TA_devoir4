#ifndef HUFFMAN_TREE
#define HUFFMAN_TREE

template <typename P> class huffman_tree_factory
{
public:
  class huffman_tree
  {
  public:
    class node
    {
    private:
      P symbol = P();
      unsigned frequency = 1;
      unsigned depth = 0;
      unsigned code = 0;
      node *left = NULL;
      node *right = NULL;

      void update_children(uint8_t direction)
      {
        depth++;
        code <<= 1;
        code |= direction;

        if (left)
        {
          left->update_children(direction);
        }
        if (right)
        {
          right->update_children(direction);
        }
      }

    public:
      bool is_leaf() const
      {
        return (left == NULL) && (right == NULL);
      }

      void inc_frequency()
      {
        frequency++;
      }

      unsigned get_frequency() const
      {
        return frequency;
      }

      unsigned get_depth() const
      {
        return depth;
      }

      unsigned get_code() const
      {
        return code;
      }

      node(P symbol_) : symbol(symbol_)
      {
      }

      node(node *left_, node *right_) : left(left_), right(right_)
      {
        frequency = left->get_frequency() + right->get_frequency();
        left->update_children(0);
        right->update_children(1);
      }

      ~node()
      {
        if (left)
        {
          delete left;
        }
        if (right)
        {
          delete right;
        }
      }
    };

    huffman_tree(std::map<P, node *> symbol_node) : leaves(symbol_node)
    {
      std::multimap<unsigned, node *> frequency_nodes;
      for (auto &p : symbol_node)
      {
        node *n = p.second;
        frequency_nodes.insert(std::pair<unsigned, node *>(n->get_frequency(), n));
      }

      while (frequency_nodes.size() > 1)
      {
        auto it = frequency_nodes.begin();
        node *l = it->second;
        frequency_nodes.erase(it);
        it = frequency_nodes.begin();
        node *r = it->second;
        frequency_nodes.erase(it);

        node *n = new node(l, r);
        frequency_nodes.insert(std::pair<unsigned, node *>(n->get_frequency(), n));
      }
      root = frequency_nodes.begin()->second;
    }

    ~huffman_tree()
    {
      delete root;
    }

    friend std::ostream &operator<<(std::ostream &os, const huffman_tree &ht)
    {
      for (auto &p : ht.get_leaves())
      {
        node *n = p.second;
        unsigned code = n->get_code();

        os << (unsigned)p.first << ' ';
        for (size_t i = 0; i < n->get_depth(); i++)
        {
          if (code & 1)
          {
            os << '1';
          }
          else
          {
            os << '0';
          }
          code >>= 1;
        }
        os << std::endl;
      }
      return os;
    }

    std::map<P, node *> get_leaves() const
    {
      return leaves;
    }

  private:
    node *root;
    std::map<P, node *> leaves;
  };

  void inc_frequency(P symbol)
  {
    if (symbol_node[symbol])
    {
      symbol_node[symbol]->inc_frequency();
    }
    else
    {
      symbol_node[symbol] = new huffman_tree_node(symbol);
    }
  }

  huffman_tree *create()
  {
    return new huffman_tree(symbol_node);
  }

private:
  typedef typename huffman_tree_factory<P>::huffman_tree::node huffman_tree_node;
  std::map<P, huffman_tree_node *> symbol_node;
};

#endif
