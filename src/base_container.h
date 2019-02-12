#pragma once
#include "module.h"

namespace kagami {
  enum BaseContainerCode {
    kContainerObjectArray,
    kContainerNull
  };

  class IteratorInterface {
  public:
    virtual ~IteratorInterface() {}
    virtual void StepForward() = 0;
    virtual void StepBack() = 0;
    virtual Object &Deref() = 0;
    
  };

  template <class IteratorType>
  class STLIterator : public IteratorInterface {
  public:
    using value_type = IteratorType;

  private:
    IteratorType it_;

  public:
    STLIterator() = delete;

    STLIterator(IteratorType it) :
      it_(it) {}

    STLIterator(const STLIterator &rhs) :
      it_(rhs.it_) {}

    STLIterator(const STLIterator &&rhs) :
      STLIterator(rhs) {}

    void StepForward() {
      ++it_;
    }

    void StepBack() {
      --it_;
    }

    Object &Deref() {
      return *it_;
    }

    IteratorType &Get() {
      return it_;
    }
  };

  using ObjectArrayIterator = STLIterator<ObjectArray::iterator>;

  class IteratorPackage : public IteratorInterface {
  private:
    BaseContainerCode container_type_;
    shared_ptr<IteratorInterface> it_;

  private:
    template <class Tx>
    bool CastAndCompare(shared_ptr<IteratorInterface> &lhs,
      shared_ptr<IteratorInterface> &rhs) {
      return dynamic_pointer_cast<Tx>(lhs)->Get()
        == dynamic_pointer_cast<Tx>(rhs)->Get();
    }

  public:
    IteratorPackage() : container_type_(kContainerNull),
      it_(nullptr) {}

    IteratorPackage(const IteratorPackage &rhs) :
      container_type_(rhs.container_type_),
      it_(rhs.it_) {}

    IteratorPackage(const IteratorPackage &&rhs) :
      IteratorPackage(rhs) {}

    template <class Tx>
    IteratorPackage(Tx it, BaseContainerCode type) :
      it_(dynamic_pointer_cast<IteratorInterface>(make_shared<STLIterator<Tx>>(it))),
      container_type_(type) {}

    void StepForward() { return it_->StepForward(); }
    void StepBack() { return it_->StepBack(); }
    Object &Deref() { return it_->Deref(); }

    bool Compare(IteratorPackage &rhs) {
      bool result = false;

      if (rhs.container_type_ == container_type_ 
        && rhs.container_type_ != kContainerNull) {
        switch (container_type_) {
        case kContainerObjectArray:
          result = CastAndCompare<ObjectArrayIterator>(it_, rhs.it_);
          break;
        default:
          result = false;
          break;
        }
      }
      
      return result;
    }
  };
}