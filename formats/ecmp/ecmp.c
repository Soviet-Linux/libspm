#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "unistd.h"
#include "malloc.h"

#include "spm/libspm.h"
#include "spm/utils.h"

#include "spm/hashtable.h"
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

		{parseraw,&pkg->info.make,NULL},
		{parseraw,&pkg->info.install,NULL},
		{parseraw,&pkg->info.download,NULL},
		{parseraw,&pkg->info.prepare,NULL},
		{parseraw,&pkg->info.special,NULL},

		{parsenl,&pkg->dependencies,&pkg->dependenciesCount},
		{parsenl,&pkg->makedependencies,&pkg->makedependenciesCount},
		{parsenl,&pkg->optionaldependencies,&pkg->optionaldependenciesCount},
		{parsenl,&pkg->locations,&pkg->locationsCount},

	};

	void* pairs[][2] = {
		{"info",parsers[0]},

		{"make",parsers[1]},
		{"install",parsers[2]},
		{"download",parsers[3]},
		{"prepare",parsers[4]},
		{"special",parsers[5]},

		{"dependencies",parsers[6]},
		{"makedependencies",parsers[7]},
		{"optionaldependencies",parsers[8]},
		{"locations",parsers[9]}
	};
	void* infodict[][2] = {
		{"name",&pkg->name},
		{"version",&pkg->version},
		{"type",&pkg->type},
		{"url",&pkg->url},
		{"license",&pkg->license}	
	};
	hm = hm_init(pairs,sizeof(pairs)/sizeof(pairs[0]));
	infohm = hm_init(infodict,sizeof(infodict)/sizeof(infodict[0]));



	unsigned int (*parser)(char* s,void* dest);

	section** sections;
	uint count = getsections(path,&sections);


	for (int i = 0; i < count; i++) {
		printf("section : %s\n",sections[i]->name);
		void** options = hm_get(hm,sections[i]->name);
		if (options == NULL) {
			msg(WARNING,"Unknown section : %s",sections[i]->name);
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
			msg(WARNING,"Unknown parser for section : %s",sections[i]->name);
		}
	}
	msg(INFO,"Parsing done\n");
	return 0;
}




unsigned int parsenl(char* s,char*** dest)
{
	char* str;
	
	unsigned int count = ncountc(s,parseraw(s,&str),'\n');
	printf("count : %d\n",count);
	*dest = calloc(sizeof(char*),count+2);
	return splitm(str,'\n',*dest,count+2);
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

unsigned int parseinfo(char *s, struct package* dest)
{
	char* p = s;
	while (*p!='\0') {
		if (*p == ' ') {
			popcharn(s,strlen(s),p-s);
		}
		p++;
	}
	// split with nl
	char** nlist;
	int count = parsenl(s,&nlist);
	printf("count : %d\n",count);
	for (int i = 0;i < count;i++) {
		char* key = strtok(nlist[i],"=");
		char* value = strtok(NULL,"=");
		// add to correspondng value in dict
		char** destbuff =  hm_get(infohm,key);
		if (destbuff == NULL) {
			msg(WARNING,"Unknown key : '%s'",key);
			continue;
		}
		*destbuff = value;

	}
	return 0;
}


unsigned int getsections(char* path,section*** sections) {
	FILE* fp = fopen(path,"r");
	char* line = NULL;
	size_t len = 0;
	ssize_t read;
	*sections = calloc(16,sizeof(section));
	unsigned int sectionsalloc = 256;
	unsigned int sectionscount = 0;

	section* current = NULL;
	unsigned int alloc = 0;

	while ((read = getline(&line,&len,fp)) != EOF) {
		if (line[0] == '#' || line[0] == '\n' || strlen(line) < 2) {
			continue;
		}
		if (line[0] == '[') {
			section *sec = calloc(1,sizeof(section));
			sec->name = strdup(strtok(line,"[]"));

			sec->buff = calloc(256,sizeof(char));
			(*sections)[sectionscount++] = sec;

			alloc = 256;
			current = sec;

			continue;
		}
		if (strlen((*sections)[sectionscount-1]->buff) + strlen(line) >= alloc) {
			alloc += strlen(line) + 64;
			(*sections)[sectionscount-1]->buff = realloc((*sections)[sectionscount-1]->buff,alloc);
		}
		strcat((*sections)[sectionscount-1]->buff,line);
	}
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
		{"download",pkg->info.download,NULL},
		{"prepare",pkg->info.prepare,NULL},
		{"make",pkg->info.make,NULL},
		{"install",pkg->info.install,NULL},
		{"special",pkg->info.special,NULL},

		{"dependencies",pkg->dependencies,&pkg->dependenciesCount},
		{"makedependencies",pkg->makedependencies,&pkg->makedependenciesCount},
		{"optionaldependencies",pkg->optionaldependencies,&pkg->optionaldependenciesCount},

		{"locations",pkg->locations,&pkg->locationsCount},
	};

	FILE* ecmp = fopen(path,"w");

	// print info
	fprintf(ecmp,"[info]\n");
	// add  NULL check before each line
	if (pkg->name != NULL) fprintf(ecmp,"name = %s\n",pkg->name);
	if (pkg->version != NULL) fprintf(ecmp,"version = %s\n",pkg->version);
	if (pkg->type != NULL) fprintf(ecmp,"type = %s\n",pkg->type);
	if (pkg->license != NULL) fprintf(ecmp,"license = %s\n",pkg->license);
	if (pkg->url != NULL) fprintf(ecmp,"url = %s\n",pkg->url);
	fprintf(ecmp,"\n"); // for impoved readability

	for (int i = 0;i < sizeof(list) / sizeof(list[0]);i++ )
	{
		if (list[i][1] == NULL) {
			printf("Empty section\n");
			continue;
		}

		//printf("%d - [%s]\n",i,(char*)list[i][0]);
		if (list[i][2] == NULL) {

			printf("[%s] -> %p\n",(char*)list[i][0],(char*)list[i][0]);
			fprintf(ecmp,"[%s]\n",(char*)list[i][0]);
			if ((char*)list[i][1] != NULL) {
				fprintf(ecmp,"%s\n",(char*)list[i][1]);
			}
			
		}
		else {
			if (*(int*)(list[i][2]) != 0) {
				fprintf(ecmp,"[%s]\n",(char*)list[i][0]);
				printf("%d - [%s]\n",i,(char*)list[i][0]);
				for (int j = 0;j< *(int*)(list[i][2]);j++) {
					fprintf(ecmp,"%s\n",((char**)list[i][1])[j]);
				}
			}	
		}
		
	}

	return 0;

} 

