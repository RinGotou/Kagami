#include "module.h"

#if defined (_WIN32)
namespace kagami {
  template <class Tx>
  Message TCPConnectorSend(ObjectMap &p) {
    EXPECT(IsStringObject(p["content"]), "Invalid content string.");
    Tx &tx = p[kStrObject].Cast<Tx>();
    TCPConnector *connector = dynamic_cast<TCPConnector *>(&tx);
    string content = ParseRawString(p["content"].Cast<string>());
    return Message(util::MakeBoolean(connector->Send(content)));
  }

  template<class Tx>
  Message TCPConnectorReceive(ObjectMap &p) {
    auto &dest = p["dest"];
    Tx &tx = p[kStrObject].Cast<Tx>();
    TCPConnector *connector = dynamic_cast<TCPConnector *>(&tx);
    string dest_buf;
    string result = util::MakeBoolean(connector->Receive(dest_buf));
    dest.ManageContent(make_shared<string>(dest_buf), kTypeIdString);
    return Message(result);
  }

  template <class Tx>
  Message TCPConnectorClose(ObjectMap &p) {
    Tx &tx = p[kStrObject].Cast<Tx>();
    TCPConnector *connector = dynamic_cast<TCPConnector *>(&tx);
    connector->Close();
    return Message();
  }

  template <class Tx>
  Message TCPConnectorGood(ObjectMap &p) {
    Tx &tx = p[kStrObject].Cast<Tx>();
    TCPConnector *connector = dynamic_cast<TCPConnector *>(&tx);
    return Message(util::MakeBoolean(connector->Good()));
  }

  template <class Tx>
  Message WSockInfoResultCode(ObjectMap &p) {
    Tx &tx = p[kStrObject].Cast<Tx>();
    WSockInfo *info = dynamic_cast<WSockInfo *>(&tx);
    return Message(to_string(info->GetLastResultCode()));
  }
}
#endif
