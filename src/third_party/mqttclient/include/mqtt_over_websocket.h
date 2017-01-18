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
 * @brief mqttclient over websocket
 * @date 2017-01-10
 * @author neeker
 */

#ifndef __mqtt_over_websocket_h__
#define __mqtt_over_websocket_h__

#include "mqtt_client.h"
#include "base/single_thread_task_runner.h"
#include "url/gurl.h"
#include "url/origin.h"

namespace net {

class MQTTOverWebSocketClient : public MQTTClient {
public:

  virtual ~MQTTOverWebSocketClient() {}

  class Params : public MQTTClient::Params {
  public:
    Params() : MQTTClient::Params() {
      accept_language = "zh,zhCN";
      user_agent = "MQTT Client";
    }

    Params(const Params &o) : MQTTClient::Params(o) {
      *this = o;
    }

    Params &operator=(const Params & o) {
      ((MQTTClient::Params*)this)->operator=(o);

      return *this;
    }

    GURL websocket_url;

    url::Origin websocket_origin;

    std::string accept_language;

    std::string user_agent;

  };

  static scoped_refptr<MQTTOverWebSocketClient> Build(
    const MQTTOverWebSocketClient::Params &params,
    scoped_refptr<base::SingleThreadTaskRunner> task_runner = NULL);

};

}

#endif /** __mqtt_over_websocket_h__ */

