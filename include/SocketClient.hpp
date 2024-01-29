#pragma once

#include <curl/curl.h>
#include <string>

/*! \class SocketClient
    \brief SocketClient class is used to send HTTP requests to a docker unix socket using libcurl.
*/
class SocketClient
{
    /*! \struct MemoryStruct
        \brief MemoryStruct struct is used to store the response of the HTTP request.
    */
    struct MemoryStruct {
        //! \brief The response of the HTTP request.
        char *memory;
        //! \brief The size of the response of the HTTP request.
        size_t size;
    };

    /*! \struct WriteThis
        \brief WriteThis struct is used to store the data of the HTTP request.
    */
    struct WriteThis {
        const char *readptr;
        size_t sizeleft;
    };      
    private:
        /*! \brief Constructor of SocketClient class.
            \param[in] url The url of the docker unix socket.
        */
        SocketClient(const std::string& url);

        /*! \brief Destructor of SocketClient class.
        */
        ~SocketClient();
    
    public:

        /*! \brief Get the SocketClient instance.
            \param[in] url The url of the docker unix socket.
            \return The SocketClient instance.
        */
        static SocketClient* getInstance(const std::string& url);

        /*! \brief Send a HTTP GET request to the docker unix socket.
            \param[in] path The path of the HTTP request.
            \param[out] response The response of the HTTP request.
            \param[in] isdel This is use to specify if we want to sen DELETE request instead GET
            \return The HTTP response code.
        */
        long get(const std::string& path, std::string& response, bool isDel=false);

        /*! \brief Send a HTTP POST request to the docker unix socket.
            \param[in] path The path of the HTTP request.
            \param[in] data The data of the HTTP request.
            \param[out] response The response of the HTTP request.
            \return The HTTP response code.
        */
        long post(const std::string& path, const std::string& data, std::string& response);

        /*! \brief Send a HTTP DELETE request to the docker unix socket.
            \param[in] path The path of the HTTP request.
            \param[out] response The response of the HTTP request.
            \return The HTTP response code.
        */
        long del(const std::string& path, std::string& response);

    private:
        /*! \brief the callback function use by curl to write output data
            \param[in] contents The response of the HTTP request.
            \param[in] size The size of the response of the HTTP request.
            \param[in] nmemb The size of the response of the HTTP request.
            \param[inout] userp The response of the HTTP request.
            \return The size of the response of the HTTP request.
        */
        static size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp);

        /*! \brief The callback function used by libcurl to read the data to send in the HTTP request.
            \param[in] ptr The data to send in the HTTP request.
            \param[in] size The size of the data to send in the HTTP request.
            \param[in] nmemb The size of the data to send in the HTTP request.
            \param[inout] userp The data to send in the HTTP request.
            \return The size of the data to send in the HTTP request.
        */
        static size_t read_callback(char *dest, size_t size, size_t nmemb, void *userp);

        /*! \brief The url of the docker unix socket.
        */
        std::string m_url;

        /*! \brief The instance of SocketClient.
        */
        static SocketClient* m_instance;
};
