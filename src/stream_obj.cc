#include "machine.h"

namespace kagami {
  template <class StreamType>
  Message StreamFamilyState(ObjectMap &p) {
    StreamType &stream = p.Cast<StreamType>(kStrMe);
    return Message().SetObject(stream.Good());
  }

  ///////////////////////////////////////////////////////////////
  // InStream implementations
  Message NewInStream(ObjectMap &p) {
    EXPECT_TYPE(p, "path", kTypeIdString);
    string path = p.Cast<string>("path");

    shared_ptr<InStream> ifs = make_shared<InStream>(path);

    return Message().SetObject(Object(ifs, kTypeIdInStream));
  }

  Message InStreamGet(ObjectMap &p) {
    InStream &ifs = p.Cast<InStream>(kStrMe);

    if (!ifs.Good()) {
      return Message(kCodeBadStream, "Invalid instream.", kStateError);
    }

    string result = ifs.GetLine();

    return Message().SetObject(result);
  }

  Message InStreamEOF(ObjectMap &p) {
    InStream &ifs = p.Cast<InStream>(kStrMe);
    return Message().SetObject(ifs.eof());
  }
  ///////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////
  // OutStream implementations
  Message NewOutStream(ObjectMap &p) {
    EXPECT_TYPE(p, "path", kTypeIdString);
    EXPECT_TYPE(p, "mode", kTypeIdString);

    string path = p.Cast<string>("path");
    string mode = p.Cast<string>("mode");

    shared_ptr<OutStream> ofs;
    bool append = (mode == "append");
    bool truncate = (mode == "truncate");

    if (!append && truncate) {
      ofs = make_shared<OutStream>(path, "w");
    }
    else if (append && !truncate) {
      ofs = make_shared<OutStream>(path, "a+");
    }

    return Message().SetObject(Object(ofs, kTypeIdOutStream));
  }

  Message OutStreamWrite(ObjectMap &p) {
    OutStream &ofs = p.Cast<OutStream>(kStrMe);
    auto &obj = p["str"];
    bool result = true;

    if (obj.GetTypeId() == kTypeIdString) {
      string str = obj.Cast<string>();
      result = ofs.WriteLine(str);
    }
    else {
      result = false;
    }

    return Message().SetObject(result);
  }
  ///////////////////////////////////////////////////////////////

  void InitStreamComponents() {
    using namespace management::type;

    ObjectTraitsSetup(kTypeIdInStream, ShallowDelivery<InStream>, PointerHasher())
      .InitConstructor(
        FunctionImpl(NewInStream, "path", "instream")
      )
      .InitMethods(
        {
          FunctionImpl(InStreamGet, "", "get"),
          FunctionImpl(InStreamEOF, "", "eof"),
          FunctionImpl(StreamFamilyState<InStream>, "", "good"),
        }
    );

    ObjectTraitsSetup(kTypeIdOutStream, ShallowDelivery<OutStream>, PointerHasher())
      .InitConstructor(
        FunctionImpl(NewOutStream, "path|mode", "outstream")
      )
      .InitMethods(
        {
          FunctionImpl(OutStreamWrite, "str", "write"),
          FunctionImpl(StreamFamilyState<OutStream>, "", "good"),
        }
    );

    management::CreateConstantObject("kOutstreamModeAppend", Object("'append'"));
    management::CreateConstantObject("kOutstreamModeTruncate", Object("'truncate'"));

    EXPORT_CONSTANT(kTypeIdInStream);
    EXPORT_CONSTANT(kTypeIdOutStream);
  }
}