/*****************************************************************
 *author:     xudongliang
 *copyright:  www.pact518.hit.edu.cn
 *date:       2012.05.11
 *
 *****************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "hash.h"
#include "rfpwm.h"

int nfound=0;
u_int32_t SMALLEST;
int NOCASE; //�Ƿ����ִ�Сд��0�������֣�1������,Ĭ�����ִ�Сд

/*64KB Memory*/
static unsigned char Tc[64*1024];
/*
** Case Translation Table
*/
//static unsigned char xlatcase[256];

/*
* Init the xlatcase Table,Trans alpha to UpperMode
* Just for the NoCase State
*/
/*
static void init_xlatcase ()
{
	int i;
	for (i = 0; i < 256; i++)
	{
		xlatcase[i] = toupper (i);
	}
}
*/

/*
*Convert the pattern string into upper
*/
static void ConvertCaseEx (unsigned char *d, unsigned char *s, int m)
{
	int i;
	for (i = 0; i < m; i++)
	{
//		d[i] = xlatcase[s[i]];
		d[i] = toupper(s[i]);
	}
}

/******************************************
 *������RFPWM_STRUCT *rfpWM_New()
 *Ŀ�ģ�����һ��ģʽ���ṹ
 *��������
 *����ֵ��RFPWM_STRUCT *p
 ******************************************/
RFPWM_STRUCT *rfpwm_new()
{
	RFPWM_STRUCT *p=(RFPWM_STRUCT *)malloc(sizeof(RFPWM_STRUCT));
	if(!p)return 0;
	memset(p,0,sizeof(RFPWM_STRUCT));
	p->smallest = 100000;
	return p;
}

/******************************************
 *������void rfpWM_Pattern_Struct_free(RFPWM_PATTERN_STRUCT*)
 *Ŀ�ģ��ͷ�ģʽ���ṹ
 *������RFPWM__PATTERN_STRUCT *pl,ģʽ������
 *����ֵ����
 ******************************************/
void rfpwm_pattern_struct_free(RFPWM_PATTERN_STRUCT *pl)
{

	if(pl)
	{
		for(;pl!=NULL;pl=pl->next)//�ýṹ�е�pat���ͷţ�patarray�ṹ��Ҫ��
		{
			if(pl->fparray) free(pl->fparray);
		}
		free(pl);
	}
}
/******************************************
 *������void rfpWMfree(RFPWM_STRUCT*)
 *Ŀ�ģ��ͷ�ģʽ���ṹ
 *������RFPWM_STRUCT *p,ģʽ����
 *����ֵ����
 ******************************************/
void rfpwmfree(RFPWM_STRUCT *p)//�ͷſռ亯��,ʹ�����ڴ�ع���󣬸ú����Ͳ�����
{
	if(p->patarray)
	{
		int32_t i;
		for(i=0;i<p->numpat;i++)
		{
			if(p->patarray[i].pat) free(p->patarray[i].pat);
		}
		free(p->patarray);
	}
	if(p->numarray) free(p->numarray);
	if(p->hash) free(p->hash);
	if(p->shift) free(p->shift);
	if(p->fingerprint) free(p->fingerprint);
	free(p);
}

/******************************************
 *������int rfpWMaddpat(RFPWM_STRUCT*,unsigned char,u_int32_t)
 *Ŀ�ģ���ģʽ���ṹRFPWM_PATTERN_STRUCT�����һ��ģʽ��
 *������rpat=>ģʽ�������ı��ļ��е�ģʽ�����뵽�ýṹ��
	pat => ģʽ��������ģʽ������patlist��
	len => ģʽ���ĳ���
 *����ֵ���ɹ�����0������ ��1
 ******************************************/
