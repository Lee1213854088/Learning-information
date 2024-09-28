#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_PATTERNS 100000 // 假设模式集中最多有100000个模式
#define MAX_PATTERN_LENGTH 500 //// 假设每个模式的最大长度为500
#define ALPHABET_SIZE 256 // ASCII字符集大小


//定义节点的结构体
typedef struct Node {
    struct Node* children[ALPHABET_SIZE];
    struct Node* fail;
    int is_end_of_word;
    char* pattern; //用来存储到达该状态所匹配的完整模式串
} Node;



//新建并初始化节点
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



//将一个模式字符串插入到trie树中
void insert_pattern(Node* root, const char* pattern) {
    Node* current = root; //current指向当前节点
    const char* original_pattern = pattern; //保存pattern的起始地址
    while (*pattern) {
        if (current->children[*pattern] == NULL) {
            current->children[*pattern] = create_node();
        }
        current = current->children[*pattern];
        pattern++;
    }
    current->is_end_of_word = 1;
    current->pattern = _strdup(original_pattern);//将模式字符串存储在节点中
}



//构建失效函数（用链表和队列实现）
// 定义链表节点和队列结构
typedef struct QueueNode {
    Node* trie_node;
    struct QueueNode* next;
} QueueNode;

typedef struct {
    QueueNode* front;
    QueueNode* rear;
} Queue;

// 创建队列
Queue* createQueue() {
    Queue* q = (Queue*)malloc(sizeof(Queue));
    q->front = q->rear = NULL;
    return q;
}

// 入队操作
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

// 出队操作
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

// 检查队列是否为空
int isEmpty(Queue* q) {
    return q->front == NULL;
}

// 释放队列内存
void freeQueue(Queue* q) {
    while (!isEmpty(q)) {
        dequeue(q);
    }
    free(q);
}

//构建失效函数
void build_fail_pointers(Node* root) {
    // 使用BFS算法来构建fail指针
    Node* temp;
    Node* fail_node;
    Queue* queue = createQueue(); // 使用链表实现的队列

    // 第一层的节点的失败函数全部指向root节点
    int i;
    for (i = 0; i < ALPHABET_SIZE; i++) {
        if (root->children[i] != NULL) {
            root->children[i]->fail = root;
            enqueue(queue, root->children[i]);//BFS将root节点的全部子节点加入队列
        }
    }

    while (!isEmpty(queue)) {
        temp = dequeue(queue); // temp的失效函数是已知的，即非空
        for (i = 0; i < ALPHABET_SIZE; i++) {
            if (temp->children[i] != NULL) {
                fail_node = temp->fail;
                //如果当前节点的失效函数指向的节点的对应子节点是空的，追溯失效函数指向的失效函数
                while (fail_node != NULL && fail_node->children[i] == NULL) {
                    fail_node = fail_node->fail;
                }
                //一直追溯到失效函数指向空，即根节点的失效函数
                if (fail_node == NULL) {
                    temp->children[i]->fail = root;
                }
                //或者找到了合适的非空子节点，将这个子节点赋给当前节点子节点的失效函数
                else {
                    temp->children[i]->fail = fail_node->children[i];
                }
                //将该子节点入队
                enqueue(queue, temp->children[i]);
            }
        }
    }

    freeQueue(queue); // 释放队列的内存
}



//在给定的文本中搜索trie树中的模式
int search_text(Node* root, const char* text) {
    Node* current = root;
    int match_count = 0;
    while (*text) {
        //如果当前节点不为空，并且在当前节点的子节点中没有对应当前字符的分支，则进入循环
        while (current != NULL && current->children[*text] == NULL) {
            //在 Trie 树中向上回溯，通过 fail 指针移动到其他分支
            current = current->fail;
        }
        // 如果当前节点为空，则已经回溯到根节点且没有找到对应的分支
        if (current == NULL) {
            current = root;//开始新一轮的匹配
        }
        else {
            current = current->children[*text];
        }
        Node* temp = current;
        while (temp != root && temp->is_end_of_word) {
            printf("找到模式串: %s\n", temp->pattern);
            match_count++;
            // 将临时节点移动到其失效函数指向的节点，以检查其他可能的匹配
            temp = temp->fail;
        }
        text++;
    }
    return match_count;
}



