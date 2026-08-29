#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVector>

struct AACVocabItem {
    int id = -1;
    QString label;
    QString iconPath;
    QString category;
};

class QSqlDatabase;

class AACVocabularyManager : public QObject {
    Q_OBJECT
public:
    explicit AACVocabularyManager(QObject* parent = nullptr);

    bool initialize();

    QStringList categories() const;
    QVector<AACVocabItem> wordsInCategory(const QString& category) const;

    bool addCategory(const QString& name, const QString& iconPath);
    bool addWord(const QString& category, const QString& label, const QString& iconPath);

    bool updateWord(int id, const QString& newLabel, const QString& newIconPath);
    bool deleteWord(int id);

signals:
    void categoriesChanged();
    void vocabularyChanged();

private:
    bool createSchema();
    bool ensureUserIconDir();

    QSqlDatabase* m_db = nullptr;
    QString m_dbPath;
};