int rfpwmaddpat(RFPWM_STRUCT *p, unsigned char *pat,u_int32_t len)
{
	RFPWM_PATTERN_STRUCT *rpat;
	//����RFPWM_PATTERN_STRUCT�ṹֻ����ʱ�洢�ṹ�����Բ����ڴ��������ռ�
	rpat = (RFPWM_PATTERN_STRUCT *)malloc(sizeof(RFPWM_PATTERN_STRUCT));
	if(!rpat)return -1;
	//���ڸýṹ�е�ģʽ����RFPWM_PATARRAY_STRUCT�ṹ���õ����������ڴ��������
	rpat->pat = (unsigned char*)malloc(len+1);
	if(!(rpat->pat))return -1;
	memset(rpat->pat+len,0,1);
	if(NOCASE == 0)//�����ִ�Сд,���д�Сдת��
	{
		ConvertCaseEx(rpat->pat,pat,len);
	}
	else //���ִ�Сд
		memcpy(rpat->pat,pat,len);

	rpat->len = len;
	p->numpat++;

	if(len<p->smallest) p->smallest = len;
	rpat->next =p->patlist;
	p->patlist = rpat;

	return 0;
}

/******************************************
 *������int winfingerprint(RFPWM_STRUCT*)
 *Ŀ�ģ������ÿ��ģʽ���е�ÿ�����ڵ�ָ��,���ó�ÿ��ģʽ����Ψһָ��
 *������RFPWM_STRUCT *p => ģʽ�����Ľṹ��
 *����ֵ���ɹ�����0������ ��1
 ******************************************/
int winfingerprint(RFPWM_STRUCT *p)
{
	int32_t i,j;
	int32_t minindex=0;//��С����ֵ
	int32_t minfrequency;//ָ�Ƴ��ֵ���СƵ��
	int32_t winlen;//���ڳ��ȣ��������ģʽ���ĳ���
	int32_t winnum;//���ڵ�����������һ��ģʽ���ж��ٸ�ָ��
	unsigned char *ps;
	FINGER_PRINT *fparray;
	RFPWM_PATTERN_STRUCT *patlist,*pl;

	patlist = p->patlist;
	winlen = p->smallest;//���ģʽ���ĳ��Ⱦ��Ǵ��ڵĳ���

	//����ÿ��ģʽ����ÿ�����ڵ�ָ��
	for(patlist=p->patlist;patlist!=NULL;patlist=patlist->next)
	{
		winnum = patlist->len - winlen + 1;
		fparray= (FINGER_PRINT *)malloc(winnum*sizeof(FINGER_PRINT));
		if(!fparray) return -1;
		memset(fparray,0,winnum*sizeof(FINGER_PRINT));
		patlist->fparray = fparray;

		ps = patlist->pat;
		for(i=0;i<winnum;i++)
		{
			patlist->fparray[i].fingerprint=FingerPrint(ps++,winlen);//����ÿ�����ڵ�ָ��
//			printf("%d fingerprint=%u  ",i,patlist->fparray[i].fingerprint);
		}
//		printf("\n");
	}
	//ͳ��ÿ��ģʽ����ÿ������ָ�Ƴ��ֵ�Ƶ��
	for(patlist=p->patlist;patlist!=NULL;patlist=patlist->next)
	{
		winnum = patlist->len - winlen + 1;
		for(i=0;i<winnum;i++)
		{

			for(pl=p->patlist;pl!=NULL;pl=pl->next)
			{
				for(j=0;j<winnum;j++)
				{
					if(patlist->fparray[i].fingerprint==pl->fparray[j].fingerprint)
						patlist->fparray[i].frequency++;

				}
			}

		}
	}

	//����ÿ��ģʽ����Ψһָ�ƣ�����˵�����ģʽ������һ�����ڵ�ָ�Ƴ��ֵĴ������٣�������Ϊ��ģʽ����ָ��
	for(pl=p->patlist;pl!=NULL;pl=pl->next)
	{
		winnum = pl->len - winlen + 1;
		minfrequency=100000;
		for(i=0;i<winnum;i++)
		{
			if(minfrequency > pl->fparray[i].frequency)
			{
				minindex = i;
				minfrequency = pl->fparray[i].frequency;
			}
		}
		pl->index = minindex;//������Ƶ����С�Ĵ�����������ģʽ��Ψһָ������
	}
	return 0;
}

