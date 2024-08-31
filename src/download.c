#include "math.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <curl/curl.h>

// Include necessary headers

// class stuff
#include "libspm.h"
#include "cutils.h"

int download(char* url, FILE* fp)
{
    CURL *curl = curl_easy_init();
    if(curl)
    {
        CURLcode res;
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "CCCP/1.0 (https://www.sovietlinux.org/)");
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }
    return 0;
}