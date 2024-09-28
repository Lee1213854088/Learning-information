#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_PATTERNS 100000 // ����ģʽ���������100000��ģʽ
#define MAX_PATTERN_LENGTH 500 //// ����ÿ��ģʽ����󳤶�Ϊ500
#define ALPHABET_SIZE 256 // ASCII�ַ�����С


//����ڵ�Ľṹ��
typedef struct Node {
    struct Node* children[ALPHABET_SIZE];
    struct Node* fail;
    int is_end_of_word;
    char* pattern; //�����洢�����״̬��ƥ�������ģʽ��
} Node;



//�½�����ʼ���ڵ�
Node* create_node() {
    Node* new_node = (Node*)malloc(sizeof(Node));
    int i;
    for (i = 0; i < ALPHABET_SIZE; i++) {
        new_node->children[i] = NULL;
    }
    new_node->fail = NULL;
    new_node->is_end_of_word = 0;
    new_node->pattern = NULL;
    return new_node;
}



//��һ��ģʽ�ַ������뵽trie����
void insert_pattern(Node* root, const char* pattern) {
    Node* current = root; //currentָ��ǰ�ڵ�
    const char* original_pattern = pattern; //����pattern����ʼ��ַ
    while (*pattern) {
        if (current->children[*pattern] == NULL) {
            current->children[*pattern] = create_node();
        }
        current = current->children[*pattern];
        pattern++;
    }
    current->is_end_of_word = 1;
    current->pattern = _strdup(original_pattern);//��ģʽ�ַ����洢�ڽڵ���
}



//����ʧЧ������������Ͷ���ʵ�֣�
// ��������ڵ�Ͷ��нṹ
typedef struct QueueNode {
    Node* trie_node;
    struct QueueNode* next;
} QueueNode;

typedef struct {
    QueueNode* front;
    QueueNode* rear;
} Queue;

// ��������
Queue* createQueue() {
    Queue* q = (Queue*)malloc(sizeof(Queue));
    q->front = q->rear = NULL;
    return q;
}

// ��Ӳ���
void enqueue(Queue* q, Node* node) {
    QueueNode* temp = (QueueNode*)malloc(sizeof(QueueNode));
    temp->trie_node = node;
    temp->next = NULL;

    if (q->rear == NULL) {
        q->front = q->rear = temp;
        return;
    }

    q->rear->next = temp;
    q->rear = temp;
}

// ���Ӳ���
Node* dequeue(Queue* q) {
    if (q->front == NULL) {
        return NULL;
    }

    QueueNode* temp = q->front;
    Node* node = temp->trie_node;
    q->front = q->front->next;

    if (q->front == NULL) {
        q->rear = NULL;
    }

    free(temp);
    return node;
}

// �������Ƿ�Ϊ��
int isEmpty(Queue* q) {
    return q->front == NULL;
}

// �ͷŶ����ڴ�
void freeQueue(Queue* q) {
    while (!isEmpty(q)) {
        dequeue(q);
    }
    free(q);
}

//����ʧЧ����
void build_fail_pointers(Node* root) {
    // ʹ��BFS�㷨������failָ��
    Node* temp;
    Node* fail_node;
    Queue* queue = createQueue(); // ʹ������ʵ�ֵĶ���

    // ��һ��Ľڵ��ʧ�ܺ���ȫ��ָ��root�ڵ�
    int i;
    for (i = 0; i < ALPHABET_SIZE; i++) {
        if (root->children[i] != NULL) {
            root->children[i]->fail = root;
            enqueue(queue, root->children[i]);//BFS��root�ڵ��ȫ���ӽڵ�������
        }
    }

    while (!isEmpty(queue)) {
        temp = dequeue(queue); // temp��ʧЧ��������֪�ģ����ǿ�
        for (i = 0; i < ALPHABET_SIZE; i++) {
            if (temp->children[i] != NULL) {
                fail_node = temp->fail;
                //�����ǰ�ڵ��ʧЧ����ָ��Ľڵ�Ķ�Ӧ�ӽڵ��ǿյģ�׷��ʧЧ����ָ���ʧЧ����
                while (fail_node != NULL && fail_node->children[i] == NULL) {
                    fail_node = fail_node->fail;
                }
                //һֱ׷�ݵ�ʧЧ����ָ��գ������ڵ��ʧЧ����
                if (fail_node == NULL) {
                    temp->children[i]->fail = root;
                }
                //�����ҵ��˺��ʵķǿ��ӽڵ㣬������ӽڵ㸳����ǰ�ڵ��ӽڵ��ʧЧ����
                else {
                    temp->children[i]->fail = fail_node->children[i];
                }
                //�����ӽڵ����
                enqueue(queue, temp->children[i]);
            }
        }
    }

    freeQueue(queue); // �ͷŶ��е��ڴ�
}



//�ڸ������ı�������trie���е�ģʽ
int search_text(Node* root, const char* text) {
    Node* current = root;
    int match_count = 0;
    while (*text) {
        //�����ǰ�ڵ㲻Ϊ�գ������ڵ�ǰ�ڵ���ӽڵ���û�ж�Ӧ��ǰ�ַ��ķ�֧�������ѭ��
        while (current != NULL && current->children[*text] == NULL) {
            //�� Trie �������ϻ��ݣ�ͨ�� fail ָ���ƶ���������֧
            current = current->fail;
        }
        // �����ǰ�ڵ�Ϊ�գ����Ѿ����ݵ����ڵ���û���ҵ���Ӧ�ķ�֧
        if (current == NULL) {
            current = root;//��ʼ��һ�ֵ�ƥ��
        }
        else {
            current = current->children[*text];
        }
        Node* temp = current;
        while (temp != root && temp->is_end_of_word) {
            printf("�ҵ�ģʽ��: %s\n", temp->pattern);
            match_count++;
            // ����ʱ�ڵ��ƶ�����ʧЧ����ָ��Ľڵ㣬�Լ���������ܵ�ƥ��
            temp = temp->fail;
        }
        text++;
    }
    return match_count;
}