/************************************************************
 *������void sort(RFPWM_STRUCT *)
 *Ŀ�ģ�������ģʽ������������������ַ��Ĺ�ϣֵ��С��������
 *������RFPWM_STRUCT *p => ģʽ�����Ľṹ��
 *����ֵ����
 * *********************************************************/
void sort(RFPWM_STRUCT *p)
{
	u_int32_t winlen=p->smallest;//���ڳ���
	int i,j,flag;//ð�������־λ����һ�˱Ƚ��޽���ʱ��˵���Ѿ��������
	struct rfpwm_patarray_struct *pt1,*pt2,pt_temp;
	HASH_TYPE h_temp1,h_temp2;
	//ð������
	for(i=p->numpat-1,flag=1;i>0&&flag;i--)
	{
		flag=0;
		for(j=0;j<i;j++)
		{
			pt1=&(p->patarray[j]);
			pt2=&(p->patarray[j+1]);
			h_temp2 = HASH16(&(pt2->pat[pt2->index+winlen-2]));
			h_temp1 = HASH16(&(pt1->pat[pt1->index+winlen-2]));
			if(h_temp2<h_temp1)//�Ƚ�ÿ���ַ�����ȡ������������ַ��Ĺ�ϣֵ
			{
				flag=1;
				memcpy(&pt_temp,pt2,sizeof(RFPWM_PATARRAY_STRUCT));
				memcpy(pt2,pt1,sizeof(RFPWM_PATARRAY_STRUCT));
				memcpy(pt1,&pt_temp,sizeof(RFPWM_PATARRAY_STRUCT));
			}
		}
	}
}
/************************************************************
 *������int compare(const void *a,const void *b)
 *Ŀ�ģ�qsort�еıȽϺ������Ƚ�����Ԫ��֮��Ĵ�С
 *������
 *����ֵ:
 * *********************************************************/
int compare(const void *a, const void *b)
{
	HASH_TYPE h_temp1,h_temp2;
	RFPWM_PATARRAY_STRUCT *pt1= (RFPWM_PATARRAY_STRUCT *)a;
	RFPWM_PATARRAY_STRUCT *pt2= (RFPWM_PATARRAY_STRUCT *)b;
	h_temp2 = HASH16(&(pt2->pat[pt2->index+SMALLEST-2]));
	h_temp1 = HASH16(&(pt1->pat[pt1->index+SMALLEST-2]));
	return h_temp2-h_temp1;
}
/* ****************************************************************
������static void rfpwmhashtable(RFPWM_STRUCT *)
Ŀ�ģ����㹲�ж��ٸ���ͬ�Ĺ�ϣֵ���Ҵ�С����
������ps => ģʽ����
���أ�
****************************************************************/
static void rfpwmhashtable(RFPWM_STRUCT *p)
{
	int i;
	HASH_TYPE hindex,sindex,snum;
	int32_t winindex;//ÿ��ģʽ���Ĵ�������
	u_int32_t winlen=p->smallest;
	RFPWM_PATARRAY_STRUCT *pt;
	pt=p->patarray;

	p->hash=(HASH_TYPE*)malloc(sizeof(HASH_TYPE)*HASHTABLESIZE);//HASH��
	if(!p->hash)
	{
		fprintf(stderr,"HASH table malloc error @ %s:%d\n",__FILE__,__LINE__);
		return;
	}
	for(i=0;i<HASHTABLESIZE;i++)
	{
		p->hash[i]=(HASH_TYPE)-1;
	}
	for(i=0;i<p->numpat;i++)
	{
		winindex=pt[i].index;
		hindex = HASH16(&(pt[i].pat[winindex+winlen-2]));
		sindex=p->hash[hindex]=i;
		snum=1;
		//��ʱ��ϣ���Ѿ�������
		while((++i<p->numpat)&&hindex==HASH16(&(pt[i].pat[pt[i].index+winlen-2])))//�Һ�׺��ͬ���Ӵ���
			snum++;
		p->numarray[sindex]=snum;//��iģʽ��������ģʽ�������׺��ͬ���Ӵ��ĸ���
		i--;
	}
}

