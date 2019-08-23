#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "student.pb-c.h"
#include "printf.h"

#define ID_LEN         32
#define NAME_LEN       32
#define GENDER_LEN     10
#define OBJECT_LEN     20
#define HOME_ADDR_LEN  96
#define PHONE_LEN      12

static int malloc_student_info(Student *stu)
{
    stu->id = (char*)malloc(ID_LEN);
    if (!stu->id)
    {
    goto FAILED;
    }
    stu->name = (char*)malloc(NAME_LEN);
    if (!stu->name)
    {
    goto FAILED;
    }
    stu->gender = (char*)malloc(GENDER_LEN);
    if (!stu->gender)
    {
    goto FAILED;
    }
    stu->object = (char*)malloc(OBJECT_LEN);
    if (!stu->object)
    {
    goto FAILED;
    }
    stu->home_address = (char*)malloc(HOME_ADDR_LEN);
    if (!stu->home_address)
    {
    goto FAILED;
    }
    stu->phone = (char*)malloc(PHONE_LEN);
    if (!stu->phone)
    {
    goto FAILED;
    }
    return 0;
FAILED:
    fprintf(stdout, "malloc error.errno:%u,reason:%s\n",
        errno, strerror(errno));
    return -1;
}

static void free_student_info(Student *stu)
{
    if (stu->id)
    {
    free(stu->id);
    stu->id = NULL;
    }
    if (stu->name)
    {
    free(stu->name);
    stu->name = NULL;
    }
    if (stu->gender)
    {
    free(stu->gender);
    stu->gender = NULL;
    }
    if (stu->object)
    {
    free(stu->object);
    stu->object = NULL;
    }
    if (stu->home_address)
    {
    free(stu->home_address);
    stu->home_address = NULL;
    }
    if (stu->phone)
    {
    free(stu->phone);
    stu->phone = NULL;
    }
}

static void set_student_info(Student *stu)
{
    const char *id = "4211231990082676132";
    const char *name = "Wencai Dong";
    const char *gender = "male";
    const char *object = "computer";
    const char *address = "shen zheng";
    const char *phone = "13409879163";
    
    strncpy(stu->id, id, strlen(id));
    strncpy(stu->name, name, strlen(name));
    strncpy(stu->gender, gender, strlen(gender));
    stu->age = 23;
    strncpy(stu->object, object, strlen(object));
    strncpy(stu->home_address, address, strlen(address));
    strncpy(stu->phone, phone, strlen(phone));
}

void print_student_info(Student *stu)
{
    printf_("id: %s\n",stu->id);
    printf_("name: %s\n",stu->name);
    printf_("age: %d\n",stu->age);
    printf_("gender:%s\n",stu->gender);
    printf_("object: %s\n",stu->object);
    printf_("home address: %s\n",stu->home_address);
    printf_("phone: %s\n",stu->phone);
}
    Student stu = STUDENT__INIT;
    void *buf = NULL;
    unsigned int len ;
    Student *msg = NULL;
int __main__()
{

    if (malloc_student_info(&stu) == -1) {
        exit(0);
    }   
    set_student_info(&stu);

    //get student packed size
    len = student__get_packed_size(&stu);
    printf_("size of student info : %u\n",len);
    buf = malloc(len);
    //put student info pack to buf
    student__pack(&stu, buf);

    //unpack student info from buf
    msg = student__unpack(NULL, len, buf);
    print_student_info(msg);
    //free msg
    student__free_unpacked(msg, NULL);

    free(buf);
    free_student_info(&stu);

    return 0;
}