//�ͷ���trie���з���������ڴ�
void free_memory(Node* root) {
    if (root == NULL) {
        return;
    }
    int i;
    for (i = 0; i < ALPHABET_SIZE; i++) {
        free_memory(root->children[i]);
    }
    if (root->pattern != NULL) {
        free(root->pattern);
    }
    free(root);
}



int main() {
    Node* root = create_node();
    char pattern[MAX_PATTERN_LENGTH];
    FILE* pattern_file=NULL;
    FILE* text_file;
    char* text_data;
    long length;
    int match_count;
    clock_t start_time_p, end_time_p, start_time_t, end_time_t;
    errno_t err;
    int p=0;

    while (p != 1 && p != 2 && p != 3)
    {
        printf("������ģʽ�ļ����(1~3)��\n");
        //scanf("%d",&p);
        scanf_s("%d", &p, sizeof(p));
        //printf("������Ҫ���Ե��ļ���ţ�\n");
        //scanf("%d", &t);
        //scanf_s("%s", &t, sizeof(t));
        if (p == 1) {
            // ����ģʽ��������AC�Զ���
            err = fopen_s(&pattern_file, "pattern1w.txt", "r");
            if (err != 0) {
                fprintf(stderr, "�޷��� 'pattern1w.txt'\n");
                return 1;
            }
        }
        if (p == 2) {
            // ����ģʽ��������AC�Զ���
            err = fopen_s(&pattern_file, "pattern2w.txt", "r");
            if (err != 0) {
                fprintf(stderr, "�޷��� 'pattern2w.txt'\n");
                return 1;
            }
        }
        if (p == 3) {
            // ����ģʽ��������AC�Զ���
            err = fopen_s(&pattern_file, "pattern3w.txt", "r");
            if (err != 0) {
                fprintf(stderr, "�޷��� 'pattern3w.txt'\n");
                return 1;
            }
        }
    }
    start_time_p = clock();
    while (fgets(pattern, MAX_PATTERN_LENGTH, pattern_file)) {
        // ȥ�����ܵĻ��з�
        pattern[strcspn(pattern, "\r\n")] = 0;
        insert_pattern(root, pattern);
    }
    build_fail_pointers(root);
    end_time_p = clock();
    //printf("��ʼ��pattern1w��ʱ: %f ��\n", (double)(end_time_p - start_time_p) / CLOCKS_PER_SEC);
    fclose(pattern_file);

    // ��ȡ�ı�����
    //text_file = fopen("text.txt", "r");
    err = fopen_s(&text_file, "text.txt", "r");
    if (err != 0) {
        fprintf(stderr, "�޷��� 'text.txt'\n");
        return 1;
    }
    //if (!text_file) {
     //   fprintf(stderr, "Error opening text file.\n");
      //  fclose(pattern_file);
        //free_memory(root);
        //return 1; // �˳�
        //����
    //}
    fseek(text_file, 0, SEEK_END);
    length = ftell(text_file);
    fseek(text_file, 0, SEEK_SET);
    text_data = (char*)malloc(length + 1); // ��ʽת��Ϊ char*
    
    if (text_data) {
        //fread(text_data, 1, length, text_file);
        size_t bytes_read = fread(text_data, 1, length, text_file);
        text_data[bytes_read] = '\0'; // �����ַ�����ֹ��
    }
    else {
        fprintf(stderr, "Ϊ�ı����ݷ����ڴ�ʧ�ܡ�\n");
        fclose(text_file);
        fclose(pattern_file);
        free_memory(root);
        return 1; // �˳�����
    }
    fclose(text_file);

    // ����ƥ��
    start_time_t = clock();
    match_count = search_text(root, text_data);
    end_time_t = clock();
    if (p == 1) {
        printf("��ʼ��pattern1w��ʱ: %f ��\n", (double)(end_time_p - start_time_p) / CLOCKS_PER_SEC);
        printf("���� pattern1w ��ʱ�䣺%f ��\n", (double)(end_time_t - start_time_t) / CLOCKS_PER_SEC);
    }
    if (p == 2) {
        printf("��ʼ��pattern2w��ʱ: %f ��\n", (double)(end_time_p - start_time_p) / CLOCKS_PER_SEC);
        printf("���� pattern2w ��ʱ�䣺%f ��\n", (double)(end_time_t - start_time_t) / CLOCKS_PER_SEC);
    }
    if (p == 3) {
        printf("��ʼ��pattern3w��ʱ: %f ��\n", (double)(end_time_p - start_time_p) / CLOCKS_PER_SEC);
        printf("���� pattern3w ��ʱ�䣺%f ��\n", (double)(end_time_t - start_time_t) / CLOCKS_PER_SEC);
    }
    //printf("��ʼ��pattern1w��ʱ: %f ��\n", (double)(end_time_p - start_time_p) / CLOCKS_PER_SEC);
    //printf("���� pattern1w ��ʱ�䣺%f ��\n", (double)(end_time_t - start_time_t) / CLOCKS_PER_SEC);
    printf("�ҵ�����ƥ����: %d\n", match_count);

    free(text_data);
    free_memory(root);

    return 0;
}