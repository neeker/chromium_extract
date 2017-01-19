/*********************************************
                   _ooOoo_
                  o8888888o
                  88" . "88
                  (| -_- |)
                  O\  =  /O
               ____/`---'\____
             .'  \\|     |//  `.
            /  \\|||  :  |||//  \
           /  _||||| -:- |||||-  \
           |   | \\\  -  /// |   |
           | \_|  ''\---/''  |   |
           \  .-\__  `-`  ___/-. /
         ___`. .'  /--.--\  `. . __
      ."" '<  `.___\_<|>_/___.'  >'"".
     | | :  `- \`.;`\ _ /`;.`/ - ` : | |
     \  \ `-.   \_ __\ /__ _/   .-` /  /
======`-.____`-.___\_____/___.-`____.-'======
                   `=---='

^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
           佛祖保佑       永无BUG
           心外无法       法外无心
           三宝弟子       飞猪宏愿
*********************************************/


#include "mqtt_over_websocket.h"
#include "base/threading/thread.h"
#include "base/run_loop.h"
#include "base/synchronization/waitable_event.h"
#include "base/lazy_instance.h"
#include "net/websockets/websocket_errors.h"
#include "net/url_request/url_request_context_builder.h"
#include "net/websockets/websocket_channel.h"
#include "net/websockets/websocket_handshake_request_info.h"
#include "net/websockets/websocket_handshake_response_info.h"
#include "net/http/http_response_headers.h"
#include "net/url_request/url_request_context.h"
#include "net/ssl/ssl_info.h"
#include "MQTTPacket.h"
#include <vector>

namespace net {

namespace {

class MQTTOverWebSocketEventInterface : public WebSocketEventInterface {
public:

  enum {
    PING_CHECKING = 1L << 0,
    CHANNEL_CONNECTED = 1L << 1,
    PUBLISHING_TOPIC = 1L << 2,
  };

  struct SubscribeTopicItem : public base::RefCounted<SubscribeTopicItem> {

    SubscribeTopicItem(const std::string &n, int q) : msgid(0), topic_name(n), qos(q) {}

    SubscribeTopicItem(const SubscribeTopicItem &o) {
      *this = o;
    }

    SubscribeTopicItem &operator=(const SubscribeTopicItem&o) {
      topic_name = o.topic_name;
      qos = o.qos;
      return *this;
    }

    uint16_t msgid;

    std::string topic_name;

    int qos;

    friend bool operator<(const SubscribeTopicItem &l, const SubscribeTopicItem &r) {
      return l.topic_name < r.topic_name;
    }

    bool operator==(const SubscribeTopicItem& o) {
      return topic_name.compare(o.topic_name) == 0;
    }

  };

  struct PublishTopicItem : public base::RefCounted<PublishTopicItem> {
  public:

    PublishTopicItem(const std::string &n,
      const std::string &p,
      const std::vector<char> &d,
      int q = 0,
      bool r = false) :
      msgid(0),
      topic_name(n),
      data(d),
      qos(q),
      retain(r),
      dup(0),
      send_times(0),
      pid_(p) {
    }

    PublishTopicItem(const PublishTopicItem& o) {
      *this = o;
    }

    PublishTopicItem &operator=(const PublishTopicItem &o) {
      pid_ = o.pid_;
      msgid = o.msgid;
      topic_name = o.topic_name;
      data = o.data;
      qos = o.qos;
      retain = o.retain;
      dup = o.dup;
      send_times = o.send_times;
      return *this;
    }

    void clearSession() {
      msgid = 0;
      dup = 0;
      send_times = 0;
    }

    uint16_t msgid;

    std::string topic_name;

    std::vector<char> data;

    int qos;

    bool retain;

    uint8_t dup;

    size_t send_times;

    const std::string &pid() const {
      return pid_;
    }

  private:

    std::string pid_;

  };

  class ParamsGetter {
  public:
    virtual bool get_channel(WebSocketChannel **channel) const = 0;

    virtual const std::string &get_username() const = 0;

    virtual const std::string &get_password() const = 0;

    virtual const std::string &get_client_id() const = 0;

    virtual size_t get_alive_check_internal() const = 0;

    virtual uint8_t get_mqtt_version() const = 0;

    virtual bool get_clear_session() const = 0;

    virtual scoped_refptr<SubscribeTopicItem> take_queued_subscribe_topic() = 0;

    virtual void return_queued_subscribe_topic(scoped_refptr<SubscribeTopicItem> item) = 0;

    virtual scoped_refptr<PublishTopicItem> get_queued_publish_topic_front() = 0;

    virtual size_t get_queued_publish_topic_count() const = 0;

    virtual bool is_running() const = 0;

  };

  class Observer {
  public:

    virtual void OnChannelConnect() = 0;

    virtual void OnMQTTConnect() = 0;

    virtual void OnChannelDisConnect() = 0;

    virtual void OnSubscribeTopicSubmit(scoped_refptr<SubscribeTopicItem> item) = 0;

    virtual void OnTopicUnSubscribed(scoped_refptr<SubscribeTopicItem> item) = 0;

    virtual void OnSubscribeTopicFinish(int qos) = 0;

    virtual void OnTopicMessageServerReceived(uint16_t msgid, bool *handled) = 0;

    virtual void OnTopicMessagePublished(uint16_t msgid, int qos) = 0;

    virtual void OnRemoteMessageArrived(const std::string &topic_name,
      uint16_t msgid, uint8_t *data, size_t datasiz,
      int qos, bool *handled) = 0;

    virtual void OnRemoteMessageServerReply(uint16_t msgid, bool *handled) = 0;

  };

  typedef WebSocketEventInterface::ChannelState ChannelState;

  MQTTOverWebSocketEventInterface() :
    flags_(0), task_runner_(base::ThreadTaskRunnerHandle::Get()) {
  }

  ~MQTTOverWebSocketEventInterface() {}

