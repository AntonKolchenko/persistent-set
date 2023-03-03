#include <cassert>  // assert
#include <iterator> // std::reverse_iterator
#include <utility>  // std::pair, std::swap

template <typename T>
struct persistent_set {
public:
  struct iterator;
  // we don't require const iterator here to make task easier
  using reverse_iterator = std::reverse_iterator<iterator>;

  persistent_set();                            // O(1) nothrow
  persistent_set(persistent_set const& other); // O(1) nothrow
  persistent_set(persistent_set&& other);      // O(1) nothrow

  persistent_set&
  operator=(persistent_set const& other);            // O(this.size()) nothrow
  persistent_set& operator=(persistent_set&& other); // O(this.size()) nothrow

  ~persistent_set(); // O(n) nothrow

  size_t size(); // O(n) nothrow
  void clear();  // O(n) nothrow
  bool empty();  // O(1) nothrow

  iterator begin(); //   O(h)    nothrow
  iterator end();   //   O(h)    nothrow

  reverse_iterator rbegin(); //  O(h)     nothrow
  reverse_iterator rend();   //  O(h)     nothrow

  std::pair<iterator, bool> insert(T const&); // O(h) strong
  iterator erase(iterator);                   // O(h) strong
  iterator find(T const&);                    // O(h) nothrow
  iterator lower_bound(T const&);             // O(h) nothrow
  iterator upper_bound(T const&);             // O(h) nothrow

  void swap(persistent_set& other); // O(1) nothrow
};

template <typename T>
struct persistent_set<T>::iterator {
  using iterator_category = std::bidirectional_iterator_tag;
  using difference_type = std::ptrdiff_t;
  using value_type = T const;
  using pointer = T const*;
  using reference = T const&;

  iterator() = default;

  reference operator*() const; // O(1) nothrow
  pointer operator->() const;  // O(1) nothrow

  iterator& operator++() &;   //      nothrow
  iterator operator++(int) &; //      nothrow

  iterator& operator--() &;   //      nothrow
  iterator operator--(int) &; //      nothrow
};
