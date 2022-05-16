#ifndef REST_API_H_
#define REST_API_H_

#include <curl/curl.h>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/scoped_thread.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/chrono.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <string>
#include <memory>
#include <thread>
#include <vector>
#include <queue>
#include <iostream>
#include "boost/thread/thread.hpp"
#include  "boost/bind.hpp"
namespace rest_curl {

  enum class ErrCode {
    OK = 0, REQ_MALFORMED, SEND_ERROR
  };

  // for debug purposes
  inline const char* c_str(const ErrCode &code) {
    switch (code) {
      case ErrCode::OK:
        return "rest_curl::ErrCode::OK";
      case ErrCode::REQ_MALFORMED:
        return "rest_curl::ErrCode::REQ_MALFORMED";
      case ErrCode::SEND_ERROR:
        return "rest_curl::ErrCode::SEND_ERROR";
      default:
        return "rest_curl::ErrCode::UNKNOWN";
    }
  }

  struct CurlResponse {
    std::string data;
    long http_code;
  };

  struct RESTCall {
    enum class Type {
      POST, PUT
    };
    std::string msg;
    std::string url;
    std::string userpasswd;
    Type type;

    // optional
    boost::function<void (const CurlResponse &)> success_cb;
    boost::function<void (rest_curl::ErrCode, const std::string &)> err_cb;
    boost::function<bool (long , const CurlResponse &)> err_check_cb;

    RESTCall() {}

    RESTCall(const std::string &_msg, const std::string &_url, const std::string &_userpasswd)
      : msg(_msg), url(_url), userpasswd(_userpasswd) {}

    RESTCall(Type cmd_type, const std::string &_msg, const std::string &_url, const std::string &_userpasswd,
      boost::function<void (const CurlResponse &)> _success_cb,
      boost::function<void (rest_curl::ErrCode, const std::string &)> _err_cb,
      boost::function<bool (long , const CurlResponse &)>_err_check_cb)
      : type(cmd_type), msg(_msg), url(_url), userpasswd(_userpasswd) {
        success_cb = _success_cb;
        err_cb = _err_cb;
        err_check_cb = _err_check_cb;
      }
    
    RESTCall(const std::string &_msg, const std::string &_url, const std::string &_userpasswd,
      boost::function<void (const CurlResponse &)> _success_cb,
      boost::function<void (rest_curl::ErrCode, const std::string &)> _err_cb,
      boost::function<bool (long , const CurlResponse &)> _err_check_cb)
      : msg(_msg), url(_url), userpasswd(_userpasswd) {
        success_cb = _success_cb;
        err_cb = _err_cb;
        err_check_cb = _err_check_cb;
        type = Type::POST;
      }
  };

  /** Only one instance of this class (threadsafe)**/
  class RESTApi {
    public:
           static RESTApi& getInstance() {
             static RESTApi instance;
             return instance;
           }
           ~RESTApi();
           RESTApi(RESTApi const&) = delete;
           void operator=(RESTApi const&) = delete;

           /**
            * @brief Function to be called by user to send a REST request
            */
           void send(const RESTCall &call);

    private:
           RESTApi();

           /**
            * @brief Thread to handle all network requests (Consumer)
            */
           void handle_network(const std::string &thread_name);

           /**
            * @brief Delegate functions to check and returns true if request have error
            */
           inline bool is_error(boost::function<bool (long responseCode, const CurlResponse &)> fn, long responseCode, const CurlResponse &reply);

           /**
            * @brief Delegate functions to handle request failure callbacks with checking
            */
           inline void handle_error(boost::function<void (rest_curl::ErrCode, const std::string &)> fn,
               ErrCode code, const std::string &reply=std::string());

           /**
            * @brief Delegate functions to handle request success with checking
            */
           inline void handle_success(boost::function<void (const CurlResponse &)> fn,
               const CurlResponse &reply);
           /**
            * @brief Helper function to create appropriate headers for REST calls
            */
           curl_slist* format_headers(const RESTCall &call);

           /**
            * @brief Helper functions to set all the options needed for CURL request
            */
           CURLcode set_options(const RESTCall& call, const curl_slist* headers, CURL* curl, CurlResponse *p_rsp);

           const int MAX_RETRIES = 5;

           /** Thread safety related variables **/
           std::unique_ptr<boost::thread> mt_;
           boost::mutex cl_mutex_;
           boost::condition_variable q_cond_;
           std::queue<RESTCall> call_queue_;

  };

}

#endif