  ChannelState OnDataFrame(
    bool fin,
    WebSocketMessageType type,
    const std::vector<char>& data) override {

    ChannelState channel_state = ChannelState::CHANNEL_ALIVE;
    uint8_t packet_type, dup;
    uint16_t msgid;

    int rc = MQTTDeserialize_ack(&packet_type, &dup, &msgid, (uint8_t*)&data[0], data.size());
    switch (packet_type) {
    case CONNACK:
      channel_state = OnMQTTConnectACK(data);
      break;
    case SUBACK:
      channel_state = OnMQTTSubscribeACK(data);
      break;
    case PUBLISH:
      channel_state = OnMQTTPublish(data);
      break;
    case PUBACK:
      channel_state = OnMQTTPublishACK(data);
      break;
    case PUBREC:
      channel_state = OnMQTTPublishReceived(data);
      break;
    case PUBREL:
      channel_state = OnMQTTPublishReceivedReply(data);
      break;
    case PUBCOMP:
      channel_state = OnMQTTPublishReceivedComplete(data);
      break;
    case PINGREQ:
      channel_state = OnMQTTPing();
      break;
    case PINGRESP:
      channel_state = OnMQTTPingACK();
      break;
    case UNSUBACK:
      channel_state = OnMQTTUnSubscribeACK(data);
      break;
    case DISCONNECT:
      channel_state = OnMQTTDisconnect();
      break;
    default:
      break;
    }

    if (channel_state != ChannelState::CHANNEL_ALIVE) {
      task_runner_->PostTask(FROM_HERE, base::Bind(&MQTTOverWebSocketEventInterface::OnMQTTConnectAbort, base::Unretained(this)));
      return channel_state;
    }

    ResetMQTTCheckAlive();
    channel_state = PublishQueuedTopics();
    if (channel_state != ChannelState::CHANNEL_ALIVE) {
      task_runner_->PostTask(FROM_HERE, base::Bind(&MQTTOverWebSocketEventInterface::OnMQTTConnectAbort, base::Unretained(this)));
    }
    ReadMQTTDatas(data.size());
    return channel_state;
  }

  void ResetMQTTCheckAlive() {
    SetPingCheckingFlagOn(false);
    if (alive_check_timer_) {
      alive_check_timer_->Reset();
    } else {
      alive_check_timer_.reset(new base::Timer(FROM_HERE,
        base::TimeDelta::FromSeconds(mqtt_params_getter_->get_alive_check_internal()),
        base::Bind(&MQTTOverWebSocketEventInterface::CheckMQTTAlive, base::Unretained(this)), true));
      alive_check_timer_->SetTaskRunner(task_runner_);
    }
  }

  void StopMQTTCheckAlive() {
    if (alive_check_timer_) {
      alive_check_timer_->Stop();
    }
  }


  //Qos1 for client publish topic service side reply PUBLISH_ACK
  ChannelState OnMQTTPublishACK(const std::vector<char>& data) {
    uint8_t packet_type, dup;
    uint16_t msgid;
    int rc = MQTTDeserialize_ack(&packet_type, &dup, &msgid, (uint8_t*)&data[0], data.size());
    if (rc == 0) return ChannelState::CHANNEL_DELETED;
    if (IsPublishingTopicFlagOn()) {
      SetPublishingTopicFlagOn(false);
    }
    observer_->OnTopicMessagePublished(msgid, 1);
    return ChannelState::CHANNEL_ALIVE;
  }

  //QoS2 for client publish topic 
  //server --->PUBREC---> client
  //server <---PUBREL<--- client
  ChannelState OnMQTTPublishReceived(const std::vector<char>& data) {
    uint8_t packet_type, dup;
    uint16_t msgid;
    int rc = MQTTDeserialize_ack(&packet_type, &dup, &msgid, (uint8_t*)&data[0], data.size());
    if (rc == 0) return ChannelState::CHANNEL_DELETED;

    bool handled = false;
    observer_->OnTopicMessageServerReceived(msgid, &handled);
    if (handled == false) {
      return ChannelState::CHANNEL_ALIVE;
    }
    uint8_t outbuf[128];
    rc = MQTTSerialize_pubrel(outbuf, 128, 0, msgid);
    return SendMQTTDataFrame(std::vector<char>(&outbuf[0], &outbuf[rc]));
  }

  //QoS2 for client publish topic 
  //server --->PUBCOMP---> client
  ChannelState OnMQTTPublishReceivedComplete(const std::vector<char>& data) {
    uint8_t packet_type, dup;
    uint16_t msgid;
    int rc = MQTTDeserialize_ack(&packet_type, &dup, &msgid, (uint8_t*)&data[0], data.size());
    if (IsPublishingTopicFlagOn()) {
      SetPublishingTopicFlagOn(false);
    }
    observer_->OnTopicMessagePublished(msgid, 2);
    return ChannelState::CHANNEL_ALIVE;
  }

  //for server publish topic
  ChannelState OnMQTTPublish(const std::vector<char>& data) {
    unsigned char dup;
    int qos;
    unsigned char retained;
    unsigned short msgid;
    int payloadlen_in;
    unsigned char* payload_in;
    int rc;
    MQTTString received_topic;
    rc = MQTTDeserialize_publish(&dup, &qos, &retained, &msgid, &received_topic, &payload_in, &payloadlen_in, (uint8_t*)&data[0], data.size());

    bool handled = false;
    observer_->OnRemoteMessageArrived(
      std::string(received_topic.lenstring.data, (std::string::size_type)received_topic.lenstring.len),
      msgid, payload_in, payloadlen_in, qos, &handled);

    if (qos == 0) return ChannelState::CHANNEL_ALIVE;

    if (!handled) {
      return ChannelState::CHANNEL_ALIVE;
    }

    if (qos == 1) {
      uint8_t outbuf[128];
      int rc = MQTTSerialize_puback(outbuf, 128, msgid);
      return SendMQTTDataFrame(std::vector<char>(&outbuf[0], &outbuf[rc]));
    }
    if (qos == 2) {
      uint8_t outbuf[128];
      int rc = MQTTSerialize_ack(outbuf, 128, PUBREC, 0, msgid);
      return SendMQTTDataFrame(std::vector<char>(&outbuf[0], &outbuf[rc]));
    }
    LOG(ERROR) << L"服务端发过来的MQTT消息的Qos参数错误: Qos=" << qos;
    return ChannelState::CHANNEL_DELETED;
  }

