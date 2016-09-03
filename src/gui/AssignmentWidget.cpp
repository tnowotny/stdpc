#include "AssignmentWidget.h"
#include "ObjectDataTypes.h"

template class AssignmentWidget<CurrentAssignment>;
template class AssignmentWidget<SynapseAssignment>;
template class AssignmentWidget<GapJunctionAssignment>;

template <class A>
AssignmentWidget<A>::AssignmentWidget(QVector<Dropdown<A>> drops, QWidget *parent = 0) :
    QComboBox(parent),
    drops(d)
{
    QStringList labels("Active");
    int i = 1;
    setColumnCount(drops.size() + 1);
    setColumnWidth(0, 50);
    for ( Dropdown<A> &d : drops ) {
        setColumnWidth(i++, d.columnWidth);
        labels.append(d.label);
    }
    setHorizontalHeaderLabels(labels);
    growTable();
}

template <class A>
void AssignmentWidget<A>::exportData(std::vector<A> &p)
{
    A a;
    p.clear();
    for ( int i = 0; i < rowCount() - 1; i++ ) {
        a.active = boxes[i]->isChecked();
        for ( Dropdown<A> &d : drops )
            a.*(d.channel) = d.combo[i]->currentData().toPoint().x();
        p.push_back(a);
    }
}

template <class A>
void AssignmentWidget<A>::importData(std::vector<A> &p)
{
    QCheckBox *box;

    boxes.clear();
    for ( Dropdown<A> &d : drops ) {
        d.combo.clear();
        d.connection.disconnect();
    }
    setRowCount(0);

    int i = 0;
    for ( A const& a : p ) {
        box = new QCheckBox(this);
        box->setChecked(a.active);
        addRow(i, box);
        for ( Dropdown<A> &d : drops ) {
            d.combo[i]->setCurrentIndex(d.model->index(a.*(d.channel)));
            ChannelListModel::fixComboBoxWidth(d.combo[i]);
        }
        ++i;
    }

    growTable();
}

template <class A>
void AssignmentWidget<A>::addRow(int row, QCheckBox *box)
{
    insertRow(row);
    setRowHeight(row, 25);

    QWidget *widget = new QWidget();
    QHBoxLayout *layout = new QHBoxLayout(widget);
    layout->addWidget(box);
    layout->setAlignment(Qt::AlignCenter);
    layout->setMargin(0);
    widget->setLayout(layout);
    setCellWidget(row, 0, widget);
    boxes.insert(row, box);

    int i = 1;
    for ( Dropdown<A> &d : drops ) {
        QComboBox *combo = new QComboBox(this);
        combo->setModel(d.model);
        connect(d, ChannelListModel::layoutChanged(), [=](){ChannelListModel::fixComboBoxWidth(d.model)});
        setCellWidget(row, i++, combo);
        ChannelListModel::fixComboBoxWidth(combo);
        d.combo.insert(row, combo);
    }
}

template <class A>
void AssignmentWidget<A>::grow()
{
    QCheckBox *box = new QCheckBox();
    if ( !boxes.empty() )
        boxes.last()->setChecked(true);

    addRow(rowCount(), box);

    disconnect(boxc);
    boxc = connect(box, SIGNAL(stateChanged(int)), this, SLOT(growTable()));

    for ( Dropdown<A> &d : drops ) {
        disconnect(d.connection);
        d.connection = connect(d.combo.last(), SIGNAL(currentIndexChanged(int)), this, SLOT(grow()));
    }
}
