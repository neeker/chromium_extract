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

/**
 * @brief mqttclient
 * @date 2017-01-10
 * @author neeker
 */

#ifndef __mqtt_client_h__
#define __mqtt_client_h__

#include "base/memory/ref_counted.h"
#include "base/time/time.h"
#include <string>
#include <vector>

namespace net {

//MQTT客户端
class MQTTClient : public base::RefCounted<MQTTClient> {
public:

  //QoS级别
  enum QoSLevel {
    Qos0,
    Qos1,
    Qos2
  };

  //MQTT版本常量
  enum MQTTVersion {
    MQTTV31 = 3,
    MQTTV311 = 4,
  };

  class Params {
  public:

    Params() :
      mqtt_version(MQTTVersion::MQTTV311),
      reconnect_delay_seconds(30),
      keep_alive_interval(30),
      clear_session(true),
      publish_retry_max(3) {
    }

    Params(const Params& o) {
      *this = o;
    }

    Params& operator=(const Params& o) {
      mqtt_version = o.mqtt_version;
      reconnect_delay_seconds = o.reconnect_delay_seconds;
      keep_alive_interval = o.keep_alive_interval;
      client_id = o.client_id;
      username = o.username;
      password = o.password;
      clear_session = o.clear_session;
      publish_retry_max = o.publish_retry_max;
      return *this;
    }

    //设置MQTT版本
    MQTTVersion mqtt_version;

    //设置重连延迟秒数
    size_t reconnect_delay_seconds;

    //设置KeepAlive间隔
    size_t keep_alive_interval;

    //设置客户端ID
    std::string client_id;

    //设置用户名
    std::string username;

    //设置用户密码
    std::string password;

    //设置退出是否清除会话
    bool clear_session;

    //设置发布最大重试次数
    size_t publish_retry_max;

  };

  virtual ~MQTTClient() {}

  //启动
  virtual bool Startup() = 0;

  //停止
  virtual void Shutdown() = 0;

  //等待停止
  virtual void WaitShutdown() = 0;

  //是否正在运行
  virtual bool IsRunning() const = 0;

  //等待停止(超时返回false)
  virtual bool TimeWaitShutdown(const base::TimeDelta &max_time) = 0;

  //订阅主题
  virtual void SubscribeTopic(const std::string &topic_name, int qos = 0) = 0;

  //取消订阅主题
  virtual void UnSubscribeTopic(const std::string &topic_name) = 0;

  //发布主题, pid为消息唯一编号
  virtual void PublishMessage(const std::string &topic_name,
    const std::string &pid, const std::vector<char> &data,
    int qos = 0, bool retain = false) = 0;

  //发布消息错误
  enum PublishError {

    //达到最大重试次数
    ERROR_PUBISH_MAXRETRY = 8,

    //已被取消(客户端被停止时)
    ERROR_PUBLISH_CANCEL = 9,

  };

  //客户端状态
  enum ClientState {

    //已启动
    MQTT_ClientStartup,

    //连接中
    MQTT_RemoteConnecting,

    //无法连接
    MQTT_RemoteConnectError,

    //已连接
    MQTT_RemoteConnected,

    //已断开
    MQTT_RemoteDisConnected,

    //已停止
    MQTT_ClientShutdown,

  };

  class Observer {
  public:

    //已启动
    virtual void OnStateChanged(int state) = 0;

    //已订阅了主题
    virtual void OnTopicSubscribed(const std::string &topic_name, int qos) = 0;

    //已解订了主题
    virtual void OnTopicUnSubscribed(const std::string &topic_name) = 0;

    //服务端已收到消息(QoS=2)时, handled=true表示继续
    virtual void OnPublishMessageServerReceived(const std::string &topic_name, const std::string &pid, bool *handled) = 0;

    //消息已发布, PID表示传入的消息pid
    virtual void OnPublishMessageFinalFinished(const std::string &topic_name, const std::string &pid) = 0;

    //消息到达，返回*handle=true表示已处理, qos=2时，需要在OnMessageArrivaledServerReply处理msgid的消息
    virtual void OnRemoteMessageArrivaled(const std::string &topic_name, uint8_t *data, size_t datasiz,
      int msgid, int qos, bool *handled) = 0;

    //消息到达后服务端应答返回(QoS=2)才会
    virtual void OnRemoteMessageServerReply(int msgid, bool *handled) = 0;

    //消息发布错误(重试最大次数之后或停止后还有未发完的)
    virtual void OnPublishMessageOccurError(int err,
      const std::string &topic_name,
      const std::string &pid, const std::vector<char> &data,
      int qos, bool retain) = 0;

    virtual void OnIdle() = 0;

  };

  virtual void SetObserver(Observer *val) = 0;

};

}

#endif /** __mqtt_client_h__ */

