#ifndef CHECKBOXDELEGATE_H
#define CHECKBOXDELEGATE_H

#include <QApplication>
#include <QItemDelegate>
#include <QCheckBox>
#include <QPainter>

class CheckBoxDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    explicit CheckBoxDelegate(QObject *parent = 0);

    void paint( QPainter *painter,
                        const QStyleOptionViewItem &option,
                        const QModelIndex &index ) const override;

    QWidget *createEditor( QWidget *parent,
                        const QStyleOptionViewItem &option,
                        const QModelIndex &index ) const override;

    void setEditorData( QWidget *editor,
                        const QModelIndex &index ) const override;

    void setModelData( QWidget *editor,
                        QAbstractItemModel *model,
                        const QModelIndex &index ) const override;

    void updateEditorGeometry( QWidget *editor,
                        const QStyleOptionViewItem &option,
                        const QModelIndex &index ) const override;

    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;


    void setColumn(int value);

signals:
    void rowChecked(const QModelIndex &index, bool state);

public slots:
    void commitAndCloseEditor();

private:
    int checkBoxColumnIx;
};

#endif // CHECKBOXDELEGATE_H
