#pragma once
#include "module.h"
/*
  Base container implementations for Kagami script.
*/
namespace kagami {
  /* Runtime type identifier for IteratorPackage */
  enum BaseContainerCode {
    kContainerObjectArray,
    kContainerNull
  };

  /* Unified iterator wrapper interface */
  class IteratorInterface {
  public:
    virtual ~IteratorInterface() {}
    virtual void StepForward() = 0;
    virtual void StepBack() = 0;
    virtual Object &Deref() = 0;
    
  };

  /* 
    Base wrapper for STL Iterator class.
    Any class that is inherited from standard iterator interface can
    be packed safely.
  */
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

  /*
    Top iterator wrapper.
    Provide unified methods for iterator type in script.
  */
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

    /* 
      Hint: You must add new type identifier code in BaseContainerCode and add 
      casting actions below(Compare() and CreateCopy()) before add new items!
    */
    template <class Tx>
    IteratorPackage(Tx it, BaseContainerCode type) :
      it_(dynamic_pointer_cast<IteratorInterface>(make_shared<STLIterator<Tx>>(it))),
      container_type_(type) {}

    void StepForward() { it_->StepForward(); }
    void StepBack() { it_->StepBack(); }

    void StepForward(size_t step) {
      for (size_t idx = 0; idx < step; idx += 1) {
        it_->StepForward();
      }
    }

    void StepBack(size_t step) {
      for (size_t idx = 0; idx < step; idx += 1) {
        it_->StepBack();
      }
    }

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

    IteratorPackage CreateCopy() {
      IteratorPackage pkg;
      switch (container_type_) {
      case kContainerObjectArray:
        pkg.it_.reset(
          dynamic_cast<IteratorInterface *>(new ObjectArrayIterator(
            *dynamic_pointer_cast<ObjectArrayIterator>(it_)
          )));
        pkg.container_type_ = container_type_;
      default:
        break;
      }

      return pkg;
    }
  };
}