/*****************************************************************
*author:     xudongliang
*copyright:  www.pact518.hit.edu.cn
*date:       2014.01.02
*
*****************************************************************/
#ifndef _MULTI_HASH_TABLE_H_
#define _MULTI_HASH_TABLE_H_



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <openssl/md5.h>

#include "hash.h"


//URL��һ������Ĺ�ϣ�ṹ��
typedef struct _hvalue_next_component_struct_
{
	unsigned int hash;//URL��һ������Ĺ�ϣֵ
	struct _hvalue_next_component_struct_ *next;
}HNEXTCOMPONENT;

//����ṹ��
typedef struct _component_struct_
{
	char *str;
	HNEXTCOMPONENT *hnext_compt;
//	struct _havlue_next_component_struct_ *hnext_compt;
	struct _component_struct_ *next;//��ϣ��ͻ���
}COMPONENT;



int hcompt_harray_build(COMPONENT **harray, char str[][30]);

int url_search(COMPONENT **harray, char str[][30]);

void hcompt_harray_destroy(COMPONENT **harray);

void test_harray(COMPONENT **harray);


#endif
