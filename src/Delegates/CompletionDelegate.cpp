#include "CompletionDelegate.h"
#include <QComboBox>
#include <QCompleter>

CompletionDelegate::CompletionDelegate(QObject *parent) :
		QItemDelegate(parent)
{
}

QWidget *CompletionDelegate::createEditor (QWidget* parent, const QStyleOptionViewItem &option,
					   const QModelIndex& index ) const
{
	const QAbstractItemModel *model = index.model();
	if (!model)
		return QItemDelegate::createEditor(parent, option, index);

	QComboBox *box = new QComboBox(parent);
	box->setEditable(true);
    box->setCompleter(new QCompleter(const_cast<QAbstractItemModel *>(model), box));
    box->setModel(const_cast<QAbstractItemModel *>(model));
    box->setModelColumn(index.column());
    box->installEventFilter(const_cast<CompletionDelegate *>(this));

    return box;
}

void CompletionDelegate::setEditorData(QWidget* editor,
				       const QModelIndex & index ) const
{
	QComboBox* box = qobject_cast<QComboBox*>(editor);

	const QAbstractItemModel *model = index.model();
    if (!box || !model) {
        QItemDelegate::setEditorData(editor, index);
        return;
    }

    box->setCurrentIndex(index.row());
}

void CompletionDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
				      const QModelIndex &index) const
{
	if (!index.isValid())
		return;

	QComboBox* box = qobject_cast<QComboBox*>(editor);
	if (!box)
		return QItemDelegate::setModelData(editor, model, index);

	model->setData(index, box->currentText(), Qt::DisplayRole);
	model->setData(index, box->currentText(), Qt::EditRole);
}

void CompletionDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
					      const QModelIndex &/* index */) const
{
	editor->setGeometry(option.rect);
}
