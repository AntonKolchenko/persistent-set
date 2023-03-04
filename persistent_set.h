#include <cassert>  // assert
#include <iterator> // std::reverse_iterator
#include <utility>  // std::pair, std::swap

template <typename T>
struct persistent_set {
private:
  struct node {
    node() = default;

    explicit node(T const& value) {
      this->value = std::make_shared<T>(value);
    }

    node(T&& value) {
      this->value = std::make_shared<T>(std::move(value));
    }

    node(node const& other)
        : value(other.value), left(other.left), right(other.right) {}

    node(node&& other)
        : value(std::move(other.value)), left(std::move(other.left)),
          right(std::move(other.right)) {}

    node& operator=(node const& other) {
      if (this != &other) {
        auto tmp(other);
        swap(tmp);
      }
      return *this;
    }

    node& operator=(node&& other) {
      if (this != &other) {
        node tmp(std::move(other));
        swap(tmp);
      }
      return *this;
    }

    friend bool operator==(node const& lhs, node const& rhs) {
      return lhs.left == rhs.left && lhs.right == rhs.right &&
             lhs.value == rhs.value;
    }

    friend bool operator!=(node const& lhs, node const& rhs) {
      return !(lhs == rhs);
    }

    void swap(node& other) {
      std::swap(value, other.value);
      std::swap(left, other.left);
      std::swap(right, other.right);
    }

    std::shared_ptr<T> value;
    std::shared_ptr<node> left, right;
  };

public:
  struct iterator {
    friend struct persistent_set;
    using iterator_category = std::bidirectional_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = T const;
    using pointer = T const*;
    using reference = T const&;

    iterator() = default;

    iterator(iterator const& other)
        : node_ptr(other.node_ptr), set_ptr(other.set_ptr) {}

    iterator& operator=(iterator const& other) {
      node_ptr = other.node_ptr;
      set_ptr = other.set_ptr;
      return *this;
    }

    reference operator*() const {
      return *node_ptr->value.get();
    }

    pointer operator->() const {
      return node_ptr->value.get();
    }

    iterator& operator++() & {
      *this = set_ptr->upper_bound(*node_ptr->value.get());
      return *this;
    }

    iterator operator++(int) & {
      auto tmp = *this;
      *this = set_ptr->upper_bound(*node_ptr->value.get());
      return tmp;
    }

    iterator& operator--() & {
      *this = set_ptr->rev_upper_bound(node_ptr->value.get());
      return *this;
    }

    iterator operator--(int) & {
      auto tmp = *this;
      *this = set_ptr->rev_upper_bound(node_ptr->value.get());
      return tmp;
    }

    friend bool operator==(iterator const& lhs, iterator const& rhs) {
      return lhs.set_ptr == rhs.set_ptr && lhs.node_ptr == rhs.node_ptr;
    }

    friend bool operator!=(iterator const& lhs, iterator const& rhs) {
      return !(lhs == rhs);
    }

  private:
    iterator(node const* node_ptr, persistent_set<T> const* set_ptr) {
      this->node_ptr = const_cast<node*>(node_ptr);
      this->set_ptr = const_cast<persistent_set<T>*>(set_ptr);
    }

    node* node_ptr;
    persistent_set<T>* set_ptr;
  };

  // we don't require const iterator here to make task easier
  using reverse_iterator = std::reverse_iterator<iterator>;

  persistent_set() = default;

  persistent_set(persistent_set const& other)
      : m_size(other.m_size), m_root(other.m_root) {}

  persistent_set(persistent_set&& other)
      : m_size(other.m_size), m_root(std::move(other.m_root)) {}

  persistent_set& operator=(persistent_set const& other) {
    if (*this != other) {
      persistent_set(other).swap(*this);
    }
    return *this;
  }

  persistent_set& operator=(persistent_set&& other) {
    if (*this != other) {
      persistent_set(std::move(other)).swap(*this);
    }
    return *this;
  }

  ~persistent_set() = default;

  size_t size() {
    return m_size;
  }

  void clear() {
    m_size = 0;
    m_root.left.reset();
  }

  bool empty() {
    return size() == 0;
  }

  iterator begin() const {
    auto* ptr = &m_root;
    while (ptr->left) {
      ptr = ptr->left.get();
    }
    return iterator(ptr, this);
  }

  iterator end() const {
    return iterator(&m_root, this);
  }

  reverse_iterator rbegin() const {
    return std::make_reverse_iterator(end());
  }

  reverse_iterator rend() const {
    return std::make_reverse_iterator(begin());
  }

  std::pair<iterator, bool> insert(T const& value) {
    if (find(value) != end()) {
      return {find(value), false};
    }

    auto back_up = m_root;
    auto back_up_size = m_size;

    try {
      node tmp = m_root;
      if (tmp.left) {
        tmp.left = std::make_shared<node>(*tmp.left);
        insert(*tmp.left, value);
      } else {
        tmp.left = std::make_shared<node>(value);
      }

      m_root = std::move(tmp);
      ++m_size;

      return {find(value), true};
    } catch (...) {
      m_root = back_up;
      m_size = back_up_size;
      throw;
    }
  }

