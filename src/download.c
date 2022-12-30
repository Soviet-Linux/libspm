#include "math.h"
#include <stdio.h>
#include <stdlib.h>
#include "string.h"

#include <curl/curl.h>

// class stuff
#include "libspm.h"
#include "utils.h"


int downloadRepo(const char* url_path,const char* file_path)
{
    for (int i = 0;i < REPO_COUNT;i++)
    {
        // get the url
        char* repo = REPOS[i];
        printf("REPOS[%d] is '%s'\n",i,repo);

        char* url = calloc(strlen(repo)+strlen(url_path)+8,sizeof(char));
        sprintf(url,"%s/%s",repo,url_path);

        msg(INFO, "Downloading %s", url);
        
        if (downloadFile(url,file_path) == 0)
        {
            free(url);
            return 0;
        }
        free(url);
    
    }
    return 1;
    
} 
int downloadFile(const char* url,const char* file_path)
{
    CURL *curl;
    FILE *fp;
    CURLcode res;
    curl = curl_easy_init();      
    dbg(3,"curl_easy_init() returned %p",curl);                                                                                                                                                                                                                                                     
    if (!curl)
    {
        msg(ERROR,"curl_easy_init() failed");
        return -1;  
    } 

    
    fp = fopen(file_path,"wb");
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
    // Internal CURL progressmeter must be disabled if we provide our own callback
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, false);
    // Install the callback function
    
    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    fclose(fp);
    printf("\n");

    if (res != CURLE_OK)
    {
        return -1;
    }
    return 0;

}


bool is_in_repo(CURL* session)
{
    CURLcode curl_code;
    curl_code = curl_easy_perform (session);
    long http_code = 0;
    curl_easy_getinfo (session, CURLINFO_RESPONSE_CODE, &http_code);
    if (http_code == 200 && curl_code != CURLE_ABORTED_BY_CALLBACK)
    {
        printf("found in repo\n");
        return true;
    }
    else
    {
        return false;
    }
}