  //QoS2 for server publish topic 
  //server --->PUBREL---> client
  //server <---PUBCOMP<--- client
  ChannelState OnMQTTPublishReceivedReply(const std::vector<char>& data) {
    uint8_t packet_type, dup;
    uint16_t msgid;
    int rc = MQTTDeserialize_ack(&packet_type, &dup, &msgid, (uint8_t*)&data[0], data.size());
    if (rc == 0) return ChannelState::CHANNEL_DELETED;

    bool handled = false;
    observer_->OnRemoteMessageServerReply(msgid, &handled);
    if (handled == false) return ChannelState::CHANNEL_ALIVE;

    uint8_t outbuf[128];
    rc = MQTTSerialize_pubcomp(outbuf, 128, msgid);
    return SendMQTTDataFrame(std::vector<char>(&outbuf[0], &outbuf[rc]));
  }

  ChannelState OnMQTTUnSubscribeACK(const std::vector<char>& data) {
    uint16_t msgid = 0;
    MQTTDeserialize_unsuback(&msgid, (uint8_t*)&data[0], data.size());
    return ChannelState::CHANNEL_ALIVE;
  }

  ChannelState OnMQTTDisconnect() {
    WebSocketChannel *channel = NULL;
    if (!mqtt_params_getter_->get_channel(&channel)) return ChannelState::CHANNEL_DELETED;
    channel->StartClosingHandshake(kWebSocketNormalClosure, "");
    return ChannelState::CHANNEL_ALIVE;
  }

  ChannelState OnMQTTConnectACK(const std::vector<char>& data) {
    ChannelState channel_state = ChannelState::CHANNEL_ALIVE;
    unsigned char session_present, connack_rc;
    if (MQTTDeserialize_connack(&session_present, &connack_rc, (unsigned char *)&data[0], data.size()) != 1 || !!connack_rc) {
      channel_state = ChannelState::CHANNEL_DELETED;
    } else {
      observer_->OnMQTTConnect();
      channel_state = SubscribeTopics();
    }
    return channel_state;
  }

  bool IsPingCheckingFlagOn() {
    return (flags_ & PING_CHECKING) != 0;
  }

  void SetPingCheckingFlagOn(bool val) {
    if (val) {
      flags_ |= PING_CHECKING;
    } else {
      flags_ &= ~PING_CHECKING;
    }
  }

  bool IsChannelConnectedFlagOn() {
    return (flags_ & CHANNEL_CONNECTED) != 0;
  }

  void SetChannelConnectedFlagOn(bool val) {
    if (val) {
      flags_ |= CHANNEL_CONNECTED;
    } else {
      flags_ &= ~CHANNEL_CONNECTED;
    }
  }

  bool IsPublishingTopicFlagOn() {
    return (flags_ & PUBLISHING_TOPIC) != 0;
  }

  void SetPublishingTopicFlagOn(bool val) {
    if (val) {
      flags_ |= PUBLISHING_TOPIC;
    } else {
      flags_ &= ~PUBLISHING_TOPIC;
    }
  }

  void CheckMQTTAlive() {
    if (IsPingCheckingFlagOn()) {
      OnMQTTConnectAbort();
    } else if (IsPublishingTopicFlagOn()) {
      if (PublishQueuedTopics() != ChannelState::CHANNEL_ALIVE) {
        OnMQTTConnectAbort();
      }
    } else {
      SetPingCheckingFlagOn(true);
      if (SendMQTTPing() != ChannelState::CHANNEL_ALIVE) {
        OnMQTTConnectAbort();
      }
    }
  }

  void OnMQTTConnectAbort() {
    observer_->OnChannelDisConnect();
  }

  ChannelState OnMQTTPing() {
    uint8_t data[128];
    int rc = 0;
    rc = MQTTSerialize_ack(data, 128, PINGRESP, 0, 0);
    return SendMQTTDataFrame(std::vector<char>(&data[0], &data[rc]));
  }

  ChannelState OnMQTTPingACK() {
    return ChannelState::CHANNEL_ALIVE;
  }

  ChannelState SendMQTTDataFrame(const std::vector<char> &data) {
    WebSocketChannel *channel = NULL;
    if (!mqtt_params_getter_->get_channel(&channel)) return ChannelState::CHANNEL_DELETED;
    return channel->SendFrame(true, WebSocketFrameHeader::kOpCodeBinary, data);
  }

  ChannelState SendMQTTPing() {
    uint8_t data[128];
    int rc = MQTTSerialize_pingreq(data, 128);
    return SendMQTTDataFrame(std::vector<char>(&data[0], &data[rc]));
  }

  ChannelState OnMQTTSubscribeACK(const std::vector<char>& data) {
    unsigned short submsgid;
    int subcount;
    int granted_qos[10];
    int rc = MQTTDeserialize_suback(&submsgid, 10, &subcount, &granted_qos[0], (uint8_t*)&data[0], data.size());
    if (!rc) {
      LOG(ERROR) << L"无法解码MQTT订阅应答包!";
      return ChannelState::CHANNEL_DELETED;
    }
    for (int i = 0; i < subcount; i++) {
      observer_->OnSubscribeTopicFinish(granted_qos[i]);
    }
    return ChannelState::CHANNEL_ALIVE;
  }

  ChannelState ConnectMQTT() {
    MQTTPacket_connectData conn_data = MQTTPacket_connectData_initializer;
    std::string username = mqtt_params_getter_->get_username();
    std::string password = mqtt_params_getter_->get_password();
    std::string clientid = mqtt_params_getter_->get_client_id();
    conn_data.MQTTVersion = mqtt_params_getter_->get_mqtt_version();
    if (mqtt_params_getter_->get_clear_session()) {
      conn_data.cleansession = 1;
    } else {
      conn_data.cleansession = 0;
    }

    conn_data.clientID.cstring = &clientid[0];
    conn_data.username.cstring = &username[0];
    conn_data.password.cstring = &password[0];
    conn_data.keepAliveInterval = mqtt_params_getter_->get_alive_check_internal();
    uint8_t conn_outbuf[1024];
    int rc = MQTTSerialize_connect(conn_outbuf, 1024, &conn_data);
    return SendMQTTDataFrame(std::vector<char>(&conn_outbuf[0], &conn_outbuf[rc]));
  }

