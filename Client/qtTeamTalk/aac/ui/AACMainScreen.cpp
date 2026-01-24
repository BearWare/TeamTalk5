#include "AACMainScreen.h"

#include <QLabel>
#include <QPushButton>
#include <QSpacerItem>
#include <QSizePolicy>

AACMainScreen::AACMainScreen(AACAccessibilityManager* mgr, QWidget* parent)
    : QWidget(parent)
    , m_mgr(mgr)
    , m_pred(mgr ? mgr->predictionEngine() : nullptr)
{
    m_rootLayout = new QVBoxLayout(this);
    m_rootLayout->setContentsMargins(8, 8, 8, 8);
    m_rootLayout->setSpacing(8);

    m_label = new QLabel(tr("Message:"), this);
    m_text  = new QLineEdit(this);
    m_text->setPlaceholderText(tr("Type or select symbols..."));

    connect(m_text, &QLineEdit::textChanged,
            this, &AACMainScreen::onTextChanged);

    // Prediction strip container
    m_predictiveStripContainer = new QWidget(this);
    QHBoxLayout* stripLayout = new QHBoxLayout(m_predictiveStripContainer);
    stripLayout->setContentsMargins(0, 0, 0, 0);
    stripLayout->setSpacing(4);

    m_predictionBar = new QListWidget(m_predictiveStripContainer);
    m_predictionBar->setSelectionMode(QAbstractItemView::SingleSelection);
    m_predictionBar->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_predictionBar->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_predictionBar->setWrapping(true);
    m_predictionBar->setFlow(QListView::LeftToRight);

    stripLayout->addWidget(m_predictionBar);

    connect(m_predictionBar, &QListWidget::itemClicked,
            this, &AACMainScreen::onPredictionClicked);

    // Commit / delete buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    buttonLayout->setSpacing(8);

    m_commitButton = new QPushButton(tr("Speak / Send"), this);
    m_deleteButton = new QPushButton(tr("Delete word"), this);

    connect(m_commitButton, &QPushButton::clicked,
            this, &AACMainScreen::onCommitButtonClicked);
    connect(m_deleteButton, &QPushButton::clicked,
            this, &AACMainScreen::onDeleteButtonClicked);

    buttonLayout->addWidget(m_commitButton);
    buttonLayout->addWidget(m_deleteButton);
    buttonLayout->addStretch(1);

    // Assemble layout
    m_rootLayout->addWidget(m_label);
    m_rootLayout->addWidget(m_text);
    m_rootLayout->addWidget(m_predictiveStripContainer);
    m_rootLayout->addLayout(buttonLayout);
    m_rootLayout->addStretch(1);

    // Periodic tick for ignore‑timeout / decay
    connect(&m_tickTimer, &QTimer::timeout, this, [this]() {
        if (m_pred)
            m_pred->tick();
    });
    m_tickTimer.start(250);
}

QList<QWidget*> AACMainScreen::interactiveWidgets() const
{
    QList<QWidget*> out;
    out << const_cast<QLineEdit*>(m_text);
    out << const_cast<QListWidget*>(m_predictionBar);
    out << const_cast<QPushButton*>(m_commitButton);
    out << const_cast<QPushButton*>(m_deleteButton);
    return out;
}

QList<QWidget*> AACMainScreen::primaryWidgets() const
{
    QList<QWidget*> out;
    out << const_cast<QLineEdit*>(m_text);
    out << const_cast<QListWidget*>(m_predictionBar);
    out << const_cast<QPushButton*>(m_commitButton);
    return out;
}

QLayout* AACMainScreen::rootLayout() const
{
    return m_rootLayout;
}

QWidget* AACMainScreen::predictiveStripContainer() const
{
    return m_predictiveStripContainer;
}

void AACMainScreen::setText(const QString& text)
{
    m_text->setText(text);
}

QString AACMainScreen::extractPrevWord(const QString& text) const
{
    QString trimmed = text.trimmed();
    if (trimmed.isEmpty())
        return QString();

    const QStringList parts =
        trimmed.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
    if (parts.isEmpty())
        return QString();

    return parts.last();
}

QString AACMainScreen::extractSecondLastWord(const QString& text) const
{
    QString trimmed = text.trimmed();
    const QStringList parts =
        trimmed.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
    if (parts.size() < 2)
        return QString();
    return parts[parts.size() - 2];
}

void AACMainScreen::onTextChanged(const QString& text)
{
    if (!m_pred || !m_mgr || !m_mgr->predictionEnabled())
        return;

    const std::string prefix = text.toStdString();
    auto suggestions = m_pred->Predict(prefix, 5);

    m_predictionBar->clear();
    if (!suggestions.empty()) {
        m_pred->onPredictionBarShown();
        for (const auto& s : suggestions)
            m_predictionBar->addItem(QString::fromStdString(s));
    }
}

void AACMainScreen::onPredictionClicked(QListWidgetItem* item)
{
    if (!item || !m_pred)
        return;

    const QString chosenQ = item->text();
    const std::string chosen = chosenQ.toStdString();

    const QString currentText = m_text->text();
    QString prevWord = extractPrevWord(currentText);
    const std::string prev = prevWord.toLower().toStdString();

    std::vector<std::string> shown;
    shown.reserve(m_predictionBar->count());
    for (int i = 0; i < m_predictionBar->count(); ++i)
        shown.push_back(m_predictionBar->item(i)->text().toStdString());

    m_pred->reinforceChoice(prev, chosen);
    m_pred->onUserSelected(prev, chosen, shown);

    QStringList parts =
        currentText.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
    if (!parts.isEmpty())
        parts.removeLast();
    parts << chosenQ;

    QString newText = parts.join(" ") + " ";
    m_text->setText(newText);
    emit textCommitted(newText);
}

void AACMainScreen::onCommitButtonClicked()
{
    const QString text = m_text->text().trimmed();
    if (text.isEmpty())
        return;

    // Treat last word as "actual typed" if predictions were visible
    if (m_pred && m_mgr && m_mgr->predictionEnabled()) {
        QString actualWord = extractPrevWord(text);
        QString prevWord   = extractSecondLastWord(text);

        const std::string actual = actualWord.toLower().toStdString();
        const std::string prev   = prevWord.toLower().toStdString();

        std::vector<std::string> shown;
        shown.reserve(m_predictionBar->count());
        for (int i = 0; i < m_predictionBar->count(); ++i)
            shown.push_back(m_predictionBar->item(i)->text().toStdString());

        if (!actual.empty())
            m_pred->penalizeIgnored(prev, shown, actual);
    }

    emit textCommitted(text);
}

void AACMainScreen::onDeleteButtonClicked()
{
    QString current = m_text->text();
    QString prevWord = extractPrevWord(current);
    if (prevWord.isEmpty())
        return;

    if (m_pred)
        m_pred->onUserDeletedAutocompleted(prevWord.toLower().toStdString());

    QString trimmed = current.trimmed();
    QStringList parts =
        trimmed.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
    if (!parts.isEmpty())
        parts.removeLast();

    m_text->setText(parts.join(" ") + (parts.isEmpty() ? "" : " "));
}
