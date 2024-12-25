#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "unistd.h"
#include "malloc.h"

#include "../../include/libspm.h"
#include "../../include/cutils.h"

#include "../../include/hashtable.h"
#include <stdio.h>

#define uint unsigned int


/*
If you are currently trying to understand what i did below , 
you need o kno that im very proud of it
*/

typedef struct ecmp_section {
    char *name;
    char *buff; 
}section;


unsigned int getsections(char* path,section*** sectionq);

unsigned int parseinfo(char* s, struct package* dest);
unsigned int parseraw(char* s, char** dest);
unsigned int parsenl(char* s,char*** dest);

hashtable* hm;
hashtable* infohm;

#ifdef STATIC
int open_ecmp(char* path,struct package* pkg)
#else
int open(char* path,struct package* pkg)
#endif
{
	if (access(path,F_OK) != 0) {
		msg(ERROR, "File not found");
        return 1;
	}
	
	void* parsers[][3] = {
        {parseinfo,pkg,NULL},

        {parseraw,&pkg->info.install,NULL},
        {parseraw,&pkg->info.prepare,NULL},
        {parseraw,&pkg->info.special,NULL},

        {parsenl,&pkg->files,&pkg->filesCount},
        {parsenl,&pkg->dependencies,&pkg->dependenciesCount},
        {parsenl,&pkg->optional,&pkg->optionalCount},

        {parsenl,&pkg->locations,&pkg->locationsCount},
		{parseraw,&pkg->description,NULL},
		{parsenl,&pkg->config,&pkg->configCount}
    };

    void* pairs[][2] = {
        {"info",parsers[0]},

        {"install",parsers[1]},
        {"prepare",parsers[2]},
        {"special",parsers[3]},

        {"files",parsers[4]},
        {"dependencies",parsers[5]},
        {"optional",parsers[6]},

        {"locations",parsers[7]},
		{"description",parsers[8]},
		{"config",parsers[9]},
        {NULL,NULL}
    };

	void* infodict[][2] = {
		// This is very stupid, but basically I assume that the name was obtained from the database
		// This is to go around a memory leak caused by overwriting name when opening a package
		// This is very stupid
		{"name",&pkg->name},
		{"version",&pkg->version},
		{"type",&pkg->type},
		{"url",&pkg->url},
		{"license",&pkg->license},
		{"environment",&pkg->environment},
		{NULL,NULL}
	};
	hm = hm_init(pairs,sizeof(pairs)/sizeof(pairs[0]));
	infohm = hm_init(infodict,sizeof(infodict)/sizeof(infodict[0]));



	unsigned int (*parser)(char* s,void* dest);

	section** sections;
	uint count = getsections(path,&sections);

	for (unsigned int i = 0; i < count; i++) {
		void** options = hm_get(hm,sections[i]->name);
		if (options == NULL) {
			msg(FATAL,"Unknown section : %s",sections[i]->name);
			free(sections[i]->buff);
			continue;
		}
		parser = options[0];
		
		if (parser != NULL) {
			int ret = parser(sections[i]->buff,options[1]);
			if (options[2] != NULL) {
				*(int*)options[2] = ret;
			}

		}
		else {
			msg(FATAL,"Unknown parser for section : %s",sections[i]->name);
		}
	}
	dbg(2,"done parsing | returning");

	// free sections
	for (unsigned int i = 0; i < count; i++) {
		free(sections[i]->name);
		free(sections[i]);
	}
	free(sections);

    hm_destroy(hm);
    hm_destroy(infohm);

	return 0;
}




unsigned int parsenl(char* s,char*** dest)
{
	char* str;
	// the parseraw below is useless but i'll keep since in case
	parseraw(s,&str);
	return splita(str,'\n',dest);
}
unsigned int parseraw(char* s, char** dest)
{	
	// So here we are doing something weird
	// We suppose that our `s` string is dynamically allocated and that it wont be arbitarily freed
	// So we are just going to copy the pointer to it
	// In the last version , we were copying the string to a new buffer
	// Because the `s` string was a buffer that was going to be freed by `getline()`
	*dest = s;
	return strlen(s);
}

