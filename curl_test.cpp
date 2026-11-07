
#include <iostream>
#include <curl/curl.h>

int main()
{
    CURL *curl = curl_easy_init();

    if (curl)
    {
        std::cout << "libcurl initialized successfully!" << std::endl;
        curl_easy_cleanup(curl);
        return 0;
    }

    std::cout << "libcurl initialization failed!" << std::endl;
    return 1;
}
