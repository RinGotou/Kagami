#pragma once
#include "machine.h"
/*
  Base container implementations for Kagami script.
*/
namespace kagami {
  /* Runtime type identifier for UnifiedIterator */
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
  class BasicIterator : public IteratorInterface {
  public:
    using value_type = IteratorType;

  private:
    IteratorType it_;

  public:
    BasicIterator() = delete;

    BasicIterator(IteratorType it) :
      it_(it) {}

    BasicIterator(const BasicIterator &rhs) :
      it_(rhs.it_) {}

    BasicIterator(const BasicIterator &&rhs) :
      BasicIterator(rhs) {}

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

  using ObjectArrayIterator = BasicIterator<ObjectArray::iterator>;

  /*
    Top iterator wrapper.
    Provide unified methods for iterator type in script.
  */
  class UnifiedIterator : public IteratorInterface {
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
    UnifiedIterator() : container_type_(kContainerNull),
      it_(nullptr) {}

    UnifiedIterator(const UnifiedIterator &rhs) :
      container_type_(rhs.container_type_),
      it_(rhs.it_) {}

    UnifiedIterator(const UnifiedIterator &&rhs) :
      UnifiedIterator(rhs) {}

    /* 
      Hint: You must add new type identifier code in BaseContainerCode and add 
      casting actions below(Compare() and CreateCopy()) before add new items!
    */
    template <class Tx>
    UnifiedIterator(Tx it, BaseContainerCode type) :
      it_(dynamic_pointer_cast<IteratorInterface>(make_shared<BasicIterator<Tx>>(it))),
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

    bool Compare(UnifiedIterator &rhs) {
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

    UnifiedIterator CreateCopy() {
      UnifiedIterator pkg;
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