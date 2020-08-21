/********************************************************************************
** Form generated from reading UI file 'filetransfer.ui'
**
** Created by: Qt User Interface Compiler version 5.15.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_FILETRANSFER_H
#define UI_FILETRANSFER_H

#include <QtCore/QLocale>
#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_FileTransferDlg
{
public:
    QVBoxLayout *verticalLayout_2;
    QFormLayout *formLayout;
    QLabel *label;
    QLabel *filenameLabel;
    QLabel *label_2;
    QLabel *filesizeLabel;
    QLabel *label_3;
    QLabel *throughputLabel;
    QLabel *label_4;
    QLabel *destinationLabel;
    QVBoxLayout *verticalLayout;
    QProgressBar *progressBar;
    QLabel *transferredLabel;
    QHBoxLayout *horizontalLayout;
    QCheckBox *closeChkBox;
    QSpacerItem *horizontalSpacer;
    QPushButton *cancelButton;
    QPushButton *openButton;

    void setupUi(QDialog *FileTransferDlg)
    {
        if (FileTransferDlg->objectName().isEmpty())
            FileTransferDlg->setObjectName(QString::fromUtf8("FileTransferDlg"));
        FileTransferDlg->resize(324, 177);
        FileTransferDlg->setLocale(QLocale(QLocale::C, QLocale::AnyCountry));
        FileTransferDlg->setSizeGripEnabled(true);
        verticalLayout_2 = new QVBoxLayout(FileTransferDlg);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        formLayout = new QFormLayout();
        formLayout->setObjectName(QString::fromUtf8("formLayout"));
        label = new QLabel(FileTransferDlg);
        label->setObjectName(QString::fromUtf8("label"));

        formLayout->setWidget(0, QFormLayout::LabelRole, label);

        filenameLabel = new QLabel(FileTransferDlg);
        filenameLabel->setObjectName(QString::fromUtf8("filenameLabel"));
        filenameLabel->setText(QString::fromUtf8("TextLabel"));

        formLayout->setWidget(0, QFormLayout::FieldRole, filenameLabel);

        label_2 = new QLabel(FileTransferDlg);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        formLayout->setWidget(1, QFormLayout::LabelRole, label_2);

        filesizeLabel = new QLabel(FileTransferDlg);
        filesizeLabel->setObjectName(QString::fromUtf8("filesizeLabel"));
        filesizeLabel->setText(QString::fromUtf8("TextLabel"));

        formLayout->setWidget(1, QFormLayout::FieldRole, filesizeLabel);

        label_3 = new QLabel(FileTransferDlg);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        formLayout->setWidget(2, QFormLayout::LabelRole, label_3);

        throughputLabel = new QLabel(FileTransferDlg);
        throughputLabel->setObjectName(QString::fromUtf8("throughputLabel"));
        throughputLabel->setText(QString::fromUtf8("TextLabel"));

        formLayout->setWidget(2, QFormLayout::FieldRole, throughputLabel);

        label_4 = new QLabel(FileTransferDlg);
        label_4->setObjectName(QString::fromUtf8("label_4"));

        formLayout->setWidget(3, QFormLayout::LabelRole, label_4);

        destinationLabel = new QLabel(FileTransferDlg);
        destinationLabel->setObjectName(QString::fromUtf8("destinationLabel"));
        destinationLabel->setText(QString::fromUtf8("TextLabel"));

        formLayout->setWidget(3, QFormLayout::FieldRole, destinationLabel);


        verticalLayout_2->addLayout(formLayout);

        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        progressBar = new QProgressBar(FileTransferDlg);
        progressBar->setObjectName(QString::fromUtf8("progressBar"));
        progressBar->setValue(0);
        progressBar->setTextVisible(false);
        progressBar->setFormat(QString::fromUtf8("%p%"));

        verticalLayout->addWidget(progressBar);

        transferredLabel = new QLabel(FileTransferDlg);
        transferredLabel->setObjectName(QString::fromUtf8("transferredLabel"));
        transferredLabel->setText(QString::fromUtf8("TextLabel"));
        transferredLabel->setAlignment(Qt::AlignCenter);

        verticalLayout->addWidget(transferredLabel);


        verticalLayout_2->addLayout(verticalLayout);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        closeChkBox = new QCheckBox(FileTransferDlg);
        closeChkBox->setObjectName(QString::fromUtf8("closeChkBox"));

        horizontalLayout->addWidget(closeChkBox);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        cancelButton = new QPushButton(FileTransferDlg);
        cancelButton->setObjectName(QString::fromUtf8("cancelButton"));

        horizontalLayout->addWidget(cancelButton);

        openButton = new QPushButton(FileTransferDlg);
        openButton->setObjectName(QString::fromUtf8("openButton"));

        horizontalLayout->addWidget(openButton);


        verticalLayout_2->addLayout(horizontalLayout);


        retranslateUi(FileTransferDlg);

        QMetaObject::connectSlotsByName(FileTransferDlg);
    } // setupUi

    void retranslateUi(QDialog *FileTransferDlg)
    {
        FileTransferDlg->setWindowTitle(QCoreApplication::translate("FileTransferDlg", "File Transfer", nullptr));
        label->setText(QCoreApplication::translate("FileTransferDlg", "Filename:", nullptr));
        label_2->setText(QCoreApplication::translate("FileTransferDlg", "File size:", nullptr));
        label_3->setText(QCoreApplication::translate("FileTransferDlg", "Throughput:", nullptr));
        label_4->setText(QCoreApplication::translate("FileTransferDlg", "Destination:", nullptr));
        closeChkBox->setText(QCoreApplication::translate("FileTransferDlg", "C&lose when completed", nullptr));
        cancelButton->setText(QCoreApplication::translate("FileTransferDlg", "&Cancel", nullptr));
        openButton->setText(QCoreApplication::translate("FileTransferDlg", "&Open", nullptr));
    } // retranslateUi

};

namespace Ui {
    class FileTransferDlg: public Ui_FileTransferDlg {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_FILETRANSFER_H