  iterator erase(iterator it) {
    auto back_up = m_root;
    auto back_up_size = m_size;

    try {
      std::shared_ptr<T> tmp_val = it.node_ptr->value;

      auto tmp = m_root;
      erase(*tmp_val, &tmp);
      m_root = tmp;
      --m_size;

      return lower_bound(*tmp_val.get());
    } catch (...) {
      m_root = back_up;
      m_size = back_up_size;
      throw;
    }
  }

  iterator find(T const& value) {
    if (lower_bound(value) == end() || *lower_bound(value) != value) {
      return end();
    }
    return lower_bound(value);
  }

  iterator lower_bound(T const& value) {
    return lower_bound(m_root.left.get(), value);
  }

  iterator upper_bound(T const& value) {
    return upper_bound(m_root.left.get(), value);
  }

  friend void swap(persistent_set& left, persistent_set& right) {
    left.swap(right);
  }

  friend bool operator==(persistent_set const& lhs, persistent_set const& rhs) {
    return lhs.m_root == rhs.m_root && lhs.m_size == rhs.m_size;
  }

  friend bool operator!=(persistent_set const& lhs, persistent_set const& rhs) {
    return !(lhs == rhs);
  }

private:
  void swap(persistent_set& other) {
    std::swap(this->m_size, other.m_size);
    std::swap(this->m_root, other.m_root);
  }

  iterator rev_upper_bound(T* value) {
    if (value == nullptr) {
      auto* ptr = m_root.left.get();
      while (ptr->right) {
        ptr = ptr->right.get();
      }
      return iterator(ptr, this);
    }
    return rev_upper_bound(m_root.left.get(), value);
  }

  iterator rev_upper_bound(node* ptr, T* value) {
    if (ptr == nullptr) {
      return end();
    }

    if (*value > *ptr->value.get()) { /// >
      auto it = rev_upper_bound(ptr->right.get(), value);
      if (it == end()) {
        return iterator(ptr, this);
      }
      return it;
    }

    return rev_upper_bound(ptr->left.get(), value);
  }

  iterator lower_bound(node* ptr, T const& value) {
    if (ptr == nullptr) {
      return end();
    }

    if (*ptr->value.get() == value) { /// ==
      return iterator(ptr, this);
    }

    if (value > *ptr->value) {
      return lower_bound(ptr->right.get(), value);
    }

    auto it = lower_bound(ptr->left.get(), value);
    if (it == end()) {
      return iterator(ptr, this);
    }
    return it;
  }

  iterator upper_bound(node* ptr, T const& value) {
    if (ptr == nullptr) {
      return end();
    }

    if (value < *ptr->value) { /// <
      auto it = upper_bound(ptr->left.get(), value);
      if (it == end()) {
        return iterator(ptr, this);
      }
      return it;
    }

    return upper_bound(ptr->right.get(), value);
  }

  void insert(node& next, T const& val) {
    if (val < *next.value) {
      if (next.left) {
        next.left = std::make_shared<node>(*next.left);
        insert(*next.left, val);
      } else {
        next.left = std::make_shared<node>(val);
      }
    } else {
      if (next.right) {
        next.right = std::make_shared<node>(*next.right);
        insert(*next.right, val);
      } else {
        next.right = std::make_shared<node>(val);
      }
    }
  }

  void erase_impl(node* parent, node* son) {
    if (!son->left && !son->right) {
      if (parent->left.get() == son) {
        parent->left.reset();
      } else {
        parent->right.reset();
      }
      return;
    }

    if (parent->left.get() == son) {
      if (son->left) {
        parent->left = son->left;
      } else {
        parent->left = son->right;
      }
    } else {
      if (son->left) {
        parent->right = son->left;
      } else {
        parent->right = son->right;
      }
    }
  }

  void erase(T const& value, node* prev) {
    prev->left = std::make_shared<node>(*prev->left);
    auto* next = prev->left.get();

    while (true) {
      if (*next->value == value) {
        if (next->left && next->right) { /// 2 sons
          prev = next;

          auto* tmp_prev = prev;

          next->left = std::make_shared<node>(*next->left);
          next = next->left.get();

          while (next->right) {
            tmp_prev = next;
            next->right = std::make_shared<node>(*next->right);
            next = next->right.get();
          }

          std::swap(prev->value,
                    next->value); /// erase like in AVL-tree without correction
          erase_impl(tmp_prev, next); /// for readability of code
          return;
        }

        erase_impl(prev, next);
        return;
      }

      prev = next;
      if (value < *next->value) {
        next->left = std::make_shared<node>(*next->left);
        next = next->left.get();
      } else {
        next->right = std::make_shared<node>(*next->right);
        next = next->right.get();
      }
    }
  }

  node m_root;
  size_t m_size = 0;
};