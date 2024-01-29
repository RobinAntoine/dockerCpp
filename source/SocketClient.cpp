#include "SocketClient.hpp"

#include <curl/curl.h>
#include <string>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>

SocketClient* SocketClient::m_instance = nullptr;

/*! \brief Get the SocketClient instance.
    \param[in] url The url of the docker unix socket.
    \return The SocketClient instance.
*/
SocketClient* SocketClient::getInstance(const std::string& url)
{
    if(url.empty()) {
        throw std::runtime_error("The url of the docker unix socket is empty.");
    }
    if(m_instance == nullptr) {
        m_instance = new SocketClient(url);
    }
    return m_instance;
}

/*! \brief Constructor of SocketClient class.
    \param[in] url The url of the docker unix socket.
*/
SocketClient::SocketClient(const std::string& url)
{
    curl_global_init(CURL_GLOBAL_ALL);
    m_url = url;
}

/*! \brief Destructor of SocketClient class.
*/
SocketClient::~SocketClient()
{
    curl_global_cleanup();
}

/*! \brief The callback function used by libcurl to store the response of the HTTP request.
    \param[in] contents The response of the HTTP request.
    \param[in] size The size of the response of the HTTP request.
    \param[in] nmemb The size of the response of the HTTP request.
    \param[in] userp The response of the HTTP request.
    \return The size of the response of the HTTP request.
*/
size_t SocketClient::write_callback(void *contents, size_t size, size_t nmemb, void *userp)
{
  size_t realsize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)userp;

  mem->memory = (char*)realloc(mem->memory, mem->size + realsize + 1);
  if(mem->memory == NULL) {
    /* out of memory! */
    printf("not enough memory (realloc returned NULL)\n");
    return 0;
  }

  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;

  return realsize;
}

/*! \brief Send a HTTP GET request to the docker unix socket.
    \param[in] path The path of the HTTP request.
    \param[out] response The response of the HTTP request.
    \return The HTTP response code.
*/
long SocketClient::get(const std::string& path, std::string& response, bool isDel)
{
    CURL* curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_UNIX_SOCKET_PATH, m_url.c_str());
    }
    if(curl == nullptr) {
        throw std::runtime_error("The curl instance is null.");
    }
    if(path.empty()) {
        throw std::runtime_error("The path of the HTTP request is empty.");
    }
    if(!response.empty()) {
        throw std::runtime_error("The response of the HTTP request is not empty.");
    }
    struct MemoryStruct chunk;
    chunk.memory = (char*)malloc(1);
    chunk.size = 0;

    struct curl_slist* headers = NULL;
    curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    if(isDel){
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
    }else{
        curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
    }

    curl_easy_setopt(curl, CURLOPT_URL, path.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

    CURLcode res = curl_easy_perform(curl);
    if(res != CURLE_OK) {
        throw std::runtime_error("curl_easy_perform() failed: " + std::string(curl_easy_strerror(res)));
    }
    response.append(std::string(chunk.memory,chunk.size));
    curl_slist_free_all(headers);
    free(chunk.memory);
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

    curl_easy_cleanup(curl);

    return http_code;
}

/*! \brief The callback function used by libcurl to read input data.
    \param[in] dest The response of the HTTP request.
    \param[in] size The size of the response of the HTTP request.
    \param[in] nmemb The size of the response of the HTTP request.
    \param[in] userp The response of the HTTP request.
    \return The size of the response of the HTTP request.
*/
size_t SocketClient::read_callback(char *dest, size_t size, size_t nmemb, void *userp)
{
  struct WriteThis *wt = (struct WriteThis *)userp;
  size_t buffer_size = size*nmemb;

  if(wt->sizeleft) {
    /* copy as much as possible from the source to the destination */
    size_t copy_this_much = wt->sizeleft;
    if(copy_this_much > buffer_size)
      copy_this_much = buffer_size;
    std::cout << "Data Lenght : " << copy_this_much <<std::endl;
    memcpy(dest, wt->readptr, copy_this_much);
 
    wt->readptr += copy_this_much;
    wt->sizeleft -= copy_this_much;

    std::cout << "Data Lenght : " << copy_this_much <<std::endl;
    std::cout << "Data : " << dest <<std::endl;
    
    return copy_this_much; /* we copied this many bytes */
  }
 
  return 0; /* no more data left to deliver */
}

/*! \brief Send a HTTP POST request to the docker unix socket.
    \param[in] path The path of the HTTP request.
    \param[in] data The data of the HTTP request.
    \param[out] response The response of the HTTP request.
    \return The HTTP response code.
*/
long SocketClient::post(const std::string& path, const std::string& data, std::string& response)
{

    CURL* curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_UNIX_SOCKET_PATH, m_url.c_str());
    }

    if(curl == nullptr) {
        throw std::runtime_error("The curl instance is null.");
    }
    if(path.empty()) {
        throw std::runtime_error("The path of the HTTP request is empty.");
    }
    if(data.empty()) {
        throw std::runtime_error("The data of the HTTP request is empty.");
    }
    if(!response.empty()) {
        throw std::runtime_error("The response of the HTTP request is not empty.");
    }
    struct WriteThis chunk;
    chunk.readptr = data.c_str();
    chunk.sizeleft = data.size();

    struct MemoryStruct chunkanswer;
    chunkanswer.memory = (char*)malloc(1);
    chunkanswer.size = 0;
    
    /* First set the URL that is about to receive our POST. */
    curl_easy_setopt(curl, CURLOPT_URL, path.c_str());
 
    /* Now specify we want to POST data */
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
 
    /* we want to use our own read function */
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);
 
    /* pointer to pass to our read function */
    curl_easy_setopt(curl, CURLOPT_READDATA, &chunk);
 
    /* get verbose debug output please */
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunkanswer);

    CURLcode res = curl_easy_perform(curl);
    if(res != CURLE_OK) {
        throw std::runtime_error("curl_easy_perform() failed: " + std::string(curl_easy_strerror(res)));
    }
    curl_slist_free_all(headers);
    response.append(std::string(chunkanswer.memory,chunkanswer.size));
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    curl_easy_cleanup(curl);
    return http_code;
}

/*! \brief Send a HTTP DELETE request to the docker unix socket.
    \param[in] path The path of the HTTP request.
    \param[out] response The response of the HTTP request.
    \return The HTTP response code.
*/
long SocketClient::del(const std::string& path, std::string& response){
    return this->get(path,response,true);
}