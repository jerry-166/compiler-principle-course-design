/*
 * sem.h - 语义分析接口（实践 2）
 */
#ifndef SEM_H
#define SEM_H

#include "node.h"

/* 入口：遍历语法树做语义检查，错误用 out() 输出。*/
void sem_analyze(Node *root);

#endif /* SEM_H */