  void DisConnectMQTT() {
    uint8_t buf[128];
    int bufsiz = MQTTSerialize_disconnect(buf, 128);
    WebSocketChannel *channel = NULL;
    if (SendMQTTDataFrame(std::vector<char>(&buf[0], &buf[bufsiz])) != ChannelState::CHANNEL_ALIVE) {
      OnMQTTConnectAbort();
    } else if (!mqtt_params_getter_->get_channel(&channel)) {
      OnMQTTConnectAbort();
    } else {
      channel->StartClosingHandshake(kWebSocketNormalClosure, "");
    }
  }

  ChannelState SubscribeTopics() {
    do {
      scoped_refptr<SubscribeTopicItem> subscribe_topic = mqtt_params_getter_->take_queued_subscribe_topic();
      if (!subscribe_topic) break;
      if (SubscribeTopic(subscribe_topic) != ChannelState::CHANNEL_ALIVE) {
        mqtt_params_getter_->return_queued_subscribe_topic(subscribe_topic);
        return ChannelState::CHANNEL_DELETED;
      }
    } while (true);
    return ChannelState::CHANNEL_ALIVE;
  }

  ChannelState SubscribeTopic(scoped_refptr<SubscribeTopicItem> item) {
    uint8_t subdata[1024];
    MQTTString topic_name = MQTTString_initializer;
    int req_qos = item->qos;
    topic_name.cstring = (char *)item->topic_name.c_str();
    int sublen = MQTTSerialize_subscribe(subdata, 1024, 0, item->msgid, 1, &topic_name, &req_qos);
    if (SendMQTTDataFrame(std::vector<char>(&subdata[0], &subdata[sublen])) != ChannelState::CHANNEL_ALIVE) {
      return ChannelState::CHANNEL_DELETED;
    }
    observer_->OnSubscribeTopicSubmit(item);
    return ChannelState::CHANNEL_ALIVE;
  }

  ChannelState UnSubscribeTopic(scoped_refptr<SubscribeTopicItem> item) {
    uint8_t subdata[1024];
    MQTTString topic_name = MQTTString_initializer;
    topic_name.cstring = (char *)item->topic_name.c_str();
    int sublen = MQTTSerialize_unsubscribe(subdata, 1024, 0, item->msgid, 1, &topic_name);
    if (SendMQTTDataFrame(std::vector<char>(&subdata[0], &subdata[sublen])) != ChannelState::CHANNEL_ALIVE) {
      return ChannelState::CHANNEL_DELETED;
    }
    observer_->OnTopicUnSubscribed(item);
    return ChannelState::CHANNEL_ALIVE;
  }

  ChannelState PublishTopic(scoped_refptr<PublishTopicItem> item) {
    std::vector<char> outbuf(item->data.size() + 32);
    MQTTString mqtt_topic_name = MQTTString_initializer;
    mqtt_topic_name.cstring = (char*)item->topic_name.c_str();
    int rc = MQTTSerialize_publish((uint8_t*)&outbuf[0], outbuf.size(),
      item->dup, item->qos, item->retain ? 1 : 0,
      item->msgid, mqtt_topic_name,
      (uint8_t*)&item->data[0], item->data.size());
    return SendMQTTDataFrame(std::vector<char>(&outbuf[0], &outbuf[rc]));
  }

  void ReadMQTTDatas(int64_t val) {
    WebSocketChannel *channel = NULL;
    if (!mqtt_params_getter_->get_channel(&channel)) return;
    channel->SendFlowControl(val);
  }

  ChannelState PublishQueuedTopics() {
    ChannelState channel_state;
    if (IsPublishingTopicFlagOn() || mqtt_params_getter_->get_queued_publish_topic_count() == 0)
      return ChannelState::CHANNEL_ALIVE;
    scoped_refptr<PublishTopicItem> item = mqtt_params_getter_->get_queued_publish_topic_front();
    SetPublishingTopicFlagOn(true);
    channel_state = PublishTopic(item);
    if (channel_state == ChannelState::CHANNEL_ALIVE) {
      item->send_times++;
      if (item->qos == 0) {
        SetPublishingTopicFlagOn(false);
        observer_->OnTopicMessagePublished(item->msgid, 0);
      }
    } else {
      SetPublishingTopicFlagOn(false);
    }
    return channel_state;
  }

  //Begin implements WebSocketChannelEventInterface
  ChannelState OnAddChannelResponse(
    const std::string& selected_subprotocol,
    const std::string& extensions) override {
    if (!mqtt_params_getter_->is_running()) {
      OnMQTTConnectAbort();
      return ChannelState::CHANNEL_DELETED;
    }
    SetChannelConnectedFlagOn(true);
    return ChannelState::CHANNEL_ALIVE;
  }

  ChannelState OnFlowControl(int64_t quota) override {
    if (!mqtt_params_getter_->is_running()) {
      OnMQTTConnectAbort();
      return ChannelState::CHANNEL_DELETED;
    }
    channel_quota_ = quota;
    return ChannelState::CHANNEL_ALIVE;
  }

  ChannelState OnClosingHandshake() override {
    return ChannelState::CHANNEL_DELETED;
  }

  ChannelState OnDropChannel(bool was_clean,
    uint16_t code,
    const std::string& reason) override {
    StopMQTTCheckAlive();
    SetChannelConnectedFlagOn(false);
    observer_->OnChannelDisConnect();
    return ChannelState::CHANNEL_DELETED;
  }

  ChannelState OnFailChannel(const std::string& message) override {
    observer_->OnChannelDisConnect();
    return ChannelState::CHANNEL_DELETED;
  }

  ChannelState OnStartOpeningHandshake(
    scoped_ptr<WebSocketHandshakeRequestInfo> request) override {
    if (mqtt_params_getter_->is_running())
      return ChannelState::CHANNEL_ALIVE;
    OnMQTTConnectAbort();
    return ChannelState::CHANNEL_DELETED;
  }

  ChannelState OnFinishOpeningHandshake(
    scoped_ptr<WebSocketHandshakeResponseInfo> response) override {
    if (!mqtt_params_getter_->is_running()) {
      OnMQTTConnectAbort();
      return ChannelState::CHANNEL_ALIVE;
    }
    ChannelState ret = MQTTOverWebSocketEventInterface::ConnectMQTT();
    ResetMQTTCheckAlive();
    ReadMQTTDatas(channel_quota_);
    return ret;
  }

