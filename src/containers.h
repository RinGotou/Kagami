#pragma once
#include "machine.h"
/*
  Base container implementations for Kagami script.
*/
namespace kagami {
  /* Runtime type identifier for UnifiedIterator */
  enum BaseContainerCode {
    kContainerObjectArray,
    kContainerObjectTable,
    kContainerNull
  };

  /* Unified iterator wrapper interface */
  class IteratorInterface {
  public:
    virtual ~IteratorInterface() {}
    virtual void StepForward() = 0;
    virtual void StepBack() = 0;
    virtual Object Unpack() = 0;
    
  };

  template <class IteratorType>
  class BasicIterator : public IteratorInterface {
  private:
    IteratorType it_;

  public:
    BasicIterator() = delete;
    BasicIterator(IteratorType it) : it_(it) {}
    BasicIterator(const BasicIterator &rhs) : it_(rhs.it_) {}
    BasicIterator(const BasicIterator &&rhs) : BasicIterator(rhs) {}

  public:
    void StepForward() { ++it_; }
    void StepBack() { --it_; }
    Object Unpack() { return Object().PackObject(*it_); }
    IteratorType &Get() { return it_; }
    bool operator==(BasicIterator<IteratorType> &rhs) const 
    { return it_ == rhs.it_; }
  };


  template <>
  class BasicIterator<ObjectTable::iterator> : public IteratorInterface {
  private:
    ObjectTable::iterator it_;

  public:
    BasicIterator() = delete;
    BasicIterator(ObjectTable::iterator it) : it_(it) {}
    BasicIterator(const BasicIterator &rhs) : it_(rhs.it_) {}
    BasicIterator(const BasicIterator &&rhs) : BasicIterator(rhs) {}

  public:
    void StepForward() { ++it_; }
    void StepBack() { --it_; }
    ObjectTable::iterator &Get() { return it_; }
    Object Unpack() {
      auto copy_left = it_->first;
      ManagedPair base = make_shared<ObjectPair>(
        Object(management::type::CreateObjectCopy(copy_left)),
        Object(management::type::CreateObjectCopy(it_->second)));
      return Object(base, kTypeIdPair);
    }
    bool operator==(BasicIterator<ObjectTable::iterator> &rhs) const 
    { return it_ == rhs.it_; }
  };

  using ObjectArrayIterator = BasicIterator<ObjectArray::iterator>;
  using ObjectTableIterator = BasicIterator<ObjectTable::iterator>;
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

    template <class Tx>
    auto GetPointer() {
      return dynamic_cast<IteratorInterface *>(new Tx(
        *dynamic_pointer_cast<Tx>(it_)));
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

    Object Unpack() { return it_->Unpack(); }

    bool Compare(UnifiedIterator &rhs) {
      bool result = false;

      if (rhs.container_type_ == container_type_ 
        && rhs.container_type_ != kContainerNull) {
        switch (container_type_) {
        case kContainerObjectArray:
          result = CastAndCompare<ObjectArrayIterator>(it_, rhs.it_);
          break;
        case kContainerObjectTable:
          result = CastAndCompare<ObjectTableIterator>(it_, rhs.it_);
          break;
        default:
          result = false;
          break;
        }
      }
      
      return result;
    }

    UnifiedIterator CreateCopy() {
      UnifiedIterator it;
      
#define COPY_ITERATOR(_Type)         \
  it.it_.reset(GetPointer<_Type>()); \
  it.container_type_ = container_type_;

      switch (container_type_) {
      case kContainerObjectArray:
        COPY_ITERATOR(ObjectArrayIterator);
        break;
      case kContainerObjectTable:
        COPY_ITERATOR(ObjectTableIterator);
        break;
      default:
        break;
      }
#undef COPY_ITERATOR
      return it;
    }
  };
}