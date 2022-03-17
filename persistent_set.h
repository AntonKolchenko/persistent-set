#include <cassert>  // assert
#include <iterator> // std::reverse_iterator
#include <utility>  // std::pair, std::swap

template <typename T>
struct persistent_set {
public:
  struct iterator;
  // we don't require const iterator here to make task easier
  using reverse_iterator = std::reverse_iterator<iterator>;

  persistent_set() noexcept;                            // O(1) nothrow
  persistent_set(persistent_set const& other) noexcept; // O(1) nothrow
  persistent_set(persistent_set&& other) noexcept;      // O(1) nothrow

  persistent_set&
  operator=(persistent_set const& other) noexcept; // O(this.size()) nothrow
  persistent_set&
  operator=(persistent_set&& other) noexcept; // O(this.size()) nothrow

  ~persistent_set(); // O(n) nothrow

  size_t size() noexcept; // O(n) nothrow
  void clear() noexcept;  // O(n) nothrow
  bool empty() noexcept;  // O(1) nothrow

  iterator begin() noexcept; //   O(h)    nothrow
  iterator end() noexcept;   //   O(h)    nothrow

  reverse_iterator rbegin() noexcept; //  O(h)     nothrow
  reverse_iterator rend() noexcept;   //  O(h)     nothrow

  std::pair<iterator, bool> insert(T const&); // O(h) strong
  iterator erase(iterator);                   // O(h) strong
  iterator find(T const&) noexcept;           // O(h) nothrow
  iterator lower_bound(T const&) noexcept;    // O(h) nothrow
  iterator upper_bound(T const&) noexcept;    // O(h) nothrow

  void swap(persistent_set& other) noexcept; // O(1) nothrow
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