/* ****************************************************************
    ������static void rfpwmshifttable(RFPWM_STRUCT *p)
	Ŀ�ģ�����shift�����ÿ���ַ���Ҫ�ƶ��ľ���
	������
		p => ģʽ����
    ���أ�

 ****************************************************************/
static void rfpwmshifttable(RFPWM_STRUCT *p)
{
	int i,k;
	u_int32_t winlen=p->smallest;//���ڳ���
	HASH_TYPE shift,cindex;
	RFPWM_PATARRAY_STRUCT *pt;

	p->shift=(HASH_TYPE *)malloc(SHIFTTABLESIZE*sizeof(HASH_TYPE));
	if(!p->shift) return;

	for(i=0;i<SHIFTTABLESIZE;i++)
	{
		p->shift[i]=(HASH_TYPE)(winlen-2+1);//��ʼ��shift����ʼֵΪ����ַ����ĳ���
	}

	for(i=0;i<p->numpat;i++)
	{
		pt=p->patarray;
		for(k=0;k<winlen-1;k++)
		{
			shift = (HASH_TYPE)(winlen-2-k);
			cindex=HASH16(&(pt[i].pat[pt[i].index+k]));
			if(shift<p->shift[cindex])
				p->shift[cindex]=shift;
		}
	}
}


/* ****************************************************************
    ������static void rfpwmfingerprinttable(RFPWM_STRUCT *p)
	Ŀ�ģ�����Prefix��
	������
		p => ģʽ����
    ���أ�
	    ��
 ****************************************************************/
//static void rfpwmfingerprinttable(RFPWM_STRUCT *p){}

/* ****************************************************************
������int rfpwmfingerprintmatch(RFPWM_STRUCT *,int ,unsigned char *,unsigned char *)
	Ŀ�ģ���׺��ϣֵ��ͬ���Ƚ�ǰ׺�Լ������ַ���ƥ��
	������
		p => ģʽ����
		lindex =>lindexΪ��׺��ϣֵ��ͬ����Щģʽ�Ӵ��е�һ��ģʽ�Ӵ���index
		Tx => Ҫ����ƥ����ַ�������
		T => ģʽ�Ӵ�
    ���أ�
	    ��
 ****************************************************************/
int rfpwmfingerprintmatch(RFPWM_STRUCT *p,int lindex ,
			unsigned char *Tx,unsigned char *T)
{
//	int nfound=0;
	int len;
	u_int32_t tfingerprint;//��ȡ�ı����ָ��
	u_int32_t winlen=p->smallest;
	unsigned char *pat;
	unsigned char *tx;
	RFPWM_PATARRAY_STRUCT *pt,*ptend;

	pt=&p->patarray[lindex];
	ptend=pt+p->numarray[lindex];

	tfingerprint=FingerPrint(T,winlen);//ȡ���ı����ָ��

	for(;pt<ptend;pt++)
	{
		if(pt->fingerprint!=tfingerprint)
			continue;
		else//���ָ����ͬ����
		{
			pat=pt->pat;
			len=pt->len;
			tx=T-(pt->index);//����ָ�Ʋ�������ģʽ���ĵ�һ���ַ���ʼ��
			if(strncmp((const char*)pat,(const char*)tx,len)==0)
			{
				nfound++;
			}
			//while(*(pat++)==*(tx++)&&(len--)!=0);
			//if(len==0)//ƥ��ɹ�
			//{
			//	nfound++;
			//}
			//while(*(pat++)==*(tx++)&&*(pat-1)!='\0');
			//if(*(pat-1)=='\0')//ƥ��ɹ�
			//{
			//	nfound++;
			//}
		}
	}
	return 0;
}

