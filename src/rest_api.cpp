#include "../include/rest_api.h"
#include <stdexcept>

using namespace rest_curl;

/** C-style callbacks **/
static size_t curl_write_cb(void *data, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    //ROS_DEBUG("Reply from server %d", realsize);
    struct CurlResponse *mem = (struct CurlResponse *)userp;

    mem->data.append((const char*) data, realsize);
    return realsize;
}

/** Class method definitions **/
RESTApi::RESTApi() {
    //ROS_INFO("Creating REST API client singleton.");
    // setup global curl environment
    curl_global_init(CURL_GLOBAL_ALL);

    mt_ = std::unique_ptr<boost::thread>(new boost::thread(boost::bind(&RESTApi::handle_network, this, "NetworkThread")));
}

RESTApi::~RESTApi() {
    // /ROS_INFO("Successfully cleaned REST handlers. Exiting now...");
    mt_->interrupt();
    mt_->join();
    curl_global_cleanup();
}

void RESTApi::send(const RESTCall &call) {
    boost::mutex::scoped_lock lock(cl_mutex_);
    call_queue_.push(call);
    //ROS_DEBUG("Pushed new REST request");
    q_cond_.notify_one();
}

void RESTApi::handle_network(const std::string &thread_name) {
    //ROS_INFO("Starting %s", thread_name.c_str());
    bool found = false;
    RESTCall call;
    CurlResponse rsp;
    CURL* curl;
    try {
        while (true) {
            {
                boost::mutex::scoped_lock lock(cl_mutex_);
                while (call_queue_.empty()) {
                    q_cond_.wait(lock);
                }

                call = call_queue_.front();
                call_queue_.pop();
            }

            // process call
            curl = curl_easy_init();
            curl_slist* headers = format_headers(call);
            if (headers == NULL) {
                handle_error(call.err_cb, ErrCode::REQ_MALFORMED);
                continue;
            }
            set_options(call, headers, curl, &rsp);

            ErrCode err = ErrCode::OK;
            if(curl) {
                int retries = 0;
                while (retries++ < MAX_RETRIES) {
                    //ROS_DEBUG("Try: %d", retries);
                    //ROS_DEBUG("Sending JSON REST string %s to %s", call.msg.c_str(), call.url.c_str());
                    rsp.data.clear();
                    CURLcode res = curl_easy_perform(curl);

                    long responseCode;
                    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);

                    /* Check for errors */
                    if(res != CURLE_OK || rsp.data.size() == 0 || responseCode >= 400) {
                        if (is_error(call.err_check_cb, responseCode, rsp)) continue;
                        //ROS_ERROR("Error code received: %d", responseCode);
                        err = ErrCode::SEND_ERROR;
                    }

                    err = ErrCode::OK;
                    rsp.http_code = responseCode;
                    break;
                }

                if (retries > MAX_RETRIES) {
                    //ROS_ERROR("Failed to send after %d retries.", retries-1);
                    handle_error(call.err_cb, err, rsp.data);
                }

                curl_slist_free_all(headers);
                if (err == ErrCode::OK) {
                    handle_success(call.success_cb, rsp);
                }
                rsp.data.clear();
            } else handle_error(call.err_cb, ErrCode::REQ_MALFORMED);

            // always cleanup
            curl_easy_cleanup(curl);
        }
    } catch (boost::thread_interrupted&) {
        //ROS_ERROR("%s preempted. Exiting now...", thread_name.c_str());
    }
}

void RESTApi::handle_error(boost::function<void (rest_curl::ErrCode, const std::string &)> fn,
        ErrCode code, const std::string &reply) {
    if (fn) fn(code, reply);
}

void RESTApi::handle_success(boost::function<void (const CurlResponse &)> fn,
        const CurlResponse &reply) {
    if (fn) { fn(reply);}
}

bool RESTApi::is_error(boost::function<bool (long responseCode, const CurlResponse &)> fn, long responseCode, const CurlResponse &reply) {
    if (fn) return fn(responseCode, reply);
    return false;
}

curl_slist* RESTApi::format_headers(const RESTCall &call) {
    curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Accept: application/json");
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, "charset: utf-8");
    return headers;
}

CURLcode RESTApi::set_options(const RESTCall& call, const curl_slist* headers, CURL* curl, CurlResponse *rsp) {
    CURLcode res = CURLE_OK;

    res = (res == 0) ? curl_easy_setopt(curl, CURLOPT_URL, call.url.c_str()) : res;
    /* Now specify the POST data */
    // TODO: change this to make it use keys instead
    // TODO: handle error codes
    // Set this option VERBOSE to 1L to enable debug verbose mode
    res = (res == 0) ? curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L) : res;
    if (call.type == RESTCall::Type::POST) res = (res == 0) ? curl_easy_setopt(curl, CURLOPT_POST, 1L) : res;
    if (call.type == RESTCall::Type::PUT        ) res = (res == 0) ? curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT") : res;

    res = (res == 0) ? curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers) : res;
    res = (res == 0) ? curl_easy_setopt(curl, CURLOPT_USERPWD, call.userpasswd.c_str()) : res;
    res = (res == 0) ? curl_easy_setopt(curl, CURLOPT_POSTFIELDS, call.msg.c_str()) : res;

    res = (res == 0) ? curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_cb) : res;
    res = (res == 0) ? curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *) rsp) : res;
    res = (res == 0) ? curl_easy_setopt(curl, CURLOPT_TIMEOUT, 1L) : res;
    //res = (res == 0) ? curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L) : res;
    return res;
}

int main(){

    std::cout << "Inside the main function!" << std::endl;
    return 0;

}