unsigned int parseinfo(char *s, struct package* dest) {
    (void)dest;
    char* p = s;
    while (*p != '\0') {
        if (*p == ' ') {
            unsigned int index = p - s; 
            // ^ this could be a long, but it doesn't need to be 64 bits
            dbg(3, "Removing space '%c']'%c'['%c' at %p (index=%d)", *(p - 1), *p, *(p + 1), p, index);
            popchar(s, index);
            p--;
        }
        p++;
    }

    // split with nl
    char** nlist;
    int count = parsenl(s, &nlist);

    dbg(3, "count : %d", count);
    for (int i = 0; i < count; i++) {
        char* key = strtok(nlist[i], "=");
        char* value = strtok(NULL, "=");
        if (key == NULL || value == NULL) {
            msg(FATAL, "Invalid key-value pair: '%s'", nlist[i]);
            continue;
        }

        // add to corresponding value in dict
        char** destbuff = hm_get(infohm, key);
		
		if(strcmp(key, "name") == 0)
		{
			// This is very stupid
			continue;
		}
        if (destbuff == NULL) {
            msg(FATAL, "Unknown key : '%s'", key);
            continue;
        }

        *destbuff = strdup(value);
        if (*destbuff == NULL) {
            msg(ERROR, "Error allocating memory for %s value", key);
            free(nlist);
            free(s);
            return 0;
        }
        dbg(3, "Setting destbuff to %p - %s", *destbuff, *destbuff);
    }

    free(nlist);
    free(s);
    return 0;
}


unsigned int getsections(char* path,section*** sections) {
	FILE* fp = fopen(path,"r");
	char* line = NULL;
	size_t len = 0;
	ssize_t read;
	*sections = calloc(16,sizeof(section));
	unsigned int sectionsalloc = 256;
	(void)sectionsalloc;
	unsigned int sectionscount = 0;

	section* current = NULL;
	(void)current;
	unsigned int alloc = 0;

	while ((read = getline(&line,&len,fp)) != EOF) 
	{
		if (line[0] == '#' || line[0] == '\n' || strlen(line) < 2) {
			continue;
		}
		if (line[0] == '[') {
			section *sec = calloc(1,sizeof(section));
			sec->name = strdup(strtok(line,"[]"));

			sec->buff = calloc(256,sizeof(char));
			dbg(3,"allocating %d bytes at %p for section %s",256,sec->buff,sec->name);
			(*sections)[sectionscount++] = sec;

			alloc = 256;
			current = sec;

			continue;
		}
		unsigned int bufflen = strlen((*sections)[sectionscount-1]->buff);
		int linelen = strlen(line);
		if ( bufflen + linelen + 2 >= alloc) {
			alloc += strlen(line) + 64;
			(*sections)[sectionscount-1]->buff = realloc((*sections)[sectionscount-1]->buff,alloc);
		}
		strcat((*sections)[sectionscount-1]->buff,line);
	}
	free(line);
	return sectionscount;
}

#ifdef STATIC
int create_ecmp(const char* path,struct package* pkg)
#else
int create(const char* path,struct package* pkg)
#endif
{

	// i love hashmaps but here we'll use maparray
	// we have the list[0] = section and list[1] = function to do stuff
	void* list[][3] = {
		{"prepare",pkg->info.prepare,NULL},
		{"install",pkg->info.install,NULL},
		{"special",pkg->info.special,NULL},

		{"dependencies",pkg->dependencies,&pkg->dependenciesCount},
		{"optional",pkg->optional,&pkg->optionalCount},
		{"description",pkg->description,NULL},		

		{"locations",pkg->locations,&pkg->locationsCount},
	};

	FILE* ecmp = fopen(path,"w");
    if (ecmp == NULL) {
        msg(FATAL,"Cannot open file %s",path);
    }

    dbg(3,"Writing info to %s",path);
	// print info
	fprintf(ecmp,"[info]\n");

	// add  NULL check before each line
	if (pkg->name != NULL) fprintf(ecmp,"name = %s\n",pkg->name);
	if (pkg->version != NULL) fprintf(ecmp,"version = %s\n",pkg->version);
	if (pkg->type != NULL) fprintf(ecmp,"type = %s\n",pkg->type);
	if (pkg->license != NULL) fprintf(ecmp,"license = %s\n",pkg->license);
	if (pkg->url != NULL) fprintf(ecmp,"url = %s\n",pkg->url);
	fprintf(ecmp,"\n"); // for improved readability

	for (unsigned int i = 0;i < sizeof(list) / sizeof(list[0]);i++ )
	{
		if (list[i][1] == NULL) {
			continue;
		}

		//printf("%d - [%s]\n",i,(char*)list[i][0]);
		if (list[i][2] == NULL) {

			dbg(2,"[%s] -> %p",(char*)list[i][0],(char*)list[i][0]);
			fprintf(ecmp,"[%s]\n",(char*)list[i][0]);
			if ((char*)list[i][1] != NULL) {
				fprintf(ecmp,"%s\n",(char*)list[i][1]);
			}
			
		}
		else {
			if (*(int*)(list[i][2]) != 0) {
				fprintf(ecmp,"[%s]\n",(char*)list[i][0]);
				for (int j = 0;j< *(int*)(list[i][2]);j++) {
					fprintf(ecmp,"%s\n",((char**)list[i][1])[j]);
				}
			}	
		}
		
	}

    fclose(ecmp);
	return 0;

} 
