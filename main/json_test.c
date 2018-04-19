#include "stdio.h"
#include "cJSON.h"

//#include "log_file.h"

/*

{
	"name": "Tom",
	"age":  25,
	"marry":        true,
	"child":        null
}

*/
char * cjson_create_test1(void)
{
   char * out;
   cJSON *root;

   root = cJSON_CreateObject();
   if(!root)
   {
       printf("cJSON create error!!\n");
       return NULL;
   }

   cJSON_AddItemToObject(root, "name", cJSON_CreateString("Tom"));
   cJSON_AddItemToObject(root, "age", cJSON_CreateNumber(25));
   cJSON_AddItemToObject(root, "marry", cJSON_CreateBool(1));
   cJSON_AddItemToObject(root, "child", cJSON_CreateNull());

   out=cJSON_Print(root);
   cJSON_Delete(root);    

   if(out)
   {
       return out;
   }

   return NULL;
}
/*
{
    "name": "Tom",
    "age":  25,
    "marry":        true,
    "children":     [{
                    "name": "May",
                    "age":  2
            }, {
                    "name": "Kendy",
                    "age":  3
            }, {
                    "name": "Mark",
                    "age":  1
            }]
}
*/
char * cjson_create_test2(void)
{
    char * out;
    cJSON *root;
    cJSON *child, *child1, *child2, *child3;

    root = cJSON_CreateObject();
    if(!root)
    {
        printf("cJSON create error!!\n");
        return NULL;
    }

    cJSON_AddItemToObject(root, "name", cJSON_CreateString("Tom"));
    cJSON_AddItemToObject(root, "age", cJSON_CreateNumber(25));
    cJSON_AddItemToObject(root, "marry", cJSON_CreateBool(1));

    child = cJSON_CreateArray();    
    //CJSON_CHK_FAIL(child, NULL);
    cJSON_AddItemToObject(root, "children", child);

    child1 = cJSON_CreateObject();    
    //CJSON_CHK_FAIL(child1, NULL);
    cJSON_AddItemToObject(child1, "name", cJSON_CreateString("May"));
    cJSON_AddItemToObject(child1, "age", cJSON_CreateNumber(2));

    child2 = cJSON_CreateObject();    
    //(child2, NULL);
    cJSON_AddItemToObject(child2, "name", cJSON_CreateString("Kendy"));
    cJSON_AddItemToObject(child2, "age", cJSON_CreateNumber(3));

    child3 = cJSON_CreateObject();    
    //CJSON_CHK_FAIL(child3, NULL);
    cJSON_AddItemToObject(child3, "name", cJSON_CreateString("Mark"));
    cJSON_AddItemToObject(child3, "age", cJSON_CreateNumber(1));

    cJSON_AddItemToArray(child, child1);
    cJSON_AddItemToArray(child, child2);
    cJSON_AddItemToArray(child, child3);

    out=cJSON_Print(root);
    cJSON_Delete(root);    

    if(out)
    {  
        return out;
    }

    return NULL;
}

void cjson_parse_test1(void)
{
    cJSON * root;
    cJSON *name, *age, *marry, *child;

    root = cJSON_Parse(cjson_create_test1());   
    if(!root)
    {
        printf("cJSON_Parse error!!\n");
        return;
    }

    name = cJSON_GetObjectItem(root, "name");
    if(name)
    {
        printf("Get name value : %s\n", name->valuestring);
    }

    age = cJSON_GetObjectItem(root, "age");
    if(age)
    {
        printf("Get age value : %d\n", age->valueint);
    }

    marry = cJSON_GetObjectItem(root, "marry");
    if(marry)
    {
        printf("Get marry value : %d\n", marry->valueint);
    }

    child = cJSON_GetObjectItem(root, "child");
    if(child)
    {
        printf("Get child value : %s\n", marry->valuestring);
    }

    cJSON_Delete(root);    
}

void cjson_parse_test2(void)
{
    cJSON * root;
    cJSON *name, *age, *marry, *children;
    cJSON *childNode, *childName, *childAge;

    root = cJSON_Parse(cjson_create_test2());   
    if(!root)
    {
        printf("cJSON_Parse error!!\n");
        return;
    }

    name = cJSON_GetObjectItem(root, "name");
    if(name)
    {
        printf("Get name value : %s\n", name->valuestring);
    }

    age = cJSON_GetObjectItem(root, "age");
    if(age)
    {
        printf("Get age value : %d\n", age->valueint);
    }

    marry = cJSON_GetObjectItem(root, "marry");
    if(marry)
    {
        printf("Get marry value : %d\n", marry->valueint);
    }

    children = cJSON_GetObjectItem(root, "children");
    if(children)
    {
        int size = cJSON_GetArraySize(children);
        printf("Get children size : %d\n", size);

        int i = 0;
        for(i=0; i<size; i++)
        {
            childNode = cJSON_GetArrayItem(children, i);
            if(childNode)
            {                
                childName = cJSON_GetObjectItem(childNode,"name");
                if(childName)
                {            
                    printf("Get child name : %s\n", childName->valuestring);
                }

                childAge = cJSON_GetObjectItem(childNode,"age");
                if(childAge)
                {            
                    printf("Get child age : %d\n", childAge->valueint);
                }
            }

        }
    }

    cJSON_Delete(root);    
}

int main(int argc, char **argv) 
{
#if 0	
	char *out = NULL;
	out = cjson_create_test1();
	if (out)
		printf("%s\n", out);

	out = cjson_create_test2();
	printf("%s\n", out);
#else
	cjson_parse_test1();
	printf("====================================\r\n");
	cjson_parse_test2();
#endif
	
	while(1)
	{
		sleep(1);
	}
 
    return 0;
}