  ChannelState OnSSLCertificateError(
    scoped_ptr<SSLErrorCallbacks> ssl_error_callbacks,
    const GURL& url,
    const SSLInfo& ssl_info,
    bool fatal) {
    if (mqtt_params_getter_->is_running())
      return ChannelState::CHANNEL_ALIVE;
    OnMQTTConnectAbort();
    return ChannelState::CHANNEL_DELETED;
  }

  //End implements WebSocketChannelEventInterface

  void SetMQTTParamsGetter(ParamsGetter *val) {
    mqtt_params_getter_ = val;
  }

  void SetObserver(Observer *val) {
    observer_ = val;
  }

private:
  uint32_t flags_;
  int64_t channel_quota_;
  scoped_ptr<base::Timer> alive_check_timer_;
  scoped_refptr<base::SingleThreadTaskRunner> task_runner_;
  ParamsGetter *mqtt_params_getter_;
  Observer *observer_;
};


class MQTTOverWebSocketThreadHolder {
public:

  MQTTOverWebSocketThreadHolder() {
    InitWebSocketThread();
  }

  ~MQTTOverWebSocketThreadHolder() {
    StopWebSocketThread();
  }

  static scoped_refptr<base::SingleThreadTaskRunner> GetTaskRunner();

protected:

  scoped_refptr<base::SingleThreadTaskRunner> task_runner() {
    return task_runner_;
  }

  void InitWebSocketThread() {
    if (!task_runner_) {
      base::Thread::Options thread_ops;
      thread_ops.message_loop_type = base::MessageLoop::TYPE_IO;
      task_thread_.reset(new base::Thread("MQTT over WebSocket"));
      task_thread_->StartWithOptions(thread_ops);
      task_runner_ = task_thread_->task_runner();
    }
  }