/* ****************************************************************
    ������int rfpwmsearch(RFPWM_STRUCT *p,unsigned char *Tx,int n)
	Ŀ�ģ��ַ���ƥ�����
	������
		p => ģʽ����
		Tx => �����ҵ��ַ�������
		n => �����ҵ��ַ�������
    ���أ�nfound,ƥ�䵽�ĸ���
 ****************************************************************/
int rfpwmsearch(RFPWM_STRUCT *p,unsigned char *Tx,int n)
{
	//int nfound=0;
	int Tleft,lindex,tshift;
	HASH_TYPE hindex;
	//Tָ�򴰿ڵĵ�һ���ַ�
	unsigned char *T,*Tend,*window;//windowָ�򴰿ڵ����һ���ַ�
	if(n<p->smallest)//�����ҵ��ַ�������С��ģʽ������
		return -1;
	if(NOCASE == 0)//�����ִ�Сд
	{
		/* Case conversion */
		ConvertCaseEx (Tc, Tx, n);
		Tend=Tc+n;
		window=Tc+p->smallest-1;
		T=Tc;
	}
	else
	{
		Tend=Tx+n;
		window=Tx+p->smallest-1;
		T=Tx;
	}
	Tleft=n;
	for(;window<Tend;T++,window++,Tleft--)
	{
		hindex=HASH16(window-1);
		tshift=	p->shift[hindex];
		while(tshift)
		{
			window+=tshift;
			T+=tshift;
			Tleft-=tshift;
			if(window>Tend) return 0;
			tshift=p->shift[HASH16(window-1)];
		}
		//tshift=0,������׺��ϣֵ�Ѿ���ͬ
		if((lindex=p->hash[HASH16(window-1)])==(HASH_TYPE)-1)
			continue;
		lindex=p->hash[HASH16(window-1)];
		rfpwmfingerprintmatch(p,lindex,Tx,T);
	}
//	return nfound;
	return 0;
}


/******************************************
 *������int rfpWMPrecpat(RFPWM_STRUCT*)
 *Ŀ�ģ������ÿ��ģʽ���е�ÿ�����ڵ�ָ��,���ó�ÿ��ģʽ����Ψһָ��
 *������RFPWM_STRUCT *p => ģʽ�����Ľṹ��
 *����ֵ���ɹ�����0������ ��1
 ******************************************/
int rfpwmprecpat(RFPWM_STRUCT *p)
{
	int i;
	RFPWM_PATTERN_STRUCT *pl;
	RFPWM_PATARRAY_STRUCT *pt;
	pl=p->patlist;
	SMALLEST=p->smallest;
	p->patarray=(RFPWM_PATARRAY_STRUCT *)malloc(sizeof(RFPWM_PATARRAY_STRUCT)*p->numpat);
	if(!p->patarray) return -1;

	p->numarray=(HASH_TYPE*)malloc(sizeof(HASH_TYPE)*p->numpat);
	if(!p->numarray) return -1;
	memset(p->numarray,0,sizeof(HASH_TYPE)*p->numpat);

	pt=p->patarray;
	for(i=0;pl!=NULL&&i<p->numpat;pl=pl->next,i++)
	{
		pt[i].pat=pl->pat;
		pt[i].index=pl->index;
		pt[i].fingerprint=pl->fparray[pl->index].fingerprint;
		pt[i].len=pl->len;
	}
	rfpwm_pattern_struct_free(pl);//�ͷ�RFPWM_PATTERN_STRUCT�ṹ
	//sort��ð������qsort�ǿ�������qsort���ٶ�Ҫ��һЩ
	sort(p);
//	qsort(p->patarray,p->numpat,sizeof(RFPWM_PATARRAY_STRUCT),compare);
	rfpwmhashtable(p);
	rfpwmshifttable(p);

	return 0;
}


