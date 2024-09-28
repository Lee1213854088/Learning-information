# coding: utf-8
import sys

import numpy as np
from sklearn import metrics
import joblib
from sklearn.feature_extraction.text import HashingVectorizer

#多项式朴素贝叶斯分类器
class MultinomialNB:
    def __init__(self, alpha=1.0):
        self.alpha = alpha  # 朴素贝叶斯分类器的平滑参数 alpha
        self.log_class_prior = None   # 每个类别的类别先验概率
        self.log_feature_proba = None   # 每个特征的类别条件概率

    # 训练模型
    def fit(self, X, y):    # X 是输入特征数据，y 是目标标签数据
        # 样本数量 类别数量
        n_samples, n_features = X.shape
        # 类别数组      每个类别样本数量数组
        self.classes, class_counts = np.unique(y, return_counts=True)
        # 使用了拉普拉斯平滑计算类别先验概率
        self.log_class_prior = np.log(class_counts + self.alpha) - np.log(n_samples + len(self.classes) * self.alpha)
        # 存储每个类别中每个特征的计数
        feature_counts = np.zeros((len(self.classes), n_features))
        for i, c in enumerate(self.classes):
            feature_counts[i, :] = X[y == c].sum(axis=0)
        # 使用了拉普拉斯平滑计算类别条件概率
        feature_proba = (feature_counts.T + self.alpha) / (class_counts + n_features * self.alpha)
        self.log_feature_proba = np.log(feature_proba).T

    # 进行预测
    def predict(self, X):
        # 计算了每个样本的对数后验概率
        log_posterior = X @ self.log_feature_proba.T + self.log_class_prior
        # 选择具有最大对数后验概率的类别作为预测结果
        return self.classes[np.argmax(log_posterior, axis=1)]

'''加载数据'''
def input_data(train_file, test_file):
    train_words = []
    train_tags = []

    for line in open(train_file, 'r', encoding='UTF-8', errors='ignore'):
        tks = line.split('\t')
        train_words.append(tks[0])
        train_tags.append(tks[1])

    test_words = []
    test_tags = []
    for line in open(test_file, 'r', encoding='UTF-8', errors='ignore'):
        tks = line.split('\t')
        test_words.append(tks[0])
        test_tags.append(tks[1])

    return train_words, train_tags, test_words, test_tags

'''文本向量化'''
def vectorize(train_words, test_words):
    # 停用词表
    with open('dict/stopwords.txt', 'r', encoding='UTF-8', errors='ignore') as f:
        stopwords = list([w.strip() for w in f])

    v = HashingVectorizer(stop_words=stopwords, n_features=30000)
    train_data = v.fit_transform(train_words)
    test_data = v.fit_transform(test_words)
    return train_data, test_data

'''计算准确率、召回率、F1'''
def evaluate(actual, pred):
    m_precision = metrics.precision_score(actual, pred, average='weighted')
    m_recall = metrics.recall_score(actual, pred, average='weighted')
    m_f1_score = metrics.f1_score(actual,pred, average='weighted')

    print(u' 准确率:     {0:0.3f}'.format(m_precision))
    print(u' 召回率:     {0:0.3f}'.format(m_recall))
    print(u' F1-score:   {0:0.3f}'.format(m_f1_score))


'''多种分类算法'''
# Multinomial Naive Bayes Classifier（多项朴素贝叶斯）
def train_clf_MNB(train_data, train_tags):
    classifer = MultinomialNB()
    train_data = train_data.toarray()
    train_tags = np.array(train_tags)
    print(train_data.min())
    train_data -= train_data.min()
    print(train_data.min())
    classifer.fit(train_data, train_tags)
    return classifer
    # 实现多项朴素贝叶斯
    # 实现多项朴素贝叶斯
    # 实现多项朴素贝叶斯
    # 实现多项朴素贝叶斯
    # 实现多项朴素贝叶斯
    # 实现多项朴素贝叶斯


def main():
    train_file = 'data/train.txt'
    test_file = 'data/test.txt'
    # 加载数据
    print('\nload =======================================')
    train_words, train_tags, test_words, test_tags = input_data(train_file, test_file)
    print('Step 1: input_data OK')
    train_data, test_data = vectorize(train_words, test_words)
    print('Step 2: vectorize OK')

    # Multinomial Naive Bayes Classifier
    print('\nMNB =======================================')
    # 训练
    clf = train_clf_MNB(train_data, train_tags)
    print('Step 3: train_clf OK')
    # 保存模型
    joblib.dump(clf, 'model/'+str(type(clf))[8:-2]+'.model')
    print('Step 4: model save OK')
    # 预测
    pred = clf.predict(test_data)
    print('Step 5: predict OK')
    # 计算准确率、召回率、F1
    evaluate(np.asarray(test_tags), pred)
    print('Step 6: evaluate OK')


if __name__ == '__main__':
    main()