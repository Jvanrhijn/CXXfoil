//
// Created by jesse on 10/22/18.
//

#ifndef _CXXFOIL_SRC_OPTIONAL_H
#define _CXXFOIL_SRC_OPTIONAL_H

#include <stdexcept>
#include <string>

#include <iostream>


namespace cxxfoil {

 class OptionalAccessException : public std::exception {
 public:
   const char *what() const noexcept {
     return "Attempted value access on empty Optional";
   }

};

 //! Wrapper that may or may not contain a value (essentially a non-nullable type)
template<class T>
class Optional {
 public:
  //! default constructor
  Optional()
    : has_value_(false)
  {}

  //! Implicit conversion constructor
  Optional(T value)
    : has_value_(true)
  {
    (void) reinterpret_cast<unsigned char *>(new(value_) T(value));
  }

  //! Perfect forwarding constructor
  template<class ...Args, size_t N=sizeof...(Args), class std::enable_if<(N>1), int>::type=0>
  Optional(Args&& ...args)
    : has_value_(true)
  {
    (void) reinterpret_cast<unsigned char *>(new(value_) T(std::forward<Args>(args)...));
  }

  ~Optional() {
    if (has_value_) {
      reinterpret_cast<T*>(value_)->~T();
    }
  }

  //! Value getter, throws on invalid access
  T value() const {
    if (!has_value_) {
      throw OptionalAccessException();
    }
    return *const_cast<T*>(reinterpret_cast<const T*>(value_));
  }

  //! Return internal flag, true if the value is set
  bool has_value() const {
    return has_value_;
  }

  //! Conversion operator to T
  operator T() const {
    return value();
  }

  //! Deref operator
  T operator*() const {
    return value();
  }

  //! assignment operator
  Optional &operator=(T value) {
    (void) new (value_) T(value);
    has_value_ = true;
    return *this;
  }

 private:
  alignas(T) unsigned char value_[sizeof(T)];
  bool has_value_;

};

} // namespace pie


#endif // _CXXFOIL_SRC__OPTIONAL_H
