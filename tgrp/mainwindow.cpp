#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "tgrp.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    startup();

    connect(ui->dateFr, &QDateEdit::dateChanged, this, &MainWindow::dateSpanChanged);
    connect(ui->dateTo, &QDateEdit::dateChanged, this, &MainWindow::dateSpanChanged);
    connect(ui->filterInput, &QLineEdit::textChanged, this, &MainWindow::filterChanged);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::stylesDefaults()
{
    fin = ui->filterInput;
    fin_ss_def = "QLineEdit{ color: white; }\nQLineEdit[text=\"\"]{ color: gray; }";
    fin->setStyleSheet(fin_ss_def); // fix: override placeholderText color by gray
}

/*
    set order in which we switch between widgets by pressing Tab key
*/
void MainWindow::setTabbingOrder()
{
    QWidget::setTabOrder(fin, ui->dateFr);
    QWidget::setTabOrder(ui->dateFr, ui->dateTo);
    QWidget::setTabOrder(ui->dateTo, ui->scrollArea);
    QWidget::setTabOrder(ui->scrollArea, ui->scrollAreaWidgetContents);
}

void MainWindow::startup()
{
    stylesDefaults();
    setTabbingOrder();
    setLastWeekSpan();
    dateSpanChanged();
}

void MainWindow::setLastWeekSpan()
{
    date_to = QDate::currentDate();
    date_fr = date_to.addDays(1-date_to.dayOfWeek()); // monday
    ui->dateFr->setDate(date_fr);
    ui->dateTo->setDate(date_to);
}

std::pair<std::string, std::string> MainWindow::getDateFrTo()
{
    date_fr = ui->dateFr->date();
    date_to = ui->dateTo->date();
    std::string fr = date_fr.toString("yyyy-MM-dd").toStdString();
    std::string to = date_to.toString("yyyy-MM-dd").toStdString();
    return std::make_pair(fr, to);
}

void MainWindow::setTxt(const QString &txt)
{
    ui->plainTextEdit->setPlainText(txt);
    qDebug() << "New text was set!";
}

void MainWindow::dateSpanChanged()
{
    date_fr = ui->dateFr->date();
    date_to = ui->dateTo->date();
    if (!date_fr.isValid() || !date_to.isValid()) {
        qDebug() << "Not valid date, processing was skipped.";
        return;
    }
    // allow fr-to dates exchange - swap variables
    if (date_to < date_fr) {
        QDate tmpdate = date_fr;
        date_fr = date_to;
        date_to = tmpdate;
    }

    std::string fr = date_fr.toString("yyyy-MM-dd").toStdString();
    std::string to = date_to.toString("yyyy-MM-dd").toStdString();
    std::string content = concat_span(fr, to);
    TXT_RAW = QString::fromStdString(content);
    setTxt(TXT_RAW);
    // try to apply filter back after changing the date span
    if (!fin->text().isEmpty())
        filterChanged();
}

void MainWindow::filterChanged()
{
    const QString pattern = fin->text();
    if (pattern.isEmpty()) {
        setTxt(TXT_RAW); // set back not filtered text after clearing filter pattern
        fin->setStyleSheet(fin_ss_def);
        return;
    }

    re_filter = QRegularExpression(pattern);
    if (!re_filter.isValid()) {
        fin->setStyleSheet("color: red"); // indicate not valid regex by the red text color
        return;
    } else {
        fin->setStyleSheet(fin_ss_def);
    }

    std::string filtered = filter_find(TXT_RAW.toStdString(), re_filter.pattern().toStdString());
    TXT_FILTERED = QString::fromStdString(filtered);
    setTxt(TXT_FILTERED);
}
