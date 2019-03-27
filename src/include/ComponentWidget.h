#ifndef COMPONENTFACTORYWIDGET_H
#define COMPONENTFACTORYWIDGET_H

#include <QWidget>
#include <QComboBox>
#include <QPushButton>
#include <QCheckBox>
#include <QLabel>

namespace Ui {
class ComponentFactoryWidget;
class ComponentWidget;
}

class ComponentFactoryWidget : public QWidget
{
    Q_OBJECT

private:
    Ui::ComponentFactoryWidget *ui;

public:
    explicit ComponentFactoryWidget(QWidget *parent = nullptr);
    ~ComponentFactoryWidget();

    QComboBox *&combo;
    QPushButton *&button;
};

class ComponentWidget : public QWidget
{
    Q_OBJECT

private:
    Ui::ComponentWidget *ui;

public:
    explicit ComponentWidget(QWidget *parent = nullptr);
    ~ComponentWidget();

    QLabel *&label;
    QPushButton *&params;
    QCheckBox *&active;
    QCheckBox *&activeSettling;
};

#endif // COMPONENTFACTORYWIDGET_H