  void StopWebSocketThread() {
    task_runner_->PostTask(FROM_HERE, base::MessageLoop::QuitWhenIdleClosure());
    task_thread_->Stop();
  }

private:
  scoped_refptr<base::SingleThreadTaskRunner> task_runner_;
  scoped_ptr<base::Thread> task_thread_;
};

base::LazyInstance<MQTTOverWebSocketThreadHolder> websocket_thread_holder;

scoped_refptr<base::SingleThreadTaskRunner> MQTTOverWebSocketThreadHolder::GetTaskRunner() {
  return websocket_thread_holder.Get().task_runner();
}

class MQTTOverWebSocketClientImpl : public MQTTOverWebSocketEventInterface::ParamsGetter,
  public MQTTOverWebSocketEventInterface::Observer,
  public MQTTOverWebSocketClient {
public:

  typedef WebSocketEventInterface::ChannelState ChannelState;

  typedef struct MQTTOverWebSocketEventInterface::SubscribeTopicItem SubscribeTopicItem;

  typedef struct MQTTOverWebSocketEventInterface::PublishTopicItem PublishTopicItem;

  MQTTOverWebSocketClientImpl(GURL socket_url, url::Origin origin) :
    mqtt_version_(MQTTV311),
    flags_(0),
    connected_count_(0),
    publish_count_(0),
    reconnect_delay_seconds_(30),
    keep_alive_interval_(30),
    clear_session_(true),
    publish_retry_max_(3),
    msgid_(1),
    shutdown_event_(true, false),
    socket_url_(socket_url),
    origin_(origin),
    mqtt_websaocket_evt_(NULL) {
  }

  MQTTOverWebSocketClientImpl() :
    mqtt_version_(MQTTV311),
    flags_(0),
    connected_count_(0),
    publish_count_(0),
    reconnect_delay_seconds_(30),
    keep_alive_interval_(30),
    clear_session_(true),
    publish_retry_max_(3),
    msgid_(1),
    shutdown_event_(true, false),
    mqtt_websaocket_evt_(NULL) {
  }


  virtual ~MQTTOverWebSocketClientImpl() {
    Shutdown();
    WaitShutdown();
  }

  bool Startup() {
    if ((flags_ & CLIENT_STARTUP) != 0) return false;
    flags_ = CLIENT_STARTUP;
    return CreateStartWebSocketChannel();
  }

  void Shutdown() {
    if (flags_ == 0 || (flags_ & CLIENT_SHUTDOWN) != 0) return;
    if (!task_runner_->RunsTasksOnCurrentThread()) {
      task_runner_->PostTask(FROM_HERE, base::Bind(&MQTTOverWebSocketClientImpl::Shutdown, this));
      return;
    }
    if (flags_ == 0 || (flags_ & CLIENT_SHUTDOWN) != 0) return;
    flags_ |= CLIENT_SHUTDOWN;
    if ((flags_ & CLIENT_CONNECTED) != 0) {
      mqtt_websaocket_evt_->DisConnectMQTT();
    } else if (websocket_channel_) {
      websocket_channel_->StartClosingHandshake(kWebSocketNormalClosure, "");
    }
  }

  void WaitShutdown() {
    if (flags_ == 0) return;
    shutdown_event_.Wait();
  }

  bool TimeWaitShutdown(const base::TimeDelta &max_time) {
    if (flags_ == 0) return true;
    return shutdown_event_.TimedWait(max_time);
  }

  bool IsRunning() const override {
    return (flags_ & CLIENT_SHUTDOWN) == 0 &&
      (flags_ & CLIENT_STARTUP);
  }

  void set_websocket_url(const GURL &url) {
    socket_url_ = url;
  }

  void set_websocket_origin(const url::Origin &origin) {
    origin_ = origin;
  }

  void SubscribeTopic(const std::string &val, int qos = 0) override {
    if (!task_runner_) {
      if (subscribe_topic_names_.find(val) != subscribe_topic_names_.end()) return;
      subscribe_topic_names_.insert(val);
      queued_subscribe_topics_.push(make_scoped_refptr(new SubscribeTopicItem(val, qos)));
      return;
    }
    if (!task_runner_->RunsTasksOnCurrentThread()) {
      task_runner_->PostTask(FROM_HERE, base::Bind(&MQTTOverWebSocketClientImpl::SubscribeTopic,
        this, val, qos));
      return;
    }
    if (subscribe_topic_names_.find(val) != subscribe_topic_names_.end()) return;
    subscribe_topic_names_.insert(val);
    scoped_refptr<SubscribeTopicItem> item = make_scoped_refptr(new SubscribeTopicItem(val, qos));
    if ((flags_ & CLIENT_CONNECTED) != 0) {
      if (mqtt_websaocket_evt_->SubscribeTopic(item) != ChannelState::CHANNEL_ALIVE) {
        queued_subscribe_topics_.push(item);
      }
    } else {
      queued_subscribe_topics_.push(item);
    }
  }

  void UnSubscribeTopic(const std::string &topic_name) override {
    if (!task_runner_) return;
    if (!task_runner_->RunsTasksOnCurrentThread()) {
      task_runner_->PostTask(FROM_HERE, base::Bind(&MQTTOverWebSocketClientImpl::UnSubscribeTopic,
        this, topic_name));
      return;
    }
    if (subscribe_topic_names_.find(topic_name) == subscribe_topic_names_.end()) return;
    std::queue< scoped_refptr<SubscribeTopicItem> > tmp_queued;
    queued_subscribe_topics_.swap(tmp_queued);

    scoped_refptr<SubscribeTopicItem> unitem;

    while (tmp_queued.size()) {
      scoped_refptr<SubscribeTopicItem> item = tmp_queued.front();
      tmp_queued.pop();
      if (item->topic_name.compare(topic_name) == 0) {
        unitem = item;
        continue;
      }
      queued_subscribe_topics_.push(item);
    }

    subscribe_topic_names_.erase(topic_name);

    if (unitem) {
      OnTopicUnSubscribed(unitem);
    } else {
      std::map < std::string, scoped_refptr<SubscribeTopicItem> >::iterator uit = subscribed_topics_.find(topic_name);
      if (uit != subscribed_topics_.end()) {
        unitem = uit->second;
        subscribed_topics_.erase(uit);
        if ((flags_ & CLIENT_CONNECTED) != 0) {
          unitem->msgid = generate_msgid();
          if (mqtt_websaocket_evt_->UnSubscribeTopic(unitem) != ChannelState::CHANNEL_ALIVE) {
            OnTopicUnSubscribed(unitem);
          }
        } else {
          OnTopicUnSubscribed(unitem);
        }
      }
    }
  }

  void PublishMessage(const std::string &val,
    const std::string &pid,
    const std::vector<char> &data,
    int qos = 0, bool retain = false) override {
    if (!task_runner_) {
      queued_publish_topics_.push(make_scoped_refptr(new PublishTopicItem(val, pid, data, qos, retain)));
      return;
    }
    if (!task_runner_->RunsTasksOnCurrentThread()) {
      task_runner_->PostTask(FROM_HERE, base::Bind(&MQTTOverWebSocketClientImpl::PublishMessage,
        this, val, pid, data, qos, retain));
      return;
    }
    queued_publish_topics_.push(make_scoped_refptr(new PublishTopicItem(val, pid, data, qos, retain)));
    if ((flags_ & CLIENT_CONNECTED) != 0 && !mqtt_websaocket_evt_->IsPublishingTopicFlagOn()) {
      if (ChannelState::CHANNEL_ALIVE != mqtt_websaocket_evt_->PublishQueuedTopics()) {
        OnChannelDisConnect();
      }
    }
  }

  void set_accept_language(const std::string &val) {
    accept_language_ = val;
  }

  void set_user_agent(const std::string &val) {
    user_agent_ = val;
  }

  void set_mqtt_version(MQTTVersion val) {
    mqtt_version_ = val;
  }

  void set_reconnect_delay_seconds(size_t val) {
    reconnect_delay_seconds_ = val;
  }

  void set_keep_alive_interval(size_t val) {
    keep_alive_interval_ = val;
  }

  void set_client_id(const std::string &val) {
    client_id_ = val;
  }

  void set_username(const std::string &val) {
    username_ = val;
  }

  void set_password(const std::string &val) {
    password_ = val;
  }

  void set_clear_session(bool val) {
    clear_session_ = val;
  }

  void set_publish_retry_max(size_t val) {
    publish_retry_max_ = val;
  }

  void SetObserver(MQTTOverWebSocketClient::Observer *val) override {
    observer_ = val;
  }

  //implements ParamsGetter
  bool get_channel(WebSocketChannel **channel) const override {
    *channel = websocket_channel_.get();
    return !!websocket_channel_;
  }

  size_t get_alive_check_internal() const override {
    return keep_alive_interval_;
  }

  const std::string &get_username() const override {
    return username_;
  }

  const std::string &get_password() const override {
    return password_;
  }

  const std::string &get_client_id() const override {
    return client_id_;
  }

  uint8_t get_mqtt_version() const override {
    return (uint8_t)mqtt_version_;
  }

  scoped_refptr<SubscribeTopicItem> take_queued_subscribe_topic() override {
    if (queued_subscribe_topics_.size() == 0) return scoped_refptr<SubscribeTopicItem>();
    scoped_refptr<SubscribeTopicItem> result = queued_subscribe_topics_.front();
    result->msgid = generate_msgid();
    queued_subscribe_topics_.pop();
    return result;
  }

  void return_queued_subscribe_topic(scoped_refptr<SubscribeTopicItem> item) override {
    queued_subscribe_topics_.push(item);
  }

  bool get_clear_session() const override {
    return clear_session_;
  }

  void set_task_runner(scoped_refptr<base::SingleThreadTaskRunner> val) {
    task_runner_ = val;
  }

  scoped_refptr<PublishTopicItem> get_queued_publish_topic_front() override {
    scoped_refptr<PublishTopicItem> item;
  re_take:
    if (queued_publish_topics_.size() == 0) return item;
    item = queued_publish_topics_.front();
    if (item->msgid == 0) {
      item->msgid = generate_msgid();
    } else {
      item->dup = 1;
      if (item->send_times >= publish_retry_max_) {
        queued_publish_topics_.pop();
        observer_->OnPublishMessageOccurError(PublishError::ERROR_PUBISH_MAXRETRY,
          item->topic_name, item->pid(), item->data, item->qos, item->retain);
        item = NULL;
        goto re_take;
      }
    }
    last_publish_time_ = base::Time::Now();
    return item;
  }

  size_t get_queued_publish_topic_count() const override {
    return queued_publish_topics_.size();
  }

  bool is_running() const override {
    return IsRunning();
  }

  //Implement Observer
  void OnChannelConnect() override {
    flags_ |= CLIENT_CONNECTING;
  }

  void OnMQTTConnect() override {
    flags_ &= ~CLIENT_CONNECTING;
    flags_ |= CLIENT_SESSION;
    flags_ |= CLIENT_CONNECTED;
    connected_count_++;
    observer_->OnStateChanged(MQTT_RemoteConnected);
  }

  void OnChannelDisConnect() override {
    if ((flags_ & CLIENT_DISCONNECTING) != 0) return;
    flags_ &= ~CLIENT_CONNECTED;
    flags_ |= CLIENT_DISCONNECTING;
    observer_->OnStateChanged(MQTT_RemoteDisConnected);
    task_runner_->PostTask(FROM_HERE, base::Bind(&MQTTOverWebSocketClientImpl::ReleaseClientContext, this));
  }

  void OnTopicUnSubscribed(scoped_refptr<SubscribeTopicItem> item) {
    observer_->OnTopicUnSubscribed(item->topic_name);
  }

  void OnSubscribeTopicFinish(int qos) override {
    DCHECK(subscribing_topics_.size());
    scoped_refptr<SubscribeTopicItem> item = subscribing_topics_.front();
    item->qos = qos;
    subscribing_topics_.pop();
    if (subscribe_topic_names_.find(item->topic_name) == subscribe_topic_names_.end()) {
      item->msgid = generate_msgid();
      mqtt_websaocket_evt_->UnSubscribeTopic(item);
    } else {
      subscribed_topics_[item->topic_name] = item;
    }
    observer_->OnTopicSubscribed(item->topic_name, item->qos);
  }

  void OnSubscribeTopicSubmit(scoped_refptr<SubscribeTopicItem> item) override {
    subscribing_topics_.push(item);
  }

  void OnTopicMessagePublished(uint16_t msgid, int qos) override {
    if (queued_publish_topics_.size() == 0) {
      LOG(ERROR) << L"收到发布完成应答(QoS=" << qos << L"),但是发送队列里没有数据!!!";
      return;
    }

    scoped_refptr<PublishTopicItem> item = queued_publish_topics_.front();
    if (item->msgid == msgid) {
      publish_delta_total_ += base::Time::Now() - last_publish_time_;
      publish_count_++;

      queued_publish_topics_.pop();
      observer_->OnPublishMessageFinalFinished(item->topic_name, item->pid());
    } else {
      LOG(ERROR) << L"收到发布完成应答(QoS=" << qos << L"),但是发送队列头消息ID="
        << item->msgid << L"与应带消息ID=" << msgid << L"不一致!!!";
    }
  }

  void OnTopicMessageServerReceived(uint16_t msgid, bool *handled) override {
    if (queued_publish_topics_.size() == 0) {
      LOG(ERROR) << L"收到发布已接收(PUBLISH_REC, QoS=2),但是发送队列里没有数据!!!";
      return;
    }
    scoped_refptr<PublishTopicItem> item = queued_publish_topics_.front();
    if (item->msgid == msgid) {
      observer_->OnPublishMessageServerReceived(item->topic_name, item->pid(), handled);
    } else {
      LOG(ERROR) << L"收到发布已接收(PUBLISH_REC, QoS=2),但是发送队列头消息ID="
        << item->msgid << L"与应带消息ID=" << msgid << L"不一致!!!";
    }
  }

  void OnRemoteMessageArrived(const std::string &topic_name,
    uint16_t msgid, uint8_t *data, size_t datasiz,
    int qos, bool *handled) {
    observer_->OnRemoteMessageArrivaled(topic_name, data, datasiz, msgid, qos, handled);
  }

  void OnRemoteMessageServerReply(uint16_t msgid, bool *handled) override {
    observer_->OnRemoteMessageServerReply(msgid, handled);
  }

protected:

  uint16_t generate_msgid() {
    uint16_t ret = msgid_++;
    if (ret == 0) ret = msgid_++;
    return ret;
  }

  bool CreateStartWebSocketChannel() {
    if (!task_runner_) {
      task_runner_ = MQTTOverWebSocketThreadHolder::GetTaskRunner();
    }
    return task_runner_->PostTask(FROM_HERE, base::Bind(
      &MQTTOverWebSocketClientImpl::CreateStartWebSocketChannelInThread, this));
  }

  void CreateStartWebSocketChannelInThread() {
    requested_protocols_.clear();
    if (MQTTV31 == mqtt_version_ || MQTTV311 == mqtt_version_) {
      requested_protocols_.push_back("mqttv3.1");
    } else {
      CHECK(false);
    }

    doconnect_count_++;
    flags_ |= CLIENT_CONNECTING;

    URLRequestContextBuilder url_reqctx_builder;
    url_reqctx_builder.set_accept_language(accept_language_);
    url_reqctx_builder.set_user_agent(user_agent_);
    if (!proxy_config_service_) {
      proxy_config_service_ = ProxyService::CreateSystemProxyConfigService(
        task_runner_,
        task_runner_);
      url_reqctx_builder.set_proxy_config_service(std::move(proxy_config_service_));
    }

    url_reqctx_builder.SetFileTaskRunner(task_runner_);

    //复制已提交的订阅到排队发送
    while (subscribing_topics_.size() > 0) {
      queued_subscribe_topics_.push(subscribing_topics_.front());
      subscribing_topics_.pop();
    }

    //清理消息会话
    if (clear_session_) {
      if (queued_publish_topics_.size() > 0) {
        std::queue< scoped_refptr<PublishTopicItem> > tmp_publish_topics;
        while (queued_publish_topics_.size() > 0) {
          scoped_refptr<PublishTopicItem> item = queued_publish_topics_.front();
          queued_publish_topics_.pop();
          item->clearSession();
          tmp_publish_topics.push(item);
        }
        queued_publish_topics_ = tmp_publish_topics;
      }
      for (auto sit : subscribed_topics_) {
        queued_subscribe_topics_.push(sit.second);
      }
      subscribed_topics_.clear();
    }

    url_request_context_ = url_reqctx_builder.Build();
    websocket_channel_.reset(new WebSocketChannel(MakeWebSocketEventInterface(), url_request_context_.get()));
    websocket_channel_->SendAddChannelRequest(socket_url_, requested_protocols_, origin_);
    observer_->OnStateChanged(MQTT_ClientStartup);
  }

  scoped_ptr<WebSocketEventInterface> MakeWebSocketEventInterface() {
    scoped_ptr<MQTTOverWebSocketEventInterface> result(new MQTTOverWebSocketEventInterface());
    result->SetObserver(this);
    result->SetMQTTParamsGetter(this);
    mqtt_websaocket_evt_ = result.release();
    return make_scoped_ptr<WebSocketEventInterface>(mqtt_websaocket_evt_);
  }

  void ReleaseClientContext() {
    websocket_channel_.reset();
    url_request_context_.reset();
    if ((flags_ & CLIENT_SHUTDOWN) != 0) {
      while (queued_publish_topics_.size()) {
        scoped_refptr<PublishTopicItem> item = queued_publish_topics_.front();
        queued_publish_topics_.pop();
        observer_->OnPublishMessageOccurError(PublishError::ERROR_PUBLISH_CANCEL,
          item->topic_name, item->pid(), item->data, item->qos, item->retain);
      }
      observer_->OnStateChanged(MQTT_ClientShutdown);
      flags_ &= ~CLIENT_CONNECTING;
      flags_ &= ~CLIENT_CONNECTED;
      flags_ &= ~CLIENT_DISCONNECTING;
      flags_ &= ~CLIENT_SESSION;
      shutdown_event_.Signal();
    } else {
      flags_ &= ~CLIENT_DISCONNECTING;
      flags_ |= CLIENT_CONNECTING;
    }
    task_runner_->PostDelayedTask(FROM_HERE, base::Bind(&MQTTOverWebSocketClientImpl::CreateStartWebSocketChannelInThread, this),
      base::TimeDelta::FromSeconds(reconnect_delay_seconds_));
  }

private:

  enum {
    CLIENT_STARTUP = 1L << 0,
    CLIENT_CONNECTING = 1L << 1,
    CLIENT_CONNECTED = 1L << 2,
    CLIENT_DISCONNECTING = 1L << 3,
    CLIENT_SHUTDOWN = 1L << 4,
    CLIENT_SESSION = 1L << 5,
  };

  MQTTVersion mqtt_version_;

  uint32_t flags_;

  //已成功连接次数
  size_t connected_count_;

  //尝试连接次数
  size_t doconnect_count_;

  //最后发布数据时间
  base::Time last_publish_time_;

  //总发布数量
  size_t publish_count_;

  //平均发布事件
  base::TimeDelta publish_delta_total_;

  //平均发布最小时间
  base::TimeDelta publish_delta_min_;

  //平均发布最大时间
  base::TimeDelta publish_delta_max_;

  //重连间隔(秒)
  size_t reconnect_delay_seconds_;

  //keepalive间隔(秒)
  size_t keep_alive_interval_;

  //客户端CLIENTID
  std::string client_id_;

  //用户名
  std::string username_;

  //密码
  std::string password_;

  //断开是否清理会话
  bool clear_session_;

  //发送最大重试次数
  size_t publish_retry_max_;

  //内部消息id
  uint16_t msgid_;

  //停止事件
  base::WaitableEvent shutdown_event_;

  //WebSocket地址
  GURL socket_url_;

  //WebSocket来源
  url::Origin origin_;

  //语言
  std::string accept_language_;

  //UserAgent
  std::string user_agent_;

  //执行任何运行器
  scoped_refptr<base::SingleThreadTaskRunner> task_runner_;

  //WebSocket通道
  scoped_ptr<WebSocketChannel> websocket_channel_;

  //URL上下文
  scoped_ptr<URLRequestContext> url_request_context_;

  //mqtt over websocket事件处理接口
  MQTTOverWebSocketEventInterface *mqtt_websaocket_evt_;

  //请求协议(MQTT头)
  std::vector<std::string> requested_protocols_;

  //代理配置服务
  scoped_ptr<ProxyConfigService> proxy_config_service_;

  //已订阅的主题名称
  std::set<std::string> subscribe_topic_names_;

  //排队中的订阅主题
  std::queue< scoped_refptr<SubscribeTopicItem> > queued_subscribe_topics_;

  //已订阅的主题
  std::map<std::string, scoped_refptr<SubscribeTopicItem> > subscribed_topics_;

  //向服务器发送订阅包的主题
  std::queue< scoped_refptr<SubscribeTopicItem> > subscribing_topics_;

  //排队发送的消息
  std::queue< scoped_refptr<PublishTopicItem> > queued_publish_topics_;

  //观察对象
  MQTTOverWebSocketClient::Observer *observer_;

};

}

scoped_refptr<MQTTOverWebSocketClient> MQTTOverWebSocketClient::Build(
  const MQTTOverWebSocketClient::Params &params,
  scoped_refptr<base::SingleThreadTaskRunner> task_runner) {
  MQTTOverWebSocketClientImpl *mqttclient = new MQTTOverWebSocketClientImpl();
  mqttclient->set_mqtt_version(params.mqtt_version);
  mqttclient->set_reconnect_delay_seconds(params.reconnect_delay_seconds);
  mqttclient->set_publish_retry_max(params.publish_retry_max);
  mqttclient->set_keep_alive_interval(params.keep_alive_interval);
  mqttclient->set_client_id(params.client_id);
  mqttclient->set_username(params.username);
  mqttclient->set_password(params.password);
  mqttclient->set_clear_session(params.clear_session);
  mqttclient->set_websocket_url(params.websocket_url);
  mqttclient->set_websocket_origin(params.websocket_origin);
  mqttclient->set_accept_language(params.accept_language);
  mqttclient->set_user_agent(params.user_agent);
  mqttclient->set_task_runner(task_runner);
  return make_scoped_refptr<MQTTOverWebSocketClient>(mqttclient);
}

}

