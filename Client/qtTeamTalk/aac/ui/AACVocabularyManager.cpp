#include "AACVocabularyManager.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDir>
#include <QStandardPaths>
#include <QDebug>

AACVocabularyManager::AACVocabularyManager(QObject* parent)
    : QObject(parent)
{
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dataDir);

    m_dbPath = dataDir + "/vocabulary.db";
}

bool AACVocabularyManager::initialize()
{
    m_db = new QSqlDatabase(QSqlDatabase::addDatabase("QSQLITE", "aac_vocab"));
    m_db->setDatabaseName(m_dbPath);

    if (!m_db->open()) {
        qWarning() << "Failed to open vocabulary DB:" << m_db->lastError();
        return false;
    }

    ensureUserIconDir();
    return createSchema();
}

bool AACVocabularyManager::ensureUserIconDir()
{
    QString iconDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
                      + "/user_icons/";
    return QDir().mkpath(iconDir);
}

bool AACVocabularyManager::createSchema()
{
    QSqlQuery q(*m_db);

    if (!q.exec(R"(
        CREATE TABLE IF NOT EXISTS categories (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT UNIQUE,
            iconPath TEXT
        );
    )")) return false;

    if (!q.exec(R"(
        CREATE TABLE IF NOT EXISTS words (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            categoryId INTEGER,
            label TEXT,
            iconPath TEXT,
            FOREIGN KEY(categoryId) REFERENCES categories(id)
        );
    )")) return false;

    return true;
}

QStringList AACVocabularyManager::categories() const
{
    QStringList list;
    QSqlQuery q(*m_db);

    q.exec("SELECT name FROM categories ORDER BY name ASC;");
    while (q.next())
        list << q.value(0).toString();

    return list;
}

QVector<AACVocabItem> AACVocabularyManager::wordsInCategory(const QString& category) const
{
    QVector<AACVocabItem> items;

    QSqlQuery q(*m_db);
    q.prepare(R"(
        SELECT w.id, w.label, w.iconPath, c.name
        FROM words w
        JOIN categories c ON w.categoryId = c.id
        WHERE c.name = :cat
        ORDER BY w.label ASC;
    )");
    q.bindValue(":cat", category);
    q.exec();

    while (q.next()) {
        AACVocabItem item;
        item.id = q.value(0).toInt();
        item.label = q.value(1).toString();
        item.iconPath = q.value(2).toString();
        item.category = q.value(3).toString();
        items.append(item);
    }

    return items;
}

bool AACVocabularyManager::addCategory(const QString& name, const QString& iconPath)
{
    QSqlQuery q(*m_db);
    q.prepare("INSERT INTO categories (name, iconPath) VALUES (:n, :i);");
    q.bindValue(":n", name);
    q.bindValue(":i", iconPath);

    if (!q.exec())
        return false;

    emit categoriesChanged();
    return true;
}

bool AACVocabularyManager::addWord(const QString& category, const QString& label, const QString& iconPath)
{
    QSqlQuery q(*m_db);

    // Find category ID
    q.prepare("SELECT id FROM categories WHERE name = :n;");
    q.bindValue(":n", category);
    q.exec();

    if (!q.next())
        return false;

    int catId = q.value(0).toInt();

    // Insert word
    q.prepare("INSERT INTO words (categoryId, label, iconPath) VALUES (:c, :l, :i);");
    q.bindValue(":c", catId);
    q.bindValue(":l", label);
    q.bindValue(":i", iconPath);

    if (!q.exec())
        return false;

    emit vocabularyChanged();
    return true;
}

bool AACVocabularyManager::updateWord(int id, const QString& newLabel, const QString& newIconPath)
{
    QSqlQuery q(*m_db);
    q.prepare("UPDATE words SET label = :l, iconPath = :i WHERE id = :id;");
    q.bindValue(":l", newLabel);
    q.bindValue(":i", newIconPath);
    q.bindValue(":id", id);

    if (!q.exec())
        return false;

    emit vocabularyChanged();
    return true;
}

bool AACVocabularyManager::deleteWord(int id)
{
    QSqlQuery q(*m_db);
    q.prepare("DELETE FROM words WHERE id = :id;");
    q.bindValue(":id", id);

    if (!q.exec())
        return false;

    emit vocabularyChanged();
    return true;
}
