#include <stdio.h>
#include <string.h>
int Kmp (char* S, char* T,int* num,int* next);
int BF (char* S, char* T,int* num);
int main(void)
{
	char S[]="GCACTGCAGCACAGCAGCAGTACG";
	char T[]="GCAGCAG";
	int numkmp[2]={0,0};
	int numbf[2]={0,0};
	int next[7]={-1,0,0,0,1,2,3};
	Kmp(S,T,numkmp,next); 
	BF(S,T,numbf);
	printf("KMP: 比较次数是%d ，对齐次数是%d\n",numkmp[0],numkmp[1]);
	printf("BF: 比较次数是%d ，对齐次数是%d",numbf[0],numbf[1]);
	return 0;
}
int Kmp (char* S, char* T,int* num,int* next){
    int i = 0;  int j = 0; 
    int LenS = strlen(S); 
    int LenT = strlen(T); 
    while (i < LenS && j < LenT) {
      if (j == -1 || S[i] == T[j]) {
           i++;
           j++;
      } 
      else{ 
            j = next[j];
            num[1]++;
       }
       num[0]++;
     } 
     if (j == LenT)
          return i - j; 
     else
          return -1;
}
int BF (char* S, char* T,int* num){ 
    int i = 0;  int j = 0; 
    int LenS = strlen(S); 
    int LenT = strlen(T); 
    while (i < LenS && j < LenT) {
      if (S[i] == T [j]) {  
           i++;   
           j++; 
      } 
      else {
          i=i-j+1;
          j = 0 ;
          num[1]++;
      } 
      num[0]++;
   }
    if (j == LenT)   
        return i - j; 
    else   
       return -1;
}
