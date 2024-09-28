#include <stdio.h>
#define XSIZE 8

void suffixes(char *x, int m, int *suff);
void preBmGs(char *x, int m, int bmGs[]);

int main() {
    char x[] = "GCAGCGAG";
    int suff[8], bmGs[8];
    int m = 8, i;

    suffixes(x, m, suff);
    printf("suff\n");
    for (i = 0; i < 8; i++) {
        printf("%c    %d\n", x[i], suff[i]);
    }

    preBmGs(x, m, bmGs);
    printf("bmGs\n");
    for (i = 0; i < 8; i++) {
        printf("%c    %d\n", x[i], bmGs[i]);
    }

    return 0;
}

void suffixes(char *x, int m, int *suff) {
    suff[m - 1] = m;
    int i, q;
    for (i = m - 2; i >= 0; --i) {
        q = i;
        while (q >= 0 && x[q] == x[m - 1 - i + q]) {
            --q;
        }
        suff[i] = i - q;
    }
}

void preBmGs(char *x, int m, int bmGs[]) {
    int i, j, suff[XSIZE];
    suffixes(x, m, suff);  // ��ģʽ������Ԥ����

    for (i = 0; i < m; ++i) {
        bmGs[i] = m;     // ��bmGs����ĳ�ʼ��
    }
    
    j = 0;
    for (i = m - 1; i >= 0; --i) {
        if (suff[i] == i + 1) {  // ����ҵ�һ�����ǰ׺
            for (; j < m - 1 - i; ++j) {
                if (bmGs[j] == m) {
                    bmGs[j] = m - 1 - i;
                }
            }
        }
    }

    for (i = 0; i <= m - 2; ++i) {
        bmGs[m - 1 - suff[i]] = m - 1 - i;
    }
}