//释放在trie树中分配的所有内存
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
        printf("请输入模式文件序号(1~3)：\n");
        //scanf("%d",&p);
        scanf_s("%d", &p, sizeof(p));
        //printf("请输入要测试的文件序号：\n");
        //scanf("%d", &t);
        //scanf_s("%s", &t, sizeof(t));
        if (p == 1) {
            // 载入模式集并构建AC自动机
            err = fopen_s(&pattern_file, "pattern1w.txt", "r");
            if (err != 0) {
                fprintf(stderr, "无法打开 'pattern1w.txt'\n");
                return 1;
            }
        }
        if (p == 2) {
            // 载入模式集并构建AC自动机
            err = fopen_s(&pattern_file, "pattern2w.txt", "r");
            if (err != 0) {
                fprintf(stderr, "无法打开 'pattern2w.txt'\n");
                return 1;
            }
        }
        if (p == 3) {
            // 载入模式集并构建AC自动机
            err = fopen_s(&pattern_file, "pattern3w.txt", "r");
            if (err != 0) {
                fprintf(stderr, "无法打开 'pattern3w.txt'\n");
                return 1;
            }
        }
    }
    start_time_p = clock();
    while (fgets(pattern, MAX_PATTERN_LENGTH, pattern_file)) {
        // 去除可能的换行符
        pattern[strcspn(pattern, "\r\n")] = 0;
        insert_pattern(root, pattern);
    }
    build_fail_pointers(root);
    end_time_p = clock();
    //printf("初始化pattern1w耗时: %f 秒\n", (double)(end_time_p - start_time_p) / CLOCKS_PER_SEC);
    fclose(pattern_file);

    // 读取文本数据
    //text_file = fopen("text.txt", "r");
    err = fopen_s(&text_file, "text.txt", "r");
    if (err != 0) {
        fprintf(stderr, "无法打开 'text.txt'\n");
        return 1;
    }
    //if (!text_file) {
     //   fprintf(stderr, "Error opening text file.\n");
      //  fclose(pattern_file);
        //free_memory(root);
        //return 1; // 退出
        //程序
    //}
    fseek(text_file, 0, SEEK_END);
    length = ftell(text_file);
    fseek(text_file, 0, SEEK_SET);
    text_data = (char*)malloc(length + 1); // 显式转换为 char*
    
    if (text_data) {
        //fread(text_data, 1, length, text_file);
        size_t bytes_read = fread(text_data, 1, length, text_file);
        text_data[bytes_read] = '\0'; // 设置字符串终止符
    }
    else {
        fprintf(stderr, "为文本数据分配内存失败。\n");
        fclose(text_file);
        fclose(pattern_file);
        free_memory(root);
        return 1; // 退出程序
    }
    fclose(text_file);

    // 搜索匹配
    start_time_t = clock();
    match_count = search_text(root, text_data);
    end_time_t = clock();
    if (p == 1) {
        printf("初始化pattern1w耗时: %f 秒\n", (double)(end_time_p - start_time_p) / CLOCKS_PER_SEC);
        printf("搜索 pattern1w 的时间：%f 秒\n", (double)(end_time_t - start_time_t) / CLOCKS_PER_SEC);
    }
    if (p == 2) {
        printf("初始化pattern2w耗时: %f 秒\n", (double)(end_time_p - start_time_p) / CLOCKS_PER_SEC);
        printf("搜索 pattern2w 的时间：%f 秒\n", (double)(end_time_t - start_time_t) / CLOCKS_PER_SEC);
    }
    if (p == 3) {
        printf("初始化pattern3w耗时: %f 秒\n", (double)(end_time_p - start_time_p) / CLOCKS_PER_SEC);
        printf("搜索 pattern3w 的时间：%f 秒\n", (double)(end_time_t - start_time_t) / CLOCKS_PER_SEC);
    }
    //printf("初始化pattern1w耗时: %f 秒\n", (double)(end_time_p - start_time_p) / CLOCKS_PER_SEC);
    //printf("搜索 pattern1w 的时间：%f 秒\n", (double)(end_time_t - start_time_t) / CLOCKS_PER_SEC);
    printf("找到的总匹配数: %d\n", match_count);

    free(text_data);
    free_memory(root);

    return 0;
}